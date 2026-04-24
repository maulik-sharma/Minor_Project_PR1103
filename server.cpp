#include "server.h"
#include "config.h"
#include "client_handler.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <cstring>
#include <iostream>

Server::Server(const std::string& port, const std::string& root_dir,
               int backlog, unsigned int n_threads)
    : port(port), root_dir(root_dir), backlog(backlog),
      n_threads(n_threads ? n_threads : 4) {}

Server::~Server() {
    stop();
}

void Server::add_route(const std::string& method, const std::string& path, Handler handler) {
    router.add_route(method, path, handler);
}

// ---------------------------------------------------------------------------
// make_listening_socket
//
// Called by every worker thread independently. Each thread gets its own fd
// bound to the same port via SO_REUSEPORT — the kernel then distributes
// incoming connections across all of them with no userspace locking.
// ---------------------------------------------------------------------------
int Server::make_listening_socket() const {
    struct addrinfo hints{}, *serverinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    int rv;
    if ((rv = getaddrinfo(nullptr, port.c_str(), &hints, &serverinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return -1;
    }

    int fd = -1;
    int yes = 1;
    for (p = serverinfo; p != nullptr; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd == -1) { perror("socket"); continue; }

        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
        setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof yes);

        if (bind(fd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind"); close(fd); fd = -1; continue;
        }
        break;
    }
    freeaddrinfo(serverinfo);

    if (fd == -1) {
        std::cerr << "Failed to bind on port " << port << std::endl;
        return -1;
    }

    if (listen(fd, backlog) == -1) {
        perror("listen"); close(fd); return -1;
    }

    return fd;
}

// ---------------------------------------------------------------------------
// worker_loop
//
// Each thread runs this independently:
//   1. Create its own listening socket with SO_REUSEPORT
//   2. Loop: accept → set TCP_NODELAY → handle the connection
//
// No shared queue, no mutex, no condition variable on the hot path.
// The kernel picks which thread's socket receives each new connection.
// ---------------------------------------------------------------------------
void Server::worker_loop() {
    int listen_fd = make_listening_socket();
    if (listen_fd == -1) return;

    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    while (running) {
        sin_size = sizeof their_addr;
        int client_fd = accept(listen_fd, (sockaddr*)&their_addr, &sin_size);

        if (client_fd == -1) {
            if (!running) break;
            perror("accept");
            continue;
        }

        // Disable Nagle's algorithm: send small responses immediately
        // without waiting to fill a full TCP segment.
        int flag = 1;
        setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof flag);

        if (!g_silent) {
            void* addr_ptr = their_addr.ss_family == AF_INET
                ? (void*)&((sockaddr_in*)&their_addr)->sin_addr
                : (void*)&((sockaddr_in6*)&their_addr)->sin6_addr;
            inet_ntop(their_addr.ss_family, addr_ptr, s, sizeof s);
            std::cout << "Connection from " << s << std::endl;
        }

        ClientHandler handler(client_fd, root_dir, router);
        handler.handle();
    }

    close(listen_fd);
}

// ---------------------------------------------------------------------------
// start / stop
// ---------------------------------------------------------------------------

void Server::start() {
    running = true;

    if (!g_silent)
        std::cout << "Server listening on port " << port
                  << "  root: " << root_dir
                  << "  threads: " << n_threads
                  << "  [SO_REUSEPORT + O2]" << std::endl;

    // Spawn N-1 worker threads; the main thread becomes the Nth worker.
    std::vector<std::thread> workers;
    workers.reserve(n_threads - 1);
    for (unsigned int i = 1; i < n_threads; ++i)
        workers.emplace_back([this] { worker_loop(); });

    worker_loop();  // main thread works too

    for (auto& t : workers) t.join();
}

void Server::stop() {
    running = false;
    // Worker threads blocked in accept() will be unblocked when the process
    // receives a signal (e.g. SIGTERM/Ctrl-C), which closes all fds.
}
