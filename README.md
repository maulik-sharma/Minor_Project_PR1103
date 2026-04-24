# Simple HTTP Server (C++)

A custom HTTP/1.1 server written in C++ using Linux socket APIs.  
Supports static file serving, keep-alive connections, and full CRUD operations via HTTP methods.

## Requirements

- Linux (tested on Fedora)
- g++ with C++17 support
- make

Install dependencies on Fedora:
```bash
sudo dnf install gcc-c++ make
```

On Ubuntu/Debian:
```bash
sudo apt install g++ make
```

## Build

```bash
git clone <repo-url>
cd <repo-folder>
make
```

## Run

```bash
./server <root_dir> <port>

# example
./server content 8080
```

## Supported Methods

| Method | Behaviour |
|--------|-----------|
| `GET` | Serves a static file from the root directory |
| `POST` | Creates a new file at the given path with the request body as content |
| `PATCH` | Appends the request body to an existing file |
| `DELETE` | Removes the file at the given path |
| `HEAD` | Returns the same headers as GET but with no response body |

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

## Benchmarking

Benchmark scripts are in `test/`. They compare this server against Python's built-in `http.server` and Apache httpd.

### vs Python http.server

Install ApacheBench first:

```bash
# Fedora
sudo dnf install httpd-tools

# Ubuntu/Debian
sudo apt install apache2-utils
```

Start both servers:

```bash
./server content 8080 &
python3 -m http.server 9090 --directory content &
```

Run the benchmark:

```bash
bash test/run_ab.sh
bash test/compare.sh
```

Kill both servers when done:

```bash
kill $(lsof -t -i:8080) $(lsof -t -i:9090) 2>/dev/null
```

### vs Apache httpd

Install and start Apache:

```bash
# Fedora
sudo dnf install httpd

# Ubuntu/Debian
sudo apt install apache2
```

Set up equivalent API endpoints on Apache:

```bash
sudo mkdir -p /var/www/html/api
echo '{"status":"ok"}' | sudo tee /var/www/html/api/ping
echo '{"message":"hello world"}' | sudo tee /var/www/html/api/hello
sudo systemctl start httpd
```

Start the C++ server and benchmark:

```bash
./server content 8080 &
ab -n 5000 -c 10 -H "Connection: close" http://localhost:8080/api/ping
ab -n 5000 -c 10 -H "Connection: close" http://localhost:80/api/ping
```

Stop Apache when done:

```bash
sudo systemctl stop httpd
```

### Sample Results

**vs Python http.server** (static file serving):

| Metric | Our C++ Server | Python Server |
|--------|---------------|---------------|
| Requests/sec | 8449 | 1993 |
| Mean latency | 1.18 ms | 5.01 ms |
| p99 latency | 3 ms | 8 ms |

**vs Apache httpd** (API endpoint):

| Metric | Our C++ Server | Apache 2.4.66 |
|--------|---------------|---------------|
| Requests/sec | 4539 | 4392 |
| Mean latency | 2.20 ms | 2.27 ms |
| p99 latency | 5 ms | 3 ms |

## Notes

- Default port is `3490` if not specified
- `POST` returns `409` if the file already exists — use `PATCH` to update
- `PATCH` returns `404` if the file does not exist — use `POST` to create it
- Path traversal attempts (`../`) are blocked with `403 Forbidden`
- Built with C++17
