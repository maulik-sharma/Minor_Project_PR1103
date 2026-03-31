#pragma once
#include "http_request.h"
#include "http_response.h"
#include <functional>
#include <map>
#include <string>

using Handler = std::function<HttpResponse(const HttpRequest&)>;

class Router {
public:
    void add_route(const std::string& method, const std::string& path, Handler handler);
    bool is_route(const std::string& method, const std::string& path) const;
    // Returns the appropriate HttpResponse for this request.
    // Falls back to 404 if no route matches.
    HttpResponse dispatch(const HttpRequest& req) const;

private:
    // Key: "METHOD /path"  e.g. "GET /api/ping"
    std::map<std::string, Handler> routes;

    static std::string make_key(const std::string& method, const std::string& path);
};
