# scripts/profile_server.sh
#!/usr/bin/env bash

## This is the flamegraph generating script

BIN=./build/server
DURATION=${2:-10}   # seconds

sudo perf record -F 997 -g -- "$BIN" & # For reasons that'll take a while to explain, do NOT make it a multiple of 10.
PID=$!
echo "Server PID=$PID. Profiling for $DURATION s…"

sleep 4 # sleep long enough to start the server

./build/client 

# sleep "$DURATION"
sudo kill -INT "$PID"   # graceful Ctrl-C

mkdir -p ./profiling-data
sudo perf script -i ./perf.data | perl ./external-tools/FlameGraph/stackcollapse-perf.pl | perl ./external-tools/FlameGraph/flamegraph.pl > ./profiling-data/flame.svg
sudo mv perf.data* ./profiling-data/

echo "Flame graph written to ./profiling-data/flame.svg"