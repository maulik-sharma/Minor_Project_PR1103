# Simple HTTP Server (C++)

This is a simple HTTPS server written in C++ using Linux socket APIs currently serving get requests.  
It listens on port `3490`, accepts a client connection, receives a message, and prints it to the console.

Also recieves `POST` requests, returns the request body as response

## TO RUN:
```
make

./server <root_dir> <port>
```
