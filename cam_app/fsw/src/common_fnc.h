
/*
This is common_function file for Encrypt
*/

#ifndef COMMON_FNC_H
#define COMMON_FNC_H

#ifndef BYTE_TYPE_DEFINED
#define BYTE_TYPE_DEFINED
typedef unsigned char byte;
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <security.h>

bool read_image_data(byte** data, size_t* size, const char* filename);

void read_encrypted_data(byte** data, size_t* size, const char* filename);

void write_image_data(const byte* data, size_t size, const char* filename);

void pad_data(byte** data, size_t* size);

void unpad_data(byte** data, size_t* size);

//void read_hex_data(byte** data, size_t* size, const char* filename);

#endif /* COMMON_FNC_H */
