#include "graph.h"
#include <stdio.h>
#include "colorizer.h"
Graph* create_graph(int num_vertices) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->num_vertices = num_vertices;
    graph->matrix = (int**)calloc(num_vertices, sizeof(int*));
    for (int i = 0; i < num_vertices; i++) {
        graph->matrix[i] = (int*)calloc(num_vertices, sizeof(int));
    }
    return graph;
}
void free_graph(Graph* graph) {
    if (graph) {
        for (int i = 0; i < graph->num_vertices; i++) {
            free(graph->matrix[i]);
        }
        free(graph->matrix);
        free(graph);
    }
}
static inline void add_edge_fast(Graph* graph, int v1, int v2) {
    graph->matrix[v1][v2] = 1;
    graph->matrix[v2][v1] = 1;
}
void add_edge(Graph* graph, int v1, int v2) {
    if (v1 != v2) {
        add_edge_fast(graph, v1, v2);
    }
}
Graph* build_adjacency_graph(int* region_map, int width, int height, int num_regions) {
    log_message("\nSTEP 0: Building adjacency graph\n");
    log_message("=================================\n");
    log_message("Image dimensions: %d x %d\n", width, height);
    log_message("Number of regions: %d\n", num_regions);
    Graph* graph = create_graph(num_regions + 1);
    int edge_count = 0;
    int* region_pixel_count = (int*)calloc(num_regions + 1, sizeof(int));
    int* region_border_pixel_count = (int*)calloc(num_regions + 1, sizeof(int));
    const int width_const = width; 
    for (int y = 0; y < height; y++) {
        int y_offset = y * width_const; 
        for (int x = 0; x < width; x++) {
            int current_idx = y_offset + x;
            int current_region = region_map[current_idx];
            if (current_region > 0) {
                region_pixel_count[current_region]++;
                if (y > 0) {
                    int neighbor_idx = (y - 1) * width_const + x;
                    int neighbor_region = region_map[neighbor_idx];
                    if (neighbor_region > 0 && neighbor_region != current_region) {
                        int* row = graph->matrix[current_region];
                        if (!row[neighbor_region]) {
                            add_edge_fast(graph, current_region, neighbor_region);
                            edge_count++;
                            log_message("Added edge: Region %d <-> Region %d (direct contact at %d,%d)\n", 
                                      current_region, neighbor_region, x, y);
                        }
                    } else if (neighbor_region == 0) {
                        region_border_pixel_count[current_region]++;
                    }
                }
                if (y < height - 1) {
                    int neighbor_idx = (y + 1) * width_const + x;
                    int neighbor_region = region_map[neighbor_idx];
                    if (neighbor_region > 0 && neighbor_region != current_region) {
                        int* row = graph->matrix[current_region];
                        if (!row[neighbor_region]) {
                            add_edge_fast(graph, current_region, neighbor_region);
                            edge_count++;
                            log_message("Added edge: Region %d <-> Region %d (direct contact at %d,%d)\n", 
                                      current_region, neighbor_region, x, y);
                        }
                    }
                }
                if (x > 0) {
                    int neighbor_idx = y_offset + (x - 1);
                    int neighbor_region = region_map[neighbor_idx];
                    if (neighbor_region > 0 && neighbor_region != current_region) {
                        int* row = graph->matrix[current_region];
                        if (!row[neighbor_region]) {
                            add_edge_fast(graph, current_region, neighbor_region);
                            edge_count++;
                            log_message("Added edge: Region %d <-> Region %d (direct contact at %d,%d)\n", 
                                      current_region, neighbor_region, x, y);
                        }
                    }
                }
                if (x < width - 1) {
                    int neighbor_idx = y_offset + (x + 1);
                    int neighbor_region = region_map[neighbor_idx];
                    if (neighbor_region > 0 && neighbor_region != current_region) {
                        int* row = graph->matrix[current_region];
                        if (!row[neighbor_region]) {
                            add_edge_fast(graph, current_region, neighbor_region);
                            edge_count++;
                            log_message("Added edge: Region %d <-> Region %d (direct contact at %d,%d)\n", 
                                      current_region, neighbor_region, x, y);
                        }
                    }
                }
            }
        }
    }
    log_message("\nSearching for regions adjacent through borders...\n");
    for (int y = 1; y < height - 1; y++) {
        int y_offset = y * width_const;
        for (int x = 1; x < width - 1; x++) {
            int current_idx = y_offset + x;
            int current_region = region_map[current_idx];
            if (current_region == 0) {
                unsigned int region_mask = 0;
                int found_regions[4];
                int region_count = 0;
                int up_region = region_map[(y - 1) * width_const + x];
                if (up_region > 0 && !(region_mask & (1u << up_region))) {
                    region_mask |= (1u << up_region);
                    found_regions[region_count++] = up_region;
                }
                int down_region = region_map[(y + 1) * width_const + x];
                if (down_region > 0 && !(region_mask & (1u << down_region))) {
                    region_mask |= (1u << down_region);
                    found_regions[region_count++] = down_region;
                }
                int left_region = region_map[y_offset + (x - 1)];
                if (left_region > 0 && !(region_mask & (1u << left_region))) {
                    region_mask |= (1u << left_region);
                    found_regions[region_count++] = left_region;
                }
                int right_region = region_map[y_offset + (x + 1)];
                if (right_region > 0 && !(region_mask & (1u << right_region))) {
                    region_mask |= (1u << right_region);
                    found_regions[region_count++] = right_region;
                }
                if (region_count > 1) {
                    for (int i = 0; i < region_count; i++) {
                        int r1 = found_regions[i];
                        int* row1 = graph->matrix[r1];
                        for (int j = i + 1; j < region_count; j++) {
                            int r2 = found_regions[j];
                            if (!row1[r2]) {
                                add_edge_fast(graph, r1, r2);
                                edge_count++;
                                log_message("Added edge: Region %d <-> Region %d (through border at %d,%d)\n", 
                                          r1, r2, x, y);
                            }
                        }
                    }
                }
            }
        }
    }
    log_message("\nGraph construction complete:\n");
    log_message("  Total edges added: %d\n", edge_count);
    log_message("  Graph vertices: %d\n", graph->num_vertices);
    log_message("\nRegion statistics:\n");
    for (int i = 1; i <= num_regions; i++) {
        log_message("  Region %d: %d pixels, %d border pixels\n", 
                   i, region_pixel_count[i], region_border_pixel_count[i]);
    }
    free(region_pixel_count);
    free(region_border_pixel_count);
    log_message("\nAdjacency matrix:\n");
    log_message("     ");
    for (int i = 1; i < graph->num_vertices; i++) {
        log_message("%2d ", i);
    }
    log_message("\n");
    for (int i = 1; i < graph->num_vertices; i++) {
        log_message("%2d: ", i);
        for (int j = 1; j < graph->num_vertices; j++) {
            log_message("%2d ", graph->matrix[i][j]);
        }
        log_message("\n");
    }
    return graph;
}