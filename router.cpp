#include "router.h"
#include <iostream>

void Router::add_route(const std::string& method, const std::string& path, Handler handler) {
    routes[make_key(method, path)] = handler;
}

HttpResponse Router::dispatch(const HttpRequest& req) const {
    std::string key = make_key(req.method, req.path);
    auto it = routes.find(key);
    if (it != routes.end()) {
        return it->second(req);
    }
    return HttpResponse::make_404();
}

std::string Router::make_key(const std::string& method, const std::string& path) {
    return method + " " + path;
}

bool Router::is_route(const std::string& method, const std::string& path) const {
    return routes.find(make_key(method, path)) != routes.end();
}