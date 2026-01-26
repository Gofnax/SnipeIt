#!/usr/bin/env python3
"""
person_streamer.py (IPC-only detector)

Purpose
- Connects as a *client* to the C IPC server (Unix Domain Socket).
- Waits for START/STOP commands (newline-delimited JSON).
- On START: opens the provided video_path (file or RTSP), runs Person-only Object Detection
  on EdgeTPU (Coral USB) using:
    - tflite_runtime + EdgeTPU delegate (libedgetpu.so.1)
    - model: ssd_mobilenet_v2_coco_quant_postprocess_edgetpu.tflite (EdgeTPU compiled)
- Sends detections back to the C server via IPC as newline-delimited JSON in this exact format:
  {
    "type": "target_detection",
    "timestamp_ms": <ms since start of current session/video>,
    "detections": [
      {
        "id": "1",
        "class": "HUMAN",
        "confidence": 0.85,
        "bbox": {"x": 100, "y": 50, "width": 200, "height": 400}
      }
    ]
  }

Notes
- No WebSocket / no app communication here. Only IPC with the Pi C server.
- START command keys used (as in test_ipc.py):
    cmd='start', video_path, duration_sec, fps, loop, frame_interval
- STOP command:
    cmd='stop'
"""

import argparse
import time
from dataclasses import dataclass
from typing import List, Optional, Tuple

import cv2
import numpy as np

from ipc_client import IPCClient, make_detection


# ----------------------------
# Data structures
# ----------------------------
@dataclass
class Detection:
    label: str  # "person"
    score: float
    bbox_xywh: Tuple[int, int, int, int]  # x, y, w, h in pixels


