#include "utils.h"

void start_timer(Timer* timer) {
    timer->start = clock();
}

void stop_timer(Timer* timer) {
    timer->end = clock();
}

double get_duration(Timer* timer) {
    return ((double)(timer->end - timer->start)) / CLOCKS_PER_SEC;
}