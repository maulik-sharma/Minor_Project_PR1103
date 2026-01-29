# TCP Server (C++)

This is a simple TCP server written in C++ using Linux socket APIs.  
It listens on port `8080`, accepts a client connection, receives a message, and prints it to the console.

A small Python client is included to test the connection.

## Files

- `main.cpp` – Basic TCP server
- `CMakeLists.txt` – CMake build configuration
- `client.py` – Python client for testing

## How it works

1. Create a TCP socket  
2. Bind to port `8080`  
3. Listen for a connection  
4. Accept one client  
5. Receive and print the message  
6. Close the connection
