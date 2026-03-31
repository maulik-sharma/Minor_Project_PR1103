#pragma once
#include "router.h"
#include <string>

class Server {
public:
    Server(const std::string& port, const std::string& root_dir, int backlog = 10);
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

    bool setup_socket();
};
