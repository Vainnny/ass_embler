#ifndef BMP_HANDLER_H
#define BMP_HANDLER_H

#include <stdint.h>

// Структура для хранения пикселя (24-бит)
#pragma pack(push, 1)
typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} Pixel;

typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t size_image;
    int32_t x_pels_per_meter;
    int32_t y_pels_per_meter;
    uint32_t clr_used;
    uint32_t clr_important;
} BMPInfoHeader;
#pragma pack(pop)

typedef struct {
    BMPHeader header;
    BMPInfoHeader info_header;
    Pixel* data;
} BMPImage;

BMPImage* read_bmp(const char* filename);
int write_bmp(const char* filename, BMPImage* image);
void free_bmp(BMPImage* image);

#endif // BMP_HANDLER_H