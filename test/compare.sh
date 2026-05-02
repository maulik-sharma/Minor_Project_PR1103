#!/bin/bash

RESULTS_DIR="$(dirname "$0")/results"
OURS="$RESULTS_DIR/ours.txt"
APACHE="$RESULTS_DIR/apache.txt"

# make sure benchmarks have been run first
if [[ ! -f "$OURS" || ! -f "$APACHE" ]]; then
    echo "run bash test/run_ab.sh first"
    exit 1
fi

# pull numbers out of ab output
OUR_RPS=$(grep   "Requests per second" "$OURS"   | awk '{print $4}')
OUR_MEAN=$(grep  "Time per request"    "$OURS"   | head -1 | awk '{print $4}')
OUR_P50=$(grep   "  50%"               "$OURS"   | awk '{print $2}')
OUR_P95=$(grep   "  95%"               "$OURS"   | awk '{print $2}')
OUR_P99=$(grep   "  99%"               "$OURS"   | awk '{print $2}')
OUR_FAIL=$(grep  "Failed requests"     "$OURS"   | awk '{print $3}')

AP_RPS=$(grep    "Requests per second" "$APACHE" | awk '{print $4}')
AP_MEAN=$(grep   "Time per request"    "$APACHE" | head -1 | awk '{print $4}')
AP_P50=$(grep    "  50%"               "$APACHE" | awk '{print $2}')
AP_P95=$(grep    "  95%"               "$APACHE" | awk '{print $2}')
AP_P99=$(grep    "  99%"               "$APACHE" | awk '{print $2}')
AP_FAIL=$(grep   "Failed requests"     "$APACHE" | awk '{print $3}')

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║          HTTP Latency Comparison (ApacheBench)           ║"
echo "╠══════════════════════╦═════════════════╦═════════════════╣"
echo "║ Metric               ║   Our C++ Srv   ║  Apache httpd   ║"
echo "╠══════════════════════╬═════════════════╬═════════════════╣"
printf "║ %-20s ║ %15s ║ %15s ║\n" "Requests/sec"    "${OUR_RPS} req/s"  "${AP_RPS} req/s"
printf "║ %-20s ║ %15s ║ %15s ║\n" "Mean latency"    "${OUR_MEAN} ms"    "${AP_MEAN} ms"
printf "║ %-20s ║ %15s ║ %15s ║\n" "p50 latency"     "${OUR_P50} ms"     "${AP_P50} ms"
printf "║ %-20s ║ %15s ║ %15s ║\n" "p95 latency"     "${OUR_P95} ms"     "${AP_P95} ms"
printf "║ %-20s ║ %15s ║ %15s ║\n" "p99 latency"     "${OUR_P99} ms"     "${AP_P99} ms"
printf "║ %-20s ║ %15s ║ %15s ║\n" "Failed requests" "${OUR_FAIL:-0}"    "${AP_FAIL:-0}"
echo "╚══════════════════════╩═════════════════╩═════════════════╝"

echo ""
echo "  lower latency = better  |  higher req/sec = better"
echo ""