/*
	File :- vector.h [stb style small vector library]
	
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

#ifndef VECTOR_H
#define VECTOR_H

#ifdef VECTOR_IMPLEMENTATION
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

// Character vector struct
typedef struct{
    char **data;
    size_t size;
    size_t capacity;
} charv;

// INIT function
charv *init_charv()
{
	charv *v = malloc(sizeof(charv));
	if(!v) return NULL;
	
	v->size = 0;
	v->capacity = 32;
	
	v->data = malloc(v->capacity * sizeof(*v->data));
	if(!v->data){
		free(v);
		return NULL;
	}
	return v;
}

// PUSH function
int push_charv(charv *v, char *x)
{
	char *item = strdup(x);
	if(!item) return -1;
	
	if(v->size == v->capacity){
		size_t cap = 2 * v->capacity;
		char **temp = realloc(v->data, cap * sizeof(*v->data));
		if(!temp){
			free(item);
			return -1;
		}
		v->data = temp;
		v->capacity = cap;
	}
	
	v->data[v->size] = item;
	v->size++;
	
	return 0;
}

// POP function
int pop_charv(charv *v)
{
	if(!v || !v->data) return -1;
	if(v->size == 0 ) return -1;
	free(v->data[v->size - 1]);
	v->data[v->size - 1] = NULL;
	v->size--;
	
	return 0;
}

// SIZE function
size_t size_charv(charv *v)
{
	return v->size;
}

// CAPACITY function
size_t capacity_charv(charv *v)
{
	return v->capacity;
}

// Value at a INDEX function
char *get_charv(charv *v, size_t idx)
{
	if(!v || !v->data) return NULL;
	if(v->size == 0 ) return NULL;
	if(idx >= v->size) return NULL;
	return v->data[idx];
}

// Destroying Initialized vector function
void destroy_charv(charv *v)
{
	if(!v) return;
	if(!v->data){
		free(v);
		return;
	}
	for(size_t i = 0; i < v->size; i++){
		free(v->data[i]);
		v->data[i] = NULL;
	}
	free(v->data);
	free(v);
}

#endif
#endif