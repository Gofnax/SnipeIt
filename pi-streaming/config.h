/*
    Configuration File Handler

    This module handles reading configuration from a JSON file.
    Configuration includes video path, loop settings, ports, etc.
 */

#ifndef CONFIG_H
#define CONFIG_H

/* Standard Libraries */
#include <stdbool.h>

// Default configuration file path
#define DEFAULT_CONFIG_PATH "./streaming_config.json"

// Maximum path length
#define MAX_PATH_LENGTH 512

typedef struct
{
    char video_path[MAX_PATH_LENGTH];       // Path to the video file
    char mediamtx_path[MAX_PATH_LENGTH];    // Path to mediaMTX binary
    char mediamtx_config[MAX_PATH_LENGTH];  // Path to mediaMTX config file
    int websocket_port;                     // WebSocket server port (default: 8555)
    int rtsp_port;                          // RTSP server port (default: 8554)
    char rtsp_stream_name[64];              // RTSP stream name (default: "stream")
    bool loop_video;                        // Whether to loop the video
    int detection_frame_interval;           // Process every Nth frame (default: 5)
    double video_duration_sec;              // Video duration in seconds (auto-detected)
    double video_fps;                       // Video FPS (auto-detected)
    int video_width;                        // Video width (auto-detected)
    int video_height;                       // Video height (auto-detected)
} StreamingConfig;

/**
 * @brief   Initialize config with default values.
 * @param   config A pointer to StreamingConfig structure.
 */
void config_init_defaults(StreamingConfig *config);

/**
 * @brief   Load configuration from JSON file.
 * @param   config A pointer to StreamingConfig structure.
 * @param   config_path Path to the JSON configuration file.
 * @returns 0 on success, -1 on error.
 */
int config_load(StreamingConfig *config, const char *config_path);

/**
 * @brief   Probe video file to get duration, FPS, and dimensions.
 * @details Uses ffprobe to extract video metadata.
 * @param   config A pointer to StreamingConfig structure (video_path must be set).
 * @returns 0 on success, -1 on error.
 */
int config_probe_video(StreamingConfig *config);

/**
 * @brief   Print current configuration to stdout.
 * @param   config A pointer to StreamingConfig structure.
 */
void config_print(const StreamingConfig *config);

/**
 * @brief   Validate configuration.
 * @details Checks that required files exist and values are sensible.
 * @param   config A pointer to StreamingConfig structure.
 * @returns 0 if valid, -1 if invalid.
 */
int config_validate(const StreamingConfig *config);

#endif