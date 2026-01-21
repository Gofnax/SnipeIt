#!/usr/bin/env python3
"""
test_ipc.py

Test script for Unix Domain Socket IPC

This simulates the Python detection script:
1. Connects to the C server
2. Waits for START command
3. Sends detection messages
4. Waits for STOP command
5. Exits

Run:
    First start the C server: ./test_ipc
    Then run this: python3 test_ipc.py
"""

import sys
import time
import random

# Import our IPC client module
from ipc_client import IPCClient, make_detection

VIDEO_FPS = 30

def simulate_detection(frame_num: int):
    """
    Simulate object detection on a frame.
    
    Returns a list of detections (sometimes empty, sometimes multiple).
    """
    detections = []
    
    # Randomly decide how many detections (0-2)
    num_detections = random.choices([0, 1, 2], weights=[0.3, 0.5, 0.2])[0]
    
    for i in range(num_detections):
        # Generate random bounding box
        x = random.randint(50, 500)
        y = random.randint(50, 300)
        width = random.randint(50, 200)
        height = random.randint(100, 300)
        confidence = random.uniform(0.6, 0.99)
        
        detection = make_detection(
            detection_id=str(i + 1),
            obj_class="person",
            confidence=confidence,
            x=x,
            y=y,
            width=width,
            height=height
        )
        detections.append(detection)
    
    return detections


def main():
    print("==========================================")
    print("  IPC Test - Python Client")
    print("==========================================\n")
    
    # Create IPC client
    client = IPCClient()
    
    # Connect to C server
    print("[TEST] Connecting to C server...")
    if not client.connect(timeout=30.0):
        print("[TEST] Failed to connect to server")
        print("[TEST] Make sure './test_ipc' is running first")
        sys.exit(1)
    
    print("[TEST] Connected!")
    
    # Main loop
    running = True
    frame_num = 0
    video_path = None
    processing = False
    
    while running and client.is_connected():
        # Check for commands from C server
        cmd = client.check_for_command()
        
        if cmd:
            if cmd.get('cmd') == 'start':
                video_path = cmd.get('video_path', 'unknown')
                print(f"\n[TEST] Received START command")
                print(f"[TEST] Video path: {video_path}")
                print("[TEST] Starting detection simulation...\n")
                processing = True
                frame_num = 0
                
            elif cmd.get('cmd') == 'stop':
                print("\n[TEST] Received STOP command")
                processing = False
                running = False
                break
        
        # Simulate detection processing
        if processing:
            # Simulate processing every 5th frame (like your 5-8 frame interval)
            if frame_num % 5 == 0:
                # Calculate timestamp (assuming 30fps video)
                timestamp_ms = int(frame_num * (1000 / VIDEO_FPS))
                
                # Get simulated detections
                detections = simulate_detection(frame_num)
                
                # Send detection data
                if detections:
                    print(f"[TEST] Frame {frame_num} (ts={timestamp_ms}ms): "
                          f"Found {len(detections)} detection(s)")
                    client.send_detection(timestamp_ms, detections)
                else:
                    print(f"[TEST] Frame {frame_num} (ts={timestamp_ms}ms): "
                          f"No detections")
                    # Still send message with empty detections
                    client.send_detection(timestamp_ms, [])
            
            frame_num += 1
            
            # Simulate frame processing time
            time.sleep(0.1)  # 100ms per frame for demo
    
    # Cleanup
    print("\n[TEST] Closing connection...")
    client.close()
    
    print("\n==========================================")
    print(f"  Test Complete - Processed {frame_num} frames")
    print("==========================================")


if __name__ == "__main__":
    main()