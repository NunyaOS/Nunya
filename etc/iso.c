/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "iso.h"
#include <string.h>
#include "kerneltypes.h"

#define PVD_OFFSET 156
#define ISO_BLOCK 2048

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

struct directory_record get_directory_record(FILE *fp);
int have_equal_extents(char *first, char *second);
int hex_to_int(char *src, int len);
int is_dir(int flags);
int is_valid_record(struct directory_record *dr);
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

void print_dr_name(struct directory_record *dr) {
    if (dr->file_identifier[0] > 0x20) {
        printf("File/Dir Identifier: %s\n", dr->file_identifier);
    }
    else {
        printf("File/Dir Identifier: /\n");
    }
}

void print_file_system(FILE *fp) {
    fseek(fp, (16 * ISO_BLOCK) + PVD_OFFSET, SEEK_SET);
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
    if (argc != 2) {
        printf("Usage: %s iso-file-name\n", argv[0]);
        return 1;
    }

    char fblock[2048];

    FILE *fp;
    fp = fopen(argv[1],"r");
    if (!fp) {
        printf("Failed to open file\n");
        return -1;
    }
    print_file_system(fp);
}
