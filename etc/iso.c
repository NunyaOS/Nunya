/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "iso.h"
#include <string.h>
//#include "kerneltypes.h"
#include <stdlib.h>
#include <stdint.h>

#define ISO_BLOCK 2048
#define PVD_OFFSET 16 * ISO_BLOCK
#define ROOT_DR_OFFSET (PVD_OFFSET) + 156
#define PVD_PATH_TABLE_SIZE_OFFSET (PVD_OFFSET) + 132
#define PVD_PATH_TABLE_OFFSET (PVD_OFFSET) + 148

const char *iso_file_name = "/home/jesse/Documents/Open Source Software Development/Nunya/etc/filesystem.iso";

struct directory_record {
    uint8_t length_of_record;
    uint8_t length_of_ext_record;
    char loc_of_ext[8];
    char data_length[8];
    char rec_date_time[7];
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

struct directory_record get_directory_record(FILE *fp);
struct path_table_entry get_path_table_entry(FILE *fp);
int have_equal_extents(char *first, char *second);
int hex_to_int(char *src, int len);
int is_dir(int flags);
int is_valid_record(struct directory_record *dr);
int look_up_directory_offset(const char *dir_name);
int look_up_file_offset(int dir_offset, const char *fname);
int look_up_path_offset(const char *pname);
void print_dr_name(struct directory_record *dr);
void print_n_tabs(int n);
void print_subdirectory(FILE *fp, char *loc_of_subdir, char *loc_of_parent, int depth);

struct directory_record get_directory_record(FILE *fp) {
    int bytes_read = 0;

    struct directory_record dr;
    fread(&(dr.length_of_record), sizeof(dr.length_of_record), 1, fp);   //1 for bytes read
    fread(&(dr.length_of_ext_record), 1, sizeof(dr.length_of_ext_record), fp);   //1 for bytes read
    fread(dr.loc_of_ext, 1, 8, fp);
    fread(dr.data_length, 1, 8, fp);
    fread(dr.rec_date_time, 1, 7, fp);
    fread(dr.file_flags, 1, 1, fp);
    fread(dr.file_flags_interleaved, 1, 1, fp);
    fread(&(dr.interleave_gap_size), 1, sizeof(dr.interleave_gap_size), fp);
    fread(dr.vol_seq_num, 1, 4, fp);
    fread(&(dr.len_identifier), 1, sizeof(dr.len_identifier), fp);
    bytes_read += 33;

    fread(dr.file_identifier, 1, dr.len_identifier, fp);
    dr.file_identifier[dr.len_identifier] = '\0';
    bytes_read += dr.len_identifier;

    fseek(fp, dr.length_of_record - bytes_read, SEEK_CUR);
    return dr;
}

struct path_table_entry get_path_table_entry(FILE *fp){
	int bytes_read = 0;

	struct path_table_entry pt;
	fread(&(pt.length_of_identifier), sizeof(pt.length_of_identifier), 1, fp);
	fread(&(pt.attr_length), sizeof(pt.attr_length), 1, fp);
	fread(pt.loc_of_ext, 1, 4, fp);
	fread(&(pt.parent_dir_num), sizeof(uint16_t), 1, fp);
	bytes_read += 8;

