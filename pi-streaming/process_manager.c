#include "process_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void pm_init(ProcessManager *pm)
{
    *pm = (ProcessManager){
        .mediamtx_pid = 0,
        .ffmpeg_pid = 0,
        .mediamtx_running = false,
        .ffmpeg_running = false
    };
}

int pm_start_mediamtx(ProcessManager *pm, const StreamingConfig *config)
{
    if (pm->mediamtx_running)
    {
        printf("[PM] mediaMTX is already running (PID: %d)\n", pm->mediamtx_pid);
        return 0;
    }
    
    printf("[PM] Starting mediaMTX...\n");
    
    pid_t pid = fork();
    
    if (pid < 0)
    {
        perror("[PM] fork failed for mediaMTX");
        return -1;
    }
    
    if (pid == 0)
    {
        // Child process - exec mediaMTX
        
        // Redirect stdout/stderr to /dev/null or log file
        // (mediaMTX is quite verbose)
        freopen("/tmp/mediamtx.log", "w", stdout);
        freopen("/tmp/mediamtx.log", "a", stderr);
        
        // Execute mediaMTX
        execl(config->mediamtx_path, 
              config->mediamtx_path,
              config->mediamtx_config,
              (char *)NULL);
        
        // If exec fails
        perror("[PM] execl mediaMTX failed");
        _exit(127);
    }
    
    // Parent process
    pm->mediamtx_pid = pid;
    pm->mediamtx_running = true;
    
    printf("[PM] mediaMTX started (PID: %d)\n", pm->mediamtx_pid);
    
    return 0;
}

void pm_stop_mediamtx(ProcessManager *pm)
{
    if (!pm->mediamtx_running || pm->mediamtx_pid <= 0)
    {
        return;
    }
    
    printf("[PM] Stopping mediaMTX (PID: %d)...\n", pm->mediamtx_pid);
    
    // Send SIGTERM first (graceful shutdown)
    if (kill(pm->mediamtx_pid, SIGTERM) == 0)
    {
        // Wait up to 3 seconds for graceful exit
        int wait_count = 0;
        while (wait_count < 30)
        {
            int status;
            pid_t result = waitpid(pm->mediamtx_pid, &status, WNOHANG);
            
            if (result == pm->mediamtx_pid)
            {
                // Process exited
                printf("[PM] mediaMTX stopped gracefully\n");
                pm->mediamtx_pid = 0;
                pm->mediamtx_running = false;
                return;
            }
            
            usleep(100000); // 100ms
            wait_count++;
        }
        
        // Still running, force kill
        printf("[PM] mediaMTX not responding, sending SIGKILL\n");
        kill(pm->mediamtx_pid, SIGKILL);
        waitpid(pm->mediamtx_pid, NULL, 0);
    }
    
    pm->mediamtx_pid = 0;
    pm->mediamtx_running = false;
    printf("[PM] mediaMTX stopped\n");
}

bool pm_is_mediamtx_running(ProcessManager *pm)
{
    pm_check_processes(pm);
    return pm->mediamtx_running;
}

int pm_start_ffmpeg(ProcessManager *pm, const StreamingConfig *config)
{
    if (pm->ffmpeg_running)
    {
        printf("[PM] FFmpeg is already running (PID: %d)\n", pm->ffmpeg_pid);
        return 0;
    }
    
    printf("[PM] Starting FFmpeg stream...\n");
    
    pid_t pid = fork();
    
    if (pid < 0)
    {
        perror("[PM] fork failed for FFmpeg");
        return -1;
    }
    
    if (pid == 0)
    {
        // Child process - exec FFmpeg
        
        // Redirect stdout/stderr to log file
        freopen("/tmp/ffmpeg.log", "w", stdout);
        freopen("/tmp/ffmpeg.log", "a", stderr);
        
        // Build RTSP URL
        char rtsp_url[256];
        snprintf(rtsp_url, sizeof(rtsp_url), "rtsp://localhost:%d/%s",
                 config->rtsp_port, config->rtsp_stream_name);
        
        // Build loop argument if needed
        // Note: -stream_loop -1 means infinite loop
        // Re-encode with frequent keyframes to fix green screen on late-joining clients
        // Use baseline profile for maximum Android compatibility
        if (config->loop_video)
        {
            execl("/usr/bin/ffmpeg", "ffmpeg",
                  "-re",                        // Read at native framerate
                  "-stream_loop", "-1",         // Loop infinitely
                  "-i", config->video_path,     // Input file
                  "-c:v", "libx264",            // Re-encode video with H.264
                  "-profile:v", "baseline",     // Baseline profile for Android
                  "-level", "4.0",              // Level 4.0 for 1080p compatibility
                  "-preset", "ultrafast",       // Fastest encoding (low CPU)
                  "-tune", "zerolatency",       // Minimize latency
                  "-pix_fmt", "yuv420p",        // Explicit pixel format for compatibility
                  "-g", "30",                   // Keyframe every 30 frames (~1 sec)
                  "-keyint_min", "30",          // Minimum keyframe interval
                  "-b:v", "2M",                 // Video bitrate (some decoders need this)
                  "-maxrate", "2M",             // Max bitrate
                  "-bufsize", "4M",             // Buffer size
                  "-an",                        // No audio (source has none)
                  "-f", "rtsp",                 // Output format
                  "-rtsp_transport", "tcp",     // Use TCP for RTSP
                  rtsp_url,                     // Output URL
                  (char *)NULL);
        }
        else
        {
            execl("/usr/bin/ffmpeg", "ffmpeg",
                  "-re",                        // Read at native framerate
                  "-i", config->video_path,     // Input file
                  "-c:v", "libx264",            // Re-encode video with H.264
                  "-profile:v", "baseline",     // Baseline profile for Android
                  "-level", "4.0",              // Level 4.0 for 1080p compatibility
                  "-preset", "ultrafast",       // Fastest encoding (low CPU)
                  "-tune", "zerolatency",       // Minimize latency
                  "-pix_fmt", "yuv420p",        // Explicit pixel format for compatibility
                  "-g", "30",                   // Keyframe every 30 frames (~1 sec)
                  "-keyint_min", "30",          // Minimum keyframe interval
                  "-b:v", "2M",                 // Video bitrate (some decoders need this)
                  "-maxrate", "2M",             // Max bitrate
                  "-bufsize", "4M",             // Buffer size
                  "-an",                        // No audio (source has none)
                  "-f", "rtsp",                 // Output format
                  "-rtsp_transport", "tcp",     // Use TCP for RTSP
                  rtsp_url,                     // Output URL
                  (char *)NULL);
        }
        
        // If exec fails
        perror("[PM] execl ffmpeg failed");
        _exit(127);
    }
    
    // Parent process
    pm->ffmpeg_pid = pid;
    pm->ffmpeg_running = true;
    
    printf("[PM] FFmpeg started (PID: %d), streaming to RTSP port %d\n", 
           pm->ffmpeg_pid, config->rtsp_port);
    
    return 0;
}

