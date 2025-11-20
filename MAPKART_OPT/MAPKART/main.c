#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp_handler.h"
#include "region_detector.h"
#include "graph.h"
#include "colorizer.h"
#include "utils.h"

static int should_disable_logging() {
    char response[8];
    while (1) {
        printf("Disable logging? (y/n): ");
        if (!fgets(response, sizeof(response), stdin)) {
            printf("Input error detected. Logging will stay enabled.\n");
            return 0;
        }

        if (response[0] == 'y' || response[0] == 'Y') {
            printf("Logging is disabled for this session.\n");
            return 1;
        }

        if (response[0] == 'n' || response[0] == 'N') {
            printf("Logging remains enabled.\n");
            return 0;
        }

        printf("Please answer with 'y' or 'n'.\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file.bmp> <output_file.bmp>\n", argv[0]);
        return 1;
    }

    const char* input_fn = argv[1];
    const char* output_fn = argv[2];

    int logging_disabled = should_disable_logging();

    char log_filename[256] = {0};
    if (!logging_disabled) {
        snprintf(log_filename, sizeof(log_filename), "map_coloring_log_%s.txt", output_fn);
        init_logging(log_filename);
    }
    
    log_message("MAP COLORING PROCESS STARTED\n");
    log_message("============================\n");
    log_message("Input file: %s\n", input_fn);
    log_message("Output file: %s\n", output_fn);
    if (!logging_disabled) {
        log_message("Log file: %s\n", log_filename);
    }

    Timer total_timer, coloring_timer;
    start_timer(&total_timer);

    log_message("\nSTEP -1: Reading BMP file\n");
    log_message("=========================\n");
    printf("Reading BMP file: %s\n", input_fn);
    BMPImage* image = read_bmp(input_fn);
    if (!image) {
        log_message("ERROR: Failed to read BMP file\n");
        close_logging();
        return 1;
    }
    log_message("BMP file read successfully\n");
    log_message("Image dimensions: %d x %d pixels\n", image->info_header.width, image->info_header.height);

    log_message("\nSTEP -2: Region detection\n");
    log_message("=========================\n");
    int region_count = 0;
    int* region_map = find_regions(image, &region_count);
    if (!region_map) {
        log_message("ERROR: Failed to detect regions\n");
        free_bmp(image);
        close_logging();
        return 1;
    }
    printf("Found %d regions.\n", region_count);
    log_message("Region detection completed successfully\n");
    log_message("Total regions found: %d\n", region_count);

    printf("Building adjacency graph...\n");
    Graph* graph = build_adjacency_graph(region_map, image->info_header.width, image->info_header.height, region_count);

    printf("Coloring graph...\n");
    int num_colors = 0;

    start_timer(&coloring_timer);
    int* colors = color_graph(graph, &num_colors);
    stop_timer(&coloring_timer);

    printf("Coloring complete.\n");

    printf("Applying colors to image...\n");
    apply_colors_to_image(image, region_map, colors);

    printf("Writing output file: %s\n", output_fn);
    if (!write_bmp(output_fn, image)) {
        fprintf(stderr, "Failed to write BMP file.\n");
        log_message("ERROR: Failed to write BMP file\n");
    } else {
        log_message("Output file written successfully: %s\n", output_fn);
    }

    stop_timer(&total_timer);

    log_message("\nFINAL STATISTICS\n");
    log_message("================\n");
    log_message("Number of colors used: %d\n", num_colors);
    log_message("Coloring algorithm time: %.4f seconds\n", get_duration(&coloring_timer));
    log_message("Total execution time: %.4f seconds\n", get_duration(&total_timer));
    
    printf("\n--- Results ---\n");
    printf("Number of colors used: %d\n", num_colors);
    printf("Coloring algorithm time: %.4f seconds\n", get_duration(&coloring_timer));
    printf("Total execution time: %.4f seconds\n", get_duration(&total_timer));
    if (!logging_disabled) {
        printf("Log file created: %s\n", log_filename);
    } else {
        printf("Logging was disabled for this run.\n");
    }
    printf("---------------\n");

    free(region_map);
    free(colors);
    free_graph(graph);
    free_bmp(image);
    
    if (!logging_disabled) {
        close_logging();
    }

    return 0;
}