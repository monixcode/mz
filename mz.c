/*
	File :- mz.c [main controller of the program]
	
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "init/init.h"

// For Vector datatype
#define MZ_LIB_VECTOR_IMPLEMENTATION
#include "lib/vector/vector.h"

// For file walking
#define MZ_LIB_FILEWALKER_IMPLEMENTATION
#include "lib/filewalker/filewalker.h"

// For archiving
#define MZ_MODE_ARCHIVER_IMPLEMENTATION
#include "modes/archiver/archiver.h"

// For extracting
#define MZ_MODE_EXTRACTOR_IMPLEMENTATION
#include "modes/extractor/extractor.h"

// For specific extracting
#define MZ_MODE_SPECIFIC_EXTRACTOR_IMPLEMENTATION
#include "modes/specific_extractor/specific_extractor.h"

// For extracting
#define MZ_MODE_FILESIN_IMPLEMENTATION
#include "modes/filesin/filesin.h"

// For extracting
#define MZ_MODE_INFO_IMPLEMENTATION
#include "modes/info/info.h"

// For parsing arguments
#define MZ_PARSER_IMPLEMENTATION
#include "parser/parser.h"

// MZ main function
int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if(argc < 2){
		printf("Use : mz --help for more info\n");
		return MZ_EXIT_SUCCESS;
	}
	MZ_ARGS args = mz_parse_args(argc, argv);

	if(args.EXIT_CODE != MZ_EXIT_SUCCESS){
		destroy_charv(args.files);
		destroy_charv(args.folders);
		return MZ_EXIT_FAILURE;
	}
	
	bool verbose = args.flag.verbose;
	bool error = args.flag.error;
	
	if(args.input.dir){
		size_t folder_count = size_charv(args.folders);
		for(size_t i = 0; i < folder_count; i++)
		{
			char *folder = get_charv(args.folders, i);
			if(!folder){
				args.EXIT_CODE = MZ_EXIT_FAILURE;
				if (error) fprintf(stderr, "Error : UNABLE TO GET FOLDER\n");
				goto cleanFail;
			}
			charv *files_in_folder = filewalk(folder, verbose);
			if(!files_in_folder){
				args.EXIT_CODE = MZ_EXIT_FAILURE;
				if (error) fprintf(stderr, "Error : UNABLE TO FILEWALK\n");
				goto cleanFail;
			}

			size_t size = size_charv(files_in_folder);

			for(size_t f = 0; f < size; f++){
				char *file_in_files = get_charv(files_in_folder, f);
				if(!file_in_files){
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					if (error) fprintf(stderr, "Error : UNABLE TO GET FILENAME \n");
					goto cleanFail;
				}
				int r = push_charv(args.files, file_in_files);
				if(r == -1){
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					if (error) fprintf(stderr, "Error : UNABLE TO PUSH FILE TO HEAP\n");
					goto cleanFail;
				}
			}
			destroy_charv(files_in_folder);
		}
		
		destroy_charv(args.folders);
		
	}

	if(args.flag.help){
		printf("MZ File Archiver\n");
		printf("Bundle multiple files into a single archive.\n\n");

		printf("USAGE\n");
		printf("  mz [options]\n\n");

		printf("ARCHIVE OPERATIONS\n");
		printf("  -a <files> -o <archive.mz>\n");
		printf("      Create an archive containing one or more files.\n\n");

		printf("  -x <archive.mz> ...\n");
		printf("      Extract all files from an archive.\n\n");
		
		printf("  -xs <archive.mz> [filename] [filename]\n");
		printf("      Extract specific files from an archive.\n\n");

		printf("ARCHIVE INFORMATION\n");
		printf("  -fi <archive.mz> ...\n");
		printf("      List all files stored in the archive\n");
		
		printf("  -info <archive.mz> ...\n");
		printf("      Prints the Metadata of the File\n");
		printf("      Header, File count, Creation Date\n\n");
		
		printf("INPUT OPTIONS\n");
		printf("  -Idir <folder>\n");
		printf("      Recursively collect files from a folder and use them\n");
		printf("      as archive input.\n");
		printf("      Note: Files larger than 2 GB may not be supported due\n");
		printf("      to system stat() limitations.\n\n");
		printf("  -Ifile <file>\n");
		printf("      Explicitly takes in a file, in case normal passes does not work\n\n");

		printf("OUTPUT OPTIONS\n");
		printf("  --verbose\n");
		printf("      Display detailed operation information.\n\n");

		printf("  --error\n");
		printf("      Display internal warnings and verboses.\n\n");

		printf("GENERAL OPTIONS\n");
		printf("  -o <file>\n");
		printf("      Specify the output archive filename.\n\n");

		printf("  --help\n");
		printf("      Show this help message.\n\n");

		printf("  --version\n");
		printf("      Show version information.\n\n");

		printf("EXAMPLES\n");
		printf("  mz -a file1.txt file2.txt -o backup.mz\n");
		printf("  mz -x backup.mz\n");
		printf("  mz -xs backup.mz docs/image.png videos/video.mp4\n");
		printf("  mz -a -Idir project -o project.mz\n");
		printf("  mz -fi backup.mz\n\n");
		goto cleanSucc;

	}else if(args.flag.version){
		printf("%s - %s\n",MZ_HEADER, MZ_VERSION);
		goto cleanSucc;

	}else if(args.mode == MODE_FILES_IN){
		size_t size = size_charv(args.files);
		size_t sizedir = size_charv(args.folders);
		if(size == 0 && sizedir == 0){
			if (error) fprintf(stderr, "Error : No Files Passed\n");
			goto cleanFail;
		}
		for(uint64_t i = 0 ; i < size; i++){
			char *filename = get_charv(args.files, i);
			if(!filename){
				if (error) fprintf(stderr, "Error : Unable to Find Files\n");
				goto cleanFail;
			}
			int filesin = mz_filesin(filename, verbose);
			if(filesin != 0){
				if (error) fprintf(stderr, "Error : Unable to Find Files in %s\n", filename);
				goto cleanFail;
			}
		}
		goto cleanSucc;

	}else if(args.mode == MODE_ARCHIVE){

		if((size_charv(args.files) == 0) && (size_charv(args.folders) == 0)){
			if (error) fprintf(stderr, "Error : No Files Passed\n");
			goto cleanFail;
		}

		if(args.outfile == NULL){
			if (error) fprintf(stderr, "Error : No Output File Passed\n");
			goto cleanFail;
		}
		bool verbose = args.flag.verbose;
		
		int archive = mz_archive(args.files, args.outfile, verbose, error);
		if(archive != 0){
			if (error) fprintf(stderr, "Error : Unable to Archive\n");
			goto cleanFail;
		}
		printf("Archiving Completed\n");
		goto cleanSucc;
		
	}else if(args.mode == MODE_EXTRACT){
		
		size_t size = size_charv(args.files);
		if((size == 0) && (size_charv(args.folders) == 0)){
			if (error) fprintf(stderr, "Error : No Files Passed\n");
			goto cleanFail;
		}
		bool verbose = args.flag.verbose;
		
		for(uint64_t i = 0 ; i < size; i++){
			char *filename = get_charv(args.files, i);
			if(!filename){
				if (error) fprintf(stderr, "Error : Unable to Extract \n");
				goto cleanFail;
			}
			int extract = mz_extract(filename, verbose, error);
			if(extract != 0){
				if (error) fprintf(stderr, "Error : Unable to Extract %s\n", filename);
				goto cleanFail;
			}
			printf("Extraction Completed\n");
		}
		goto cleanSucc;
		
	}else if(args.mode == MODE_EXTRACT_SPECIFIC){
		
		size_t size = size_charv(args.files);
		if((size == 0) && (size_charv(args.folders) == 0)){
			if (error) fprintf(stderr, "Error : No Archive Passed\n");
			goto cleanFail;
		}
		else if((size == 1) && (size_charv(args.folders) == 0)){
			if (error) fprintf(stderr, "Error : No Specified Files Passed\n");
			goto cleanFail;
		}
		bool verbose = args.flag.verbose;
		
		int extract_specific = mz_extract_specific(args.files, verbose, error);
		if(extract_specific != 0){
			if (error) fprintf(stderr, "Error : Unable to Specific Extract\n");
			goto cleanFail;
		}
		printf("Specific Extraction Completed\n");
		
	}else if(args.mode == MODE_INFO){
		size_t size = size_charv(args.files);
		size_t sizedir = size_charv(args.folders);
		if(size == 0 && sizedir == 0){
			if (error) fprintf(stderr, "Error : No Files Passed\n");
			goto cleanFail;
		}
		for(uint64_t i = 0 ; i < size; i++){
			char *filename = get_charv(args.files, i);
			if(!filename){
				if (error) fprintf(stderr, "Error : Unable to Get Filename\n");
				goto cleanFail;
			}
			int info = mz_info(filename, error);
			if(info != 0){
				if (error) fprintf(stderr, "Error : Unable to get File info \n");
				goto cleanFail;
			}
		}
		goto cleanSucc;
	}
	goto cleanSucc;
cleanSucc:
    destroy_charv(args.files);
	destroy_charv(args.folders);
    return MZ_EXIT_SUCCESS;
cleanFail:
    destroy_charv(args.files);
	destroy_charv(args.folders);
	if(!error) printf("Error occurred. Re-run with --error to view internal details.\n");
    return MZ_EXIT_FAILURE;
}
