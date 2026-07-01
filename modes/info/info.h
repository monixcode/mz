/*
	File :- info.c [prints metatdata about the archive]
	
	MIT License

	Copyright (c) 2026 Moinak Debnath

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef MZ_MODE_INFO_H
#define MZ_MODE_INFO_H

#ifdef MZ_MODE_INFO_IMPLEMENTATION

#include "../../init/init.h"

int mz_info(char *mz_file, bool error)
{
	// For Cleanup references
	FILE *in = NULL;
	
	in = fopen(mz_file, "rb");
	if(!in){
		if(error) fprintf(stderr, "Error while printing info : Unable to Open Archive File\n");
		return MZ_EXIT_FAILURE;
	}

	char check_header[3];
	uint64_t file_count;
	
	if(fread(check_header, sizeof(char), 2, in) != 2){
		if(error) fprintf(stderr, "Error while printing info : Invalid Header\n");
		goto cleanErr;
	}
	check_header[2] = '\0';

	if(memcmp(check_header, MZ_HEADER, 2) != 0){
		if(error) fprintf(stderr, "Error while printing info : Invalid Header\n");
		goto cleanErr;
	}

	if(fread(&file_count, sizeof(uint64_t), 1, in) != 1){
		if(error) fprintf(stderr, "Error while printing info : Invalid File Count\n");
		goto cleanErr;
	}

	for(uint64_t file = 0; file < file_count; file++){
		uint64_t filenamelength;
		if(fread(&filenamelength, sizeof(uint64_t), 1, in) != 1){
			if(error) fprintf(stderr, "Error while printing info : Unable to read filenamelength\n");
			goto cleanErr;
		}
		if(filenamelength == 0 || filenamelength > 4096){
			if(error) fprintf(stderr, "Error while printing info : Invalid filename length\n");
			goto cleanErr;
		}
		mz_fseek(in, filenamelength, SEEK_CUR);

		uint64_t filecontentsize;
		if(fread(&filecontentsize, sizeof(uint64_t), 1, in) != 1){
			if(error) fprintf(stderr, "Error while printing info : Unable to read filecontentsize\n");
			goto cleanErr;
		}
		
		mz_fseek(in, filecontentsize, SEEK_CUR);
	}
	
	uint64_t timestamp;

	if (fread(&timestamp, sizeof(uint64_t), 1, in) != 1) {
		if (error)
			fprintf(stderr, "Error while printing info: Unable to read time of creation\n");
		goto cleanErr;
	}

	time_t t = (time_t)timestamp;
	struct tm *local_time = localtime(&t);

	if (local_time == NULL) {
		if (error) fprintf(stderr, "Error while printing info : Failed to convert timestamp.\n");
		goto cleanErr;
	}
	
	uint64_t bytes = mz_helper_file_size(in);
	double mib = (double)bytes / (1024.0 * 1024.0);
	
	printf("\n");
	printf("Archive Name     : %s\n", mz_file);
	printf("Archive Size     : %.2f MiB\n", mib);
	printf("Header           : %s\n", check_header);
	printf("File Count       : %" PRIu64 "\n", file_count);
	printf("Creation Date    : %04d-%02d-%02d %02d:%02d:%02d\n",
		   local_time->tm_year + 1900,
		   local_time->tm_mon + 1,
		   local_time->tm_mday,
		   local_time->tm_hour,
		   local_time->tm_min,
		   local_time->tm_sec);
	printf("\n");
		
	fclose(in);
	return MZ_EXIT_SUCCESS;
	
cleanErr:
	if(in != NULL) fclose(in);
	return MZ_EXIT_FAILURE;
}

#endif
#endif