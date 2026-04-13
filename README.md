# Simple HTTP Server (C++)

A custom HTTP/1.1 server written in C++ using Linux socket APIs.  
Supports static file serving, keep-alive connections, and full CRUD operations via HTTP methods.

## Supported Methods

| Method | Behaviour |
|--------|-----------|
| `GET` | Serves a static file from the root directory |
| `POST` | Creates a new file at the given path with the request body as content |
| `PATCH` | Appends the request body to an existing file |
| `DELETE` | Removes the file at the given path |
| `HEAD` | Returns the same headers as GET but with no response body |

## TO RUN
```
make

./server <root_dir> <port>

Ex : ./server content/ 8080
```
## API Usage

```bash
# GET — fetch a file
curl http://localhost:8080/index.html

# HEAD — check headers without downloading the body
curl -I http://localhost:8080/index.html

# POST — create a new file
curl -X POST http://localhost:8080/hello.txt \
     -H "Content-Type: text/plain" \
     -d "Hello, World!"

# PATCH — append to an existing file
curl -X PATCH http://localhost:8080/hello.txt \
     -H "Content-Type: text/plain" \
     -d " More content."

# DELETE — remove a file
curl -X DELETE http://localhost:8080/hello.txt
```

## Custom API Routes

Register custom routes in `main.cpp` before calling `server.start()`:

```cpp
server.add_route("GET", "/api/ping", [](const HttpRequest&) {
    return HttpResponse::make_200("{\"status\":\"ok\"}", "application/json");
});
```

Custom routes take priority over static file serving.

## Notes

- Default port is `3490` if not specified
- `POST` returns `409` if the file already exists — use `PATCH` to update
- `PATCH` returns `404` if the file does not exist — use `POST` to create it
- Path traversal attempts (`../`) are blocked with `403 Forbidden`
- Built with C++17
