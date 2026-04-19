#!/bin/bash

# runs apachebench against both servers and dumps results to test/results/
# usage: bash test/run_ab.sh  (from project root)
#
# start both servers before running this:
#   ./server content 8080
#   python3 test/python_server.py

OUR_URL="http://localhost:8080/index.html"
PYTHON_URL="http://localhost:9090/index.html"

RESULTS_DIR="$(dirname "$0")/results"
mkdir -p "$RESULTS_DIR"

REQUESTS=5000
CONCURRENCY=10

echo "=================================================="
echo "  HTTP Latency Benchmark — ab"
echo "=================================================="
echo "  Requests    : $REQUESTS"
echo "  Concurrency : $CONCURRENCY"
echo "  File        : index.html"
echo "=================================================="

echo "[1/2] Benchmarking OUR C++ server on port 8080..."
ab -n "$REQUESTS" -c "$CONCURRENCY" -q -H "Connection: close" "$OUR_URL" > "$RESULTS_DIR/ours.txt" 2>&1
FAILED_OURS=$(grep "Failed requests" "$RESULTS_DIR/ours.txt" | awk '{print $3}')
echo "      Failed requests: ${FAILED_OURS:-0}"
echo "      Done. Results saved to results/ours.txt"

echo "[2/2] Benchmarking Python http.server on port 9090..."
ab -n "$REQUESTS" -c "$CONCURRENCY" -q -H "Connection: close" "$PYTHON_URL" > "$RESULTS_DIR/python.txt" 2>&1
FAILED_PY=$(grep "Failed requests" "$RESULTS_DIR/python.txt" | awk '{print $3}')
echo "      Failed requests: ${FAILED_PY:-0}"
echo "      Done. Results saved to results/python.txt"

echo "=================================================="
echo "  Run  bash test/compare.sh  to see the comparison"
echo "=================================================="