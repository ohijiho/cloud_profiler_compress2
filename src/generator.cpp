//
// Created by jiho on 20. 9. 21..
//

#include <time_tsc.h>
#include <gendata.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <functional>
#include <vector>

static bool prefix_skip(const char *p, const char *&s) {
    size_t n = strlen(p);
    if (strncmp(p, s, n) == 0) {
        s += n;
        return true;
    }
    return false;
}

int main(int argc, char **argv) {
    bool help = false, null = false;
    size_t block_size = 4096, blocks = 1024;
    gendata_timing tm = {
            .clock_per_nsec = 2.5,
    };
    const char *method = "ts";
    double mean = 200, stddev = 50, lowest = 10, highest = 1000;
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (a[0] == '-') {
            if (a[1] == '-') {
                a += 2;
                if (prefix_skip("block-size=", a)) {
                    block_size = strtol(a, nullptr, 0);
                    //TODO: exception handling
                } else if (prefix_skip("blocks=", a)) {
                    blocks = strtol(a, nullptr, 0);
                    //TODO: exception handling
                } else if (prefix_skip("method=", a)) {
                    method = a;
                } else if (prefix_skip("mean=", a)) {
                    mean = strtod(a, nullptr);
                    //TODO: exception handling
                } else if (prefix_skip("stddev=", a)) {
                    stddev = strtod(a, nullptr);
                    //TODO: exception handling
                } else if (prefix_skip("lowest=", a)) {
                    lowest = strtod(a, nullptr);
                    //TODO: exception handling
                } else if (prefix_skip("highest=", a)) {
                    highest = strtod(a, nullptr);
                    //TODO: exception handling
                } else if (strcmp("null", a) == 0) {
                    null = true;
                } else if (strcmp("help", a) == 0) {
                    help = true;
                } else {
                    std::cerr << "invalid arguments" << std::endl;
                    return 1;
                }
            } else {
                a += 1;
                for (; *a; a++) {
                    switch (*a) {
                    case 'h':
                        help = true;
                        break;
                    default:
                        std::cerr << "invalid arguments" << std::endl;
                        return 1;
                    }
                }
            }
        }
    }
    if (help) {
        //TODO: help
        printf("Usage: %s [OPTIONS]...\n", argv[0]);
        printf("Encode or decode standard input and print to standard output\n");
        printf("\n");
        printf("      --block-size=SIZE  encode each block with SIZE log elements\n");
        return 0;
    }

    {
        raw_log_element raw; // NOLINT(cppcoreguidelines-pro-type-member-init)
        gendata_tight_loop(&raw, 1, &tm);
        log_element log = log_element_from_raw(raw);
        tm.last = {
                .tuple = -1,
                .nsec = log.nsec,
                .tsc = log.tsc,
        };
    }

    {
        std::ostream *out = &std::cout;
        std::ofstream fout;
        if (null) {
            fout.open("/dev/null");
            out = &fout;
        }
        std::function<void(raw_log_element *dst, size_t n, const gendata_timing *tm)> gen_f;
        if (strcmp("ts", method) == 0) {
            gen_f = gendata_tight_loop;
        } else if (strcmp("n", method) == 0) {
            gen_f = [mean, stddev](raw_log_element *dst, size_t n, const gendata_timing *tm) {
                gendata_normal(dst, n, tm, mean, stddev);
            };
        } else if (strcmp("u", method) == 0) {
            gen_f = [lowest, highest](raw_log_element *dst, size_t n, const gendata_timing *tm) {
                gendata_uniform(dst, n, tm, lowest, highest);
            };
        } else if (strcmp("e", method) == 0) {
            gen_f = [mean](raw_log_element *dst, size_t n, const gendata_timing *tm) {
                gendata_exp(dst, n, tm, mean);
            };
        } else {
            std::cerr << "Unknwon method: " << method << std::endl;
            return 1;
        }
        for (int i = 0; i < (ptrdiff_t)blocks; i++) {
            std::vector<raw_log_element> buf(block_size);
            gen_f(buf.data(), block_size, &tm);
            out->write((char*)buf.data(), block_size * sizeof(raw_log_element));
            tm.last = log_element_from_raw(buf.back());
        }
    }
    return 0;
}
