#include "colorizer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

static FILE* log_file = NULL;

void init_logging(const char* log_filename) {
    log_file = fopen(log_filename, "w");
    if (!log_file) {
        fprintf(stderr, "Failed to create log file: %s\n", log_filename);
        return;
    }
    
    time_t now = time(0);
    char* time_str = ctime(&now);
    fprintf(log_file, "=== MAP COLORING LOG ===\n");
    fprintf(log_file, "Started at: %s", time_str);
    fprintf(log_file, "========================================\n\n");
}

void close_logging() {
    if (log_file) {
        time_t now = time(0);
        char* time_str = ctime(&now);
        fprintf(log_file, "\n========================================\n");
        fprintf(log_file, "Log completed at: %s", time_str);
        fclose(log_file);
        log_file = NULL;
    }
}

// Log function
void log_message(const char* format, ...) {
    if (log_file) {
        va_list args;
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
        fflush(log_file);
    }
}

int get_degree(Graph* graph, int vertex) {
    int degree = 0;
    for (int i = 1; i < graph->num_vertices; i++) {
        if (graph->matrix[vertex][i]) {
            degree++;
        }
    }
    return degree;
}

int is_color_safe(Graph* graph, int vertex, int color, int* result_colors) {
    for (int i = 1; i < graph->num_vertices; i++) {
        if (graph->matrix[vertex][i] && result_colors[i] == color) {
            return 0;
        }
    }
    return 1;
}

int* color_graph(Graph* graph, int* num_colors) {
    log_message("STEP 1: Starting graph coloring process\n");
    log_message("=====================================\n");
    
    int num_vertices = graph->num_vertices;
    log_message("Total vertices in graph: %d\n", num_vertices);
    
    int* result_colors = (int*)malloc(num_vertices * sizeof(int));
    
    for (int i = 0; i < num_vertices; i++) {
        result_colors[i] = 0;
    }

    int* vertices_by_degree = (int*)malloc((num_vertices - 1) * sizeof(int));
    int* degrees = (int*)malloc((num_vertices - 1) * sizeof(int));
    
    log_message("\nSTEP 2: Calculating vertex degrees\n");
    log_message("==================================\n");
    
    for (int i = 1; i < num_vertices; i++) {
        vertices_by_degree[i - 1] = i;
        degrees[i - 1] = get_degree(graph, i);
        log_message("Vertex %d: degree = %d\n", i, degrees[i - 1]);
    }
    
    log_message("\nSTEP 3: Sorting vertices by degree (descending)\n");
    log_message("==============================================\n");
    
    for (int i = 0; i < num_vertices - 2; i++) {
        for (int j = 0; j < num_vertices - 2 - i; j++) {
            if (degrees[j] < degrees[j + 1]) {
                int temp_degree = degrees[j];
                degrees[j] = degrees[j + 1];
                degrees[j + 1] = temp_degree;
                
                int temp_vertex = vertices_by_degree[j];
                vertices_by_degree[j] = vertices_by_degree[j + 1];
                vertices_by_degree[j + 1] = temp_vertex;
            }
        }
    }
    
    log_message("Sorted order (by degree): ");
    for (int i = 0; i < num_vertices - 1; i++) {
        log_message("%d(%d) ", vertices_by_degree[i], degrees[i]);
    }
    log_message("\n");
    
    int max_color = 0;
    const int MAX_COLORS = 4;
    
    log_message("\nSTEP 4: Coloring vertices using Welsh-Powell algorithm\n");
    log_message("=====================================================\n");
    log_message("Maximum colors allowed: %d\n", MAX_COLORS);
    
    for (int i = 0; i < num_vertices - 1; i++) {
        int vertex = vertices_by_degree[i];
        
        log_message("\nProcessing vertex %d (degree %d):\n", vertex, degrees[i]);
        
        for (int color = 1; color <= MAX_COLORS; color++) {
            if (is_color_safe(graph, vertex, color, result_colors)) {
                result_colors[vertex] = color;
                if (color > max_color) {
                    max_color = color;
                }
                log_message("  -> Assigned color %d (safe)\n", color);
                break;
            } else {
                log_message("  -> Color %d not safe (conflicts with adjacent vertices)\n", color);
            }
        }
        
        if (result_colors[vertex] == 0) {
            log_message("  -> WARNING: Could not color vertex %d with 4 colors! Using fallback.\n", vertex);
            result_colors[vertex] = 1;
            if (max_color == 0) max_color = 1;
        }
    }
    
    log_message("\nSTEP 5: Coloring results summary\n");
    log_message("===============================\n");
    log_message("Final coloring:\n");
    for (int i = 1; i < num_vertices; i++) {
        log_message("  Region %d: Color %d\n", i, result_colors[i]);
    }
    log_message("\nTotal colors used: %d\n", max_color);
    
    free(vertices_by_degree);
    free(degrees);
    
    *num_colors = max_color;
    return result_colors;
}


void apply_colors_to_image(BMPImage* image, int* region_map, int* colors) {
    log_message("\nSTEP 6: Applying colors to image\n");
    log_message("=================================\n");
    
    int width = image->info_header.width;
    int height = image->info_header.height;
    log_message("Image dimensions: %d x %d pixels\n", width, height);

    Pixel color_palette[] = {
            {0, 0, 0},       // 0 - Black (unused/borders)
            {255, 0, 0},     // 1 - Red
            {0, 255, 0},     // 2 - Green
            {0, 0, 255},     // 3 - Blue
            {0, 255, 255},   // 4 - Yellow
    };
    int palette_size = sizeof(color_palette) / sizeof(Pixel);
    
    log_message("Color palette:\n");
    log_message("  Color 0: Black (borders)\n");
    log_message("  Color 1: Red (255, 0, 0)\n");
    log_message("  Color 2: Green (0, 255, 0)\n");
    log_message("  Color 3: Blue (0, 0, 255)\n");
    log_message("  Color 4: Yellow (255, 255, 0)\n");
    
    int colored_pixels = 0;
    int border_pixels = 0;
    
    for (int i = 0; i < width * height; i++) {
        int region_id = region_map[i];
        if (region_id > 0) {
            int color_id = colors[region_id];
            if (color_id >= 1 && color_id <= 4) {
                image->data[i] = color_palette[color_id];
                colored_pixels++;
            } else {
                image->data[i] = color_palette[1];
                colored_pixels++;
                log_message("WARNING: Invalid color ID %d for region %d, using red\n", color_id, region_id);
            }
        } else {
            image->data[i] = (Pixel){0, 0, 0};
            border_pixels++;
        }
    }
    
    log_message("\nPixel statistics:\n");
    log_message("  Colored pixels: %d\n", colored_pixels);
    log_message("  Border pixels: %d\n", border_pixels);
    log_message("  Total pixels: %d\n", width * height);
}