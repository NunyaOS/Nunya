/*
 * iso.c
 *
 *  Created on: Jan 29, 2016
 *      Author: jesse
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

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

void print_dr_name(struct directory_record *DR);
void print_iso_date(char *src);
int hex_to_int(char *src, int len);
void print_file_system(FILE *fp);
struct directory_record get_directory_record(FILE *fp);
void print_directory_record(struct directory_record *DR);
int is_dir(int flags);
int is_valid_record(struct directory_record *DR);
void print_subdirectory(FILE *fp, char *loc_of_subdir, char *loc_of_parent);
int have_equal_extents(char *first, char *second);

void print_file_system(FILE *fp){
	fseek(fp, (16 * ISO_BLOCK) + PVD_OFFSET, SEEK_SET);
	struct directory_record DR;
	char loc_of_parent[8];
	DR = get_directory_record(fp);	// This is the DR for the PVD
	if (is_valid_record(&DR)) {
		print_dr_name(&DR);
		printf("\n");
	}
	memcpy(loc_of_parent, DR.loc_of_ext, 8);
	print_subdirectory(fp, DR.loc_of_ext, loc_of_parent);
}

void print_subdirectory(FILE *fp, char *loc_of_subdir, char *loc_of_parent){
	fseek(fp, ISO_BLOCK * hex_to_int(loc_of_subdir + 4, 4), SEEK_SET);	// Go to extent of subdir
	struct directory_record DR;
	DR = get_directory_record(fp);
	while(is_valid_record(&DR)){
		if (is_dir(DR.file_flags[0])) {	// If it's a directory
			if (have_equal_extents(DR.loc_of_ext, loc_of_parent)){	// Doesn't point to parent directory
				DR = get_directory_record(fp);	// Get the next DR in this extent
				continue;
			}
			else if (have_equal_extents(DR.loc_of_ext, loc_of_subdir)) {	// Doesn't point to self
				DR = get_directory_record(fp);	// Get the next DR in this extent
				continue;
			}
			else {
				print_dr_name(&DR);
				int i_cur_pos = fseek(fp, 0, SEEK_CUR);
				i_cur_pos = ftell(fp);
				print_subdirectory(fp, DR.loc_of_ext, loc_of_subdir);
				fseek(fp, i_cur_pos, SEEK_SET);
				DR = get_directory_record(fp);
				continue;
			}
		}
		else {
			print_dr_name(&DR);
			DR = get_directory_record(fp);
		}

	}
	printf("\n");
}

int main() {
	char fblock[2048];
	char *fname = "/home/jesse/Documents/Open Source Software Development/test_ISOs/filesystem4.iso";

	FILE *fp;
	fp = fopen(fname,"r");
	if (!fp) {
		printf("Failed to open file\n");
		return -1;
	}

	print_file_system(fp);
}


int hex_to_int(char *src, int len){
	int i, sum = 0;
	for(i = 0; i < len; i++){
		sum *= 16;
		sum += src[i];
	}
	return sum;
}
// Print ISO Date in format h:m:s m/d/y
void print_iso_date(char *src){
	printf("%d:%d:%d %d/%d/%d\n", src[3], src[4], src[5], src[1], src[2], src[0] + 1900);
}

int is_valid_record(struct directory_record *DR){
	if(DR->rec_date_time[3] < 0 || DR->rec_date_time[3] >= 24){	// validate hours
		return 0;
	}
	if(DR->rec_date_time[4] < 0 || DR->rec_date_time[4] >= 60){	// validate minutes
		return 0;
	}
	if(DR->rec_date_time[5] < 0 || DR->rec_date_time[5] >= 60){	// validate seconds
		return 0;
	}
	if(DR->rec_date_time[1] <= 0 || DR->rec_date_time[1] > 12){	// validate month
		return 0;
	}
	if(DR->rec_date_time[2] <= 0 || DR->rec_date_time[2] > 31){	// validate day
		return 0;
	}
	if(DR->rec_date_time[0] < 0){	// validate year
		return 0;
	}
	return 1;
}

struct directory_record get_directory_record(FILE *fp){
	int bytes_read = 0;

	struct directory_record DR;
	fread(&(DR.length_of_record), sizeof(DR.length_of_record), 1, fp);
	uint8_t length_of_dir_record = (DR.length_of_record);
	fread(&(DR.length_of_ext_record), 1, sizeof(DR.length_of_ext_record), fp);
	fread(DR.loc_of_ext, 1, 8, fp);
	fread(DR.data_length, 1, 8, fp);
	fread(DR.rec_date_time, 1, 7, fp);
	fread(DR.file_flags, 1, 1, fp);
	int flags = (int) (DR.file_flags[0]);
	fread(DR.file_flags_interleaved, 1, 1, fp);
	fread(&(DR.interleave_gap_size), 1, sizeof(DR.interleave_gap_size), fp);
	fread(DR.vol_seq_num, 1, 4, fp);
	fread(&(DR.len_identifier), 1, sizeof(DR.len_identifier), fp);
	bytes_read += 33;
	fread(DR.file_identifier, 1, DR.len_identifier, fp);
	bytes_read += (DR.len_identifier);
	fseek(fp, length_of_dir_record - bytes_read, SEEK_CUR);
	return DR;
}

void print_dr_name(struct directory_record *DR){
	DR->file_identifier[(DR->len_identifier)] = '\0';

	if (DR->file_identifier[0] > 0x20) {
		printf("File Identifier: %s\n", DR->file_identifier);
	}

	else {
		printf("File Identifier: /\n");
	}
}

void print_directory_record(struct directory_record *DR){
	uint8_t length_of_dir_record = DR->length_of_record;
	printf("Length of Record: %d\n", length_of_dir_record);
	printf("length of Extended Record: %d\n", DR->length_of_ext_record);
	printf("Location of Extent (LE): %x..%x..%x..%x\n", DR->loc_of_ext[0], DR->loc_of_ext[1], DR->loc_of_ext[2], DR->loc_of_ext[3]);
	printf("Location of Extent (BE): %x..%x..%x..%x\n", DR->loc_of_ext[4], DR->loc_of_ext[5], DR->loc_of_ext[6], DR->loc_of_ext[7]);
	printf("Offset to extent: %d\n", hex_to_int(DR->loc_of_ext+4, 4));
	printf("Length of Data (LE): %x..%x..%x..%x\n", DR->data_length[0], DR->data_length[1], DR->data_length[2], DR->data_length[3]);
	printf("Length of Data (BE): %x..%x..%x..%x\n", DR->data_length[4], DR->data_length[5], DR->data_length[6], DR->data_length[7]);
	print_iso_date(DR->rec_date_time);
	printf("Interleaved Gap Size: %d\n", DR->interleave_gap_size);
	printf("Volume Sequence Number (LE): %x..%x\n", DR->vol_seq_num[0], DR->vol_seq_num[1]);
	printf("Volume Sequence Number (BE): %x..%x\n", DR->vol_seq_num[2], DR->vol_seq_num[3]);
	printf("Length of Identifier: %d\n", DR->len_identifier);
	DR->file_identifier[(DR->len_identifier)] = '\0';

	if (DR->file_identifier[0] > 0x20) {
		printf("File Identifier: %s\n", DR->file_identifier);
	}

	else {
		printf("/\n");
	}
}

int is_dir(int flags){
	if((flags >> 1) & 1) {
		return 1;
	}
	else {
		return 0;
	}
}

int have_equal_extents(char *first_ext, char *second_ext){
	int ext1 = hex_to_int((first_ext) + 4, 4);
	int ext2 = hex_to_int((second_ext) + 4, 4);
	if(ext1 == ext2) {
		return 1;
	}
	return 0;
}