	fread(pt.dir_identifier, 1, pt.length_of_identifier, fp);
	pt.dir_identifier[pt.length_of_identifier] = '\0';
	bytes_read += pt.length_of_identifier;

//	fseek(fp, pt.length_of_identifier - bytes_read, SEEK_CUR);
	if (pt.length_of_identifier % 2 == 1) {
		fseek(fp, 1, SEEK_CUR);
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

int is_dir(int flags) {
    if ((flags >> 1) & 1) {
        return 1;
    }
    else {
        return 0;
    }
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

int look_up_directory_offset(const char *dir_name) {
	FILE *fp = fopen(iso_file_name, "r");
	if (!fp) {
		printf("Failed to open file\n");
		return -1;
	}
	fseek(fp, PVD_PATH_TABLE_SIZE_OFFSET, SEEK_SET);
	char path_table_length[8];
	fread(path_table_length, 1, 8, fp);
	int path_table_size = hex_to_int(path_table_length + 4, 4);	// use MSB of path_table_length chars
	fseek(fp, PVD_PATH_TABLE_OFFSET, SEEK_SET);
	char path_table_loc[4];
	fread(path_table_loc, 1, 4, fp);
	fseek(fp, ISO_BLOCK * hex_to_int(path_table_loc, 4), SEEK_SET);

	struct path_table_entry pt;
	pt = get_path_table_entry(fp);
	while (pt.length_of_identifier > 0) {
		if (strcmp(dir_name, pt.dir_identifier) == 0) {
			break;
		}
		else {
			pt = get_path_table_entry(fp);
		}
	}
	fclose(fp);
	if (pt.length_of_identifier <= 0) {
		printf("Directory not found.\n");
		return -1;
	}
	else {
		return hex_to_int(pt.loc_of_ext, 4);
	}
}

int look_up_file_offset(int dir_offset, const char *fname){
	FILE *fp = fopen(iso_file_name, "r");
	if (!fp) {
		printf("Failed to open file\n");
		return -1;
	}
	fseek(fp, ISO_BLOCK * dir_offset, SEEK_SET);
	struct directory_record dr;
	dr = get_directory_record(fp);
	char dr_name[2048];
	while (is_valid_record(&dr)) {
		if (strlen(dr.file_identifier) < 2) {
			dr = get_directory_record(fp);
			continue;
		}
		memset(dr_name, '\0', 2048);
		memcpy(dr_name, dr.file_identifier, strlen(dr.file_identifier) - 2);
		dr_name[strlen(dr.file_identifier) - 1] = '\0';
		if (strcmp(fname, dr_name) == 0) {
			break;
		}
		dr = get_directory_record(fp);
	}
	fclose(fp);
	if (!is_valid_record(&dr)) {
		printf("File not found.\n");
		return -1;
	}
	else {
;		return hex_to_int(dr.loc_of_ext + 4, 4);
	}
}

int look_up_path_offset(const char *pname){
	int i;
	for(i = strlen(pname) - 1; i >= 0; i--) {
		if(pname[i] == '/'){
			break;
		}
	}
	int j;
	if (i == 0) {
		j = 0;
	}
	else {
		for (j = i-1; j >= 0; j--) {
			if (pname[j] == '/') {
				break;
			}
		}
	}
	char subdir[i - j], fname[strlen(pname) - i];
	if (i == 0) {
		strcpy(subdir, "/");
	}
	else {
		memcpy(subdir, pname + j + 1, i - j - 1);
		subdir[i - j] = '\0';
	}
	strcpy(fname, pname + i + 1);

	int dir_offset = look_up_directory_offset(subdir);
	if (dir_offset < 0) {
		printf("File path does not exist.\n");
		return -1;
	}

	int file_offset = look_up_file_offset(dir_offset, fname);
	if (file_offset < 0) {
		printf("File path does not exist.\n");
		return -1;
	}
	return file_offset;
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

void print_file_system(FILE *fp) {
    fseek(fp, ROOT_DR_OFFSET, SEEK_SET);
    struct directory_record dr;
    char loc_of_parent[8];
    dr = get_directory_record(fp);	// This is the dr for the PVD
    if (is_valid_record(&dr)) {
        print_dr_name(&dr);
        printf("\n");
    }
    memcpy(loc_of_parent, dr.loc_of_ext, 8);
    print_subdirectory(fp, dr.loc_of_ext, loc_of_parent, 1);
}

void print_n_tabs(int n) {
    int i;
    for (i = 0; i < n; i++) {
        printf("\t");
    }
}

void print_subdirectory(FILE *fp, char *loc_of_subdir, char *loc_of_parent, int depth) {
    fseek(fp, ISO_BLOCK * hex_to_int(loc_of_subdir + 4, 4), SEEK_SET);	// Go to extent of subdir
    struct directory_record dr;
    dr = get_directory_record(fp);
    while (is_valid_record(&dr)) {
        if (is_dir(dr.file_flags[0])) {	// If it's a directory
            if (have_equal_extents(dr.loc_of_ext, loc_of_parent)) {	// Doesn't point to parent directory
                dr = get_directory_record(fp);	// Get the next dr in this extent
                continue;
            }
            else if (have_equal_extents(dr.loc_of_ext, loc_of_subdir)) { // Doesn't point to self
                dr = get_directory_record(fp); // Get the next dr in this extent
                continue;
            }
            else {
                print_n_tabs(depth);
                print_dr_name(&dr);
                int i_cur_pos = fseek(fp, 0, SEEK_CUR);
                i_cur_pos = ftell(fp);
                print_subdirectory(fp, dr.loc_of_ext, loc_of_subdir, depth+1);
                fseek(fp, i_cur_pos, SEEK_SET);
                dr = get_directory_record(fp);
                continue;
            }
        }
        else {
            print_n_tabs(depth);
            print_dr_name(&dr);
            dr = get_directory_record(fp);
        }
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
	struct iso_file *iso_fp = iso_open("/USR/BIN/TOUCH.TXT");
	char buff[256];
	iso_read(buff, 1, 256, iso_fp);
	buff[255] = '\0';
	printf("%s\n", buff);
	iso_close(iso_fp);
}

struct iso_file *iso_open(const char *pname) {
	struct iso_file *file = malloc(sizeof(struct iso_file));
	memcpy(file->pname, pname, strlen(pname));
	int file_offset = look_up_path_offset(pname);
	if (file_offset < 0) {
		printf("Failed to iso_open the file path.\n");
		return NULL;
	}
	file->cur_offset = 0;
	file->extent_offset = file_offset;
	return file;
}

int iso_close(struct iso_file *file) {
	free(file);
	return 0;
}

int iso_read(char *dest, int elem_size, int num_elem, struct iso_file *file) {
	FILE *fp = fopen(iso_file_name, "r");
	if (!fp) {
		printf("Failed to open file\n");
		return -1;
	}
	fseek(fp, (ISO_BLOCK * file->extent_offset) + file->cur_offset, SEEK_SET);
	int bytes_read = fread(dest, elem_size, num_elem, fp);
	file->cur_offset += bytes_read;
	fclose(fp);
	return bytes_read;
}
