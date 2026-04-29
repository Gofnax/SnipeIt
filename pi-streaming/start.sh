#!/bin/bash
# Starter script for the full SnipeIt Pi pipeline.
# Run with: ./start.sh    (Ctrl+C to stop everything cleanly)

set -e
cd "$(dirname "$0")"   # always run from pi-streaming/

# ---- 0. Clean up leftovers from a previous run --------------------------------
echo "[start] Cleaning up previous processes..."
pkill -f streaming_server 2>/dev/null || true
pkill -f mediamtx           2>/dev/null || true
pkill -f person_streamer.py 2>/dev/null || true
sleep 2

# ---- 1. Verify Coral USB before doing anything --------------------------------
if ! lsusb | grep -qiE 'global unichip|google'; then
    echo "[start] ERROR: Coral USB not detected. Plug it into a blue USB-3 port and retry."
    exit 1
fi
echo "[start] Coral USB detected."

# ---- 2. Start the C server in the background, log to file ---------------------
echo "[start] Launching streaming_server (logs: /tmp/streaming_server.log)..."
./streaming_server > /tmp/streaming_server.log 2>&1 &
SERVER_PID=$!

# Wait until it's blocked on the IPC accept (= ready for the detector)
for i in $(seq 1 30); do
    if grep -q "Waiting for Python" /tmp/streaming_server.log 2>/dev/null; then
        echo "[start] streaming_server ready (PID $SERVER_PID)"
        break
    fi
    if ! kill -0 "$SERVER_PID" 2>/dev/null; then
        echo "[start] ERROR: streaming_server died early. Last 20 log lines:"
        tail -20 /tmp/streaming_server.log
        exit 1
    fi
    sleep 0.5
done

# ---- 3. Cleanup trap so Ctrl+C kills everything -------------------------------
cleanup() {
    echo
    echo "[start] Stopping..."
    kill "$SERVER_PID" 2>/dev/null || true
    pkill -f mediamtx  2>/dev/null || true
    exit 0
}
trap cleanup INT TERM

# ---- 4. Activate venv & run the Python detector in foreground -----------------
echo "[start] Activating edgetpu venv & starting person_streamer.py..."
echo "[start] (Press Ctrl+C to stop the whole pipeline.)"
source ~/venvs/edgetpu/bin/activate
python3 person_streamer.py \
    --model  /home/snipeit/SnipeIt/pi-streaming/models/ssd_mobilenet_v2_coco_quant_postprocess_edgetpu.tflite \
    --labels /home/snipeit/SnipeIt/pi-streaming/models/coco_labels.txt \
    --threshold 0.5
