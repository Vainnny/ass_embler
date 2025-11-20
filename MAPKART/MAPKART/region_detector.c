#include "region_detector.h"
#include <stdlib.h>
#include <stdio.h>

// Проверка, является ли пиксель белым
int is_white(Pixel p) {
    return p.r > 250 && p.g > 250 && p.b > 250;
}

void flood_fill(int x, int y, int width, int height, Pixel* data, int* region_map, int current_region_id) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;

    int index = y * width + x;
    if (region_map[index] != 0 || !is_white(data[index])) return;

    region_map[index] = current_region_id;

    flood_fill(x + 1, y, width, height, data, region_map, current_region_id);
    flood_fill(x - 1, y, width, height, data, region_map, current_region_id);
    flood_fill(x, y + 1, width, height, data, region_map, current_region_id);
    flood_fill(x, y - 1, width, height, data, region_map, current_region_id);
}

int* find_regions(BMPImage* image, int* region_count) {
    int width = image->info_header.width;
    int height = image->info_header.height;
    int* region_map = (int*)calloc(width * height, sizeof(int));

    if (!region_map) {
        fprintf(stderr, "Failed to allocate memory for region map.\n");
        return NULL;
    }

    int current_region_id = 1;
    long total_pixels = (long)width * height;
    long processed_pixels = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            if (is_white(image->data[index]) && region_map[index] == 0) {
                flood_fill(x, y, width, height, image->data, region_map, current_region_id);
                current_region_id++;
            }

            processed_pixels++;
            int progress = (int)(100.0 * processed_pixels / total_pixels);
            printf("\rFinding regions... %d%%", progress);
            fflush(stdout);
        }
    }
    printf("\nRegion detection complete. Total regions: %d\n", current_region_id);
    printf("\nRegion detection complete.\n");


    *region_count = current_region_id - 1;
    return region_map;
}