# ----------------------------
# EdgeTPU SSD Person detector (tflite_runtime only; no pycoral)
# ----------------------------
class EdgeTPUPersonDetector:
    def __init__(self, model_path: str, labels_path: Optional[str], threshold: float):
        self.model_path = model_path
        self.labels_path = labels_path
        self.threshold = float(threshold)

        if not self.model_path:
            raise ValueError("Missing --model (EdgeTPU SSD .tflite)")

        try:
            import tflite_runtime.interpreter as tflite
        except ImportError as e:
            raise RuntimeError("tflite_runtime not installed. Install with: pip install tflite-runtime") from e

        self._tflite = tflite

        # Load EdgeTPU delegate (requires EdgeTPU runtime installed)
        try:
            delegate = self._tflite.load_delegate("libedgetpu.so.1")
        except Exception as e:
            raise RuntimeError(
                "Failed to load EdgeTPU delegate 'libedgetpu.so.1'. "
                "Verify EdgeTPU runtime is installed and the Coral USB is connected."
            ) from e

        self.interpreter = self._tflite.Interpreter(
            model_path=self.model_path,
            experimental_delegates=[delegate],
        )
        self.interpreter.allocate_tensors()

        in_details = self.interpreter.get_input_details()[0]
        self._in_index = in_details["index"]
        self._in_dtype = in_details["dtype"]
        in_shape = in_details["shape"]  # [1, H, W, 3]
        self._in_h = int(in_shape[1])
        self._in_w = int(in_shape[2])

        self._out_details = self.interpreter.get_output_details()
        self._boxes_i, self._classes_i, self._scores_i, self._count_i = self._map_outputs()

        self.labels = self._read_labels(self.labels_path) if self.labels_path else {}

        # Resolve person class IDs
        self._person_ids = set()
        for k, v in self.labels.items():
            if str(v).strip().lower() == "person":
                self._person_ids.add(int(k))
        # fallback for COCO conventions (0-based / 1-based)
        if not self._person_ids:
            self._person_ids.update([0, 1])

    def _read_labels(self, path: str) -> dict:
        labels = {}
        with open(path, "r", encoding="utf-8") as f:
            lines = [ln.strip() for ln in f.readlines() if ln.strip() and not ln.strip().startswith("#")]
        for idx, ln in enumerate(lines):
            parts = ln.replace(":", " ").split()
            if len(parts) >= 2 and parts[0].isdigit():
                labels[int(parts[0])] = " ".join(parts[1:])
            else:
                labels[idx] = ln
        return labels

    def _map_outputs(self):
        """
        Typical SSD postprocess outputs:
          0: boxes [1,N,4] (ymin,xmin,ymax,xmax) normalized
          1: classes [1,N]
          2: scores [1,N]
          3: num_detections [1]
        Names often contain 'TFLite_Detection_PostProcess' and suffixes ':1/:2/:3'.
        """
        name_to_i = {d.get("name", ""): i for i, d in enumerate(self._out_details)}

        boxes_i = classes_i = scores_i = count_i = None

        # Prefer name-based mapping
        for name, i in name_to_i.items():
            if "TFLite_Detection_PostProcess" in name:
                if name.endswith("TFLite_Detection_PostProcess") or name.endswith(":0"):
                    boxes_i = i
                elif name.endswith(":1"):
                    classes_i = i
                elif name.endswith(":2"):
                    scores_i = i
                elif name.endswith(":3"):
                    count_i = i

        # Common fallback
        if boxes_i is None and len(self._out_details) == 4:
            boxes_i, classes_i, scores_i, count_i = 0, 1, 2, 3

        # Heuristic: boxes has last dim 4
        if boxes_i is None:
            for i, d in enumerate(self._out_details):
                shape = d.get("shape", [])
                if len(shape) >= 2 and int(shape[-1]) == 4:
                    boxes_i = i
                    break

        return boxes_i, classes_i, scores_i, count_i

    def detect(self, frame_bgr: np.ndarray) -> List[Detection]:
        if frame_bgr is None or frame_bgr.size == 0:
            return []

        orig_h, orig_w = frame_bgr.shape[:2]

        # BGR -> RGB and resize to model input
        rgb = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)
        resized = cv2.resize(rgb, (self._in_w, self._in_h), interpolation=cv2.INTER_LINEAR)

        if self._in_dtype == np.uint8:
            input_tensor = resized.astype(np.uint8)
        else:
            input_tensor = (resized.astype(np.float32) / 255.0)

        input_tensor = np.expand_dims(input_tensor, axis=0)
        self.interpreter.set_tensor(self._in_index, input_tensor)
        self.interpreter.invoke()

        if self._boxes_i is None or self._classes_i is None or self._scores_i is None:
            raise RuntimeError("Could not map SSD outputs for this model.")

        boxes = self.interpreter.get_tensor(self._out_details[self._boxes_i]["index"])
        classes = self.interpreter.get_tensor(self._out_details[self._classes_i]["index"])
        scores = self.interpreter.get_tensor(self._out_details[self._scores_i]["index"])

        if self._count_i is not None:
            count = int(self.interpreter.get_tensor(self._out_details[self._count_i]["index"])[0])
        else:
            count = int(scores.shape[1]) if scores.ndim == 2 else int(scores.shape[0])

        # Unbatch
        if boxes.ndim == 3:
            boxes = boxes[0]
        if classes.ndim == 2:
            classes = classes[0]
        if scores.ndim == 2:
            scores = scores[0]

        n = min(count, len(scores), len(classes), len(boxes))
        dets: List[Detection] = []

        for i in range(n):
            score = float(scores[i])
            if score < self.threshold:
                continue

            cls = int(classes[i])

            # Keep only person
            if cls not in self._person_ids:
                name = self.labels.get(cls, "")
                if str(name).strip().lower() != "person":
                    continue

            # SSD box order: [ymin, xmin, ymax, xmax] normalized
            ymin, xmin, ymax, xmax = [float(x) for x in boxes[i]]

            x1 = int(round(xmin * orig_w))
            y1 = int(round(ymin * orig_h))
            x2 = int(round(xmax * orig_w))
            y2 = int(round(ymax * orig_h))

            # Clamp
            x1 = max(0, min(x1, orig_w - 1))
            y1 = max(0, min(y1, orig_h - 1))
            x2 = max(0, min(x2, orig_w - 1))
            y2 = max(0, min(y2, orig_h - 1))

            w = max(1, x2 - x1)
            h = max(1, y2 - y1)

            dets.append(Detection(label="person", score=score, bbox_xywh=(x1, y1, w, h)))

        dets.sort(key=lambda d: d.score, reverse=True)
        return dets


