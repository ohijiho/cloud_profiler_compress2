//
// Created by jiho on 20. 9. 20..
//

#include <thcomp.h>

thcomp_handler::thcomp_handler() : q1(4), q2(4) {
    th.emplace_back([this]() {
        comp_thread();
    });
    th.emplace_back([this]() {
        io_thread();
    });
}

thcomp_handler::~thcomp_handler() = default;

void thcomp_handler::comp_thread() {
    std::vector<char> tmp;
    for (;;) {
        auto x = q1.pop();
        if (x.empty()) {
            q2.push({});
            break;
        }
        compress(std::move(x), tmp);
        q2.push(std::move(tmp));
    }
}

void thcomp_handler::io_thread() {
    for (;;) {
        auto x = q2.pop();
        if (x.empty()) {
            write({});
            break;
        }
        write(std::move(x));
    }
}

void thcomp_handler::close() {
    q1.push({});
    for (auto &t : th)
        t.join();
}

void thcomp_handler::input(std::vector<char> &&x) {
    q1.push(x);
}
