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
./server <root_dir> <port> [options]

# example
./server content 8080

# silent mode — suppresses all stdout logging (useful for benchmarking)
./server content 8080 -q
./server content 8080 --quiet
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

Benchmark scripts are in `test/`. They compare this server against Python's built-in `http.server` and Apache httpd using ApacheBench (`ab`).

### Install ApacheBench

```bash
# Fedora
sudo dnf install httpd-tools

# Ubuntu/Debian
sudo apt install apache2-utils
```

### 3-way benchmark (C++ vs Python vs Apache)

**Step 1 — Install Apache and configure it to serve `content/` on port 8081** (one-time setup):

```bash
# Ubuntu/Debian
sudo apt install apache2
bash test/setup_apache.sh

# Fedora
sudo dnf install httpd
bash test/setup_apache.sh
```

**Step 2 — Start the C++ and Python servers:**

```bash
./server content 8080 -q &
python3 test/python_server.py &
```

> Apache runs as a systemd service and is already listening on port 8081 after setup.

**Step 3 — Run the benchmark:**

```bash
bash test/run_ab.sh
bash test/compare.sh
```

**Stop servers when done:**

```bash
kill $(lsof -t -i:8080) $(lsof -t -i:9090) 2>/dev/null
sudo systemctl stop apache2
```

### Sample Results

3-way comparison (5000 requests, concurrency 10, localhost, logging disabled):

| Metric | Our C++ Server | Python http.server | Apache httpd 2.4 |
|--------|---------------|--------------------|------------------|
| Requests/sec | ~17,700 | ~6,600 | ~19,000 |
| Mean latency | 0.57 ms | 1.51 ms | 0.51 ms |
| p95 latency | 1 ms | 1 ms | 1 ms |
| p99 latency | 2 ms | 1 ms | 1 ms |
| Failed requests | 0 | 0 | 0 |

The C++ server is ~2.7× faster than Python and within ~7% of Apache httpd, with zero failed requests.

## Notes

- Default port is `3490` if not specified
- Pass `-q` or `--quiet` to suppress all stdout logging
- `POST` returns `409` if the file already exists — use `PATCH` to update
- `PATCH` returns `404` if the file does not exist — use `POST` to create it
- Path traversal attempts (`../`) are blocked with `403 Forbidden`
- Built with C++17
