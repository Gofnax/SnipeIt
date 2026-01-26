/*
    Pi Streaming Server - Main Orchestrator

    This program coordinates all components:
    - mediaMTX (RTSP server)
    - FFmpeg (video streaming)
    - WebSocket server (Android communication)
    - Unix socket IPC (Python detection communication)

    Flow:
    1. Load configuration
    2. Start mediaMTX
    3. Initialize WebSocket server
    4. Initialize Unix socket IPC
    5. Wait for Python detection script to connect
    6. Wait for Android app to connect (WebSocket)
    7. On Android connect: Start FFmpeg, send START to Python
    8. Forward detection data from Python to Android
    9. On Android disconnect: Stop FFmpeg, send STOP to Python
    10. Loop back to step 6
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <stdbool.h>

#include "config.h"
#include "unix_socket.h"
#include "websocket_server.h"
#include "process_manager.h"

// Global state for signal handler
static volatile bool g_running = true;

// Application state
typedef struct
{
    StreamingConfig config;
    IPCConnection ipc;
    WebSocketServer ws;
    ProcessManager pm;
    bool python_connected;
    bool android_connected;
    bool streaming_active;
} AppState;

// Signal handler for clean shutdown
static void signal_handler(int sig)
{
    (void)sig;
    printf("\n[MAIN] Received shutdown signal\n");
    g_running = false;
}

// Callback when Android connects via WebSocket
static void on_android_connect(void *user_data)
{
    AppState *app = (AppState *)user_data;

    printf("[MAIN] Android client connected!\n");
    app->android_connected = true;

    // Start fresh video stream when Android connects
    if (app->python_connected && !app->streaming_active)
    {
        printf("[MAIN] Starting video stream...\n");

        // Stop any existing FFmpeg (in case of reconnect)
        if (pm_is_ffmpeg_running(&app->pm))
        {
            pm_stop_ffmpeg(&app->pm);
        }

        // Start FFmpeg - video begins from start
        if (pm_start_ffmpeg(&app->pm, &app->config) != 0)
        {
            fprintf(stderr, "[MAIN] Failed to start FFmpeg\n");
            return;
        }

        // Wait for FFmpeg to initialize and start publishing to mediaMTX
        printf("[MAIN] Waiting for stream to initialize...\n");
        usleep(2000000);  // 2 seconds for FFmpeg to start publishing

        // Send START command to Python to begin detections
        if (ipc_send_start(&app->ipc,
                           app->config.video_path,
                           app->config.video_duration_sec,
                           app->config.video_fps,
                           app->config.loop_video,
                           app->config.detection_frame_interval) != 0)
        {
            fprintf(stderr, "[MAIN] Failed to send START to Python\n");
            pm_stop_ffmpeg(&app->pm);
            return;
        }

        app->streaming_active = true;

        // Send stream_ready message to Android
        // Android knows the Pi's IP from the WebSocket connection, so we just send port and stream
        char ready_msg[256];
        snprintf(ready_msg, sizeof(ready_msg),
                 "{\"event\":\"stream_ready\",\"rtsp_port\":%d,\"stream_name\":\"%s\"}",
                 app->config.rtsp_port,
                 app->config.rtsp_stream_name);
        ws_send(&app->ws, ready_msg);
        ws_service(&app->ws, 0);  // Flush immediately

        printf("[MAIN] Streaming started, sent stream_ready to Android\n");
        printf("[MAIN] Android should connect to: rtsp://<PI_IP>:%d/%s\n",
               app->config.rtsp_port, app->config.rtsp_stream_name);
    }
}

// Callback when Android disconnects
static void on_android_disconnect(void *user_data)
{
    AppState *app = (AppState *)user_data;

    printf("[MAIN] Android client disconnected\n");
    app->android_connected = false;

    // Stop streaming completely
    if (app->streaming_active)
    {
        printf("[MAIN] Stopping video stream...\n");

        // Send STOP command to Python
        if (app->python_connected)
        {
            ipc_send_stop(&app->ipc);
        }

        // Stop FFmpeg - will restart fresh when Android reconnects
        pm_stop_ffmpeg(&app->pm);

        app->streaming_active = false;
        printf("[MAIN] Streaming stopped, waiting for new connection\n");
    }
}

// Forward detection data from Python to Android
static void forward_detection(AppState *app, const char *json, size_t len)
{
    if (app->android_connected)
    {
        if (ws_send_json(&app->ws, json, len) != 0)
        {
            fprintf(stderr, "[MAIN] Failed to forward detection to Android\n");
        }
        else
        {
            // IMPORTANT: Flush immediately! ws_send_json uses a single buffer
            // that gets overwritten on each call. We must service the WebSocket
            // to actually send the message before the next one overwrites it.
            ws_service(&app->ws, 0);
        }
    }
}

static void print_usage(const char *program)
{
    printf("Usage: %s [config_file]\n", program);
    printf("\n");
    printf("Arguments:\n");
    printf("  config_file   Path to JSON configuration file\n");
    printf("                (default: ./streaming_config.json)\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s\n", program);
    printf("  %s /path/to/config.json\n", program);
}

int main(int argc, char *argv[])
{
    const char *config_path = DEFAULT_CONFIG_PATH;
    
    // Parse command line arguments
    if (argc > 2)
    {
        print_usage(argv[0]);
        return 1;
    }
    
    if (argc == 2)
    {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            print_usage(argv[0]);
            return 0;
        }
        config_path = argv[1];
    }
    
    printf("==========================================\n");
    printf("  Pi Streaming Server\n");
    printf("==========================================\n\n");
    
    // Initialize application state
    AppState app = {
        .python_connected = false,
        .android_connected = false,
        .streaming_active = false
    };
    
    // Set up signal handlers for clean shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    // Note: SIGCHLD handler set after config_probe_video() to allow pclose() to work

    // Load configuration
    printf("[MAIN] Loading configuration from %s\n", config_path);
    if (config_load(&app.config, config_path) != 0)
    {
        fprintf(stderr, "[MAIN] Failed to load configuration\n");
        return 1;
    }
    
    // Probe video file
    if (config_probe_video(&app.config) != 0)
    {
        fprintf(stderr, "[MAIN] Failed to probe video file\n");
        return 1;
    }
    
    // Validate configuration
    if (config_validate(&app.config) != 0)
    {
        fprintf(stderr, "[MAIN] Configuration validation failed\n");
        return 1;
    }
    
    config_print(&app.config);

    // Now set SIGCHLD to ignore to prevent zombie processes from child processes
    // (mediaMTX, FFmpeg). This must be after config_probe_video() which uses pclose().
    signal(SIGCHLD, SIG_IGN);

    // Initialize process manager
    pm_init(&app.pm);
    
    // Start mediaMTX
    printf("[MAIN] Starting mediaMTX...\n");
    if (pm_start_mediamtx(&app.pm, &app.config) != 0)
    {
        fprintf(stderr, "[MAIN] Failed to start mediaMTX\n");
        return 1;
    }
    
    // Wait for mediaMTX to be ready
    if (pm_wait_for_mediamtx_ready(&app.config, 10) != 0)
    {
        fprintf(stderr, "[MAIN] mediaMTX not ready\n");
        pm_cleanup(&app.pm);
        return 1;
    }
    
    // Initialize WebSocket server
    printf("[MAIN] Initializing WebSocket server on port %d...\n", app.config.websocket_port);
    if (ws_init(&app.ws, app.config.websocket_port) != 0)
    {
        fprintf(stderr, "[MAIN] Failed to initialize WebSocket server\n");
        pm_cleanup(&app.pm);
        return 1;
    }
    
    ws_set_callbacks(&app.ws, on_android_connect, on_android_disconnect, &app);
    
    // Initialize Unix socket IPC
    printf("[MAIN] Initializing IPC server...\n");
    if (ipc_server_init(&app.ipc) != 0)
    {
        fprintf(stderr, "[MAIN] Failed to initialize IPC server\n");
        ws_cleanup(&app.ws);
        pm_cleanup(&app.pm);
        return 1;
    }
    
    // Wait for Python detection script to connect
    printf("[MAIN] Waiting for Python detection script to connect...\n");
    printf("[MAIN] (Run 'python3 detection.py' in another terminal)\n\n");
    
    if (ipc_accept_client(&app.ipc) != 0)
    {
        fprintf(stderr, "[MAIN] Failed to accept Python connection\n");
        ipc_cleanup(&app.ipc);
        ws_cleanup(&app.ws);
        pm_cleanup(&app.pm);
        return 1;
    }
    app.python_connected = true;

    printf("\n[MAIN] System ready!\n");
    printf("[MAIN] Waiting for Android app to connect via WebSocket...\n");
    printf("[MAIN] Android should connect to: ws://<PI_IP>:%d\n", app.config.websocket_port);
    printf("[MAIN] Video stream will be at: rtsp://<PI_IP>:%d/%s\n\n",
           app.config.rtsp_port, app.config.rtsp_stream_name);
    
    // Main event loop
    char msg_buffer[MAX_MSG_SIZE];
    int ws_service_counter = 0;

    while (g_running)
    {
        int did_work = 0;

        // Check for messages from Python FIRST - prioritize IPC throughput
        if (app.python_connected && ipc_check_client_connected(&app.ipc))
        {
            int len;
            int msg_count = 0;
            // Read all available messages to prevent buffer buildup
            while ((len = ipc_recv_message(&app.ipc, msg_buffer, sizeof(msg_buffer))) > 0)
            {
                msg_count++;
                did_work = 1;
                // Forward to Android
                forward_detection(&app, msg_buffer, len);
            }
            if (msg_count > 0)
            {
                printf("[MAIN] Processed %d detection messages\n", msg_count);
            }

            if (len < 0)
            {
                // Python disconnected
                printf("[MAIN] Python detection script disconnected\n");
                app.python_connected = false;

                // Stop streaming if active
                if (app.streaming_active)
                {
                    pm_stop_ffmpeg(&app.pm);
                    app.streaming_active = false;
                }
            }
        }

        // Service WebSocket on fixed interval to not block IPC
        if (++ws_service_counter >= 100)
        {
            ws_service(&app.ws, 0);  // Non-blocking
            ws_service_counter = 0;
        }

        // Small sleep when no IPC work to prevent 100% CPU
        if (!did_work)
        {
            usleep(100);  // 100 microseconds
        }

        // Check if FFmpeg is still running (for non-looping video)
        if (app.streaming_active && !app.config.loop_video)
        {
            pm_check_processes(&app.pm);
            
            if (!pm_is_ffmpeg_running(&app.pm))
            {
                printf("[MAIN] Video playback ended\n");
                
                // Send STOP to Python
                if (app.python_connected)
                {
                    ipc_send_stop(&app.ipc);
                }
                
                app.streaming_active = false;
            }
        }
        
        // Only sleep if no work was done this iteration
        // This ensures we process IPC as fast as possible when data is available
    }
    
    // Cleanup
    printf("\n[MAIN] Shutting down...\n");
    
    // Stop streaming if active
    if (app.streaming_active)
    {
        if (app.python_connected)
        {
            ipc_send_stop(&app.ipc);
        }
        pm_stop_ffmpeg(&app.pm);
    }
    
    ipc_cleanup(&app.ipc);
    ws_cleanup(&app.ws);
    pm_cleanup(&app.pm);
    
    printf("[MAIN] Shutdown complete\n");
    return 0;
}