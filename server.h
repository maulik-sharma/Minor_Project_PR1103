#pragma once
#include "router.h"
#include "thread_pool.h"
#include <string>
#include <memory>

class Server {
public:
    // n_threads: worker pool size — defaults to hardware concurrency
    Server(const std::string& port, const std::string& root_dir,
           int backlog = 10,
           unsigned int n_threads = std::thread::hardware_concurrency());
    ~Server();

    // Register a custom route before calling start()
    void add_route(const std::string& method, const std::string& path, Handler handler);

    // Blocks forever — sets up the socket and runs the accept loop
    void start();

    // Closes the listening socket
    void stop();

private:
    std::string port;
    std::string root_dir;
    int         backlog;
    int         sockfd = -1;
    Router      router;

    std::unique_ptr<ThreadPool> pool;
    unsigned int n_threads;

    bool setup_socket();
};
