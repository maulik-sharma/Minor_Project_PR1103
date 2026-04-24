#!/bin/bash

RESULTS_DIR="$(dirname "$0")/results"
OURS="$RESULTS_DIR/ours.txt"
PYTHON="$RESULTS_DIR/python.txt"
APACHE="$RESULTS_DIR/apache.txt"

# make sure benchmarks have been run first
if [[ ! -f "$OURS" || ! -f "$PYTHON" ]]; then
    echo "run bash test/run_ab.sh first"
    exit 1
fi

HAS_APACHE=false
[[ -f "$APACHE" ]] && HAS_APACHE=true

# pull numbers out of ab output
OUR_RPS=$(grep   "Requests per second" "$OURS"   | awk '{print $4}')
OUR_MEAN=$(grep  "Time per request"    "$OURS"   | head -1 | awk '{print $4}')
OUR_P50=$(grep   "  50%"               "$OURS"   | awk '{print $2}')
OUR_P95=$(grep   "  95%"               "$OURS"   | awk '{print $2}')
OUR_P99=$(grep   "  99%"               "$OURS"   | awk '{print $2}')
OUR_FAIL=$(grep  "Failed requests"     "$OURS"   | awk '{print $3}')

PY_RPS=$(grep    "Requests per second" "$PYTHON" | awk '{print $4}')
PY_MEAN=$(grep   "Time per request"    "$PYTHON" | head -1 | awk '{print $4}')
PY_P50=$(grep    "  50%"               "$PYTHON" | awk '{print $2}')
PY_P95=$(grep    "  95%"               "$PYTHON" | awk '{print $2}')
PY_P99=$(grep    "  99%"               "$PYTHON" | awk '{print $2}')
PY_FAIL=$(grep   "Failed requests"     "$PYTHON" | awk '{print $3}')

if $HAS_APACHE; then
    AP_RPS=$(grep    "Requests per second" "$APACHE" | awk '{print $4}')
    AP_MEAN=$(grep   "Time per request"    "$APACHE" | head -1 | awk '{print $4}')
    AP_P50=$(grep    "  50%"               "$APACHE" | awk '{print $2}')
    AP_P95=$(grep    "  95%"               "$APACHE" | awk '{print $2}')
    AP_P99=$(grep    "  99%"               "$APACHE" | awk '{print $2}')
    AP_FAIL=$(grep   "Failed requests"     "$APACHE" | awk '{print $3}')

    echo ""
    echo "╔══════════════════════════════════════════════════════════════════════════╗"
    echo "║              HTTP Latency Comparison (ApacheBench)                      ║"
    echo "╠══════════════════════╦═════════════════╦═════════════════╦═════════════╣"
    echo "║ Metric               ║   Our C++ Srv   ║  Python Server  ║ Apache httpd║"
    echo "╠══════════════════════╬═════════════════╬═════════════════╬═════════════╣"
    printf "║ %-20s ║ %15s ║ %15s ║ %11s ║\n" "Requests/sec"    "${OUR_RPS} req/s"  "${PY_RPS} req/s"  "${AP_RPS} req/s"
    printf "║ %-20s ║ %15s ║ %15s ║ %11s ║\n" "Mean latency"    "${OUR_MEAN} ms"    "${PY_MEAN} ms"    "${AP_MEAN} ms"
    printf "║ %-20s ║ %15s ║ %15s ║ %11s ║\n" "p50 latency"     "${OUR_P50} ms"     "${PY_P50} ms"     "${AP_P50} ms"
    printf "║ %-20s ║ %15s ║ %15s ║ %11s ║\n" "p95 latency"     "${OUR_P95} ms"     "${PY_P95} ms"     "${AP_P95} ms"
    printf "║ %-20s ║ %15s ║ %15s ║ %11s ║\n" "p99 latency"     "${OUR_P99} ms"     "${PY_P99} ms"     "${AP_P99} ms"
    printf "║ %-20s ║ %15s ║ %15s ║ %11s ║\n" "Failed requests" "${OUR_FAIL:-0}"    "${PY_FAIL:-0}"    "${AP_FAIL:-0}"
    echo "╚══════════════════════╩═════════════════╩═════════════════╩═════════════╝"
else
    echo ""
    echo "╔══════════════════════════════════════════════════════════╗"
    echo "║          HTTP Latency Comparison (ApacheBench)           ║"
    echo "╠══════════════════════╦═════════════════╦═════════════════╣"
    echo "║ Metric               ║   Our C++ Srv   ║  Python Server  ║"
    echo "╠══════════════════════╬═════════════════╬═════════════════╣"
    printf "║ %-20s ║ %15s ║ %15s ║\n" "Requests/sec"    "${OUR_RPS} req/s"  "${PY_RPS} req/s"
    printf "║ %-20s ║ %15s ║ %15s ║\n" "Mean latency"    "${OUR_MEAN} ms"    "${PY_MEAN} ms"
    printf "║ %-20s ║ %15s ║ %15s ║\n" "p50 latency"     "${OUR_P50} ms"     "${PY_P50} ms"
    printf "║ %-20s ║ %15s ║ %15s ║\n" "p95 latency"     "${OUR_P95} ms"     "${PY_P95} ms"
    printf "║ %-20s ║ %15s ║ %15s ║\n" "p99 latency"     "${OUR_P99} ms"     "${PY_P99} ms"
    printf "║ %-20s ║ %15s ║ %15s ║\n" "Failed requests" "${OUR_FAIL:-0}"    "${PY_FAIL:-0}"
    echo "╚══════════════════════╩═════════════════╩═════════════════╝"
fi

echo ""
echo "  lower latency = better  |  higher req/sec = better"
echo ""