#include "colorizer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

// Структура для сортировки вершин по степени
typedef struct {
    int vertex;
    int degree;
} VertexDegree;

// Функция сравнения для qsort (по убыванию степени)
static int compare_vertex_degree(const void* a, const void* b) {
    const VertexDegree* va = (const VertexDegree*)a;
    const VertexDegree* vb = (const VertexDegree*)b;
    return vb->degree - va->degree; // Убывание
}

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

// Оптимизированная функция вычисления степени
// Индуктивная переменная: начинаем с 1
static inline int get_degree(Graph* graph, int vertex) {
    int degree = 0;
    int num_v = graph->num_vertices;
    int* row = graph->matrix[vertex]; // Снижение мощности - один раз получаем указатель
    
    // Оптимизация: для малых графов простая проверка быстрее
    if (num_v <= 8) {
        for (int i = 1; i < num_v; i++) {
            degree += row[i];
        }
        return degree;
    }
    
    // Развертка цикла для лучшей производительности (для больших графов)
    int i = 1;
    int end = num_v - 3; // Для развертки
    for (; i < end; i += 4) {
        degree += row[i] + row[i+1] + row[i+2] + row[i+3];
    }
    // Обработка оставшихся элементов
    for (; i < num_v; i++) {
        degree += row[i];
    }
    return degree;
}

