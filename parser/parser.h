/*
	File :- parser.h [For Parsing Arguments]
	
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

#ifndef MZ_PARSER_H
#define MZ_PARSER_H

#ifdef MZ_PARSER_IMPLEMENTATION

#include "../init/init.h"

// MZ parsing arguments function
// Error message prefix :- Error while parsing
MZ_ARGS mz_parse_args(int argc, char *argv[])
{
	MZ_ARGS args = {0};
	
	args.mode = MODE_NONE;
	
	args.flag.help = false;
	args.flag.version = false;
	args.flag.verbose = false;
	args.flag.error = false;
	
	args.input.dir = false;
	args.input.file = false;
	
	args.files = init_charv();
	args.folders = init_charv();
	if(!args.folders){
		fprintf(stderr, "Error while parsing : UNABLE TO INITIALIZE FOLDERS\n");
		args.EXIT_CODE = MZ_EXIT_FAILURE;
		return args;
	}
	if(!args.files){
		fprintf(stderr, "Error while parsing : UNABLE TO INITIALIZE FILES\n");
		args.EXIT_CODE = MZ_EXIT_FAILURE;
		return args;
	}
	args.outfile = NULL;
	args.EXIT_CODE = MZ_EXIT_FAILURE;

	bool continue_loop = false;

	for(int arg = 1; arg < argc; arg++){
		if(continue_loop){
			continue_loop = false;
			continue;
		}
		char *argument = argv[arg];
		// option parsing
		if(argument[0] == '-'){
			char *opt = argument + 1;
			char *flag = argument + 2;
			char *input = argument + 2;
			char *print = argument + 2;

			// optional flag parsing
			if(argument[1] == '-'){
				if(strcmp(flag, "help") == 0){
					if(arg > 1){
						fprintf(stderr, "Error while parsing : EXTRA ARGUMENTS PASSED FOR HELP\n");
						args.EXIT_CODE = MZ_EXIT_FAILURE;
						return args;
					}
					args.flag.help = true;
				}else if(strcmp(flag, "version") == 0){
					if(arg > 1){
						fprintf(stderr, "Error while parsing : EXTRA ARGUMENTS PASSED FOR VERSION\n");
						args.EXIT_CODE = MZ_EXIT_FAILURE;
						return args;
					}
					args.flag.version = true;
				}else if(strcmp(flag, "error") == 0){
					args.flag.error = true;
				}else if(strcmp(flag, "verbose") == 0){
					args.flag.verbose = true;
					continue;
				}else{
					fprintf(stderr, "Error while parsing : INVALID OPTIONAL FLAG\n");
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
			}else if(argument[1] == 'I'){
				if(strcmp(input, "dir") == 0){
					if(arg == argc - 1){
						fprintf(stderr, "Error while parsing : FOLDERNAME NOT SPECIFIED\n");
						args.EXIT_CODE = MZ_EXIT_FAILURE;
						return args;
					}
					if(push_charv(args.folders, argv[arg + 1]) == -1){
						fprintf(stderr, "Error while parsing : UNABLE TO PUSH FOLDER NAME\n");
						args.EXIT_CODE = MZ_EXIT_FAILURE;
						return args;
					}
					continue_loop = true;
					args.input.dir = true;
					continue;
				}else if(strcmp(input, "file") == 0){
					if(arg == argc - 1){
						fprintf(stderr, "Error while parsing : FILENAME NOT SPECIFIED\n");
						args.EXIT_CODE = MZ_EXIT_FAILURE;
						return args;
					}
					if(push_charv(args.files, argv[arg + 1]) == -1){
						fprintf(stderr, "Error while parsing : UNABLE TO PUSH FILE NAME\n");
						args.EXIT_CODE = MZ_EXIT_FAILURE;
						return args;
					}
					continue_loop = true;
					args.input.file = true;
					continue;
				}else{
					fprintf(stderr, "Error while parsing : INVALID -I options : %s\n", print);
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
			}else if(strcmp(opt, "a") == 0){
				if(args.mode != MODE_NONE){
					fprintf(stderr, "Error while parsing : MULTIPLE MODE OPTIONS\n");
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
				args.mode = MODE_ARCHIVE;
				continue;
			}else if(strcmp(opt, "x") == 0){
				if(args.mode != MODE_NONE){
					fprintf(stderr, "Error while parsing : MULTIPLE MODE OPTIONS\n");
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
				args.mode = MODE_EXTRACT;
				continue;
			}else if(strcmp(opt, "xs") == 0){ // Takes Filename after -xs command as the mz input .
				if(args.mode != MODE_NONE){
					fprintf(stderr, "Error while parsing : MULTIPLE MODE OPTIONS\n");
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
				if(arg == argc - 1){
					fprintf(stderr, "Error while parsing : MZ FILENAME NOT SPECIFIED\n");
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
				if (push_front_charv(args.files, argv[arg + 1]) != 0){ // pushing at first of the vector to ensure mz file is selected
					fprintf(stderr, "Error while parsing : UNABLE TO PUSH FILE TO HEAP\n");
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
				continue_loop = true;
				args.mode = MODE_EXTRACT_SPECIFIC;
				continue;
			}else if(strcmp(opt, "fi") == 0){
				if(args.mode != MODE_NONE){
					fprintf(stderr, "Error while parsing : MULTIPLE MODE OPTIONS\n");
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
				args.mode = MODE_FILES_IN;
				continue;
			}else if(strcmp(opt, "info") == 0){
				if(args.mode != MODE_NONE){
					fprintf(stderr, "Error while parsing : MULTIPLE MODE OPTIONS\n");
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
				args.mode = MODE_INFO;
				continue;
			}else if(strcmp(opt, "o") == 0){
				if(args.outfile != NULL){
					fprintf(stderr, "Error while parsing : MULTIPLE OUTPUTFILE SPECIFIED\n");
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
				if(arg == argc - 1){
					fprintf(stderr, "Error while parsing : OUTPUTFILENAME NOT SPECIFIED\n");
					args.EXIT_CODE = MZ_EXIT_FAILURE;
					return args;
				}
				args.outfile = argv[arg + 1];
				continue_loop = true;
				continue;
			}else{
				fprintf(stderr, "Error while parsing : INVALID OPTION SPECIFIED\n");
				args.EXIT_CODE = MZ_EXIT_FAILURE;
				return args;
			}
		}else{
			if(push_charv(args.files, argument) == -1){
				fprintf(stderr, "Error while parsing : UNABLE TO PUSH FILE TO HEAP\n");
				args.EXIT_CODE = MZ_EXIT_FAILURE;
				return args;
			}
		}
	}

	args.EXIT_CODE = MZ_EXIT_SUCCESS;
	return args;
}

#endif
#endif