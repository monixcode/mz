/*
	File :- extractor.h [For Extraction]
	
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

#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#ifdef EXTRACTOR_IMPLEMENTATION

#include "definitions.h"

// MZ File Extracting Function
// Error message prefix :- Error while extracting
int mz_extract(char *mz_file, bool print_verbose, bool print_process, bool print_error)
{
	// For Cleanup references
	FILE *out = NULL;
	char *buffer = NULL;
	char *filename = NULL;
	FILE *in = NULL;
	
	if(print_verbose || print_process) printf("Extracting Files :-\n");
	in = fopen(mz_file, "rb");
	if(!in){
		if(print_error) fprintf(stderr, "Error while extracting : Unable to Open Archive File\n");
		return MZ_EXIT_FAILURE;
	}

	char check_header[2];
	int check_compression;
	uint64_t file_count;

	if(print_verbose) printf("Reading header from : %s\n", mz_file);
	if(fread(check_header, sizeof(char), 2, in) != 2){
		if(print_error) fprintf(stderr, "Error while extracting : Invalid Header\n");
		goto cleanErr;
	}

	if(print_verbose) printf("Checking header of : %s\n", mz_file);
	if(memcmp(check_header, MZ_HEADER, 2) != 0){
		if(print_error) fprintf(stderr, "Error while extracting : Invalid Header\n");
		goto cleanErr;
	}

	if(print_verbose) printf("Reading Compression from : %s\n", mz_file);
	if(fread(&check_compression, sizeof(int), 1, in) != 1){
		if(print_error) fprintf(stderr, "Error while extracting : Invalid Compression\n");
		goto cleanErr;
	}

	if(print_verbose) printf("Checking Compression of : %s\n", mz_file);
	if(check_compression != 0){
		if(print_error) fprintf(stderr, "Error while extracting : Invalid Compression\n");
		goto cleanErr;
	}

	if(print_verbose) printf("Reading file count from : %s\n\n", mz_file);
	if(fread(&file_count, sizeof(uint64_t), 1, in) != 1){
		if(print_error) fprintf(stderr, "Error while extracting : Invalid file count\n");
		goto cleanErr;
	}

	if(print_verbose) printf("Allocating buffer for buffered writing\n\n");
	buffer = malloc(MZ_BUFFER);
	if(!buffer){
		if(print_error) fprintf(stderr, "Error while extracting : Unable to allocate buffer size\n");
		goto cleanErr;
	}

	for(uint64_t file = 0; file < file_count; file++){
		
		if(print_verbose) printf("Extracting File number : %" PRIu64 "\n", file+1);
		
		uint64_t filenamelength;
		if(print_verbose) printf("Reading filenamelength from : %s\n", mz_file);
		if(fread(&filenamelength, sizeof(uint64_t), 1, in) != 1){
			if(print_error) fprintf(stderr, "Error while extracting : Unable to read filenamelength\n");
			goto cleanErr;
		}
		
		if(print_verbose) printf("Checking filenamelength \n");
		if (filenamelength == 0 || filenamelength > 4096){
			if(print_error) fprintf(stderr, "Error while extracting : Invalid filename length\n");
			goto cleanErr;
		}
	
		if(print_verbose) printf("Allocating filename \n");
		filename = malloc(filenamelength + 1);
		if(!filename){
			if(print_error) fprintf(stderr, "Error while extracting : Unable to allocate filename\n");
			goto cleanErr;
		}
		if(print_verbose) printf("Reading filename from : %s\n", mz_file);
		if(fread(filename, sizeof(char), filenamelength, in) != (size_t)filenamelength){
			if(print_error) fprintf(stderr, "Error while extracting : Unable to read filename\n");
			goto cleanErr;
		}
		filename[filenamelength] = '\0';

		uint64_t filecontentsize;
		if(print_verbose) printf("Reading filecontentsize from : %s\n", mz_file);
		if(fread(&filecontentsize, sizeof(uint64_t), 1, in) != 1){
			if(print_error) fprintf(stderr, "Error while extracting : Unable to read filecontentsize\n");
			goto cleanErr;
		}

		if(print_verbose) printf("Sanitizing Filepath : %s\n", filename);
		if(mz_sanitize_path(filename) == MZ_EXIT_FAILURE){
			if(print_error) fprintf(stderr, "Error while extracting : Skipping %s\n", filename);
			mz_fseek(in, filecontentsize, SEEK_CUR);
			free(filename);
			filename = NULL;
			continue;
		}

		if(print_verbose) printf("Making required directories\n");
		if(mz_check_dir(filename) != MZ_EXIT_SUCCESS){
			goto cleanErr;
		}

		out = fopen(filename, "wb");
		if(!out){
			if(print_error) fprintf(stderr, "Error while extracting : Unable to Open file\n");
			goto cleanErr;
		}

		uint64_t remaining = filecontentsize;
		if(print_verbose) printf("Chunk writing file content in : %s\n", filename);
		while(remaining != 0){
			size_t chunk = (remaining > MZ_BUFFER) ? MZ_BUFFER : (size_t)remaining;
			if(fread(buffer, 1, chunk , in) != chunk){
				if(print_error) fprintf(stderr, "Error while extracting : Unable to read file content\n");
				goto cleanErr;
			}
			if(fwrite(buffer, 1, chunk , out) != chunk){
				if(print_error) fprintf(stderr, "Error while extracting : Unable to write file content\n");
				goto cleanErr;
			}
			remaining -= chunk;
		}
		if(print_verbose || print_process) printf("Succesfully Extracted : %s\n\n", filename);
		free(filename);
		filename = NULL;
		fclose(out);
		out = NULL;
	}
	fclose(in);
	free(buffer);
	return MZ_EXIT_SUCCESS;
	
cleanErr:
    if(out) fclose(out);
	if(in != NULL) fclose(in);
	if(buffer != NULL) free(buffer);
	if(filename != NULL) free(filename);
	return MZ_EXIT_FAILURE;
}

#endif
#endif