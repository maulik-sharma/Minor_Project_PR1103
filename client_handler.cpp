#include "client_handler.h"
#include "http_request.h"
#include "http_response.h"
#include "mime_types.h"
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>

ClientHandler::ClientHandler(int client_fd, const std::string& root_dir, const Router& router)
    : client_fd(client_fd), root_dir(root_dir), router(router) {}

void ClientHandler::handle() {
    // 5-second receive timeout for keep-alive connections
    struct timeval timeout { .tv_sec = 5, .tv_usec = 0 };
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (true) {
        HttpRequest req;
        if (!req.parse(client_fd)) break;

        HttpResponse res;
        if (!validate(req, res)) {
            res.send(client_fd);
            break; // fatal validation errors close the connection
        }

        // Route: custom routes first, then fall back to static file serving
        if(router.is_route(req.method, req.path)){
            res = router.dispatch(req);
        } else if (req.method == "GET") {
            res = serve_file(req);
        } else if (req.method == "POST") {
            res = handle_post(req);
        } else if (req.method == "HEAD") {
            res = handle_head(req);
        } else if (req.method == "DELETE") {
            res = handle_delete(req);
        } else if (req.method == "PATCH") {
            res = handle_patch(req);
        } else {
            res = HttpResponse::make_501();
        }

        res.send(client_fd);

        // Honour "Connection: close"
        if (req.get_header("Connection") == "close") break;
    }

    close(client_fd);
}

bool ClientHandler::validate(const HttpRequest& req, HttpResponse& out) const {
    // RFC 9112: Host header is mandatory in HTTP/1.1
    if (!req.has_header("Host")) {
        out = HttpResponse::make_400("Missing Host header");
        return false;
    }

    // Block path traversal attempts
    if (req.path.find("../") != std::string::npos) {
        out = HttpResponse::make_403("Path traversal not allowed");
        return false;
    }

    return true;
}

HttpResponse ClientHandler::serve_file(const HttpRequest& req) const {
    std::string file_path = root_dir + (req.path == "/" ? "/index.html" : req.path);

    std::ifstream file(file_path, std::ios::binary);
    if (!file.good())
        return HttpResponse::make_404();

    std::ostringstream buf;
    buf << file.rdbuf();
    std::string body = buf.str();

    return HttpResponse::make_200(body, MimeTypes::get(file_path));
}

HttpResponse ClientHandler::handle_post(const HttpRequest& req) const {
    if (!req.has_header("Content-Length"))
        return HttpResponse::make_400("Missing Content-Length header");

    std::cout << "\n--- POST body ---\n" << req.body << "\n-----------------" << std::endl;

    // Echo body back — replace this with real logic
    return HttpResponse::make_200(req.body, "text/plain");
}

HttpResponse ClientHandler::handle_head(const HttpRequest& req) const {
    // Generate the same response as GET but strip the body before sending.
    // We reuse serve_file() so headers (Content-Type, Content-Length) are identical.
    HttpResponse res = serve_file(req);
    res.clear_body();
    return res;
}

HttpResponse ClientHandler::handle_delete(const HttpRequest& req) const {
    std::string file_path = root_dir + req.path;

    // stat the file first so we can give a proper 404
    struct stat st;
    if (stat(file_path.c_str(), &st) != 0)
        return HttpResponse::make_404();

    // Only allow deleting regular files, not directories
    if (!S_ISREG(st.st_mode))
        return HttpResponse::make_403("Cannot delete a directory");

    if (remove(file_path.c_str()) != 0)
        return HttpResponse::make_500("Failed to delete file");

    return HttpResponse::make_204();
}

HttpResponse ClientHandler::handle_patch(const HttpRequest& req) const {
    if (!req.has_header("Content-Length"))
        return HttpResponse::make_400("Missing Content-Length header");

    std::string file_path = root_dir + req.path;

    // PATCH requires the resource to already exist (use PUT to create)
    struct stat st;
    if (stat(file_path.c_str(), &st) != 0)
        return HttpResponse::make_404();

    // Append the request body to the existing file
    std::ofstream file(file_path, std::ios::app | std::ios::binary);
    if (!file.is_open())
        return HttpResponse::make_500("Could not open file for writing");

    file << req.body;
    file.close();

    return HttpResponse::make_204();
}