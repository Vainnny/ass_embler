#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define IMG_WIDTH 1000
#define IMG_HEIGHT 1000

static void set_pixel(uint8_t *pixels, int width, int height, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    size_t idx = (size_t)(y * width + x) * 3u;
    pixels[idx + 0] = b;
    pixels[idx + 1] = g;
    pixels[idx + 2] = r;
}

static void draw_line(uint8_t *pixels, int width, int height, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (1) {
        set_pixel(pixels, width, height, x0, y0, r, g, b);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void random_border_point(int width, int height, int *out_x, int *out_y) {
    int side = rand() % 4;
    switch (side) {
        case 0:
            *out_y = 0; *out_x = rand() % width; break;
        case 1:
            *out_x = width - 1; *out_y = rand() % height; break;
        case 2:
            *out_y = height - 1; *out_x = rand() % width; break;
        default:
            *out_x = 0; *out_y = rand() % height; break;
    }
}

static int write_bmp24(const char *path, const uint8_t *pixels, int width, int height) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;

    int row_stride = width * 3;
    int padding = (4 - (row_stride % 4)) % 4;
    uint32_t image_size = (uint32_t)((row_stride + padding) * height);
    uint32_t file_size = 14 + 40 + image_size;

    unsigned char file_header[14];
    memset(file_header, 0, sizeof(file_header));
    file_header[0] = 'B';
    file_header[1] = 'M';
    file_header[2] = (unsigned char)(file_size & 0xFF);
    file_header[3] = (unsigned char)((file_size >> 8) & 0xFF);
    file_header[4] = (unsigned char)((file_size >> 16) & 0xFF);
    file_header[5] = (unsigned char)((file_size >> 24) & 0xFF);
    file_header[10] = 54;

    unsigned char info_header[40];
    memset(info_header, 0, sizeof(info_header));
    info_header[0] = 40;
    info_header[4]  = (unsigned char)(width & 0xFF);
    info_header[5]  = (unsigned char)((width >> 8) & 0xFF);
    info_header[6]  = (unsigned char)((width >> 16) & 0xFF);
    info_header[7]  = (unsigned char)((width >> 24) & 0xFF);
    info_header[8]  = (unsigned char)(height & 0xFF);
    info_header[9]  = (unsigned char)((height >> 8) & 0xFF);
    info_header[10] = (unsigned char)((height >> 16) & 0xFF);
    info_header[11] = (unsigned char)((height >> 24) & 0xFF);
    info_header[12] = 1;
    info_header[14] = 24;

    info_header[20] = (unsigned char)(image_size & 0xFF);
    info_header[21] = (unsigned char)((image_size >> 8) & 0xFF);
    info_header[22] = (unsigned char)((image_size >> 16) & 0xFF);
    info_header[23] = (unsigned char)((image_size >> 24) & 0xFF);
    const uint32_t ppm = 2835;
    info_header[24] = (unsigned char)(ppm & 0xFF);
    info_header[25] = (unsigned char)((ppm >> 8) & 0xFF);
    info_header[26] = (unsigned char)((ppm >> 16) & 0xFF);
    info_header[27] = (unsigned char)((ppm >> 24) & 0xFF);
    info_header[28] = (unsigned char)(ppm & 0xFF);
    info_header[29] = (unsigned char)((ppm >> 8) & 0xFF);
    info_header[30] = (unsigned char)((ppm >> 16) & 0xFF);
    info_header[31] = (unsigned char)((ppm >> 24) & 0xFF);

    if (fwrite(file_header, 1, sizeof(file_header), f) != sizeof(file_header)) { fclose(f); return -2; }
    if (fwrite(info_header, 1, sizeof(info_header), f) != sizeof(info_header)) { fclose(f); return -3; }

    unsigned char pad[3] = {0, 0, 0};
    for (int y = height - 1; y >= 0; --y) {
        const uint8_t *row = pixels + (size_t)y * (size_t)width * 3u;
        if (fwrite(row, 1, (size_t)row_stride, f) != (size_t)row_stride) { fclose(f); return -4; }
        if (padding) {
            if (fwrite(pad, 1, (size_t)padding, f) != (size_t)padding) { fclose(f); return -5; }
        }
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    int num_lines = 0;
    const char *out_path = "lines.bmp";

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num_lines> [output.bmp]\n", argv[0]);
        return 1;
    }

    char *endptr = NULL;
    long parsed = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1] || parsed < 0 || parsed > 1000000) {
        fprintf(stderr, "Invalid number of lines: %s\n", argv[1]);
        return 1;
    }
    num_lines = (int)parsed;

    if (argc >= 3) {
        out_path = argv[2];
    }

    uint8_t *pixels = (uint8_t *)malloc((size_t)IMG_WIDTH * (size_t)IMG_HEIGHT * 3u);
    if (!pixels) {
        fprintf(stderr, "Failed to allocate image buffer\n");
        return 1;
    }

    memset(pixels, 0xFF, (size_t)IMG_WIDTH * (size_t)IMG_HEIGHT * 3u);

    srand((unsigned int)time(NULL));

    for (int i = 0; i < num_lines; ++i) {
        int x0, y0, x1, y1;
        int attempts = 0;
        do {
            random_border_point(IMG_WIDTH, IMG_HEIGHT, &x0, &y0);
            random_border_point(IMG_WIDTH, IMG_HEIGHT, &x1, &y1);
            attempts++;
        } while (x0 == x1 && y0 == y1 && attempts < 10);

        draw_line(pixels, IMG_WIDTH, IMG_HEIGHT, x0, y0, x1, y1, 0, 0, 0);
    }

    int rc = write_bmp24(out_path, pixels, IMG_WIDTH, IMG_HEIGHT);
    free(pixels);

    if (rc != 0) {
        fprintf(stderr, "Failed to write BMP (err=%d)\n", rc);
        return 1;
    }

    printf("Wrote %s with %d line(s).\n", out_path, num_lines);
    return 0;
}