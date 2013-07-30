#ifndef IL_TIMER_H
#define IL_TIMER_H

#define timeradd(a, b, res)                         \
    (res)->tv_sec  =  (a)->tv_sec + (b)->tv_sec;    \
    (res)->tv_usec =  (a)->tv_usec + (b)->tv_usec;  \
    (res)->tv_sec  += (res)->tv_usec / 1000000;     \
    (res)->tv_usec %= 1000000;

#define timersub(a, b, res)                                     \
    (res)->tv_sec = (a)->tv_sec - (b)->tv_sec;                  \
    if ((a)->tv_usec - (b)->tv_usec < 0) {                      \
        (res)->tv_sec--;                                        \
        (res)->tv_usec = (a)->tv_usec - (b)->tv_usec + 1000000; \
    } else {                                                    \
        (res)->tv_usec = (a)->tv_usec - (b)->tv_usec;           \
    }

#endif

