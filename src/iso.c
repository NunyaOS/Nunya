/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "iso.h"
#include "memory.h"
#include "string.h"
#include "kerneltypes.h"
#include "ata.h"
#include "console.h"
#include "keyboard.h"

#define ISO_BLOCKSIZE 2048
#define ISO_UNIT 3  //Default to secondary master for testing, will update later when ata_probe leaves a map
#define PVD_OFFSET 16 * ISO_BLOCKSIZE
#define ROOT_DR_OFFSET (PVD_OFFSET) + 156
#define PVD_PATH_TABLE_SIZE_OFFSET (PVD_OFFSET) + 132
#define PVD_PATH_TABLE_OFFSET (PVD_OFFSET) + 148

#define SEEK_CUR 0
#define SEEK_SET 1

struct directory_record {
    uint8_t length_of_record;
    uint8_t length_of_ext_record;
    char loc_of_ext[8];
    char data_length[8];
    uint8_t rec_date_time[7];
    char file_flags[1];
    char file_flags_interleaved[1];
    uint8_t interleave_gap_size;
    char vol_seq_num[4];
    uint8_t len_identifier;
    char file_identifier[2048];
};

struct path_table_entry {
	uint8_t length_of_identifier;
	uint8_t attr_length;
	char loc_of_ext[4];
	uint16_t parent_dir_num;
	char dir_identifier[256];
};

struct iso_point {
	int cur_extent;
	int cur_offset;
	int data_length;
};

/**
 * @brief Acts as an fopen() for iso disk
 * @details Allocates iso_point that is initialized to the beginning of disk
 * It does not do anything to actually move the head of disk reader.
 *
 * @return Pointer to an allocated iso_point with members initialized to 0
 */
struct iso_point *iso_media_open() {
	struct iso_point *to_return = kmalloc(sizeof(struct iso_point));
	to_return->cur_extent = 0;
	to_return->cur_offset = 0;
	return to_return;
}

