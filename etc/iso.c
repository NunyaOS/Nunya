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

void print_iso_date(char *src);
int hex_to_int(char *src, int len);
char *print_directory_record(FILE *fp);
int main() {
	char fblock[2048];
	char *fname = "/home/jesse/Documents/Open Source Software Development/etc/filesystem.iso";

	FILE *fp;
	fp = fopen(fname,"r");
	if (!fp) {
		printf("Failed to open file\n");
		return -1;
	}

	fseek(fp, (1024 * 32) + 156, SEEK_SET);
	printf("Root Directory Record of PVD:\n");
	char *loc_of_ext = print_directory_record(fp);

	printf("\n\nContents of Root Directory Extent:\n");

	printf("Directory Record of Self:\n");
	fseek(fp, 2048 * hex_to_int(loc_of_ext+4, 4), SEEK_SET);

	loc_of_ext = print_directory_record(fp);

	printf("\nDirectory Record of Parent\n");
	fseek(fp, 2048 * hex_to_int(loc_of_ext + 4, 4) + 136, SEEK_SET);

	loc_of_ext = print_directory_record(fp);

	printf("\nDirectory Record of First Subdirectory\n");
	fseek(fp, 2048 * hex_to_int(loc_of_ext + 4, 4) + 136 + 102, SEEK_SET);
	loc_of_ext = print_directory_record(fp);
//	char buff[2048];
//	fread(buff, 1, 2048, fp);
//	int i, j;
//	for(i = 0; i < 2048; i += 16){
//		for(j = i; j < i + 16; j++){
//			printf("%c..", buff[j]);
//		}
//		getchar();
//	}
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

char *print_directory_record(FILE *fp){
	char length_of_record[1];
	char length_of_ext_record[1];
	char loc_of_ext[8];
	char data_length[8];
	char rec_date_time[7];
	char file_flags[1];
	char file_flags_interleaved[1];
	char interleave_gap_size[1];
	char vol_seq_num[4];
	char len_identifier[1];
	char file_identifier[2048];

	fread(length_of_record, 1, 1, fp);
	uint8_t length_of_dir_record = (uint8_t) length_of_record[0];
	fread(length_of_ext_record, 1, 1, fp);
	fread(loc_of_ext, 1, 8, fp);
	fread(data_length, 1, 8, fp);
	fread(rec_date_time, 1, 7, fp);
	fread(file_flags, 1, 1, fp);
	fread(file_flags_interleaved, 1, 1, fp);
	fread(interleave_gap_size, 1, 1, fp);
	fread(vol_seq_num, 1, 4, fp);
	fread(len_identifier, 1, 1, fp);
	fread(file_identifier, 1, (uint8_t) len_identifier, fp);

	printf("Length of Record: %d\n", length_of_dir_record);
	printf("length of Extended Record: %d\n", length_of_ext_record[0]);
	printf("Location of Extent (LE): %x..%x..%x..%x\n", loc_of_ext[0], loc_of_ext[1], loc_of_ext[2], loc_of_ext[3]);

	printf("Offset to extent: %d\n", hex_to_int(loc_of_ext+4, 4));
	printf("Location of Extent (BE): %x..%x..%x..%x\n", loc_of_ext[4], loc_of_ext[5], loc_of_ext[6], loc_of_ext[7]);
	printf("Length of Data (LE): %x..%x..%x..%x\n", data_length[0], data_length[1], data_length[2], data_length[3]);
	printf("Length of Data (BE): %x..%x..%x..%x\n", data_length[4], data_length[5], data_length[6], data_length[7]);
	print_iso_date(rec_date_time);
	printf("Interleaved Gap Size: %d\n", interleave_gap_size[0]);
	printf("Volume Sequence Number (LE): %x..%x\n", vol_seq_num[0], vol_seq_num[1]);
	printf("Volume Sequence Number (BE): %x..%x\n", vol_seq_num[2], vol_seq_num[3]);
	printf("Length of Identifier: %d\n", len_identifier[0]);
	file_identifier[(uint8_t)(len_identifier)] = '\0';
	printf("File Identifier: %s\n", file_identifier);

	return loc_of_ext;
}
