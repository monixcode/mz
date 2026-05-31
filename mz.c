#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MZ_HEADER MZ

#define MZ_SUCCESS 0
#define MZ_FAILURE 1

typedef enum{
	MODE_NONE,
	MODE_ARCHIVE,
	MODE_EXTRACT
} MZ_MODE;

typedef enum{
	FLAG_NONE,
	FLAG_HELP,
	FLAG_VERSION
} MZ_FLAG;

typedef struct{
	MZ_MODE mode;
	MZ_FLAG flag;
	char *files[4096];
	size_t file_count;
	char *outfile;
	char *ERROR_MESSAGE;
	int EXIT_CODE;
} MZ_ARGS;

MZ_ARGS mz_parse_args(int argc, char *argv[])
{
	MZ_ARGS args = {0};
	args.mode = MODE_NONE;
	args.flag = FLAG_NONE;
	args.file_count = 0;
	args.outfile = NULL;
	args.ERROR_MESSAGE = "PARSING INCOMPLETE";
	args.EXIT_CODE = MZ_FAILURE;
	
	bool outputfiledetected = false;
	
	for(int arg = 1; arg < argc; arg++){
		char *argument = argv[arg];
		if(outputfiledetected){
			args.outfile = argument;
			outputfiledetected = false;
			continue;
		}
		if(argument[0] == '-'){
			char *opt = argument + 1;
			char *flag = argument + 2;
			if(argument[1] == '-'){
				if(strcmp(flag, "help") == 0 || strcmp(flag, "h") == 0){
					if(args.flag != FLAG_NONE){
						args.ERROR_MESSAGE = "MULTIPLE OPTIONAL FLAGS";
						args.EXIT_CODE = MZ_FAILURE;
						return args;
					}
					if(args.mode != MODE_NONE){
						args.ERROR_MESSAGE = "MIXED FLAG AND OPTION";
						args.EXIT_CODE = MZ_FAILURE;
						return args;
					}
					args.flag = FLAG_HELP;
				}else if(strcmp(flag, "version") == 0 || strcmp(flag, "v") == 0){
					if(args.flag != FLAG_NONE){
						args.ERROR_MESSAGE = "MULTIPLE OPTIONAL FLAGS";
						args.EXIT_CODE = MZ_FAILURE;
						return args;
					}
					if(args.mode != MODE_NONE){
						args.ERROR_MESSAGE = "MIXED FLAG AND OPTION";
						args.EXIT_CODE = MZ_FAILURE;
						return args;
					}
					args.flag = FLAG_VERSION;
				}else{
					args.ERROR_MESSAGE = "INVALID OPTIONAL FLAG";
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
			}else if(strcmp(opt, "archive") == 0 || strcmp(opt, "a") == 0){
				if(args.mode != MODE_NONE){
					args.ERROR_MESSAGE = "MULTIPLE MODE OPTIONS";
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				if(args.flag != FLAG_NONE){
					args.ERROR_MESSAGE = "MIXED FLAG AND OPTION";
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				args.mode = MODE_ARCHIVE;
				continue;
			}else if(strcmp(opt, "extract") == 0 || strcmp(opt, "x") == 0){
				if(args.mode != MODE_NONE){
					args.ERROR_MESSAGE = "MULTIPLE MODE OPTIONS";
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				if(args.flag != FLAG_NONE){
					args.ERROR_MESSAGE = "MIXED FLAG AND OPTION";
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				args.mode = MODE_EXTRACT;
				continue;
			}else if(strcmp(opt, "output") == 0 || strcmp(opt, "o") == 0){
				if(args.outfile != NULL){
					args.ERROR_MESSAGE = "MULTIPLE OUTPUTFILE SPECIFIED";
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				if(arg == argc - 1){
					args.ERROR_MESSAGE = "OUTPUTFILENAME NOT SPECIFIED";
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				outputfiledetected = true;
				continue;
			}else{
				args.ERROR_MESSAGE = "INVALID OPTION SPECIFIED";
				args.EXIT_CODE = MZ_FAILURE;
				return args;
			}
		}else{
			args.files[args.file_count] = argument;
			args.file_count++;
		}
	}
	args.ERROR_MESSAGE = NULL;
	args.EXIT_CODE = MZ_SUCCESS;
	return args;
}

int main(int argc, char *argv[])
{
	MZ_ARGS args = mz_parse_args(argc, argv);
	if(args.EXIT_CODE != MZ_SUCCESS){
		fprintf(stderr, "Error: %s\n", args.ERROR_MESSAGE);
		return MZ_FAILURE;
	}
	printf("MODE :- %d\n", args.mode);
	printf("FLAG :- %d\n", args.flag);
	printf("FILE COUNT : %d\n", args.file_count);
	for(int i = 0; i < args.file_count; i++){
		printf("FILE %d :- %s\n", i + 1 , args.files[i]);
	}
	printf("OUTPUT FILE : %s\n", args.outfile);
	printf("ERROR MESSAGE : %s\n", args.ERROR_MESSAGE);
	printf("EXIT CODE : %d\n", args.EXIT_CODE);
	return MZ_SUCCESS;
}