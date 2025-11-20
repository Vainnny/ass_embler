#ifndef COLORIZER_H
#define COLORIZER_H

#include "graph.h"
#include "bmp_handler.h"

// Logging functions
void init_logging(const char* log_filename);
void close_logging();
void log_message(const char* format, ...);

// Main coloring functions
int* color_graph(Graph* graph, int* num_colors);
void apply_colors_to_image(BMPImage* image, int* region_map, int* colors);

#endif // COLORIZER_H