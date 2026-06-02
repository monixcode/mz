#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>

#define MZ_HEADER "MZ"
#define MZ_VERSION "1.0.0"

#define MZ_BUFFER 1048576

#define MZ_SUCCESS 0
#define MZ_FAILURE 1

int mz_archive(char *files[], size_t file_count, char *outfile, int compression);
int mz_extract(char *mz_file);
int mz_check_dir(char *filepath);

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

typedef enum{
	COM_NONE
} MZ_COM;

typedef struct{
	MZ_MODE mode;
	MZ_FLAG flag;
	MZ_COM compression;
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
	args.compression = COM_NONE;
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
	
	if(args.flag == FLAG_HELP){
		printf("\n                  MZ ARCHIVER \n\n");
		printf("A File Archiver Tool to bundle many files in a single file \n\n");
		printf("Options:-\n\n");
		printf("     For Archiving :- mz -archive/-a file.txt .... -output/-o outputfile.mz\n\n");
		printf("     For Extracting :- mz -extract/-x file.mz ....\n\n");
		printf("Optional Flags:\n\n");
		printf("     mz --help/--h\n\n");
		printf("     mz --version/--v\n\n");
		return MZ_SUCCESS;
		
	}else if(args.flag == FLAG_VERSION){
		printf("%s - %s\n",MZ_HEADER, MZ_VERSION);
		return MZ_SUCCESS;
		
	}else if(args.mode == MODE_ARCHIVE){
		
		if(args.file_count == 0){
			fprintf(stderr, "Error : No Files Passed\n");
			return MZ_FAILURE;
		}
		
		if(args.outfile == NULL){
			fprintf(stderr, "Error : No Output File Passed\n");
			return MZ_FAILURE;
		}
		int archive = mz_archive(args.files, args.file_count, args.outfile, args.compression);
		if(archive != 0){
			fprintf(stderr, "Error : Unable to Archive\n");
			return MZ_FAILURE;
		}
		return MZ_SUCCESS;
	}else if(args.mode == MODE_EXTRACT){
		
		if(args.file_count == 0){
			fprintf(stderr, "Error : No Files Passed\n");
			return MZ_FAILURE;
		}
		for(int i = 0 ; i < args.file_count; i++){
			int extract = mz_extract(args.files[i]);
			if(extract != 0){
				fprintf(stderr, "Error : Unable to Extract %s\n", args.files[i]);
				return MZ_FAILURE;
			}
		}
		return MZ_SUCCESS;
	}
	return MZ_SUCCESS;
}

int mz_archive(char *files[], size_t file_count, char *outfile, int compression)
{
	FILE *out = fopen(outfile, "wb");
	if(!out){
		fprintf(stderr, "Error : Output File Not Opening\n");
		return MZ_FAILURE;
	}
	
	if(fwrite(MZ_HEADER, sizeof(char), 2, out) != 2){
		fprintf(stderr, "Error : Unable to write header\n");
		fclose(out);
		return MZ_FAILURE;
	}
	
	if(fwrite(&compression, sizeof(int), 1, out) != 1){
		fprintf(stderr, "Error : Unable to write compression details\n");
		fclose(out);
		return MZ_FAILURE;
	}
	
	if(fwrite(&file_count, sizeof(size_t), 1, out) != 1){
		fprintf(stderr, "Error : Unable to write file count\n");
		fclose(out);
		return MZ_FAILURE;
	}
	
	char *buffer = malloc(MZ_BUFFER);
	if(!buffer){
		fprintf(stderr, "Error : Unable to allocate buffer size\n");
		fclose(out);
		return MZ_FAILURE;
	}
	
	for(size_t file = 0; file < file_count; file++){
		
		char *filename = files[file];
		int filenamelength = strlen(filename);
		
		if(fwrite(&filenamelength, sizeof(int), 1, out) != 1){
			fprintf(stderr, "Error : Unable to write file name length\n");
			fclose(out);
			free(buffer);
			return MZ_FAILURE;
		}
		
		if(fwrite(filename, sizeof(char), filenamelength, out) != filenamelength){
			fprintf(stderr, "Error : Unable to write file name\n");
			fclose(out);
			free(buffer);
			return MZ_FAILURE;
		}
		
		FILE *in = fopen(filename, "rb");
		if(!in){
			fprintf(stderr, "Error : Unable to read input file\n");
			fclose(out);
			free(buffer);
			return MZ_FAILURE;
		}
		
		fseek(in, 0, SEEK_END);
		long filecontentsize = ftell(in);
		fseek(in, 0, SEEK_SET);
		
		if(fwrite(&filecontentsize, sizeof(long), 1, out) != 1){
			fprintf(stderr, "Error : Unable to write file content size\n");
			fclose(out);
			free(buffer);
			fclose(in);
			return MZ_FAILURE;
		}
		
		long remaining = filecontentsize;
		while(remaining != 0){
			long chunk = MZ_BUFFER > remaining ? remaining : MZ_BUFFER;
			if(fread(buffer, chunk, 1, in) != 1){
				fprintf(stderr, "Error : Unable to read file content\n");
				fclose(out);
				free(buffer);
				fclose(in);
				return MZ_FAILURE;
			}
			if(fwrite(buffer, chunk, 1, out) != 1){
				fprintf(stderr, "Error : Unable to write file content\n");
				fclose(out);
				free(buffer);
				fclose(in);
				return MZ_FAILURE;
			}
			remaining -= chunk;
		}
		fclose(in);
	}
	fclose(out);
	free(buffer);
	return MZ_SUCCESS;
}

