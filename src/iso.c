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

#define MAX_DR_SIZE 64

#define SEEK_CUR 0
#define SEEK_SET 1

// Use these to prevent multiple fetches of the same
// ATAPI block for consecutive, single character reads
static char global_atapi_block[ATAPI_BLOCKSIZE];
static int global_atapi_unit = -1;
static int global_atapi_extent = -1;

struct iso_point {
    int ata_unit;
    int cur_extent;
    int cur_offset;
};

static int get_directory_record(struct iso_point *iso_p, struct directory_record *dr);
static uint32_t bendian_chars_to_int(unsigned char *src, int len);
static bool is_dir(int flags);
static long int iso_look_up(const char *pname, uint32_t *dl, int ata_unit, bool is_dir_search);
static long int iso_recursive_look_up (const char *pname, struct iso_point *iso_p, uint32_t *dl, bool is_dir_search);
static void iso_media_close(struct iso_point *iso_p);
static struct iso_point *iso_media_open(int ata_unit);
static int iso_media_peek_byte(struct iso_point *iso_p);
static int iso_media_read(void *dest, int elem_size, int num_elem, struct iso_point *stream);
static void iso_media_seek(struct iso_point *iso_p, long offset, int whence);

/**
 * @brief Reads the next directory record in the extent
 * @details Starting at the current iso_p, reads a directory record into
 * the argument dr, moving iso_p forward.
 *
 * @param iso_p A pointer to a struct iso_point which is set to the beginning of a directory record in a directory extent
 * @param dr A pointer to a directory record to be filled with the information
 * @return 1 on success, 0 on failure
 */
static int get_directory_record(struct iso_point *iso_p, struct directory_record *dr) {
    int bytes_read = 0;
    char dr_dest[MAX_DR_SIZE];
    int result = iso_media_read(dr_dest, 1, MAX_DR_SIZE, iso_p);
    if (result != MAX_DR_SIZE) {
        return 0;
    }

    /*
     * All offset and length values are specific to the ISO 9660 format.
     * For more information on each offset value, go to http://wiki.osdev.org/ISO_9660#Directories
     */
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
    if (bytes_read % 2 == 1) { // Odd length, so we need to seek one bit to pad
        iso_media_seek(iso_p, 1, SEEK_CUR);
    }
    return 1;
}

/**
 * @brief Converts a big endian character array into a base 10 integer
 *
 * @param src Array of characters in big endian format
 * @param len Number of bytes in the src to convert
 * @return The integer value of the first len bytes of characters in src
 */
