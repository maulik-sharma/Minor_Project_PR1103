#pragma once
#include <string>
#include <map>

class HttpRequest {
public:
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> query_params;
    std::string body;

    // Reads from client_fd, parses the full request (including body).
    // Returns false if the connection was closed or timed out.
    bool parse(int client_fd);

    bool has_header(const std::string& key) const;
    bool has_query_param(const std::string& key) const;

    std::string get_header(const std::string& key) const;
    std::string get_query_param(const std::string& key) const;

private:
    void parse_headers(const std::string& raw_headers);
    void parse_query_params(const std::string& query_string);
    bool read_body(int client_fd, const std::string& partial, int content_length);
};
