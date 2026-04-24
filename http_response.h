#pragma once
#include <string>
#include <map>
#include <sys/types.h>

class HttpResponse {
public:
    HttpResponse();

    void set_status(int code, const std::string& message);
    void set_header(const std::string& key, const std::string& value);
    void set_body(const std::string& content, const std::string& content_type = "text/plain");
    void clear_body(); 

    // Convenience builders
    static HttpResponse make_200(const std::string& body, const std::string& content_type);
    // Zero-copy variant: takes an open fd + file size; send() will use sendfile().
    // The caller must NOT close fd — HttpResponse::send() closes it after use.
    static HttpResponse make_200_sendfile(int fd, off_t size, const std::string& content_type);
    static HttpResponse make_201(const std::string& location);
    static HttpResponse make_204();
    static HttpResponse make_400(const std::string& reason);
    static HttpResponse make_403(const std::string& reason);
    static HttpResponse make_404();
    static HttpResponse make_409(const std::string& reason);
    static HttpResponse make_500(const std::string& reason);
    static HttpResponse make_501();


    // Writes the full HTTP response to client_fd
    bool send(int client_fd) const;

private:
    int status_code;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::string body;

    // When >= 0, send() uses sendfile() instead of writing the body string.
    int   file_fd   = -1;
    off_t file_size = 0;

    static std::string current_date();
    std::string build_headers() const;  // headers only, no body
    std::string build() const;
};
