#ifndef GRAPH_H
#define GRAPH_H

#include <stdlib.h>

typedef struct {
    int** matrix;
    int num_vertices;
} Graph;

Graph* create_graph(int num_vertices);
void free_graph(Graph* graph);
void add_edge(Graph* graph, int v1, int v2);
Graph* build_adjacency_graph(int* region_map, int width, int height, int num_regions);

#endif // GRAPH_H