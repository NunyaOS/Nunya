/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
 * This file has external facing iso_f and iso_d commands, but also
 * contains iso_media_ commands for open, read, and write. The idea is
 * that iso_f and iso_d are in regards to files and directories, while
 * iso_media_ treats the entire iso storage device as a single, long file
 * without an EOF which we can open, close, and seek around. rather than
 * doing absurd byte counting in each of the iso_f and iso_d commands.
 */

#include "iso.h"
#include "memory.h"
#include "string.h"
#include "kerneltypes.h"
#include "ata.h"
#include "console.h"
#include "keyboard.h"

#define ISO_BLOCKSIZE 2048
#define PVD_OFFSET 16 * ISO_BLOCKSIZE
#define ROOT_DR_OFFSET (PVD_OFFSET) + 156

#define SEEK_CUR 0
#define SEEK_SET 1

// Use these to prevent multiple fetches of the same
// ATAPI block for consecutive, single character reads
char global_atapi_block[ATAPI_BLOCKSIZE];
int global_atapi_unit = -1;
int global_atapi_extent = -1;

struct iso_point {
    int ata_unit;
    int cur_extent;
    int cur_offset;
};

int get_directory_record(struct iso_point *iso_p, struct directory_record *dr);
int hex_to_int(char *src, int len);
int is_dir(int flags);
long int iso_look_up(const char *pname, int *dl, int ata_unit);
long int iso_recursive_look_up (const char *pname, struct iso_point *iso_p, int *dl);
void iso_media_close(struct iso_point *iso_p);
struct iso_point *iso_media_open(int ata_unit);
int iso_media_read(void *dest, int elem_size, int num_elem, struct iso_point *stream);
void iso_media_seek(struct iso_point *iso_p, long offset, int whence);

/**
 * @brief Reads the next directory record in the extent
 * @details Starting at the current iso_p, reads a directory record into
 * the argument dr, moving iso_p forward.
 *
 * @param iso_p A pointer to a struct iso_point which is set to the beginning of a directory record in a directory extent
 * @param dr A pointer to a directory record to be filled with the information
 * @return 1 on success, 0 on failure
 */
int get_directory_record(struct iso_point *iso_p, struct directory_record *dr) {
    int bytes_read = 0;
    int max_dr_size = 64;
    char dr_dest[max_dr_size];
    int result = iso_media_read(dr_dest, 1, max_dr_size, iso_p);
    if(result != max_dr_size) {
        console_printf("get directory record fails\n");
        return 0;
    }

    memcpy(&(dr->length_of_record), dr_dest + 0, sizeof(dr->length_of_record));   //1 for bytes read
    memcpy(&(dr->length_of_ext_record), dr_dest + 1, sizeof(dr->length_of_ext_record));
    memcpy(dr->loc_of_ext, dr_dest + 2, sizeof(dr->loc_of_ext));
    memcpy((dr->data_length), dr_dest + 10, sizeof(dr->data_length));
    memcpy((dr->rec_date_time), dr_dest + 18, sizeof(dr->rec_date_time));
    memcpy((dr->file_flags), dr_dest + 25, sizeof(dr->file_flags));
    memcpy((dr->file_flags_interleaved), dr_dest + 26, sizeof(dr->file_flags_interleaved));
    memcpy(&(dr->interleave_gap_size), dr_dest + 27, sizeof(dr->interleave_gap_size));
    memcpy((dr->vol_seq_num), dr_dest + 28, sizeof(dr->vol_seq_num));
    memcpy(&(dr->len_identifier), dr_dest + 32, sizeof(dr->len_identifier));
    bytes_read += 33;

    memcpy((dr->file_identifier), dr_dest + 33, dr->len_identifier);
    dr->file_identifier[dr->len_identifier] = '\0';
    bytes_read += dr->len_identifier;

    iso_media_seek(iso_p, bytes_read - 64, SEEK_CUR);
    return 1;
}

/**
 * @brief Converts a character hex number into a base 10 integer
 *
 * @param src Array of hex characters in big endian format
 * @param len Number of bytes in the src to convert
 * @return The integer value of the first len bytes of hex characters in src
 */
int hex_to_int(char *src, int len) {
    int i, sum = 0;
    for (i = 0; i < len; i++) {
        sum *= 16;
        sum += src[i];
    }
    return sum;
}

