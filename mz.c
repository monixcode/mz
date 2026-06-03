/*
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

// Essential imports and wrappers for windows
#ifdef _WIN32
    #include <direct.h>
    #define mz_mkdir(path) _mkdir(path)

    int64_t mz_file_size(FILE *fp)
    {
        _fseeki64(fp, 0, SEEK_END);
        int64_t size = _ftelli64(fp);
        _fseeki64(fp, 0, SEEK_SET);
        return size;
    }
// Essential imports and wrappers for linux
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #define mz_mkdir(path) mkdir(path, 0755)

    int64_t mz_file_size(FILE *fp)
    {
        fseeko(fp, 0, SEEK_END);
        int64_t size = (int64_t)ftello(fp);
        fseeko(fp, 0, SEEK_SET);
        return size;
    }

#endif

// MZ definitions
#define MZ_HEADER "MZ"
#define MZ_VERSION "1.1.1"

// MZ buffer size
#define MZ_BUFFER 1048576

// MZ Exit Codes
#define MZ_SUCCESS 0
#define MZ_FAILURE 1

// MZ Function Declarations
int mz_archive(char *files[], uint64_t file_count, char *outfile, int compression);
int mz_extract(char *mz_file);
int mz_check_dir(char *filepath);
int mz_sanitize_path(const char *filepath);

// MZ structs and enums
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
	uint64_t file_count;
	char *outfile;
	char *ERROR_MESSAGE;
	int EXIT_CODE;
} MZ_ARGS;

// MZ parsing arguments function
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
		// option parsing
		if(argument[0] == '-'){
			char *opt = argument + 1;
			char *flag = argument + 2;
			
			// optional flag parsing
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
			if(args.file_count >= 4096){
				args.ERROR_MESSAGE = "TOO MANY FILES";
				return args;
			}
			args.files[args.file_count] = argument;
			args.file_count++;
		}
	}
	args.ERROR_MESSAGE = NULL;
	args.EXIT_CODE = MZ_SUCCESS;
	return args;
}

// MZ main function
int main(int argc, char *argv[])
{
	if(argc < 2){
		printf("Use : mz --help for more info\n");
		return MZ_SUCCESS;
	}
	MZ_ARGS args = mz_parse_args(argc, argv);
	
	if(args.EXIT_CODE != MZ_SUCCESS){
		fprintf(stderr, "Error: %s\n", args.ERROR_MESSAGE);
		return MZ_FAILURE;
	}
	
	if(args.flag == FLAG_HELP){
		printf("MZ - File Archiver\n");
		printf("Bundle multiple files into a single archive.\n\n");

		printf("USAGE:\n");
		printf("  mz [OPTION] [FILES] ...\n\n");

		printf("OPTIONS:\n");
		printf("  -a, -archive <files> -o <archive.mz>\n");
		printf("      Archive one or more files into an MZ archive.\n\n");

		printf("  -x, -extract <archive.mz>\n");
		printf("      Extract files from an MZ archive.\n\n");

		printf("OPTIONALS:\n");
		printf("  -o, -output <file>\n");
		printf("      Specify the output archive file.\n\n");

		printf("  --help, --h\n");
		printf("      Display this help message.\n\n");

		printf("  --version, --v\n");
		printf("      Display version information.\n\n");

		printf("EXAMPLES:\n");
		printf("  mz -a file1.txt file2.txt -o archive.mz\n");
		printf("  mz -archive image.png doc.pdf -output backup.mz\n");
		printf("  mz -x archive.mz\n");
		printf("  mz -extract backup.mz\n");
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
		for(uint64_t i = 0 ; i < args.file_count; i++){
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

// MZ File Archiving Function
int mz_archive(char *files[], uint64_t file_count, char *outfile, int compression)
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
	
	if(fwrite(&file_count, sizeof(uint64_t), 1, out) != 1){
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
		uint64_t filenamelength = strlen(filename);
		
		if(fwrite(&filenamelength, sizeof(uint64_t), 1, out) != 1){
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
		
		if(mz_sanitize_path(filename) == MZ_FAILURE){
			fprintf(stderr, "Error : Dangerous filepath\n");	
			free(buffer);
			fclose(out);
			remove(outfile);
			return MZ_FAILURE;
		}
		
		FILE *in = fopen(filename, "rb");
		if(!in){
			fprintf(stderr, "Error : Unable to read input file\n");
			fclose(out);
			free(buffer);
			return MZ_FAILURE;
		}
		
		int64_t filecontentsize = mz_file_size(in);
		
		if(fwrite(&filecontentsize, sizeof(int64_t), 1, out) != 1){
			fprintf(stderr, "Error : Unable to write file content size\n");
			fclose(out);
			free(buffer);
			fclose(in);
			return MZ_FAILURE;
		}
		
		int64_t remaining = filecontentsize;
		while(remaining != 0){
			int64_t chunk = MZ_BUFFER > remaining ? remaining : MZ_BUFFER;
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

// MZ File Extracting Function
int mz_extract(char *mz_file)
{
	FILE *in = fopen(mz_file, "rb");
	if(!in){
		fprintf(stderr, "Error : Unable to Open Archive File\n");
		return MZ_FAILURE;
	}
	
	char check_header[2];
	int check_compression;
	uint64_t file_count;
	
	if(fread(check_header, sizeof(char), 2, in) != 2){
		fprintf(stderr, "Error : Invalid Header\n");
		fclose(in);		
		return MZ_FAILURE;
	}
	
	if(memcmp(check_header, MZ_HEADER, 2) != 0){
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
	
	if(fread(&file_count, sizeof(uint64_t), 1, in) != 1){
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
	
	for(uint64_t file = 0; file < file_count; file++){
		uint64_t filenamelength;
		if(fread(&filenamelength, sizeof(uint64_t), 1, in) != 1){
			fprintf(stderr, "Error : Unable to read filenamelength\n");
			fclose(in);	
			free(buffer);			
			return MZ_FAILURE;
		}
		
		char *filename = malloc(filenamelength + 1);
		if(!filename){
			fprintf(stderr, "Error : Unable to read filename\n");
			fclose(in);	
			free(buffer);			
			return MZ_FAILURE;
		}
		if(fread(filename, sizeof(char), filenamelength, in) != (size_t)filenamelength){
			fprintf(stderr, "Error : Unable to read filename\n");
			fclose(in);	
			free(buffer);	
			free(filename);
			return MZ_FAILURE;
		}
		filename[filenamelength] = '\0';
		
		int64_t filecontentsize;
		if(fread(&filecontentsize, sizeof(int64_t), 1, in) != 1){
			fprintf(stderr, "Error : Unable to read filecontentsize\n");
			fclose(in);		
			free(buffer);
			free(filename);
			return MZ_FAILURE;
		}
		
		if(mz_sanitize_path(filename) == MZ_FAILURE){
			fprintf(stderr, "Error : Dangerous filepath\n");
			fclose(in);		
			free(buffer);
			free(filename);
			return MZ_FAILURE;
		}
		
		mz_check_dir(filename);
		
		FILE *out = fopen(filename, "wb");
		if(!out){
			fprintf(stderr, "Error : Unable to Open file\n");
			fclose(in);	
			free(buffer);
			free(filename);
			return MZ_FAILURE;
		}
		
		int64_t remaining = filecontentsize;
		while(remaining != 0){
			int64_t chunk = MZ_BUFFER > remaining ? remaining : MZ_BUFFER;
			if(fread(buffer, chunk, 1, in) != 1){
				fprintf(stderr, "Error : Unable to read file content\n");
				fclose(out);
				free(buffer);
				fclose(in);
				free(filename);
				return MZ_FAILURE;
			}
			if(fwrite(buffer, chunk, 1, out) != 1){
				fprintf(stderr, "Error : Unable to write file content\n");
				fclose(out);
				free(buffer);
				fclose(in);
				free(filename);
				return MZ_FAILURE;
			}
			remaining -= chunk;
		}
		free(filename);
		fclose(out);
	}
	fclose(in);
	free(buffer);
	return MZ_SUCCESS;
}

// MZ function for making directory of filepath
int mz_check_dir(char *filepath)
{
    int filepathsize = strlen(filepath);

    for(int i = 0; i < filepathsize; i++)
    {
        if(filepath[i] == '/' || filepath[i] == '\\')
        {
            filepath[i] = '\0';
            mz_mkdir(filepath);
            filepath[i] = '/';
        }
    }

    return 0;
}

// MZ function for path sanitization
int mz_sanitize_path(const char *filepath)
{
    if (strstr(filepath, "../") != NULL)
        return MZ_FAILURE;

    if (strstr(filepath, "..\\") != NULL)
        return MZ_FAILURE;

    if (strlen(filepath) >= 2 && filepath[1] == ':')
        return MZ_FAILURE;

    if (filepath[0] == '/')
        return MZ_FAILURE;

    if (strncmp(filepath, "\\\\", 2) == 0)
        return MZ_FAILURE;

    return MZ_SUCCESS;
}