void pm_stop_ffmpeg(ProcessManager *pm)
{
    if (!pm->ffmpeg_running || pm->ffmpeg_pid <= 0)
    {
        return;
    }
    
    printf("[PM] Stopping FFmpeg (PID: %d)...\n", pm->ffmpeg_pid);
    
    // Send SIGTERM first
    if (kill(pm->ffmpeg_pid, SIGTERM) == 0)
    {
        // Wait up to 2 seconds
        int wait_count = 0;
        while (wait_count < 20)
        {
            int status;
            pid_t result = waitpid(pm->ffmpeg_pid, &status, WNOHANG);
            
            if (result == pm->ffmpeg_pid)
            {
                printf("[PM] FFmpeg stopped gracefully\n");
                pm->ffmpeg_pid = 0;
                pm->ffmpeg_running = false;
                return;
            }
            
            usleep(100000); // 100ms
            wait_count++;
        }
        
        // Force kill
        printf("[PM] FFmpeg not responding, sending SIGKILL\n");
        kill(pm->ffmpeg_pid, SIGKILL);
        waitpid(pm->ffmpeg_pid, NULL, 0);
    }
    
    pm->ffmpeg_pid = 0;
    pm->ffmpeg_running = false;
    printf("[PM] FFmpeg stopped\n");
}

bool pm_is_ffmpeg_running(ProcessManager *pm)
{
    pm_check_processes(pm);
    return pm->ffmpeg_running;
}

void pm_check_processes(ProcessManager *pm)
{
    int status;
    
    // Check mediaMTX
    if (pm->mediamtx_pid > 0)
    {
        pid_t result = waitpid(pm->mediamtx_pid, &status, WNOHANG);
        if (result == pm->mediamtx_pid)
        {
            // Process exited
            if (WIFEXITED(status))
            {
                printf("[PM] mediaMTX exited with code %d\n", WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf("[PM] mediaMTX killed by signal %d\n", WTERMSIG(status));
            }
            pm->mediamtx_pid = 0;
            pm->mediamtx_running = false;
        }
    }
    
    // Check FFmpeg
    if (pm->ffmpeg_pid > 0)
    {
        pid_t result = waitpid(pm->ffmpeg_pid, &status, WNOHANG);
        if (result == pm->ffmpeg_pid)
        {
            if (WIFEXITED(status))
            {
                printf("[PM] FFmpeg exited with code %d\n", WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf("[PM] FFmpeg killed by signal %d\n", WTERMSIG(status));
            }
            pm->ffmpeg_pid = 0;
            pm->ffmpeg_running = false;
        }
    }
}

void pm_cleanup(ProcessManager *pm)
{
    pm_stop_ffmpeg(pm);
    pm_stop_mediamtx(pm);
    printf("[PM] Cleanup complete\n");
}

int pm_wait_for_mediamtx_ready(const StreamingConfig *config, int timeout_sec)
{
    printf("[PM] Waiting for mediaMTX to be ready on port %d...\n", config->rtsp_port);
    
    int sock;
    struct sockaddr_in addr;
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config->rtsp_port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    
    for (int i = 0; i < timeout_sec * 10; i++)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            usleep(100000); // 100ms
            continue;
        }
        
        int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        close(sock);
        
        if (result == 0)
        {
            printf("[PM] mediaMTX is ready\n");
            return 0;
        }
        
        usleep(100000); // 100ms
    }
    
    fprintf(stderr, "[PM] Timeout waiting for mediaMTX\n");
    return -1;
}