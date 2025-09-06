#pragma once
#include <stdint.h>
static inline uint64_t esp_timer_get_time(void) {
    static uint64_t t = 0;
    t += 1000;
    return t;
}