int mz_extract(char *mz_file)
{
	FILE *in = fopen(mz_file, "rb");
	if(!in){
		fprintf(stderr, "Error : Unable to Open Archive File\n");
		return MZ_FAILURE;
	}
	
	char check_header[2];
	int check_compression;
	size_t file_count;
	
	if(fread(check_header, sizeof(char), 2, in) != 2){
		fprintf(stderr, "Error : Invalid Header\n");
		fclose(in);		
		return MZ_FAILURE;
	}
	
	if(strcmp(check_header, MZ_HEADER) != 0){
		fprintf(stderr, "Error : Invalid Header\n");
		fclose(in);
		return MZ_FAILURE;
	}
	
	if(fread(&check_compression, sizeof(int), 1, in) != 1){
		fprintf(stderr, "Error : Invalid Compression\n");
		fclose(in);		
		return MZ_FAILURE;
	}
	
	if(check_compression != 0){
		fprintf(stderr, "Error : Invalid Compression\n");
		fclose(in);
		return MZ_FAILURE;
	}
	
	if(fread(&file_count, sizeof(size_t), 1, in) != 1){
		fprintf(stderr, "Error : Invalid File Count\n");
		fclose(in);		
		return MZ_FAILURE;
	}
	
	char *buffer = malloc(MZ_BUFFER);
	if(!buffer){
		fprintf(stderr, "Error : Unable to allocate buffer size\n");
		fclose(in);
		return MZ_FAILURE;
	}
	
	for(int file = 0; file < file_count; file++){
		int filenamelength;
		if(fread(&filenamelength, sizeof(int), 1, in) != 1){
			fprintf(stderr, "Error : Unable to read filenamelength\n");
			fclose(in);	
			free(buffer);			
			return MZ_FAILURE;
		}
		
		char filename[filenamelength + 1];
		if(fread(filename, sizeof(char), filenamelength, in) != filenamelength){
			fprintf(stderr, "Error : Unable to read filename\n");
			fclose(in);	
			free(buffer);			
			return MZ_FAILURE;
		}
		filename[filenamelength] = '\0';
		
		int filecontentsize;
		if(fread(&filecontentsize, sizeof(int), 1, in) != 1){
			fprintf(stderr, "Error : Unable to read filecontentsize\n");
			fclose(in);		
			free(buffer);
			return MZ_FAILURE;
		}
		
		mz_check_dir(filename);
		
		FILE *out = fopen(filename, "wb");
		if(!out){
			fprintf(stderr, "Error : Unable to Open file\n");
			fclose(in);	
			free(buffer);
			return MZ_FAILURE;
		}
		
		long remaining = filecontentsize;
		while(remaining != 0){
			long chunk = MZ_BUFFER > remaining ? remaining : MZ_BUFFER;
			if(fread(buffer, chunk, 1, in) != 1){
				fprintf(stderr, "Error : Unable to read file content\n");
				fclose(out);
				free(buffer);
				fclose(in);
				return MZ_FAILURE;
			}
			if(fwrite(buffer, chunk, 1, out) != 1){
				fprintf(stderr, "Error : Unable to write file content\n");
				fclose(out);
				free(buffer);
				fclose(in);
				return MZ_FAILURE;
			}
			remaining -= chunk;
		}
		
		fclose(out);
	}
		
	return MZ_SUCCESS;
}

int mz_check_dir(char filepath[])
{
	int filepathsize = strlen(filepath);
	for(int i = 0; i < filepathsize; i++){
		if(filepath[i] == '/'){
			filepath[i] = '\0';
			mkdir(filepath);
			filepath[i] = '/';
		}
	}
	return 0;
}