# ----------------------------
# Video processing loop controlled by IPC commands
# ----------------------------
def process_session(
    client: IPCClient,
    detector: EdgeTPUPersonDetector,
    video_path: str,
    duration_sec: float,
    fps: float,
    loop: bool,
    frame_interval: int,
    realtime: bool,
) -> int:
    """
    Opens the video and sends detections every Nth frame.
    Returns number of detection-messages sent in this session.
    """
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print(f"[person_streamer] ERROR: cannot open video source: {video_path}")
        # notify server with empty detections (optional)
        client.send_detection(0, [])
        return 0

    # Prefer capture-reported FPS if valid
    cap_fps = cap.get(cv2.CAP_PROP_FPS)
    if cap_fps and cap_fps > 1e-3:
        fps_used = cap_fps
    else:
        fps_used = fps if fps and fps > 1e-3 else 30.0

    frame_time_sec = 1.0 / fps_used
    session_start = time.time()

    sent_msgs = 0
    frame_idx = 0

    # For file sources, we can use CAP_PROP_POS_MSEC for timestamp.
    # If it returns 0 always, we fall back to frame_idx / fps.
    while client.is_connected():
        # Check STOP while running
        cmd = client.check_for_command()
        if cmd and cmd.get("cmd") == "stop":
            print("[person_streamer] Received STOP during processing")
            break

        ret, frame = cap.read()
        if not ret:
            if loop:
                cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                frame_idx = 0
                continue
            else:
                break

        if frame_interval <= 0:
            frame_interval = 1

        if (frame_idx % frame_interval) == 0:
            dets = detector.detect(frame)

            # timestamp_ms: time since start of session/video (not epoch)
            ts_ms = int(round(cap.get(cv2.CAP_PROP_POS_MSEC)))
            if ts_ms <= 0:
                ts_ms = int(round((frame_idx / fps_used) * 1000.0))

            detections_payload = []
            for i, d in enumerate(dets):
                x, y, w, h = d.bbox_xywh
                detections_payload.append(
                    make_detection(
                        detection_id=str(i + 1),
                        obj_class="HUMAN",
                        confidence=float(d.score),
                        x=int(x),
                        y=int(y),
                        width=int(w),
                        height=int(h),
                    )
                )

            # IMPORTANT: ipc_client.send_detection wraps into:
            # {type:'target_detection', timestamp_ms:..., detections:[...]}
            client.send_detection(ts_ms, detections_payload)
            sent_msgs += 1

        frame_idx += 1

        if realtime:
            # Sleep to match video FPS - enforce strict timing
            elapsed = time.time() - session_start
            target = frame_idx * frame_time_sec
            sleep_s = target - elapsed
            if sleep_s > 0:
                time.sleep(sleep_s)  # Sleep full amount, don't cap

    cap.release()
    return sent_msgs


def main():
    ap = argparse.ArgumentParser(description="IPC-only EdgeTPU person detection module (no websocket).")
    ap.add_argument("--model", required=True, help="Path to EdgeTPU SSD model (.tflite)")
    ap.add_argument("--labels", default=None, help="Path to labels file (recommended)")
    ap.add_argument("--threshold", type=float, default=0.5, help="Score threshold (default: 0.5)")
    ap.add_argument("--socket-path", default=None, help="Unix socket path override (default: /tmp/detection.sock)")
    ap.add_argument("--realtime", action="store_true", help="Try to pace processing to FPS (recommended)")
    ap.add_argument("--no-realtime", action="store_true", help="Process as fast as possible")
    args = ap.parse_args()

    realtime = True
    if args.no_realtime:
        realtime = False
    if args.realtime:
        realtime = True

    detector = EdgeTPUPersonDetector(args.model, args.labels, args.threshold)

    client = IPCClient(socket_path=args.socket_path) if args.socket_path else IPCClient()
    if not client.connect(timeout=30.0):
        raise SystemExit("[person_streamer] Could not connect to IPC server (C). Is streaming_server running?")

    print("[person_streamer] Connected. Waiting for START/STOP commands...")

    total_msgs = 0
    while client.is_connected():
        cmd = client.wait_for_command(timeout=None)
        if not cmd:
            print("[person_streamer] Connection lost while waiting for command.")
            break

        if cmd.get("cmd") == "start":
            video_path = cmd.get("video_path", "")
            duration_sec = float(cmd.get("duration_sec", 0.0) or 0.0)
            fps = float(cmd.get("fps", 0.0) or 0.0)
            loop = bool(cmd.get("loop", False))
            frame_interval = int(cmd.get("frame_interval", 5))

            print(
                f"[person_streamer] START: video_path={video_path}, "
                f"duration_sec={duration_sec}, fps={fps}, loop={loop}, frame_interval={frame_interval}"
            )

            msgs = process_session(
                client=client,
                detector=detector,
                video_path=video_path,
                duration_sec=duration_sec,
                fps=fps,
                loop=loop,
                frame_interval=frame_interval,
                realtime=realtime,
            )
            total_msgs += msgs
            print(f"[person_streamer] Session finished. Sent {msgs} detection messages. Total={total_msgs}")

        elif cmd.get("cmd") == "stop":
            print("[person_streamer] STOP received (idle).")
            # just continue waiting; server might send a new START
            continue
        else:
            print(f"[person_streamer] Unknown command: {cmd}")

    client.close()
    print("[person_streamer] Exiting.")


if __name__ == "__main__":
    main()
