/*
	File :- init.h [Contains all helper functions and definitions for initialization]
	
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

#ifndef MZ_INIT_H
#define MZ_INIT_H

#define MZ_LIB_VECTOR_IMPLEMENTATION
#include "../lib/vector/vector.h"
#include <errno.h>
#include <stdbool.h>

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
#define MZ_VERSION "v0.1.0"

// MZ buffer size
#define MZ_BUFFER 1048576

// MZ Exit Codes
#define MZ_EXIT_SUCCESS 0
#define MZ_EXIT_FAILURE 1

// Enum for option
typedef enum{
	MODE_NONE,
	MODE_ARCHIVE,
	MODE_EXTRACT,
	MODE_EXTRACT_SPECIFIC,
	MODE_FILES_IN,
	MODE_INFO
} MZ_ARGS_MODE;

// Struct for optional flag
typedef struct{
	bool help;
	bool version;
	bool verbose;
	bool error;
} MZ_ARGS_FLAG;

// Struct for storing -I options
typedef struct{
	bool dir;
	bool file;
} MZ_ARGS_INPUT;

// Struct for parsing arguments
typedef struct{
	MZ_ARGS_MODE mode;
	MZ_ARGS_FLAG flag;
	MZ_ARGS_INPUT input;
	charv *files;
	charv *folders;
	char *outfile;
	int EXIT_CODE;
} MZ_ARGS;

// MZ function for getting filecontent size
uint64_t mz_helper_file_size(FILE *fp)
{
	long long file_pos = mz_ftell(fp);
    mz_fseek(fp, 0, SEEK_END);
    uint64_t size = (uint64_t)mz_ftell(fp);
    mz_fseek(fp, file_pos, SEEK_SET);
    return size;
}

// MZ function for making directory of filepath
int mz_helper_mkdir_filepath(char *filepath, bool error)
{
    int filepathsize = strlen(filepath);

    for(int i = 0; i < filepathsize; i++)
    {
        if(filepath[i] == '/' || filepath[i] == '\\')
        {
			char sep = filepath[i];
            filepath[i] = '\0';
            if(mz_mkdir(filepath) != 0 && errno != EEXIST){
				if (error) fprintf(stderr, "Error while creating Directories : Unable to create %s\n", filepath);
				filepath[i] = sep;
				return MZ_EXIT_FAILURE;
			}
            filepath[i] = sep;
        }
    }

    return MZ_EXIT_SUCCESS;
}

// MZ function for path sanitization
// Error message prefix :- Error while sanitizing
int mz_helper_sanitize_path(const char *filepath)
{
    if (strstr(filepath, "../") != NULL){
		fprintf(stderr, "Error while sanitizing : ../ -> Bad Filepath used in %s\n", filepath);
        return MZ_EXIT_FAILURE;
	}

    if (strstr(filepath, "..\\") != NULL){
		fprintf(stderr, "Error while sanitizing : ..\\ -> Bad Filepath used in %s\n", filepath);
        return MZ_EXIT_FAILURE;
	}

    if (strlen(filepath) >= 2 && filepath[1] == ':'){
		fprintf(stderr, "Error while sanitizing : Absolute path - [%s] ,it can lead to bad extraction\n", filepath);
        return MZ_EXIT_FAILURE;
	}

    if (filepath[0] == '/'){
		fprintf(stderr, "Error while sanitizing : Absolute path - [%s] ,it can lead to bad extraction\n", filepath);
        return MZ_EXIT_FAILURE;
	}

    if (strncmp(filepath, "\\\\", 2) == 0){
		fprintf(stderr, "Error while sanitizing : \\\\ -> Bad Filepath used in %s\n", filepath);
        return MZ_EXIT_FAILURE;
	}

    return MZ_EXIT_SUCCESS;
}

#endif