static uint32_t bendian_chars_to_int(unsigned char *src, int len) {
    uint32_t i, sum = 0;
    for (i = 0; i < len; i++) {
        sum *= 256;
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
static bool is_dir(int flags) {
    if ((flags >> 1) & 1) {
        return 1;
    } else {
        return 0;
    }
}


//Documentation in header
int iso_dclose(struct iso_dir *dir) {
    if (dir) {
        kfree(dir);
        return 0;
    }
    return -1;
}

//Documentation in header
struct iso_dir *iso_dopen(const char *pname, int ata_unit) {
    uint32_t dl;  //data length, needed in iso_look_up
    int extent_num = iso_look_up(pname, &dl, ata_unit, 1);
    if (extent_num < 0) {
        return 0;
    }
    struct iso_dir *to_return = kmalloc(sizeof(*to_return));
    to_return->cur_offset = 0;
    to_return->extent_offset = extent_num;
    to_return->ata_unit = ata_unit;
    to_return->data_length = dl;
    return to_return;
}

//Documentation in header
struct directory_record *iso_dread(struct iso_dir *read_from) {
    int num_extents_in_directory = read_from->data_length / ATAPI_BLOCKSIZE;
    int final_extent_of_directory = read_from->extent_offset + (num_extents_in_directory - 1);

    struct iso_point *iso_p = iso_media_open(read_from->ata_unit);

    // seek to current offset of iso_p
    iso_media_seek(iso_p, read_from->extent_offset * ATAPI_BLOCKSIZE + read_from->cur_offset, SEEK_SET);
    if (iso_media_peek_byte(iso_p) == 0) {
        //No more records on this extent, but are there more on next extent?
        if (iso_p->cur_extent < final_extent_of_directory) {
            iso_media_seek(iso_p, (iso_p->cur_extent + 1) * ISO_BLOCKSIZE, SEEK_SET);
            read_from->cur_offset = ISO_BLOCKSIZE * (iso_p->cur_extent - read_from->extent_offset);
        } else {
            //End of this directory, return null
            iso_media_close(iso_p);
            return 0;
        }
    }

    struct directory_record *next_dr = kmalloc(sizeof(*next_dr));

    //Not at the end of the directory record, fill the dest by dereferencing dest_ptr
    int success = get_directory_record(iso_p, next_dr);
    if (!success) {
        iso_media_close(iso_p);
        return 0;
    }

    read_from->cur_offset = iso_p->cur_offset + (iso_p->cur_extent - read_from->extent_offset) * ISO_BLOCKSIZE;
    //no need to reset extent, that shouldn't be an issue
    iso_media_close(iso_p);
    return next_dr;
}

//Documentation in header
int iso_fclose(struct iso_file *file) {
    if (file) {
        kfree(file);
        return 0;
    }
    return -1;
}

//Documentation in header
struct iso_file *iso_fopen(const char *pname, int ata_unit) {
    struct iso_file *file = kmalloc(sizeof(struct iso_file));
    strcpy(file->pname, pname);
    uint32_t dl;  //the data_length of the last item on the path
    int extent_num = iso_look_up(pname, &dl, ata_unit, 0);
    if (extent_num < 0) {
        return 0;
    }
    file->cur_offset = 0;
    file->extent_offset = extent_num;
    file->ata_unit = ata_unit;
    file->at_EOF = 0;
    file->data_length = dl;

    return file;
}

//Documentation in header
int iso_fread(void *dest, int elem_size, int num_elem, struct iso_file *file) {
    struct iso_point *iso_p = iso_media_open(file->ata_unit);
    iso_media_seek(iso_p, ISO_BLOCKSIZE * file->extent_offset + file->cur_offset, SEEK_SET);

    int bytes_to_disk_read = elem_size * num_elem;
    int bytes_to_file_read = bytes_to_disk_read;
    if ((bytes_to_file_read + file->cur_offset) > file->data_length) {
        // Don't actually want to read off the end of the file
        bytes_to_file_read = file->data_length - file->cur_offset + 1;
        bytes_to_disk_read = bytes_to_file_read - 1;
        file->at_EOF = 1;
    }
    int bytes_read = iso_media_read(dest, 1, bytes_to_disk_read, iso_p);
    if (bytes_read != bytes_to_disk_read) {
        iso_media_close(iso_p);
        return -1;
    }

    iso_media_close(iso_p);
    file->cur_offset += bytes_to_file_read;
    return bytes_read;
}

/**
 * @brief Acts as an fclose() for iso disk
 * @details Frees the iso_point that was used to navigate the ISO disk.
 *
 * @param iso_p A pointer to the iso_point to close.
 */
static void iso_media_close(struct iso_point *iso_p) {
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
static struct iso_point *iso_media_open(int ata_unit) {
    struct iso_point *to_return = kmalloc(sizeof(struct iso_point));
    to_return->cur_extent = 0;
    to_return->cur_offset = 0;
    to_return->ata_unit = ata_unit;
    return to_return;
}

/**
 * @brief Peek at the next byte on the iso media
 * @details Read the next byte at the current extent and offset of iso_p, then
 * iso_seek back one byte. iso_p is unchanged after the call.
 *
 * @param iso_p Pointer to the iso_point we're peeking at.
 * @return The value of the byte iso_p is set to read next.
 */
static int iso_media_peek_byte(struct iso_point *iso_p) {
    uint8_t byte;
    iso_media_read(&byte, 1, 1, iso_p);
    iso_media_seek(iso_p, -1, SEEK_CUR);
    return byte;
}

/**
 * @brief Reads in the specified amount from the iso_p
 * @details Reads into dest the specified amount from the iso starting
 * at the current extent and offset of iso_p
 *
 * @param dest A buffer to put the read information into
 * @param elem_size The size in bytes of each element to be read
 * @param num_elem The number of elements to read into dest
 * @param stream Pointer to the struct iso_point to begin reading at.
 * @return The number of elements read in, -1 on error.
 */
static int iso_media_read(void *dest, int elem_size, int num_elem, struct iso_point *stream) {
    int bytes_needed = num_elem * elem_size;
    if (bytes_needed <= 0) {
        return 0;
    }

    // the number of bytes into the current extent which we need to stop at
    int last_byte_cur_extent = stream->cur_offset + bytes_needed;
    int atapi_blocks_to_read = last_byte_cur_extent / ATAPI_BLOCKSIZE;

    if (last_byte_cur_extent % ATAPI_BLOCKSIZE) {
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
            return -1;
        } else {
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
    } else {
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
 * @brief Moves the iso_point
 * @details Changes the offset and extent values of the iso_point as instructed
 *
 * @param iso_p Pointer to the iso_point to adjust
 * @param offset How far to move the current pointer
 * @param whence The relative place to move the pointer from, SEEK_CUR is moving forward from current position, SEEK_SET is from the zeroth byte of the iso file
 */
static void iso_media_seek(struct iso_point *iso_p, long offset, int whence) {
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
            break;
    }
}

/**
 * @brief Lookup offset of the dir/file requested
 * @details Finds the offset of the file or directory named by
 * the full pathname starting at the root of the iso filesystem
 * (no relative paths allowed).
 *
 * @param pname The string name of file or directory to lookup (requires leading /)
 * @param dl Pointer to integer to fill in indicating dir/file data length
 * @param ata_unit The ata_unit to search for the dir/file on
 * @param is_dir_search 1 if we are looking up a directory, 0 if a file
 * @return Offset (extent number) of the pathname, or -1 if not found or error
 */
static long int iso_look_up(const char *pname, uint32_t *dl, int ata_unit, bool is_dir_search) {
    int root_dr_loc;
    struct iso_point *iso_p = iso_media_open(ata_unit);
    //locate the root directory
    iso_media_seek(iso_p, ROOT_DR_OFFSET, SEEK_SET);

    //Read root directory record from PVD
    struct directory_record dr;
    unsigned char loc_of_parent[8];
    int success = get_directory_record(iso_p, &dr);
    if (!success) {
        iso_media_close(iso_p);
        return -1;
    }

    memcpy(&loc_of_parent, dr.loc_of_ext, 8);
    root_dr_loc = bendian_chars_to_int(loc_of_parent + 4, 4);

    if (strcmp(pname, "/") == 0) {
        iso_media_close(iso_p);
        if (is_dir_search) {
            *dl = bendian_chars_to_int(dr.data_length + 4, 4);
            return root_dr_loc;
        } else {  //Tried to open "/" as file
            return -1;
        }
    } else {
        iso_media_seek(iso_p, root_dr_loc * ISO_BLOCKSIZE, SEEK_SET);
        //moving forward, get rid of the first slash and give the iso_p we already seek'd for
        long int result = iso_recursive_look_up(&pname[1], iso_p, dl, is_dir_search);
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
 * @param pname The string name of file or directory to lookup (with no leading /)
 * @param iso_p Pointer to the iso point of the start of the directory extent to search.
 * @param dl Pointer to integer to fill in indicating dir/file data length
 * @param is_dir_search 1 if we are looking up a directory, 0 if a file
 * @return The extent offset for the pname, or -1 if not found or on error
 */
static long int iso_recursive_look_up (const char *pname, struct iso_point *iso_p, uint32_t *dl, bool is_dir_search) {
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

    //add number of extents - 1 to the index of the first extent to get the final
    //extent we should be reading for this directory (in the case where there are
    //lots of files and it takes more than 1 extent to describe the directory)
    int num_extents_in_directory = bendian_chars_to_int(self_dr.data_length + 4, 4) / ATAPI_BLOCKSIZE;
    int final_extent_of_directory = bendian_chars_to_int(self_dr.loc_of_ext + 4, 4) + (num_extents_in_directory - 1);
    while (!next_is_found) {
        if (iso_media_peek_byte(iso_p) == 0) {
            //This extent has no more directory records, as next byte
            //should be the record_length of the next directory record
            //between 34 and 64, but extra space at end of extent is 0'd out.
            if (final_extent_of_directory == iso_p->cur_extent) {
                //And we're on the final extent, desired path not found
                return -1;
            } else {
                //Move to the next extent and continue searching this directory.
                iso_media_seek(iso_p, (iso_p->cur_extent + 1) * ISO_BLOCKSIZE, SEEK_SET);
                continue;
            }
        }

        if (!get_directory_record(iso_p, &dr)) {
            //Failed to read disk somehow
            //Legality of dr already guaranteed by above iso_media_peek_byte()
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

    if (!next_is_found) {
        return -1;
    }

    int entity_location = bendian_chars_to_int(dr.loc_of_ext + 4, 4);
    if (strcmp(identifier_to_find, pname) == 0) {
        //If this is the last item we're searching for
        //return it's location as an extent number, and say how long it is
        if (is_dir_search) {
            if (is_dir(dr.file_flags[0])) {
                *dl = bendian_chars_to_int(dr.data_length + 4, 4);
                return entity_location;
            } else {  //Tried to open directory, but found file
                return -1;
            }
        } else {  //looking for a file
            if (!is_dir((int)dr.file_flags[0])) {  //it is a file (it's not a dir)
                *dl = bendian_chars_to_int(dr.data_length + 4, 4);
                return entity_location;
            } else {  //Tried to open file, but found dir in its place
                return -1;
            }
        }
    } else {
        iso_media_seek(iso_p, entity_location * ISO_BLOCKSIZE, SEEK_SET);
        return iso_recursive_look_up(pname + next_slash_index + 1, iso_p, dl, is_dir_search);
    }
}
