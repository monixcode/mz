/*
	File :- filesin.h [For Checking Files]
	
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

#ifndef MZ_MODE_FILESIN_H
#define MZ_MODE_FILESIN_H

#ifdef MZ_MODE_FILESIN_IMPLEMENTATION

#include "../../init/init.h"

// MZ function for printing number of files and their names stored in an archive file 
// Error message prefix :- Error while checking files
int mz_filesin(char *mz_file, bool error)
{
	// For Cleanup references
	FILE *in = NULL;
	char *filename = NULL;
	
	in = fopen(mz_file, "rb");
	if(!in){
		if(error) fprintf(stderr, "Error while checking files : Unable to Open Archive File\n");
		return MZ_EXIT_FAILURE;
	}

	char check_header[3];
	uint64_t file_count;

	if(fread(check_header, sizeof(char), 2, in) != 2){
		if(error) fprintf(stderr, "Error while checking files : Invalid Header\n");
		goto cleanErr;
	}
	check_header[2] = '\0';

	if(memcmp(check_header, MZ_HEADER, 2) != 0){
		if(error) fprintf(stderr, "Error while checking files : Invalid Header\n");
		goto cleanErr;
	}

	if(fread(&file_count, sizeof(uint64_t), 1, in) != 1){
		if(error) fprintf(stderr, "Error while checking files : Invalid File Count\n");
		goto cleanErr;
	}

	for(uint64_t file = 0; file < file_count; file++){
		uint64_t filenamelength;
		if(fread(&filenamelength, sizeof(uint64_t), 1, in) != 1){
			if(error) fprintf(stderr, "Error while checking files : Unable to read filenamelength\n");
			goto cleanErr;
		}
		if(filenamelength == 0 || filenamelength > 4096){
			if(error) fprintf(stderr, "Error while checking files : Invalid filename length\n");
			goto cleanErr;
		}
		filename = malloc(filenamelength + 1);
		if(!filename){
			if(error) fprintf(stderr, "Error while checking files : Unable to read filename\n");
			goto cleanErr;
		}
		if(fread(filename, sizeof(char), filenamelength, in) != (size_t)filenamelength){
			if(error) fprintf(stderr, "Error while checking files : Unable to read filename\n");
			goto cleanErr;
		}
		filename[filenamelength] = '\0';

		uint64_t filecontentsize;
		if(fread(&filecontentsize, sizeof(uint64_t), 1, in) != 1){
			if(error) fprintf(stderr, "Error while checking files : Unable to read filecontentsize\n");
			goto cleanErr;
		}
		
		mz_fseek(in, filecontentsize, SEEK_CUR);
		printf("%s\n", filename);
		free(filename);
		filename = NULL;
	}
	fclose(in);
	return MZ_EXIT_SUCCESS;
	
cleanErr:
	if(in != NULL) fclose(in);
	if(filename != NULL) free(filename);
	return MZ_EXIT_FAILURE;
}

#endif
#endif