/**
 * @brief Checks whether flags represent a directory
 * @details Bit compares directory record file flags to the dir flag bit
 *
 * @param flags The file flags from a directory record
 * @return 1 if dir bit is high, 0 otherwise
 */
int is_dir(int flags) {
    if ((flags >> 1) & 1) {
        return 1;
    }
    else {
        return 0;
    }
}

//Documentation in header
int iso_dclose(struct iso_dir *dir) {
    if(dir) {
        kfree(dir);
        return 0;
    }
    return -1;
}

//Documentation in header
struct iso_dir *iso_dopen(const char *pname, int ata_unit) {
    int dl;  //data length, needed in iso_look_up
    int extent_num = iso_look_up(pname, &dl, ata_unit);
    if(extent_num < 0) {
        return 0;
    }
    struct iso_dir *to_return = kmalloc(sizeof(*to_return));
    struct iso_point *iso_p = iso_media_open(ata_unit);
    iso_media_seek(iso_p, ATAPI_BLOCKSIZE * extent_num, SEEK_SET);
    to_return->cur_offset = 0;
    to_return->extent_offset = extent_num;
    to_return->ata_unit = ata_unit;
    to_return->data_length = dl;

    return to_return;
}

//Documentation in header
struct directory_record *iso_dread(struct iso_dir *read_from) {
    if(read_from->cur_offset + 1 >= read_from->data_length) {
        //"return null" if at end of directory record
        return 0;
    }

    struct iso_point *iso_p = iso_media_open(read_from->ata_unit);
    iso_media_seek(iso_p, read_from->extent_offset * ATAPI_BLOCKSIZE + read_from->cur_offset, SEEK_SET);
    struct directory_record *next_dr = kmalloc(sizeof(*next_dr));

    //if not the end of the directory record, fill the dest by dereferencing dest_ptr
    int success = get_directory_record(iso_p, next_dr);
    if (!success) {
        return 0;
    }

    read_from->cur_offset = iso_p->cur_offset;
    //no need to reset extent, that shouldn't be an issue

    return next_dr;
}

//Documentation in header
int iso_fclose(struct iso_file *file) {
    if(file) {
        kfree(file);
        return 0;
    }
    return -1;
}

//Documentation in header
struct iso_file *iso_fopen(const char *pname, int ata_unit) {
    struct iso_file *file = kmalloc(sizeof(struct iso_file));
    strcpy(file->pname, pname);
    int dl;  //the data_length of the last item on the path
    int file_offset = iso_look_up(pname, &dl, ata_unit);
    if (file_offset < 0) {
        console_printf("Failed to iso_open the file path.\n");
        return 0;
    }
    file->cur_offset = 0;
    file->extent_offset = file_offset;
    file->ata_unit = ata_unit;
    file->data_length = dl;

    return file;
}

//Documentation in header
int iso_fread(void *dest, int elem_size, int num_elem, struct iso_file *file) {
    struct iso_point *iso_p = iso_media_open(file->ata_unit);
    iso_media_seek(iso_p, ISO_BLOCKSIZE * file->extent_offset + file->cur_offset, SEEK_SET);

    int should_EOT_terminate = 0;
    int bytes_to_disk_read = elem_size * num_elem;
    int bytes_to_file_read = bytes_to_disk_read;
    if (bytes_to_file_read + file->cur_offset > file->data_length) {
        bytes_to_file_read = file->data_length - file->cur_offset + 1;
        bytes_to_disk_read = bytes_to_file_read - 1;
        should_EOT_terminate = 1;
    }
    int bytes_read = iso_media_read(dest, 1, bytes_to_disk_read, iso_p);
    if(bytes_read != bytes_to_disk_read) {
        console_printf("file reading error\n");
        iso_media_close(iso_p);
        return bytes_read;
    }
    if (should_EOT_terminate) {
        ((char *)dest)[bytes_to_file_read - 1] = 4;  //4 is ASCII for End of Transmission???
    }

    iso_media_close(iso_p);
    file->cur_offset += bytes_to_file_read;
    return bytes_read;
}

/**
 * @brief Acts as an fopen() for iso disk
 * @details Allocates iso_point that is initialized to the beginning of disk
 * It does not do anything to actually move the head of disk reader.
 *
 * @param iso_p A pointer to the iso_point to close.
 * @return Pointer to an allocated iso_point with members initialized to 0
 */
