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

void add_edge(Graph* graph, int v1, int v2) {
    if (v1 != v2) {
        graph->matrix[v1][v2] = 1;
        graph->matrix[v2][v1] = 1;
    }
}

Graph* build_adjacency_graph(int* region_map, int width, int height, int num_regions) {
    log_message("\nSTEP 0: Building adjacency graph\n");
    log_message("=================================\n");
    log_message("Image dimensions: %d x %d\n", width, height);
    log_message("Number of regions: %d\n", num_regions);
    
    Graph* graph = create_graph(num_regions + 1);
    int edge_count = 0;

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int current_region = region_map[y * width + x];
            
            if (current_region > 0) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        
                        int nx = x + dx;
                        int ny = y + dy;
                        int neighbor_region = region_map[ny * width + nx];
                        
                        if (neighbor_region > 0 && neighbor_region != current_region) {
                            if (!graph->matrix[current_region][neighbor_region]) {
                                add_edge(graph, current_region, neighbor_region);
                                edge_count++;
                                log_message("Added edge: Region %d <-> Region %d\n", current_region, neighbor_region);
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
    
    // Log adjacency matrix
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