//
// Created by jiho on 20. 9. 20..
//

#include <gendata.h>
#include <random>
#include <functional>

static void gendata_interval_func(raw_log_element *dst, size_t n,
        const gendata_timing *tm, const std::function<double()> &f) {
    double accd = 0;
    int64_t tuple_start = tm->last.tuple + 1;
    auto log = [&](long i) {
        double interval = f();
        accd += interval;
        return log_element_to_raw({tuple_start + i,
                tm->last.nsec + (int64_t)accd,
                tm->last.tsc + (int64_t)(tm->clock_per_nsec * accd)});
    };
    for (long i = 0; i < (ptrdiff_t)n; i++) {
        dst[i] = log(i);
    }
}

void gendata_normal(raw_log_element *dst, size_t n,
        const gendata_timing *tm, double mean, double stddev) {
    std::default_random_engine gen(tm->last.nsec * mean);
    std::normal_distribution<double> dist(mean, stddev);
    gendata_interval_func(dst, n, tm, [&]() {return dist(gen);});
}
void gendata_uniform(raw_log_element *dst, size_t n,
        const gendata_timing *tm, double lowest, double highest) {
    std::default_random_engine gen(tm->last.nsec * highest);
    std::uniform_real_distribution<double> dist(lowest, highest);
    gendata_interval_func(dst, n, tm, [&]() {return dist(gen);});
}
void gendata_exp(raw_log_element *dst, size_t n,
        const gendata_timing *tm, double mean) {
    std::default_random_engine gen(tm->last.nsec * mean);
    std::exponential_distribution<double> dist(1 / mean);
    gendata_interval_func(dst, n, tm, [&]() {return dist(gen);});
}