/**
 * @brief update an iso_point based on "moving along the disk"
 *
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

void iso_media_close(struct iso_point *iso_p) {
	kfree(iso_p);
	return;
}

int iso_media_read(void *dest, int elem_size, int num_elem, struct iso_point *stream) {
	int bytes_needed = num_elem * elem_size;

	int to_read_from_start_of_cur_extent = stream->cur_offset + bytes_needed;
	int atapi_blocks_to_read = to_read_from_start_of_cur_extent / ATAPI_BLOCKSIZE;

	if (to_read_from_start_of_cur_extent % ATAPI_BLOCKSIZE) {
		//integer division rounds down, but we need to round up
		//if there is rounding to be done
		atapi_blocks_to_read++;
	}

	char buffer[atapi_blocks_to_read * ATAPI_BLOCKSIZE];

	if(atapi_read(ISO_UNIT, buffer, atapi_blocks_to_read, stream->cur_extent)) {
		//Do not start at the start of the buffer, because that is the start of
		//the current extent, not where the cur_extent + cur_offset is
		memcpy(dest, buffer + stream->cur_offset, elem_size + num_elem);

		//update stream, as iso_seek is a mock call to keep track of
		//"where we are" on the ISO image and does not actually move
		//the the CD-reading head to read the ISO image, atapi_read does that
		iso_media_seek(stream, bytes_needed, SEEK_CUR);

		return num_elem;
	}
	return 0;
}

void get_directory_record(struct iso_point *iso_p, struct directory_record *dr);
struct path_table_entry get_path_table_entry(struct iso_point *iso_p);
int have_equal_extents(char *first, char *second);
int hex_to_int(char *src, int len);
int is_dir(int flags);
int is_valid_record(struct directory_record *dr);
void print_dr_name(struct directory_record *dr);
void print_dr_date_time(struct directory_record *dr);

void get_directory_record(struct iso_point *iso_p, struct directory_record *dr) {
    int bytes_read = 0;

    iso_media_read(&(dr->length_of_record), sizeof(dr->length_of_record), 1, iso_p);   //1 for bytes read
    iso_media_read(&(dr->length_of_ext_record), 1, sizeof(dr->length_of_ext_record), iso_p);   //1 for bytes read
    iso_media_read(dr->loc_of_ext, 1, 8, iso_p);
    iso_media_read(dr->data_length, 1, 8, iso_p);
    iso_media_read(dr->rec_date_time, 1, 7, iso_p);
    iso_media_read(dr->file_flags, 1, 1, iso_p);
    iso_media_read(dr->file_flags_interleaved, 1, 1, iso_p);
    iso_media_read(&(dr->interleave_gap_size), 1, sizeof(dr->interleave_gap_size), iso_p);
    iso_media_read(dr->vol_seq_num, 1, 4, iso_p);
    iso_media_read(&(dr->len_identifier), 1, sizeof(dr->len_identifier), iso_p);
    bytes_read += 33;

    iso_media_read(dr->file_identifier, 1, dr->len_identifier, iso_p);
    dr->file_identifier[dr->len_identifier] = '\0';
    bytes_read += dr->len_identifier;

    iso_media_seek(iso_p, dr->length_of_record - bytes_read, SEEK_CUR);
}

struct path_table_entry get_path_table_entry(struct iso_point *iso_p){
	int bytes_read = 0;

	struct path_table_entry pt;
	iso_media_read(&(pt.length_of_identifier), sizeof(pt.length_of_identifier), 1, iso_p);
	iso_media_read(&(pt.attr_length), sizeof(pt.attr_length), 1, iso_p);
	iso_media_read(pt.loc_of_ext, 1, 4, iso_p);
	iso_media_read(&(pt.parent_dir_num), sizeof(uint16_t), 1, iso_p);
	bytes_read += 8;

	iso_media_read(pt.dir_identifier, 1, pt.length_of_identifier, iso_p);
	pt.dir_identifier[pt.length_of_identifier] = '\0';
	bytes_read += pt.length_of_identifier;

	iso_media_seek(iso_p, pt.length_of_identifier - bytes_read, SEEK_CUR);
	if (pt.length_of_identifier % 2 == 1) {
		iso_media_seek(iso_p, 1, SEEK_CUR);
	}
	return pt;
}

int have_equal_extents(char *first_ext, char *second_ext) {
    int ext1 = hex_to_int((first_ext) + 4, 4);
    int ext2 = hex_to_int((second_ext) + 4, 4);
    if (ext1 == ext2) {
        return 1;
    }
    return 0;
}

int hex_to_int(char *src, int len) {
    int i, sum = 0;
    for (i = 0; i < len; i++) {
        sum *= 16;
        sum += src[i];
    }
    return sum;
}

int dec_to_int(char *src, int len) {
    int i, sum = 0;
    for (i = 0; i < len; i++) {
        sum *= 10;
        sum += src[i];
    }
    return sum;
}

int is_dir(int flags) {
    if ((flags >> 1) & 1) {
        return 1;
    }
    else {
        return 0;
    }
}

void print_dr_date_time(struct directory_record *dr) {
	console_printf("%d:%d:%d %d/%d/%d\n", dr->rec_date_time[3], dr->rec_date_time[4],
					                      dr->rec_date_time[5], dr->rec_date_time[1],
										  dr->rec_date_time[2], dr->rec_date_time[0] + 1900);
}

int is_valid_record(struct directory_record *dr) {
    if(dr->rec_date_time[3] < 0 || dr->rec_date_time[3] >= 24) {	// validate hours
        return 0;
    }
    if(dr->rec_date_time[4] < 0 || dr->rec_date_time[4] >= 60) {	// validate minutes
        return 0;
    }
    if(dr->rec_date_time[5] < 0 || dr->rec_date_time[5] >= 60) {	// validate seconds
        return 0;
    }
    if(dr->rec_date_time[1] <= 0 || dr->rec_date_time[1] > 12) {	// validate month
        return 0;
    }
    if(dr->rec_date_time[2] <= 0 || dr->rec_date_time[2] > 31) {	// validate day
        return 0;
    }
    if(dr->rec_date_time[0] < 0) {	// validate year
        return 0;
    }
    return 1;
}

void print_dr_name(struct directory_record *dr) {
    if (dr->file_identifier[0] > 0x20) {
    	char name[strlen(dr->file_identifier)];
    	strcpy(name, dr->file_identifier);
    	if (name[strlen(dr->file_identifier) - 1] == '1' && name[strlen(dr->file_identifier) - 2] == ';') {
        	name[strlen(dr->file_identifier) - 2] = '\0';
    	}
        printf("File/Dir Identifier: %s\n", name);
    }
    else {
        printf("File/Dir Identifier: /\n");
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
 */
