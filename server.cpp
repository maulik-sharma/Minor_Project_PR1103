#include "server.h"
#include "config.h"
#include "client_handler.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <iostream>

Server::Server(const std::string& port, const std::string& root_dir, int backlog)
    : port(port), root_dir(root_dir), backlog(backlog) {}

Server::~Server() {
    stop();
}

void Server::add_route(const std::string& method, const std::string& path, Handler handler) {
    router.add_route(method, path, handler);
}

void Server::start() {
    if (!setup_socket()) return;

    if (!g_silent)
        std::cout << "Server listening on port " << port
                  << "  root: " << root_dir << std::endl;

    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    while (true) {
        sin_size = sizeof their_addr;
        int new_fd = accept(sockfd, (sockaddr*)&their_addr, &sin_size);

        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        // Log the connecting IP
        void* addr_ptr = their_addr.ss_family == AF_INET
            ? (void*)&((sockaddr_in*)&their_addr)->sin_addr
            : (void*)&((sockaddr_in6*)&their_addr)->sin6_addr;
        inet_ntop(their_addr.ss_family, addr_ptr, s, sizeof s);
        if (!g_silent)
            std::cout << "Connection from " << s << std::endl;

        // Capture by value so the lambda owns its own copy of new_fd and root_dir
        std::thread([new_fd, this]() {
            ClientHandler handler(new_fd, root_dir, router);
            handler.handle();
        }).detach();
    }
}

void Server::stop() {
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
}

bool Server::setup_socket() {
    struct addrinfo hints{}, *serverinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    int rv;
    if ((rv = getaddrinfo(nullptr, port.c_str(), &hints, &serverinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return false;
    }

    int yes = 1;
    for (p = serverinfo; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) { perror("socket"); continue; }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
            perror("setsockopt"); close(sockfd); continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind"); close(sockfd); continue;
        }

        break;
    }

    freeaddrinfo(serverinfo);

    if (p == nullptr) {
        std::cerr << "Failed to bind." << std::endl;
        return false;
    }

    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        return false;
    }

    return true;
}