// Оптимизированная проверка безопасности цвета
// Ранний выход при первом конфликте
static inline int is_color_safe(Graph* graph, int vertex, int color, int* result_colors) {
    int num_v = graph->num_vertices;
    int* row = graph->matrix[vertex]; // Снижение мощности
    
    // Оптимизация: для малых графов простая проверка быстрее
    if (num_v <= 8) {
        for (int i = 1; i < num_v; i++) {
            // Оптимизация: проверяем сначала наличие ребра, потом цвет (short-circuit)
            if (row[i] && result_colors[i] == color) {
                return 0; // Ранний выход при конфликте
            }
        }
        return 1;
    }
    
    // Развертка цикла для ускорения (только если вершин достаточно)
    int i = 1;
    int end = num_v - 3;
    for (; i < end; i += 4) {
        // Оптимизация: проверяем все 4 элемента за одну итерацию
        // Используем short-circuit evaluation для раннего выхода
        if (row[i] && result_colors[i] == color) return 0;
        if (row[i+1] && result_colors[i+1] == color) return 0;
        if (row[i+2] && result_colors[i+2] == color) return 0;
        if (row[i+3] && result_colors[i+3] == color) return 0;
    }
    // Обработка оставшихся элементов
    for (; i < num_v; i++) {
        if (row[i] && result_colors[i] == color) {
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
    
    // Оптимизация: использование qsort вместо bubble sort
    // O(V log V) вместо O(V²)
    VertexDegree* vd_array = (VertexDegree*)malloc((num_vertices - 1) * sizeof(VertexDegree));
    for (int i = 0; i < num_vertices - 1; i++) {
        vd_array[i].vertex = vertices_by_degree[i];
        vd_array[i].degree = degrees[i];
    }
    
    qsort(vd_array, num_vertices - 1, sizeof(VertexDegree), compare_vertex_degree);
    
    // Восстанавливаем массивы из отсортированной структуры
    for (int i = 0; i < num_vertices - 1; i++) {
        vertices_by_degree[i] = vd_array[i].vertex;
        degrees[i] = vd_array[i].degree;
    }
    free(vd_array);
    
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
    
    // Оптимизация: развертка цикла для MAX_COLORS=4
    // Индуктивная переменная: предвычисление vertex
    int num_v_minus_1 = num_vertices - 1;
    
    for (int i = 0; i < num_v_minus_1; i++) {
        int vertex = vertices_by_degree[i];
        
        log_message("\nProcessing vertex %d (degree %d):\n", vertex, degrees[i]);
        
        // Развертка цикла для 4 цветов (MAX_COLORS = 4)
        // Оптимизация: проверяем цвета последовательно с ранним выходом
        // Цвет 1
        if (is_color_safe(graph, vertex, 1, result_colors)) {
            result_colors[vertex] = 1;
            if (max_color < 1) max_color = 1;
            log_message("  -> Assigned color 1 (safe)\n");
            continue; // Переход к следующей вершине
        }
        log_message("  -> Color 1 not safe (conflicts with adjacent vertices)\n");
        
        // Цвет 2
        if (is_color_safe(graph, vertex, 2, result_colors)) {
            result_colors[vertex] = 2;
            if (max_color < 2) max_color = 2;
            log_message("  -> Assigned color 2 (safe)\n");
            continue;
        }
        log_message("  -> Color 2 not safe (conflicts with adjacent vertices)\n");
        
        // Цвет 3
        if (is_color_safe(graph, vertex, 3, result_colors)) {
            result_colors[vertex] = 3;
            if (max_color < 3) max_color = 3;
            log_message("  -> Assigned color 3 (safe)\n");
            continue;
        }
        log_message("  -> Color 3 not safe (conflicts with adjacent vertices)\n");
        
        // Цвет 4
        if (is_color_safe(graph, vertex, 4, result_colors)) {
            result_colors[vertex] = 4;
            max_color = 4; // Максимальный цвет
            log_message("  -> Assigned color 4 (safe)\n");
            continue;
        }
        log_message("  -> Color 4 not safe (conflicts with adjacent vertices)\n");
        
        // Fallback (не должно происходить с 4-цветной теоремой)
        log_message("  -> WARNING: Could not color vertex %d with 4 colors! Using fallback.\n", vertex);
        result_colors[vertex] = 1;
        if (max_color == 0) max_color = 1;
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
    
    // Оптимизация: индуктивная переменная, снижение мощности
    int total_pixels = width * height;
    Pixel black_pixel = {0, 0, 0}; // Предвычисление
    
    // Оптимизация: кэшируем указатели для быстрого доступа
    Pixel* image_data = image->data;
    
    // Развертка цикла для лучшей производительности (обрабатываем по 4 пикселя)
    int i = 0;
    int end = total_pixels - 3;
    for (; i < end; i += 4) {
        // Пиксель 0
        int region_id0 = region_map[i];
        if (region_id0 > 0) {
            int color_id0 = colors[region_id0];
            // Оптимизация: убираем проверку диапазона (цвета всегда валидны после раскраски)
            image_data[i] = color_palette[color_id0];
            colored_pixels++;
        } else {
            image_data[i] = black_pixel;
            border_pixels++;
        }
        
        // Пиксель 1
        int region_id1 = region_map[i+1];
        if (region_id1 > 0) {
            image_data[i+1] = color_palette[colors[region_id1]];
            colored_pixels++;
        } else {
            image_data[i+1] = black_pixel;
            border_pixels++;
        }
        
        // Пиксель 2
        int region_id2 = region_map[i+2];
        if (region_id2 > 0) {
            image_data[i+2] = color_palette[colors[region_id2]];
            colored_pixels++;
        } else {
            image_data[i+2] = black_pixel;
            border_pixels++;
        }
        
        // Пиксель 3
        int region_id3 = region_map[i+3];
        if (region_id3 > 0) {
            image_data[i+3] = color_palette[colors[region_id3]];
            colored_pixels++;
        } else {
            image_data[i+3] = black_pixel;
            border_pixels++;
        }
    }
    
    // Обработка оставшихся пикселей
    for (; i < total_pixels; i++) {
        int region_id = region_map[i];
        if (region_id > 0) {
            image_data[i] = color_palette[colors[region_id]];
            colored_pixels++;
        } else {
            image_data[i] = black_pixel;
            border_pixels++;
        }
    }
    
    log_message("\nPixel statistics:\n");
    log_message("  Colored pixels: %d\n", colored_pixels);
    log_message("  Border pixels: %d\n", border_pixels);
    log_message("  Total pixels: %d\n", width * height);
}