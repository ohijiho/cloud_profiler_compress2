//
// Created by jiho on 20. 9. 20..
//

#ifndef COMPRESS2_THCOMP_H
#define COMPRESS2_THCOMP_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/circular_buffer.hpp>
#include <vector>
#include <atomic>

template <typename T>
class blocking_queue {
public:
    explicit blocking_queue(size_t queue_size) : q(queue_size) {}
    void push(T const &x) {
        std::unique_lock<std::mutex> lock(mutex);
        while (q.full()) {
            cond.wait(lock);
        }
        q.push_back(x);
        cond.notify_one();
    }
    void push(T &&x) {
        std::unique_lock<std::mutex> lock(mutex);
        while (q.full()) {
            cond.wait(lock);
        }
        q.push_back(x);
        cond.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> lock(mutex);
        while (q.empty()) {
            cond.wait(lock);
        }
        T ret = std::move(q.front());
        q.pop_front();
        cond.notify_one();
        return ret;
    }

private:
    boost::circular_buffer<T> q;
    std::mutex mutex;
    std::condition_variable cond;
};

class thcomp_handler {
public:
    thcomp_handler();
    virtual ~thcomp_handler();
    virtual void compress(std::vector<char> &&x, std::vector<char> &y) = 0;
    virtual void write(std::vector<char> &&x) = 0;
    void input(std::vector<char> &&x);
    void close();
private:
    void comp_thread();
    void io_thread();
    std::vector<std::thread> th;
    blocking_queue<std::vector<char>> q1, q2;
};

#endif //COMPRESS2_THCOMP_H
