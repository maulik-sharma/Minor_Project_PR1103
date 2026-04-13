#pragma once
#include "router.h"
#include <string>

class ClientHandler {
public:
    ClientHandler(int client_fd, const std::string& root_dir, const Router& router);

    // Called from the worker thread. Runs the keep-alive loop until the
    // client disconnects or sends "Connection: close".
    void handle();

private:
    int client_fd;
    std::string root_dir;
    const Router& router;

    // Validates the request and builds an error response if needed.
    // Returns true if the request passed all checks.
    bool validate(const HttpRequest& req, HttpResponse& out) const;

    // Serves a file from root_dir for GET requests.
    HttpResponse serve_file(const HttpRequest& req) const;

    // Handles POST — echoes body back (placeholder for real logic).
    HttpResponse handle_post(const HttpRequest& req) const;

    // Handles HEAD requests by returning the same headers as GET but no body.
    HttpResponse handle_head(const HttpRequest& req) const;

    // Handles DELETE requests by removing the file.
    HttpResponse handle_delete(const HttpRequest& req) const;

    // Handles PATCH requests by appending the body to the file.
    HttpResponse handle_patch(const HttpRequest& req) const;
};