long int iso_recursive_look_up (const char *pname, struct iso_point *iso_p, int *dl) {
	int next_is_found = 0;

	//Find the location of the next slash in pname
	int next_slash_index = 0;
	while(pname[next_slash_index] != '/' && next_slash_index < strlen(pname)) {
		next_slash_index++;
	}

	char identifier_to_find[256];
	memcpy(identifier_to_find, pname, next_slash_index);
	identifier_to_find[next_slash_index] = 0;
	char name[strlen(pname) + 1];
	struct directory_record dr;
	while (!next_is_found) {
		get_directory_record(iso_p, &dr);
		strcpy(name, dr.file_identifier);
		if (name[strlen(dr.file_identifier) - 1] == '1' && name[strlen(dr.file_identifier) - 2] == ';') {
			name[strlen(dr.file_identifier) - 2] = '\0';
		}

		int result = is_valid_record(&dr);
		if (result != 0) {
			if (strcmp(identifier_to_find, name) == 0) {
				next_is_found = 1;
			}
		}
		else {
			console_printf("File or directory not found\n");
			return -1;
		}
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

/**
 * @brief Lookup offset the dir/file requested
 * @details Finds the offset of the file or directory named by
 * the full pathname starting at the root of the iso filesystem
 * (no relative paths allowed).
 *
 * @param pathname The string name of file or directory to lookup (with no leading /)
 * @param offset The extent number to search the
 * @return Offset (extent number) of the
 */
long int iso_look_up(const char *pathname, int *dl) {
	int root_dr_loc;
	struct iso_point *iso_p = iso_media_open();
	//locate the root directory
	iso_media_seek(iso_p, ROOT_DR_OFFSET, SEEK_SET);

	struct directory_record dr;
	char loc_of_parent[8];
	get_directory_record(iso_p, &dr);

	if (is_valid_record(&dr)) {
		memcpy(&loc_of_parent, dr.loc_of_ext, 8);
		root_dr_loc = hex_to_int(loc_of_parent + 4, 4);
	}
	else {
		console_printf("Illegal dr for root\n");
		return -1;
	}

	if(strcmp(pathname, "/") == 0) {
		return root_dr_loc;
	}
	else {
		iso_media_seek(iso_p, root_dr_loc * ISO_BLOCKSIZE, SEEK_SET);
		//moving forward, get rid of the first slash and give the iso_p we already seek'd for
		long int result = iso_recursive_look_up(&pathname[1], iso_p, dl);
		iso_media_close(iso_p);
		return result;
	}
}

struct iso_file *iso_fopen(const char *pname) {
	struct iso_file *file = kmalloc(sizeof(struct iso_file));
	strcpy(file->pname, pname);
	int dl;  //the data_length of the last item on the path
	int file_offset = iso_look_up(pname, &dl);
	if (file_offset < 0) {
		console_printf("Failed to iso_open the file path.\n");
		return 0;
	}
	file->cur_offset = 0;
	file->extent_offset = file_offset;
	struct directory_record dr;
	int dummy;
	int directory_extent = iso_look_up(pname, &dummy);
	struct iso_point *iso_p = iso_media_open();
	iso_media_seek(iso_p, directory_extent * ISO_BLOCKSIZE, SEEK_SET);
	get_directory_record(iso_p, &dr);
	iso_media_close(iso_p);
	file->data_length = dl;
	return file;
}

int iso_fclose(struct iso_file *file) {
	kfree(file);
	return 0;
}

int iso_fread(char *dest, int elem_size, int num_elem, struct iso_file *file) {
	struct iso_point *iso_p = iso_media_open();
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
		dest[bytes_to_file_read - 1] = 4;  //End of Transmission???
	}

	iso_media_close(iso_p);
	file->cur_offset += bytes_to_file_read;
	return bytes_read;
}
