//
// Created by jiho on 20. 9. 20..
//

#ifndef COMPRESS2_LOG_ELEMENT_H
#define COMPRESS2_LOG_ELEMENT_H

#include "time_tsc.h"
#include <cstdint>

struct raw_log_element {
    int64_t tuple;
    struct timespec ts;
};
struct log_element {
    int64_t tuple;
    int64_t nsec;
    int64_t tsc;
};

static inline log_element log_element_from_raw(const raw_log_element &x) {
    return {
            .tuple = x.tuple,
            .nsec = x.ts.tv_sec * (1000 * 1000 * 1000) + x.ts.tv_nsec,
            .tsc = (int64_t) x.ts.tsc,
    };
}
static inline raw_log_element log_element_to_raw(const log_element &x) {
    return {
            .tuple = x.tuple,
            .ts = {
                    .tv_sec = x.nsec / (1000 * 1000 * 1000),
                    .tv_nsec = x.nsec % (1000 * 1000 * 1000),
                    .tsc = (unsigned long long) x.tsc,
            },
    };
}

#endif //COMPRESS2_LOG_ELEMENT_H
