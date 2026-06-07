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
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

// Essential imports and wrappers for windows and linux for cross compatibility
#ifdef _WIN32

    #include <direct.h>

    #define mz_mkdir(path) _mkdir(path)

    #define mz_fseek _fseeki64
    #define mz_ftell _ftelli64

#else

    #include <sys/types.h>
    #include <sys/stat.h>

    #define mz_mkdir(path) mkdir(path, 0755)

	#define mz_fseek fseeko
    #define mz_ftell ftello

#endif

// MZ definitions
#define MZ_HEADER "MZ"
#define MZ_VERSION "1.3.2"

// MZ buffer size
#define MZ_BUFFER 1048576

// MZ Exit Codes
#define MZ_SUCCESS 0
#define MZ_FAILURE 1
#define MZ_WARNING 2

// MZ structs and enums

// Enum for option
typedef enum{
	MODE_NONE,
	MODE_ARCHIVE,
	MODE_EXTRACT,
	MODE_FILESIN
} MZ_MODE;

// Enum for optional flag
typedef enum{
	FLAG_NONE,
	FLAG_HELP,
	FLAG_VERSION
} MZ_FLAG;

// Enum for compression type
typedef enum{
	COM_NONE
} MZ_COM;

// Struct for parsing arguments
typedef struct{
	MZ_MODE mode;
	MZ_FLAG flag;
	MZ_COM compression;
	char *files[4096];
	uint64_t file_count;
	char *outfile;
	int EXIT_CODE;
} MZ_ARGS;

// MZ Function Declarations
int mz_archive(char *files[], uint64_t file_count, char *outfile, int compression);
int mz_extract(char *mz_file);
int mz_filesin(char *mz_file);

// MZ function for getting filecontent size
int64_t mz_file_size(FILE *fp)
{
    mz_fseek(fp, 0, SEEK_END);
    int64_t size = (int64_t)mz_ftell(fp);
    mz_fseek(fp, 0, SEEK_SET);
    return size;
}

// MZ function for making directory of filepath
int mz_check_dir(char *filepath)
{
    int filepathsize = strlen(filepath);

    for(int i = 0; i < filepathsize; i++)
    {
        if(filepath[i] == '/' || filepath[i] == '\\')
        {
			char sep = filepath[i];
            filepath[i] = '\0';
            if(mz_mkdir(filepath) != 0 && errno != EEXIST){
				fprintf(stderr, "Error while creating Directories : Unable to create %s\n", filepath);
				filepath[i] = '/';
				return MZ_FAILURE;
			}
            filepath[i] = sep;
        }
    }

    return MZ_SUCCESS;
}

// MZ function for path sanitization
// Error message prefix :- Warning while sanitizing
int mz_sanitize_path(const char *filepath)
{
    if (strstr(filepath, "../") != NULL){
		fprintf(stderr, "Warning while sanitizing : ../ -> Bad Filepath used in %s\n", filepath);
        return MZ_WARNING;
	}

    if (strstr(filepath, "..\\") != NULL){
		fprintf(stderr, "Warning while sanitizing : ..\\ -> Bad Filepath used in %s\n", filepath);
        return MZ_WARNING;
	}

    if (strlen(filepath) >= 2 && filepath[1] == ':'){
		fprintf(stderr, "Warning while sanitizing : Absolute path - [%s] ,it can lead to bad extraction\n", filepath);
        return MZ_WARNING;
	}

    if (filepath[0] == '/'){
		fprintf(stderr, "Warning while sanitizing : Absolute path - [%s] ,it can lead to bad extraction\n", filepath);
        return MZ_WARNING;
	}

    if (strncmp(filepath, "\\\\", 2) == 0){
		fprintf(stderr, "Warning while sanitizing : \\\\ -> Bad Filepath used in %s\n", filepath);
        return MZ_WARNING;
	}

    return MZ_SUCCESS;
}

