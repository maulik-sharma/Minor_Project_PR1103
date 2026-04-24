#include "server.h"
#include "config.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string root_dir = (argc > 1) ? argv[1] : ".";
    std::string port     = (argc > 2) ? argv[2] : "3490";

    // Pass -q or --quiet as any argument to suppress all std::cout logging.
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-q" || arg == "--quiet") {
            g_silent = true;
        }
    }

    Server server(port, root_dir);

    // Register custom API routes here before starting.
    // Static file serving (GET) and POST echo are handled automatically.
    // Example:
    //
    server.add_route("GET", "/api/ping", [](const HttpRequest&) {
        return HttpResponse::make_200("{\"status\":\"ok\"}", "application/json");
    });

    server.add_route("GET", "/api/hello", [](const HttpRequest&) {
        return HttpResponse::make_200("{\"message\":\"hello world\"}", "application/json");
    });

    server.start();
    return 0;
}
