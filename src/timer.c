
#include "timer.h"

void timer_start(TIMER *timer) {
    clock_gettime(CLOCK_REALTIME, &(timer->time1));
}

void timer_stop(TIMER *timer) {
    clock_gettime(CLOCK_REALTIME, &(timer->time2));
}

double timer_timemsec(TIMER *timer) {
    return timer_timensec(timer)/1E6;
}

double timer_timensec(TIMER *timer) {
    return (timer->time2.tv_sec*1E9 + timer->time2.tv_nsec) - (timer->time1.tv_sec*1E9 + timer->time1.tv_nsec);
}

void msleep(double timems) {
    usleep(timems*1E3);
}