// MZ parsing arguments function
// Error message prefix :- Error while parsing
MZ_ARGS mz_parse_args(int argc, char *argv[])
{
	MZ_ARGS args = {0};
	args.mode = MODE_NONE;
	args.flag = FLAG_NONE;
	args.compression = COM_NONE;
	args.file_count = 0;
	args.outfile = NULL;
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
						fprintf(stderr, "Error while parsing : MULTIPLE OPTIONAL FLAGS");
						args.EXIT_CODE = MZ_FAILURE;
						return args;
					}
					if(args.mode != MODE_NONE){
						fprintf(stderr, "Error while parsing : MIXED FLAG AND OPTION");
						args.EXIT_CODE = MZ_FAILURE;
						return args;
					}
					args.flag = FLAG_HELP;
				}else if(strcmp(flag, "version") == 0 || strcmp(flag, "v") == 0){
					if(args.flag != FLAG_NONE){
						fprintf(stderr, "Error while parsing : MULTIPLE OPTIONAL FLAGS");
						args.EXIT_CODE = MZ_FAILURE;
						return args;
					}
					if(args.mode != MODE_NONE){
						fprintf(stderr, "Error while parsing : MIXED FLAG AND OPTION");
						args.EXIT_CODE = MZ_FAILURE;
						return args;
					}
					args.flag = FLAG_VERSION;
				}else{
					fprintf(stderr, "Error while parsing : INVALID OPTIONAL FLAG");
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
			}else if(strcmp(opt, "archive") == 0 || strcmp(opt, "a") == 0){
				if(args.mode != MODE_NONE){
					fprintf(stderr, "Error while parsing : MULTIPLE MODE OPTIONS");
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				if(args.flag != FLAG_NONE){
					fprintf(stderr, "Error while parsing : MIXED FLAG AND OPTION");
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				args.mode = MODE_ARCHIVE;
				continue;
			}else if(strcmp(opt, "extract") == 0 || strcmp(opt, "x") == 0){
				if(args.mode != MODE_NONE){
					fprintf(stderr, "Error while parsing : MULTIPLE MODE OPTIONS");
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				if(args.flag != FLAG_NONE){
					fprintf(stderr, "Error while parsing : MIXED FLAG AND OPTION");
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				args.mode = MODE_EXTRACT;
				continue;
			}else if(strcmp(opt, "filesin") == 0 || strcmp(opt, "fi") == 0){
				if(args.mode != MODE_NONE){
					fprintf(stderr, "Error while parsing : MULTIPLE MODE OPTIONS");
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				if(args.flag != FLAG_NONE){
					fprintf(stderr, "Error while parsing : MIXED FLAG AND OPTION");
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				args.mode = MODE_FILESIN;
				continue;
			}else if(strcmp(opt, "output") == 0 || strcmp(opt, "o") == 0){
				if(args.outfile != NULL){
					fprintf(stderr, "Error while parsing : MULTIPLE OUTPUTFILE SPECIFIED");
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				if(arg == argc - 1){
					fprintf(stderr, "Error while parsing : OUTPUTFILENAME NOT SPECIFIED");
					args.EXIT_CODE = MZ_FAILURE;
					return args;
				}
				outputfiledetected = true;
				continue;
			}else{
				fprintf(stderr, "Error while parsing : INVALID OPTION SPECIFIED");
				args.EXIT_CODE = MZ_FAILURE;
				return args;
			}
		}else{
			if(args.file_count >= 4096){
				fprintf(stderr, "Error while parsing : TOO MANY FILES");
				return args;
			}
			args.files[args.file_count] = argument;
			args.file_count++;
		}
	}
	args.EXIT_CODE = MZ_SUCCESS;
	return args;
}

// MZ main function
// Error message prefix :- Error
int main(int argc, char *argv[])
{
	if(argc < 2){
		printf("Use : mz --help for more info\n");
		return MZ_SUCCESS;
	}
	MZ_ARGS args = mz_parse_args(argc, argv);

	if(args.EXIT_CODE != MZ_SUCCESS){
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
		
		printf("  -fi, -filesin <archive.mz>\n");
		printf("      Prints number of files and filenames present in the MZ archive.\n\n");

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
	}else if(args.mode == MODE_FILESIN){

		if(args.file_count == 0){
			fprintf(stderr, "Error : No Files Passed\n");
			return MZ_FAILURE;
		}
		for(uint64_t i = 0 ; i < args.file_count; i++){
			int filesin = mz_filesin(args.files[i]);
			if(filesin != 0){
				fprintf(stderr, "Error : Unable to Find Files in %s\n", args.files[i]);
				return MZ_FAILURE;
			}
		}
		return MZ_SUCCESS;
	}
	return MZ_SUCCESS;
}

// MZ File Archiving Function
// Error message prefix :- Error while archiving
int mz_archive(char *files[], uint64_t file_count, char *outfile, int compression)
{
	// For Cleanup references
	FILE *in = NULL;
	FILE *out = NULL;
	char *buffer = NULL;
	
	out = fopen(outfile, "wb");
	if(!out){
		fprintf(stderr, "Error while archiving : Output File Not Opening\n");
		return MZ_FAILURE;
	}

	if(fwrite(MZ_HEADER, sizeof(char), 2, out) != 2){
		fprintf(stderr, "Error while archiving : Unable to write header\n");
		goto cleanErr;
	}

	if(fwrite(&compression, sizeof(int), 1, out) != 1){
		fprintf(stderr, "Error while archiving : Unable to write compression details\n");
		goto cleanErr;
	}

	if(fwrite(&file_count, sizeof(uint64_t), 1, out) != 1){
		fprintf(stderr, "Error while archiving : Unable to write file count\n");
		goto cleanErr;
	}

	buffer = malloc(MZ_BUFFER);
	if(!buffer){
		fprintf(stderr, "Error while archiving : Unable to allocate buffer size\n");
		goto cleanErr;
	}

	for(size_t file = 0; file < file_count; file++){

		char *filename = files[file];
		
		if(mz_sanitize_path(filename) == MZ_WARNING){
			fprintf(stderr, "Error while archiving : Sanitization warning\n");
			goto cleanErr;
		}
		
		in = fopen(filename, "rb");
		if(!in){
			fprintf(stderr, "Error while archiving : Unable to read '%s'\n", filename);
			goto cleanErr;
		}
		uint64_t filenamelength = strlen(filename);

		if(fwrite(&filenamelength, sizeof(uint64_t), 1, out) != 1){
			fprintf(stderr, "Error while archiving : Unable to write file name length\n");
			goto cleanErr;
		}

		if(fwrite(filename, sizeof(char), filenamelength, out) != filenamelength){
			fprintf(stderr, "Error while archiving : Unable to write file name\n");
			goto cleanErr;
		}

		int64_t filecontentsize = mz_file_size(in);

		if(fwrite(&filecontentsize, sizeof(int64_t), 1, out) != 1){
			fprintf(stderr, "Error while archiving : Unable to write file content size\n");
			goto cleanErr;
		}

		int64_t remaining = filecontentsize;
		while(remaining != 0){
			int64_t chunk = MZ_BUFFER > remaining ? remaining : MZ_BUFFER;
			if(fread(buffer, chunk, 1, in) != 1){
				fprintf(stderr, "Error while archiving : Unable to read file content\n");
				goto cleanErr;
			}
			if(fwrite(buffer, chunk, 1, out) != 1){
				fprintf(stderr, "Error while archiving : Unable to write file content\n");
				goto cleanErr;
			}
			remaining -= chunk;
		}
		fclose(in);
		in = NULL;
	}
	fclose(out);
	free(buffer);
	return MZ_SUCCESS;
	
cleanErr:
    if(out){
        fclose(out);
        remove(outfile);
    }
	if(in != NULL) fclose(in);
	if(buffer != NULL) free(buffer);
	return MZ_FAILURE;
}

// MZ File Extracting Function
// Error message prefix :- Error while extracting
int mz_extract(char *mz_file)
{
	// For Cleanup references
	FILE *out = NULL;
	char *buffer = NULL;
	char *filename = NULL;
	FILE *in = NULL;
	
	in = fopen(mz_file, "rb");
	if(!in){
		fprintf(stderr, "Error while extracting : Unable to Open Archive File\n");
		return MZ_FAILURE;
	}

	char check_header[2];
	int check_compression;
	uint64_t file_count;

	if(fread(check_header, sizeof(char), 2, in) != 2){
		fprintf(stderr, "Error while extracting : Invalid Header\n");
		goto cleanErr;
	}

	if(memcmp(check_header, MZ_HEADER, 2) != 0){
		fprintf(stderr, "Error while extracting : Invalid Header\n");
		goto cleanErr;
	}

	if(fread(&check_compression, sizeof(int), 1, in) != 1){
		fprintf(stderr, "Error while extracting : Invalid Compression\n");
		goto cleanErr;
	}

	if(check_compression != 0){
		fprintf(stderr, "Error while extracting : Invalid Compression\n");
		goto cleanErr;
	}

	if(fread(&file_count, sizeof(uint64_t), 1, in) != 1){
		goto cleanErr;
	}

	buffer = malloc(MZ_BUFFER);
	if(!buffer){
		fprintf(stderr, "Error while extracting : Unable to allocate buffer size\n");
		goto cleanErr;
	}

	for(uint64_t file = 0; file < file_count; file++){
		uint64_t filenamelength;
		if(fread(&filenamelength, sizeof(uint64_t), 1, in) != 1){
			fprintf(stderr, "Error while extracting : Unable to read filenamelength\n");
			goto cleanErr;
		}
		
		if(filenamelength > 4096){
			fprintf(stderr, "Error while extracting : Invalid filename length\n");
			goto cleanErr;
		}

		filename = malloc(filenamelength + 1);
		if(!filename){
			fprintf(stderr, "Error while extracting : Unable to read filename\n");
			goto cleanErr;
		}
		if(fread(filename, sizeof(char), filenamelength, in) != (size_t)filenamelength){
			fprintf(stderr, "Error while extracting : Unable to read filename\n");
			goto cleanErr;
		}
		filename[filenamelength] = '\0';

		int64_t filecontentsize;
		if(fread(&filecontentsize, sizeof(int64_t), 1, in) != 1){
			fprintf(stderr, "Error while extracting : Unable to read filecontentsize\n");
			goto cleanErr;
		}
		
		if(filecontentsize < 0){
			fprintf(stderr, "Error while extracting : Invalid file size\n");
			goto cleanErr;
		}

		if(mz_sanitize_path(filename) == MZ_WARNING){
			fprintf(stderr, "Warning while extracting : Skipping %s\n", filename);
			mz_fseek(in, filecontentsize, SEEK_CUR);
			free(filename);
			filename = NULL;
			continue;
		}

		if(mz_check_dir(filename) != MZ_SUCCESS){
			goto cleanErr;
		}

		out = fopen(filename, "wb");
		if(!out){
			fprintf(stderr, "Error while extracting : Unable to Open file\n");
			goto cleanErr;
		}

		int64_t remaining = filecontentsize;
		while(remaining != 0){
			int64_t chunk = MZ_BUFFER > remaining ? remaining : MZ_BUFFER;
			if(fread(buffer, chunk, 1, in) != 1){
				fprintf(stderr, "Error while extracting : Unable to read file content\n");
				goto cleanErr;
			}
			if(fwrite(buffer, chunk, 1, out) != 1){
				fprintf(stderr, "Error while extracting : Unable to write file content\n");
				goto cleanErr;
			}
			remaining -= chunk;
		}
		free(filename);
		filename = NULL;
		fclose(out);
		out = NULL;
	}
	fclose(in);
	free(buffer);
	return MZ_SUCCESS;
	
cleanErr:
    if(out) fclose(out);
	if(in != NULL) fclose(in);
	if(buffer != NULL) free(buffer);
	if(filename != NULL) free(filename);
	return MZ_FAILURE;
}

// MZ function for printing number of files and their names stored in an archive file 
// Error message prefix :- Error while checking files
int mz_filesin(char *mz_file)
{
	// For Cleanup references
	FILE *in = NULL;
	char *filename = NULL;
	
	in = fopen(mz_file, "rb");
	if(!in){
		fprintf(stderr, "Error while checking files : Unable to Open Archive File\n");
		return MZ_FAILURE;
	}

	char check_header[2];
	int check_compression;
	uint64_t file_count;

	if(fread(check_header, sizeof(char), 2, in) != 2){
		fprintf(stderr, "Error while checking files : Invalid Header\n");
		goto cleanErr;
	}

	if(memcmp(check_header, MZ_HEADER, 2) != 0){
		fprintf(stderr, "Error while checking files : Invalid Header\n");
		goto cleanErr;
	}

	if(fread(&check_compression, sizeof(int), 1, in) != 1){
		fprintf(stderr, "Error while checking files : Invalid Compression\n");
		goto cleanErr;
	}

	if(check_compression != 0){
		fprintf(stderr, "Error while checking files : Invalid Compression\n");
		goto cleanErr;
	}

	if(fread(&file_count, sizeof(uint64_t), 1, in) != 1){
		fprintf(stderr, "Error while checking files : Invalid File Count\n");
		goto cleanErr;
	}
	printf("Files in %s : %" PRIu64 "\n", mz_file, file_count);

	for(uint64_t file = 0; file < file_count; file++){
		uint64_t filenamelength;
		if(fread(&filenamelength, sizeof(uint64_t), 1, in) != 1){
			fprintf(stderr, "Error while checking files : Unable to read filenamelength\n");
			goto cleanErr;
		}

		filename = malloc(filenamelength + 1);
		if(!filename){
			fprintf(stderr, "Error while checking files : Unable to read filename\n");
			goto cleanErr;
		}
		if(fread(filename, sizeof(char), filenamelength, in) != (size_t)filenamelength){
			fprintf(stderr, "Error while checking files : Unable to read filename\n");
			goto cleanErr;
		}
		filename[filenamelength] = '\0';

		int64_t filecontentsize;
		if(fread(&filecontentsize, sizeof(int64_t), 1, in) != 1){
			fprintf(stderr, "Error while checking files : Unable to read filecontentsize\n");
			goto cleanErr;
		}
		
		mz_fseek(in, filecontentsize, SEEK_CUR);
		printf("%s\n", filename);
		free(filename);
		filename = NULL;
	}
	fclose(in);
	return MZ_SUCCESS;
	
cleanErr:
	if(in != NULL) fclose(in);
	if(filename != NULL) free(filename);
	return MZ_FAILURE;
}
