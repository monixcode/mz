/*
	File :- archiver.h [For Archiving]
	
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

#ifndef MZ_MODE_ARCHIVER_H
#define MZ_MODE_ARCHIVER_H

#ifdef MZ_MODE_ARCHIVER_IMPLEMENTATION

#include "../../init/init.h"
#include <time.h>

// MZ File Archiving Function
// Error message prefix :- Error while archiving
int mz_archive(charv *files, char *outfile, bool verbose, bool error)
{
	// For Cleanup references
	FILE *in = NULL;
	FILE *out = NULL;
	char *buffer = NULL;
	
	uint64_t file_count = (uint64_t)size_charv(files);
	
	printf("Archiving Files :-\n");
	out = fopen(outfile, "wb");
	if(!out){
		if(error) fprintf(stderr, "Error while archiving : Output File Not Opening\n");
		return MZ_EXIT_FAILURE;
	}

	if(verbose) printf("Writing Header in : %s\n", outfile);
	if(fwrite(MZ_HEADER, sizeof(char), 2, out) != 2){
		if(error) fprintf(stderr, "Error while archiving : Unable to write header\n");
		goto cleanErr;
	}

	if(verbose) printf("Writing File Count in : %s\n\n", outfile);
	if(fwrite(&file_count, sizeof(uint64_t), 1, out) != 1){
		if(error) fprintf(stderr, "Error while archiving : Unable to write file count\n");
		goto cleanErr;
	}

	if(verbose) printf("Allocating buffer for buffered writing\n\n");
	buffer = malloc(MZ_BUFFER);
	if(!buffer){
		if(error) fprintf(stderr, "Error while archiving : Unable to allocate buffer size\n\n");
		goto cleanErr;
	}

	for(size_t file = 0; file < file_count; file++){

		if(verbose) printf("Archiving File number : %" PRIu64 "\n", file+1);
		char *filename = get_charv(files, file);
		if(!filename){
			if(error) fprintf(stderr, "Error while archiving : Unable to initialize filename\n");
			goto cleanErr;
		}
		
		if(verbose) printf("Reading Filepath : %s\n", filename);
		in = fopen(filename, "rb");
		if(!in){
			if(error) fprintf(stderr, "Error while archiving : Unable to read '%s'\n", filename);
			goto cleanErr;
		}
		uint64_t filenamelength = strlen(filename);

		if(verbose) printf("Writing filenamelength to : %s\n", outfile);
		if(fwrite(&filenamelength, sizeof(uint64_t), 1, out) != 1){
			if(error) fprintf(stderr, "Error while archiving : Unable to write file name length\n");
			goto cleanErr;
		}

		if(verbose) printf("Writing filename to : %s\n", outfile);
		if(fwrite(filename, sizeof(char), filenamelength, out) != filenamelength){
			if(error) fprintf(stderr, "Error while archiving : Unable to write file name\n");
			goto cleanErr;
		}

		uint64_t filecontentsize = mz_helper_file_size(in);

		if(verbose) printf("Writing filecontentsize to : %s\n", outfile);
		if(fwrite(&filecontentsize, sizeof(uint64_t), 1, out) != 1){
			if(error) fprintf(stderr, "Error while archiving : Unable to write file content size\n");
			goto cleanErr;
		}

		uint64_t remaining = filecontentsize;
		
		if(verbose) printf("Chunk writing file content in : %s\n", outfile);
		while(remaining != 0){
			size_t chunk = (remaining > MZ_BUFFER) ? MZ_BUFFER : (size_t)remaining;
			if(fread(buffer, 1, chunk, in) != chunk){
				if(error) fprintf(stderr, "Error while archiving : Unable to read file content\n");
				goto cleanErr;
			}
			if(fwrite(buffer, 1, chunk, out) != chunk){
				if(error) fprintf(stderr, "Error while archiving : Unable to write file content\n");
				goto cleanErr;
			}
			remaining -= chunk;
		}
		
		printf("Succesfully Archived : %s\n", filename);
		fclose(in);
		in = NULL;
	}
	
	printf("\n");
	
	uint64_t timestamp = (uint64_t)time(NULL);
	if(fwrite(&timestamp, sizeof(uint64_t), 1, out) != 1){
		if(error) fprintf(stderr, "Error while archiving : Unable to write time of creation\n");
		goto cleanErr;
	}
	
	fclose(out);
	free(buffer);
	return MZ_EXIT_SUCCESS;
	
cleanErr:
    if(out){
        fclose(out);
        remove(outfile);
    }
	if(in != NULL) fclose(in);
	if(buffer != NULL) free(buffer);
	return MZ_EXIT_FAILURE;
}

#endif
#endif