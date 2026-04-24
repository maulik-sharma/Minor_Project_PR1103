#pragma once
#include "router.h"
#include <string>
#include <thread>
#include <atomic>

class Server {
public:
    // n_threads: how many independent accept() loops to run (one per thread).
    // Defaults to hardware concurrency.
    Server(const std::string& port, const std::string& root_dir,
           int backlog = 10,
           unsigned int n_threads = std::thread::hardware_concurrency());
    ~Server();

    // Register a custom route before calling start()
    void add_route(const std::string& method, const std::string& path, Handler handler);

    // Blocks forever — spawns N worker threads each with their own socket
    void start();

    // Signals all workers to stop accepting
    void stop();

private:
    std::string      port;
    std::string      root_dir;
    int              backlog;
    unsigned int     n_threads;
    Router           router;
    std::atomic<bool> running{false};

    // Creates a listening socket with SO_REUSEADDR + SO_REUSEPORT.
    // Each worker thread calls this independently; the kernel distributes
    // incoming connections across all sockets on the same port.
    int  make_listening_socket() const;

    // The body of each worker thread: accept → TCP_NODELAY → handle, repeat.
    void worker_loop();
};
