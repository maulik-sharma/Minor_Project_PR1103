#!/usr/bin/env python3
"""
Lightweight Python HTTP server for benchmarking comparison.
Serves the same content/ directory as our C++ server.
Run from the project root: python3 test/python_server.py
"""

import http.server
import socketserver
import os
import sys

PORT = 9090
# Serve from content/ directory (relative to project root)
SERVE_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "content")

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=SERVE_DIR, **kwargs)

    # Suppress request logs for cleaner benchmark output
    def log_message(self, format, *args):
        pass

if __name__ == "__main__":
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print(f"Python server running on port {PORT}")
        print(f"Serving from: {os.path.abspath(SERVE_DIR)}")
        print("Press Ctrl+C to stop.")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nStopped.")
            sys.exit(0)
