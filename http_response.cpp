#include "http_response.h"
#include <sys/socket.h>
#include <ctime>
#include <sstream>
#include <iostream>

HttpResponse::HttpResponse() : status_code(200), status_message("OK") {
    headers["Server"] = "Maulik-Bobby-Server/2.0";
    headers["Date"] = current_date();
}

void HttpResponse::set_status(int code, const std::string& message) {
    status_code = code;
    status_message = message;
}

void HttpResponse::set_header(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void HttpResponse::set_body(const std::string& content, const std::string& content_type) {
    body = content;
    headers["Content-Type"]   = content_type;
    headers["Content-Length"] = std::to_string(content.size());
}

HttpResponse HttpResponse::make_200(const std::string& body, const std::string& content_type) {
    HttpResponse r;
    r.set_status(200, "OK");
    r.set_body(body, content_type);
    return r;
}

HttpResponse HttpResponse::make_201(const std::string& location) {
    HttpResponse r;
    r.set_status(201, "Created");
    r.set_header("Location", location);
    r.set_body("201 Created: " + location, "text/plain");
    return r;
}

HttpResponse HttpResponse::make_204() {
    HttpResponse r;
    r.set_status(204, "No Content");
    return r;
}

HttpResponse HttpResponse::make_400(const std::string& reason) {
    HttpResponse r;
    r.set_status(400, "Bad Request");
    r.set_body("400 Bad Request: " + reason, "text/plain");
    return r;
}

HttpResponse HttpResponse::make_403(const std::string& reason) {
    HttpResponse r;
    r.set_status(403, "Forbidden");
    r.set_body("403 Forbidden: " + reason, "text/plain");
    return r;
}

HttpResponse HttpResponse::make_404() {
    HttpResponse r;
    r.set_status(404, "Not Found");
    r.set_body("404 Not Found", "text/plain");
    return r;
}

HttpResponse HttpResponse::make_500(const std::string& reason) {
    HttpResponse r;
    r.set_status(500, "Internal Server Error");
    r.set_body("500 Internal Server Error: " + reason, "text/plain");
    return r;
}

HttpResponse HttpResponse::make_501() {
    HttpResponse r;
    r.set_status(501, "Not Implemented");
    r.set_body("501 Not Implemented: ", "text/plain");
    return r;
}

// --- Send & build ---

bool HttpResponse::send(int client_fd) const {
    std::string raw = build();
    if (::send(client_fd, raw.c_str(), raw.size(), 0) == -1) {
        perror("send");
        return false;
    }
    return true;
}

std::string HttpResponse::build() const {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
    for (auto& [k, v] : headers)
        ss << k << ": " << v << "\r\n";
    ss << "\r\n" << body;
    return ss.str();
}

std::string HttpResponse::current_date() {
    time_t now = time(nullptr);
    struct tm tm = *gmtime(&now);
    char buf[128];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return std::string(buf);
}

void HttpResponse::clear_body() {
    body = "";
    headers["Content-Length"] = "0";
}