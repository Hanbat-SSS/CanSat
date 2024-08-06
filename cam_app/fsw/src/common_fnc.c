
/*
This is common_function file for Encrypt
*/

#define BLOCK_SIZE 16
#include <common_fnc.h>
#include <security.h>

// 이미지 데이터를 읽는 함수
bool read_image_data(byte** data, size_t* size, const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        perror("Failed to open image file");
        return false;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *data = (byte*)malloc(*size);
    if (*data == NULL)
    {
        perror("Failed to allocate memory");
        fclose(file);
        return false;
    }

    fread(*data, 1, *size, file);
    fclose(file);

    return true;
}

void write_image_data(const byte* data, size_t size, const char* filename)
{
    FILE* file = fopen(filename, "wb");
    if (file == NULL)
    {
        perror("Failed to open file for writing");
        return;
    }

    fwrite(data, 1, size, file);
    fclose(file);
}

void pad_data(byte** data, size_t* size)
{
    size_t pad_size = BLOCK_SIZE - (*size % BLOCK_SIZE);
    *data = (byte*)realloc(*data, *size + pad_size);
    if (*data == NULL)
    {
        perror("Failed to reallocate memory");
        return;
    }
    memset(*data + *size, pad_size, pad_size);
    *size += pad_size;
}

void unpad_data(byte** data, size_t* size)
{
    size_t pad_size = (*data)[*size - 1];
    *size -= pad_size;
    *data = (byte*)realloc(*data, *size);
}

void read_encrypted_data(byte** data, size_t* size, const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL) 
    {
        printf("Failed to open file: %s\n", filename);
        return;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file) / 2; // 16진수 문자열을 바이너리로 변환할 것이므로 크기를 반으로 줄임
    fseek(file, 0, SEEK_SET);

    *data = (byte*)malloc(*size);
    char buffer[3] = {0};
    for (size_t i = 0; i < *size; i++)
    {
        fread(buffer, 1, 2, file);
        (*data)[i] = (byte)strtol(buffer, NULL, 16);
    }
    fclose(file);
}

/*
void read_hex_data(byte** data, size_t* size, const char* filename) 
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) 
	{
        printf("Failed to open file: %s\n", filename);
        return;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *size = file_size / 2;
    *data = (byte*)malloc(*size);

    for (size_t i = 0; i < *size; i++) 
	{
        unsigned int byte_value;
        fscanf(file, "%02x", &byte_value);
        (*data)[i] = (byte)byte_value;
    }

    fclose(file);
}
*/
