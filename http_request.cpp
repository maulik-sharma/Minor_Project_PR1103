#include "http_request.h"
#include <sys/socket.h>
#include <sstream>
#include <iostream>

bool HttpRequest::parse(int client_fd) {
    std::string raw;
    std::string path_raw;
    size_t header_end = std::string::npos;
    size_t path_end;

    while (true) {
        char buffer[2048] = {0};
        int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes == 0) {
            if (raw.empty()) {
                std::cout << "Client closed the connection." << std::endl;
            }
            return false;
        }
        if (bytes < 0) {
            std::cout << "Connection timed out." << std::endl;
            return false;
        }

        raw.append(buffer, bytes);

        // Split request line + headers from body
        header_end = raw.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            break;
        }

        // Prevent memory exhaustion on excessively large headers
        if (raw.size() > 65536) {
            return false;
        }
    }

    std::string header_section = raw.substr(0, header_end);
    std::string body_partial   = raw.substr(header_end + 4);

    // Parse request line
    std::istringstream ss(header_section);
    ss >> method >> path_raw >> version;
    
    path_end = path_raw.find('?');
    if(path_end != std::string::npos){
        path = path_raw.substr(0, path_end);
        parse_query_params(path_raw.substr(path_end + 1));
    }
    else{
        path = path_raw;
    }

    // Skip the rest of the first line, then parse headers
    std::string first_line;
    std::getline(ss, first_line);
    std::string remaining;
    std::getline(ss, remaining, '\0'); // read the rest
    parse_headers(remaining);

    std::cout << "\nMethod: " << method << "  Path: " << path
              << "  Version: " << version << std::endl;
    for (auto& [k, v] : query_params) std::cout << "  " << k << ": " << v << "\n";

    // Read body if Content-Length is set
    if (has_header("Content-Length")) {
        int content_length = std::stoi(get_header("Content-Length"));
        if (!read_body(client_fd, body_partial, content_length))
            return false;
    }

    return true;
}

void HttpRequest::parse_headers(const std::string& raw) {
    std::istringstream ss(raw);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;

        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string key   = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        size_t start = value.find_first_not_of(' ');
        if (start != std::string::npos) value = value.substr(start);

        headers[key] = value;
    }
}

void HttpRequest::parse_query_params(const std::string& query_string) {
    std::istringstream ss(query_string);
    std::string pair;
    while (std::getline(ss, pair, '&')) {
        if (pair.empty()) continue;
        size_t equals = pair.find('=');
        if (equals != std::string::npos) {
            std::string key = pair.substr(0, equals);
            std::string value = pair.substr(equals + 1);
            query_params[key] = value;
        } else {
            query_params[pair] = "";
        }
    }
}

bool HttpRequest::read_body(int client_fd, const std::string& partial, int content_length) {
    body = partial;
    while ((int)body.size() < content_length) {
        char chunk[2048] = {0};
        int bytes = recv(client_fd, chunk, std::min(sizeof(chunk) - 1, (size_t)content_length - (size_t)body.size()), 0);
        if (bytes <= 0) return false;
        body.append(chunk, bytes);
    }
    body = body.substr(0, content_length); // trim any overshoot
    return true;
}

bool HttpRequest::has_header(const std::string& key) const {
    return headers.count(key) > 0;
}

std::string HttpRequest::get_header(const std::string& key) const {
    auto it = headers.find(key);
    return it != headers.end() ? it->second : "";
}

bool HttpRequest::has_query_param(const std::string& key) const{
    return query_params.count(key) > 0;
}

std::string HttpRequest::get_query_param(const std::string& key) const{
    auto it = query_params.find(key);
    return it != query_params.end() ? it->second : "";
}