#include "bmp_handler.h"
#include <stdio.h>
#include <stdlib.h>

BMPImage* read_bmp(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Failed to open input file");
        return NULL;
    }

    BMPImage* image = (BMPImage*)malloc(sizeof(BMPImage));
    if (!image) {
        fclose(f);
        return NULL;
    }

    fread(&image->header, sizeof(BMPHeader), 1, f);
    fread(&image->info_header, sizeof(BMPInfoHeader), 1, f);

    if (image->header.type != 0x4D42) { // 'BM'
        fprintf(stderr, "Error: Not a BMP file.\n");
        fclose(f);
        free(image);
        return NULL;
    }

    if (image->info_header.bit_count != 24) {
        fprintf(stderr, "Error: Only 24-bit BMP files are supported.\n");
        fclose(f);
        free(image);
        return NULL;
    }

    int width = image->info_header.width;
    int height = image->info_header.height;
    int data_size = width * height * sizeof(Pixel);
    image->data = (Pixel*)malloc(data_size);

    fseek(f, image->header.offset, SEEK_SET);

    int padding = (4 - (width * sizeof(Pixel)) % 4) % 4;

    for (int i = 0; i < height; i++) {
        fread(image->data + i * width, sizeof(Pixel), width, f);
        fseek(f, padding, SEEK_CUR);
    }

    fclose(f);
    return image;
}

int write_bmp(const char* filename, BMPImage* image) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("Failed to open output file");
        return 0;
    }

    int width = image->info_header.width;
    int height = image->info_header.height;

    fwrite(&image->header, sizeof(BMPHeader), 1, f);
    fwrite(&image->info_header, sizeof(BMPInfoHeader), 1, f);

    int padding = (4 - (width * sizeof(Pixel)) % 4) % 4;
    char pad_bytes[3] = {0,0,0};

    for (int i = 0; i < height; i++) {
        fwrite(image->data + i * width, sizeof(Pixel), width, f);
        fwrite(pad_bytes, 1, padding, f);
    }

    fclose(f);
    return 1;
}

void free_bmp(BMPImage* image) {
    if (image) {
        free(image->data);
        free(image);
    }
}