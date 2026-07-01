/*
	File :- specific_extractor.h [For Extraction]
	
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

#ifndef MZ_MODE_SPECIFIC_EXTRACTOR_H
#define MZ_MODE_SPECIFIC_EXTRACTOR_H

#ifdef MZ_MODE_SPECIFIC_EXTRACTOR_IMPLEMENTATION

#include "../../init/init.h"

static bool is_file_in_files(char *filename, charv *files, bool error)
{
	size_t size = size_charv(files);
	for(size_t i = 1; i < size; i++){
		char *in_files = get_charv(files, i);
		if(!in_files){
			if(error) fprintf(stderr, "Error while specific extracting [is_file_in_files] : Unable to Get File\n");
			continue;
		}
		if(strcmp(filename, in_files) == 0){
			return true;
		}
	}
	return false;
}

int mz_extract_specific(charv *files, bool verbose, bool error)
{
	FILE *in = NULL;
	FILE *out = NULL;
	char *buffer = NULL;
	char *filename = NULL;
	
	charv *files_extracted = init_charv();
	if(!files_extracted){
		if(error) fprintf(stderr, "Error while specific extracting : Unable to Initialize Vector\n");
		return MZ_EXIT_FAILURE;
	}
	
	printf("Extracting Files :-\n\n");
	char *mz_file = get_charv(files, 0);
	if(!mz_file){
		if(error) fprintf(stderr, "Error while specific extracting : Unable to Get Archive File\n");
		return MZ_EXIT_FAILURE;
	}
	
	in = fopen(mz_file, "rb");
	if(!in){
		if(error) fprintf(stderr, "Error while specific extracting : Unable to Open Archive File\n");
		return MZ_EXIT_FAILURE;
	}
	
	char check_header[3];
	uint64_t file_count;
	
	if(verbose) printf("Reading header from : %s\n", mz_file);
	if(fread(check_header, sizeof(char), 2, in) != 2){
		if(error) fprintf(stderr, "Error while specific extracting : Invalid Header\n");
		goto cleanErr;
	}
	check_header[2] = '\0';

	if(verbose) printf("Checking header of : %s\n", mz_file);
	if(memcmp(check_header, MZ_HEADER, 2) != 0){
		if(error) fprintf(stderr, "Error while specific extracting : Invalid Header\n");
		goto cleanErr;
	}

	if(verbose) printf("Reading file count from : %s\n\n", mz_file);
	if(fread(&file_count, sizeof(uint64_t), 1, in) != 1){
		if(error) fprintf(stderr, "Error while specific extracting : Invalid file count\n");
		goto cleanErr;
	}
	
	if(verbose) printf("Allocating buffer for buffered writing\n\n");
	buffer = malloc(MZ_BUFFER);
	if(!buffer){
		if(error) fprintf(stderr, "Error while specific extracting : Unable to allocate buffer size\n");
		goto cleanErr;
	}

	for(uint64_t file = 0; file < file_count; file++){
		
		if(verbose) printf("Extracting File number : %" PRIu64 "\n", file+1);
		
		uint64_t filenamelength;
		if(verbose) printf("Reading filenamelength from : %s\n", mz_file);
		if(fread(&filenamelength, sizeof(uint64_t), 1, in) != 1){
			if(error) fprintf(stderr, "Error while specific extracting : Unable to read filenamelength\n");
			goto cleanErr;
		}
		
		if(verbose) printf("Checking filenamelength \n");
		if (filenamelength == 0 || filenamelength > 4096){
			if(error) fprintf(stderr, "Error while specific extracting : Invalid filename length\n");
			goto cleanErr;
		}
	
		if(verbose) printf("Allocating filename \n");
		filename = malloc(filenamelength + 1);
		if(!filename){
			if(error) fprintf(stderr, "Error while specific extracting : Unable to allocate filename\n");
			goto cleanErr;
		}
		if(verbose) printf("Reading filename from : %s\n", mz_file);
		if(fread(filename, sizeof(char), filenamelength, in) != (size_t)filenamelength){
			if(error) fprintf(stderr, "Error while specific extracting : Unable to read filename\n");
			goto cleanErr;
		}
		filename[filenamelength] = '\0';
		
		uint64_t filecontentsize;
		if(verbose) printf("Reading filecontentsize from : %s\n", mz_file);
		if(fread(&filecontentsize, sizeof(uint64_t), 1, in) != 1){
			if(error) fprintf(stderr, "Error while specific extracting : Unable to read filecontentsize\n");
			goto cleanErr;
		}
		
		if(is_file_in_files(filename, files, error) != true){
			free(filename);
			filename = NULL;
			mz_fseek(in, filecontentsize, SEEK_CUR);
			continue;
		}
		
		if(verbose) printf("Sanitizing Filepath : %s\n", filename);
		if(mz_helper_sanitize_path(filename) == MZ_EXIT_FAILURE){
			if(error) fprintf(stderr, "Error while specific extracting : Skipping %s\n", filename);
			mz_fseek(in, filecontentsize, SEEK_CUR);
			free(filename);
			filename = NULL;
			continue;
		}
		
		if(verbose) printf("Making required directories\n");
		if(mz_helper_mkdir_filepath(filename, error) != MZ_EXIT_SUCCESS){
			goto cleanErr;
		}
		out = fopen(filename, "wb");
		if(!out){
			if(error) fprintf(stderr, "Error while specific extracting : Unable to Open file\n");
			goto cleanErr;
		}

		uint64_t remaining = filecontentsize;
		if(verbose) printf("Chunk writing file content in : %s\n", filename);
		while(remaining != 0){
			size_t chunk = (remaining > MZ_BUFFER) ? MZ_BUFFER : (size_t)remaining;
			if(fread(buffer, 1, chunk , in) != chunk){
				if(error) fprintf(stderr, "Error while specific extracting : Unable to read file content\n");
				goto cleanErr;
			}
			if(fwrite(buffer, 1, chunk , out) != chunk){
				if(error) fprintf(stderr, "Error while specific extracting : Unable to write file content\n");
				goto cleanErr;
			}
			remaining -= chunk;
		}
		if(push_charv(files_extracted, filename) != 0){
			if(error) fprintf(stderr, "Error while specific extracting : Unable to push filename\n");
			goto cleanErr;
		}
		printf("Succesfully Extracted : %s from %s\n", filename, mz_file);
		free(filename);
		filename = NULL;
		fclose(out);
		out = NULL;
	}
	if(size_charv(files) - 1 != size_charv(files_extracted)){
		if(error) fprintf(stderr, "Error while specific extracting : Few Files were not found\n");
		goto cleanErr;
	}
	printf("\n");
	fclose(in);
	free(buffer);
	destroy_charv(files_extracted);
	return MZ_EXIT_SUCCESS;
	
cleanErr:
    if(out) fclose(out);
	if(in != NULL) fclose(in);
	if(buffer != NULL) free(buffer);
	remove(filename);
	if(filename != NULL) free(filename);
	if(files_extracted != NULL) destroy_charv(files_extracted);
	return MZ_EXIT_FAILURE;
}

#endif
#endif