void iso_media_close(struct iso_point *iso_p) {
    kfree(iso_p);
    return;
}

/**
 * @brief Treating the iso on the given ata_unit as a file, opens it
 * @details kmalloc's and returns an iso_point which is set to the first
 * byte of the given ata_unit
 *
 * @param ata_unit The ata unit to open the iso_point on
 * @return A pointer to the iso_point at the first byte of the iso.
 */
struct iso_point *iso_media_open(int ata_unit) {
    struct iso_point *to_return = kmalloc(sizeof(struct iso_point));
    to_return->cur_extent = 0;
    to_return->cur_offset = 0;
    to_return->ata_unit = ata_unit;
    return to_return;
}

/**
 * @brief Reads in the specified amount from the iso_p
 * @details Reads into dest the specified amount from the iso starting
 * at the current extent and offset of iso_p
 *
 * @param dest A buffer to put the read information into
 * @param elem_size The size in bytes of each element to be read
 * @param num_elem The number of elements to read into dest
 * @param iso_p Pointer to the struct iso_point to begin reading at.
 * @return The number of elements read in, -1 on error.
 */
int iso_media_read(void *dest, int elem_size, int num_elem, struct iso_point *stream) {
    int bytes_needed = num_elem * elem_size;
    if(bytes_needed <= 0) {
        return 0;
    }

    int to_read_from_start_of_cur_extent = stream->cur_offset + bytes_needed;
    int atapi_blocks_to_read = to_read_from_start_of_cur_extent / ATAPI_BLOCKSIZE;

    if (to_read_from_start_of_cur_extent % ATAPI_BLOCKSIZE) {
        //integer division rounds down, but we need to round up
        //if there is rounding to be done
        atapi_blocks_to_read++;
    }

    //ensure that we need to perform the read before we do
    if (atapi_blocks_to_read > 1 ||
        global_atapi_extent != stream->cur_extent ||
        global_atapi_unit != stream->ata_unit) {
        char buffer[atapi_blocks_to_read * ATAPI_BLOCKSIZE];
        if (!atapi_read(stream->ata_unit, buffer, atapi_blocks_to_read, stream->cur_extent)) {
            console_printf("atapi_read is 0 in iso_media_read()\n");
            return -1;
        }
        else {
            //Do not start at the start of the buffer, because that is the start of
            //the current extent, not where the cur_extent + cur_offset is
            memcpy(dest, buffer + stream->cur_offset, elem_size * num_elem);

            //copy the last block read in into the global "cache"
            memcpy(global_atapi_block, buffer + (ATAPI_BLOCKSIZE * (atapi_blocks_to_read - 1)), ATAPI_BLOCKSIZE);

            //update stream, as iso_seek is a mock call to keep track of
            //"where we are" on the ISO image and does not actually move
            //the the CD-reading head to read the ISO image, atapi_read does that
            iso_media_seek(stream, bytes_needed, SEEK_CUR);
            global_atapi_unit = stream->ata_unit;
            global_atapi_extent = stream->cur_extent;
            return num_elem;
        }
    }
    else {
        //Do not start at the start of the buffer, because that is the start of
        //the current extent, not where the cur_extent + cur_offset is
        memcpy(dest, global_atapi_block + stream->cur_offset, elem_size * num_elem);

        //update stream, as iso_seek is a mock call to keep track of
        //"where we are" on the ISO image and does not actually move
        //the the CD-reading head to read the ISO image, atapi_read does that
        iso_media_seek(stream, bytes_needed, SEEK_CUR);
        return num_elem;
    }
}

/**
 * @brief Moves the iso_p
 * @details Changes the offset and extent values of the iso_point as instructed
 *
 * @param iso_p Pointer to the iso_point to adjust
 * @param offset How far to move the current pointer
 * @param whence The relative place to move the pointer from, SEEK_CUR is moving forward from current position, SEEK_SET is from the zeroth byte of the iso
 */
