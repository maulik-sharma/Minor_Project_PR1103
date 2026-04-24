#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

// A fixed-size pool of worker threads.
// Enqueue an int (client fd) and the pool will call handler_fn(fd)
// on the next available worker — no thread creation per request.
class ThreadPool {
public:
    ThreadPool(size_t n_threads, std::function<void(int)> handler_fn)
        : handler(std::move(handler_fn)), stop(false)
    {
        workers.reserve(n_threads);
        for (size_t i = 0; i < n_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    int fd;
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this] {
                            return stop.load() || !tasks.empty();
                        });
                        if (stop && tasks.empty()) return;
                        fd = tasks.front();
                        tasks.pop();
                    }
                    handler(fd);
                }
            });
        }
    }

    ~ThreadPool() {
        stop = true;
        cv.notify_all();
        for (auto& t : workers) t.join();
    }

    void enqueue(int client_fd) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(client_fd);
        }
        cv.notify_one();
    }

private:
    std::vector<std::thread>      workers;
    std::queue<int>               tasks;
    std::mutex                    mtx;
    std::condition_variable       cv;
    std::function<void(int)>      handler;
    std::atomic<bool>             stop;
};
