/*
	File :- filewalker.h [stb style file walking library]
	
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

#ifndef FILE_WALKER_H
#define FILE_WALKER_H

#define VECTOR_IMPLEMENTATION
#include "vector.h"

charv *filewalk(const char *path);

#ifdef FILE_WALKER_IMPLEMENTATION // Ensuring STB

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

charv *filewalk(const char *path)
{
    DIR *d = opendir(path);
    if (!d){
		fprintf(stderr, "Error from filewalk : Unable to Open Directory\n");
		return NULL;
	}

    struct dirent *entry;
    charv *files = init_charv();
	
	// Reading a folder and capturing all the files in it including files in the sub folders
    while ((entry = readdir(d)) != NULL)
    {
        if (entry->d_name[0] == '.') {
            if (entry->d_name[1] == '\0' ||
               (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))
                continue;
        }
		
		// Constructing fullpath for STAT function
        char fullpath[1024];
        if(snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name) < 0){
			destroy_charv(files);
			fprintf(stderr, "Error from filewalk : Unable to make fullpath\n");
			return NULL;
		}

        struct stat st;
        if (stat(fullpath, &st) == -1){
			fprintf(stderr, "Warning from filewalk : Unable to stat,  Skipping %s file\n", fullpath);
			continue;
		}

        if (S_ISREG(st.st_mode)) {
            int check = push_charv(files, strdup(fullpath));
			if(check == -1){
				destroy_charv(files);
				fprintf(stderr, "Error from filewalk : Unable to Push files\n");
				return NULL;
			}
        }
		// For sub folders , creating recursion
        else if (S_ISDIR(st.st_mode)) {

            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
                continue;

            charv *sub = filewalk(fullpath); // recursion to own function
            if (sub) {
                for (size_t i = 0; i < size_charv(sub); i++) {
					char *files_in_sub = strdup(get_charv(sub, i));
					if(!files_in_sub){
						destroy_charv(files);
						destroy_charv(sub);
						fprintf(stderr, "Error from filewalk : Unable to Duplicate string\n");
						return NULL;
					}
                    int check = push_charv(files, files_in_sub);
					if(check == -1){
						destroy_charv(files);
						destroy_charv(sub);
						fprintf(stderr, "Error from filewalk : Unable to Push files\n");
						return NULL;
					}
					free(files_in_sub);
                }
                destroy_charv(sub);
            }
        }
    }

    closedir(d);
    return files;
}

#endif
#endif