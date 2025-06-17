#!/usr/bin/env bash

## Flamegraph generating script for performance test

BIN=./build/server-bin                    # Adjust if your server binary is named differently
DURATION=${1:-10}                         # Duration to run perf profiling

echo "Starting server under perf..."
sudo perf record -F 997 -g -- "$BIN" &    # Start server with perf
PID=$!
echo "Server PID=$PID. Profiling for $DURATION seconds..."

# Wait a bit to let server initialize
sleep 4

echo "Running performance tests..."
cd test/performance || exit 1
make run-perf-tests CLIENTS=20

# Wait for remaining profiling time
sleep "$DURATION"

# Gracefully stop the server
echo "Stopping server (PID=$PID)..."
sudo kill -INT "$PID"

# Generate flamegraph
echo "Generating flamegraph..."
cd ../../
mkdir -p ./profiling-data
sudo perf script -i perf.data | \
    perl ./external-tools/FlameGraph/stackcollapse-perf.pl | \
    perl ./external-tools/FlameGraph/flamegraph.pl > ./profiling-data/flame.svg

sudo mv perf.data* ./profiling-data/

echo "Flamegraph written to ./profiling-data/flame.svg"
