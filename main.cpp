#include <iostream>
// #include <ostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {

    int s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr= {
        AF_INET,
        htons(8080),
        0
    };

    bind(s, (struct sockaddr *)&addr, sizeof (addr));

    listen(s,10);
    int client_s = accept(s, 0, 0);

    char buffer[256] = {0};
    recv(client_s, buffer, sizeof (buffer), 0);
    std::cout << buffer << std::endl;
    close(client_s);
    close(s);

    return 0;
}