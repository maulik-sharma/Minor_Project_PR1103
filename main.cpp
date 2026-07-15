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

    server.add_route("GET", "/api/server-info", [&port](const HttpRequest&) {
        std::string json =
            "{"
            "\"server\":\"Maulik-Bobby-Server/2.0\","
            "\"language\":\"C++17\","
            "\"port\":\"" + port + "\","
            "\"methods\":[\"GET\",\"POST\",\"PATCH\",\"DELETE\",\"HEAD\"],"
            "\"features\":[\"SO_REUSEPORT\",\"sendfile zero-copy\",\"TCP_NODELAY\",\"keep-alive\",\"custom routing\"],"
            "\"routes\":[\"/api/ping\",\"/api/server-info\",\"/api/echo\"]"
            "}";
        return HttpResponse::make_200(json, "application/json");
    });

    server.add_route("POST", "/api/echo", [](const HttpRequest& req) {
        return HttpResponse::make_200(req.body, "application/json");
    });

    server.start();
    return 0;
}
