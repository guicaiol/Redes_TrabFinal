#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <unistd.h>

typedef struct {
    struct timespec time1, time2;
} TIMER;

void timer_start(TIMER *timer);
void timer_stop(TIMER *timer);
double timer_timemsec(TIMER *timer);
double timer_timensec(TIMER *timer);

void msleep(double timems);

#endif // TIMER_H
