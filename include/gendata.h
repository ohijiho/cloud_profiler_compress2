//
// Created by jiho on 20. 9. 20..
//

#ifndef COMPRESS2_GENDATA_H
#define COMPRESS2_GENDATA_H

#include <log_element.h>
#include <cstddef>
#include <cstdint>

struct gendata_timing {
    log_element last;
    double clock_per_nsec;
};

extern void gendata_tight_loop(raw_log_element *dst, size_t n, const gendata_timing *tm);
extern void gendata_normal(raw_log_element *dst, size_t n, const gendata_timing *tm, double mean, double stddev);
extern void gendata_uniform(raw_log_element *dst, size_t n, const gendata_timing *tm, double lowest, double highest);
extern void gendata_exp(raw_log_element *dst, size_t n, const gendata_timing *tm, double mean);

#endif //COMPRESS2_GENDATA_H
