#include "server.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::string root_dir = (argc > 1) ? argv[1] : ".";
    std::string port     = (argc > 2) ? argv[2] : "3490";

    Server server(port, root_dir);

    // Register custom API routes here before starting.
    // Static file serving (GET) and POST echo are handled automatically.
    // Example:
    //
    server.add_route("GET", "/api/ping", [](const HttpRequest&) {
        return HttpResponse::make_200("{\"status\":\"ok\"}", "application/json");
    });

    server.start();
    return 0;
}
