//
// Created by jiho on 20. 9. 20..
//

#include <time_tsc.h>
#include <gendata.h>
#include <bits/types/clockid_t.h>
#include <ctime>

static unsigned long long rdtsc(void) { // NOLINT(modernize-redundant-void-arg)
#if defined (__x86_64__)
    unsigned low, high;
    asm volatile("rdtsc" : "=a" (low), "=d" (high));
    return low | ((unsigned long long)high) << 32; // NOLINT(hicpp-signed-bitwise)
#elif defined (__aarch64__)
    uint64_t val;
  asm volatile("mrs %0, PMCCNTR_EL0" : "=r"(val));
  return val;
#else
  #error "Unsupported architecture"
#endif
}

static int clock_gettime_tsc_s(clockid_t clk_id, struct timespec *tp) {
    int retval = clock_gettime(clk_id, tp);
    tp->tsc = rdtsc();
    return retval;
}
static int getTS(struct timespec *ts) {
    return clock_gettime_tsc_s(CLOCK_REALTIME, ts);
}

void gendata_tight_loop(raw_log_element *dst, size_t n, const gendata_timing *tm) {
    int64_t tuple_start = tm->last.tuple + 1;
    for (long i = 0; i < (ptrdiff_t)n; i++) {
        timespec ts; // NOLINT(cppcoreguidelines-pro-type-member-init)
        getTS(&ts);
        dst[i] = {tuple_start + i, ts};
    }
}
