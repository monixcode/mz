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

#ifndef ARCHIVER_H
#define ARCHIVER_H

#ifdef ARCHIVER_IMPLEMENTATION

#include "definitions.h"

// MZ File Archiving Function
// Error message prefix :- Error while archiving
int mz_archive(charv *files, char *outfile, int compression, bool print_verbose, bool print_process, bool print_error)
{
	// For Cleanup references
	FILE *in = NULL;
	FILE *out = NULL;
	char *buffer = NULL;
	
	uint64_t file_count = (uint64_t)size_charv(files);
	
	if(print_verbose || print_process) printf("Archiving Files :-\n");
	out = fopen(outfile, "wb");
	if(!out){
		if(print_error) fprintf(stderr, "Error while archiving : Output File Not Opening\n");
		return MZ_EXIT_FAILURE;
	}

	if(print_verbose) printf("Writing Header in : %s\n", outfile);
	if(fwrite(MZ_HEADER, sizeof(char), 2, out) != 2){
		if(print_error) fprintf(stderr, "Error while archiving : Unable to write header\n");
		goto cleanErr;
	}

	if(print_verbose) printf("Writing Compression in : %s\n", outfile);
	if(fwrite(&compression, sizeof(int), 1, out) != 1){
		if(print_error) fprintf(stderr, "Error while archiving : Unable to write compression details\n");
		goto cleanErr;
	}

	if(print_verbose) printf("Writing File Count in : %s\n\n", outfile);
	if(fwrite(&file_count, sizeof(uint64_t), 1, out) != 1){
		if(print_error) fprintf(stderr, "Error while archiving : Unable to write file count\n");
		goto cleanErr;
	}

	if(print_verbose) printf("Allocating buffer for buffered writing\n\n");
	buffer = malloc(MZ_BUFFER);
	if(!buffer){
		if(print_error) fprintf(stderr, "Error while archiving : Unable to allocate buffer size\n\n");
		goto cleanErr;
	}

	for(size_t file = 0; file < file_count; file++){

		if(print_verbose) printf("Archiving File number : %" PRIu64 "\n", file+1);
		char *filename = get_charv(files, file);
		if(!filename){
			if(print_error) fprintf(stderr, "Error while archiving : Unable to initialize filename\n");
			goto cleanErr;
		}
		
		if(print_verbose) printf("Sanitizing Filepath : %s\n", filename);
		if(mz_sanitize_path(filename) == MZ_EXIT_FAILURE){
			if(print_error) fprintf(stderr, "Error while archiving : Sanitization Failed\n");
			goto cleanErr;
		}
		
		if(print_verbose) printf("Reading Filepath : %s\n", filename);
		in = fopen(filename, "rb");
		if(!in){
			if(print_error) fprintf(stderr, "Error while archiving : Unable to read '%s'\n", filename);
			goto cleanErr;
		}
		uint64_t filenamelength = strlen(filename);

		if(print_verbose) printf("Writing filenamelength to : %s\n", outfile);
		if(fwrite(&filenamelength, sizeof(uint64_t), 1, out) != 1){
			if(print_error) fprintf(stderr, "Error while archiving : Unable to write file name length\n");
			goto cleanErr;
		}

		if(print_verbose) printf("Writing filename to : %s\n", outfile);
		if(fwrite(filename, sizeof(char), filenamelength, out) != filenamelength){
			if(print_error) fprintf(stderr, "Error while archiving : Unable to write file name\n");
			goto cleanErr;
		}

		uint64_t filecontentsize = mz_file_size(in);

		if(print_verbose) printf("Writing filecontentsize to : %s\n", outfile);
		if(fwrite(&filecontentsize, sizeof(uint64_t), 1, out) != 1){
			if(print_error) fprintf(stderr, "Error while archiving : Unable to write file content size\n");
			goto cleanErr;
		}

		uint64_t remaining = filecontentsize;
		
		if(print_verbose) printf("Chunk writing file content in : %s\n", outfile);
		while(remaining != 0){
			size_t chunk = (remaining > MZ_BUFFER) ? MZ_BUFFER : (size_t)remaining;
			if(fread(buffer, 1, chunk, in) != chunk){
				if(print_error) fprintf(stderr, "Error while archiving : Unable to read file content\n");
				goto cleanErr;
			}
			if(fwrite(buffer, 1, chunk, out) != chunk){
				if(print_error) fprintf(stderr, "Error while archiving : Unable to write file content\n");
				goto cleanErr;
			}
			remaining -= chunk;
		}
		if(print_verbose || print_process) printf("Succesfully Archived : %s\n\n", filename);
		fclose(in);
		in = NULL;
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