void iso_media_seek(struct iso_point *iso_p, long offset, int whence) {
    switch(whence) {
        case SEEK_SET:
            //from the start of the file
            iso_p->cur_extent = offset / ATAPI_BLOCKSIZE;
            iso_p->cur_offset = offset % ATAPI_BLOCKSIZE;
            break;
        case SEEK_CUR:
            //from the current position
            iso_p->cur_extent += offset / ATAPI_BLOCKSIZE;
            iso_p->cur_offset += offset % ATAPI_BLOCKSIZE;

            //Check if this caused a wrap around, and if so deal with it
            if (iso_p->cur_offset > ATAPI_BLOCKSIZE) {
                iso_p->cur_offset -= ATAPI_BLOCKSIZE;
                iso_p->cur_extent++;
            }
            break;
        default:
            console_printf("Illegal iso_seek mode\n");
            break;
    }
}

/**
 * @brief Lookup offset the dir/file requested
 * @details Finds the offset of the file or directory named by
 * the full pathname starting at the root of the iso filesystem
 * (no relative paths allowed).
 *
 * @param pathname The string name of file or directory to lookup (with no leading /)
 * @param offset The extent number to search the
 * @return Offset (extent number) of the pathname, or -1 if not found or error
 */
long int iso_look_up(const char *pname, int *dl, int ata_unit) {
    int root_dr_loc;
    struct iso_point *iso_p = iso_media_open(ata_unit);
    //locate the root directory
    iso_media_seek(iso_p, ROOT_DR_OFFSET, SEEK_SET);

    struct directory_record dr;
    char loc_of_parent[8];
    int success = get_directory_record(iso_p, &dr);
    if(!success) {
        return -1;
    }

    memcpy(&loc_of_parent, dr.loc_of_ext, 8);
    root_dr_loc = hex_to_int(loc_of_parent + 4, 4);

    if(strcmp(pname, "/") == 0) {
        return root_dr_loc;
    }
    else {
        iso_media_seek(iso_p, root_dr_loc * ISO_BLOCKSIZE, SEEK_SET);
        //moving forward, get rid of the first slash and give the iso_p we already seek'd for
        long int result = iso_recursive_look_up(&pname[1], iso_p, dl);
        iso_media_close(iso_p);
        return result;
    }
}

/**
 * @brief Lookup offset the dir/file requested
 * @details Only call iso_lookup, which in turn calls iso_recursive_lookup.
 * This recurses on the next directory in pathname
 * by looking at the directory record given by the current
 * disk offset, but does not handle the case of root.
 *
 * @param pathname The string name of file or directory to lookup (with no leading /)
 * @param offset The extent number to search the
 * @return The extent offset for the pname, or -1 if not found or on error
 */
long int iso_recursive_look_up (const char *pname, struct iso_point *iso_p, int *dl) {
    int next_is_found = 0;

    //Find the location of the next slash in pname
    int next_slash_index = 0;
    while(pname[next_slash_index] != '/' && next_slash_index < strlen(pname)) {
        next_slash_index++;
    }

    struct directory_record self_dr;
    int success = get_directory_record(iso_p, &self_dr);
    if (!success) {
        return -1;
    }

    int extent_dl = hex_to_int(self_dr.data_length + 4, 4);
    struct directory_record parent_dr;
    success = get_directory_record(iso_p, &parent_dr);
    if (!success) {
        return -1;
    }

    char identifier_to_find[256];
    memcpy(identifier_to_find, pname, next_slash_index);
    identifier_to_find[next_slash_index] = 0;
    char name[strlen(pname) + 1];
    struct directory_record dr;
    while (!next_is_found && (iso_p->cur_offset + 1) < extent_dl) {
        success = get_directory_record(iso_p, &dr);
        if (!success) {
            return -1;
        }

        strcpy(name, dr.file_identifier);
        if (name[strlen(dr.file_identifier) - 1] == '1' && name[strlen(dr.file_identifier) - 2] == ';') {
            name[strlen(dr.file_identifier) - 2] = '\0';
        }

        if (strcmp(identifier_to_find, name) == 0) {
            next_is_found = 1;
        }
    }

    if(!next_is_found) {
        return -1;
    }

    int entity_location = hex_to_int(dr.loc_of_ext + 4, 4);
    if (strcmp(identifier_to_find, pname) == 0) {
        //If this is the last item we're searching for
        //return it's location as an extent number, and say how long it is
        *dl = hex_to_int(dr.data_length + 4, 4);
        return entity_location;
    }
    else {
        iso_media_seek(iso_p, entity_location * ISO_BLOCKSIZE, SEEK_SET);
        return iso_recursive_look_up(pname + next_slash_index + 1, iso_p, dl);
    }
}
