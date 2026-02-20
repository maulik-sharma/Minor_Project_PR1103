# Simple HTTP Server (C++)

This is a simple HTTPS server written in C++ using Linux socket APIs currently serving get requests.  
It listens on port `3490`, accepts a client connection, receives a message, and prints it to the console.

## TO RUN:
```
g++ -pthread simpleStramServer.cpp -o server

./server
```
