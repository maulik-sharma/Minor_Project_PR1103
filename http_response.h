#pragma once
#include <string>
#include <map>

class HttpResponse {
public:
    HttpResponse();

    void set_status(int code, const std::string& message);
    void set_header(const std::string& key, const std::string& value);
    void set_body(const std::string& content, const std::string& content_type = "text/plain");
    void clear_body(); 

    // Convenience builders
    static HttpResponse make_200(const std::string& body, const std::string& content_type);
    static HttpResponse make_204();
    static HttpResponse make_400(const std::string& reason);
    static HttpResponse make_403(const std::string& reason);
    static HttpResponse make_404();
    static HttpResponse make_500(const std::string& reason);
    static HttpResponse make_501();


    // Writes the full HTTP response to client_fd
    bool send(int client_fd) const;

private:
    int status_code;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::string body;

    static std::string current_date();
    std::string build() const;
};
