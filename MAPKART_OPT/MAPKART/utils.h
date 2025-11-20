#ifndef UTILS_H
#define UTILS_H

#include <time.h>

// Структура для хранения времени выполнения
typedef struct {
    clock_t start;
    clock_t end;
} Timer;

void start_timer(Timer* timer);
void stop_timer(Timer* timer);
double get_duration(Timer* timer);

#endif // UTILS_H