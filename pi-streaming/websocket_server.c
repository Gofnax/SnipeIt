#include "websocket_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libwebsockets.h>

// Per-session data (one per connected client)
typedef struct
{
    WebSocketServer *server;    // Back-reference to server
} SessionData;

// Protocol callback function
static int ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                       void *user, void *in, size_t len)
{
    SessionData *session = (SessionData *)user;
    WebSocketServer *ws = NULL;
    
    // Get server reference from protocol user data
    if (wsi)
    {
        const struct lws_protocols *protocol = lws_get_protocol(wsi);
        if (protocol && protocol->user)
        {
            ws = (WebSocketServer *)protocol->user;
        }
    }
    
    switch (reason)
    {
        case LWS_CALLBACK_PROTOCOL_INIT:
            printf("[WS] Protocol initialized\n");
            break;
            
        case LWS_CALLBACK_ESTABLISHED:
            printf("[WS] Client connected\n");
            if (ws)
            {
                // Only allow one client at a time
                if (ws->client_connected)
                {
                    printf("[WS] Rejecting connection - already have a client\n");
                    return -1; // Reject connection
                }
                
                ws->client_wsi = wsi;
                ws->client_connected = true;
                
                // Store server reference in session
                if (session)
                {
                    session->server = ws;
                }
                
                // Call user callback
                if (ws->on_connect)
                {
                    ws->on_connect(ws->callback_user_data);
                }
            }
            break;
            
        case LWS_CALLBACK_CLOSED:
            printf("[WS] Client disconnected\n");
            if (ws && ws->client_wsi == wsi)
            {
                ws->client_wsi = NULL;
                ws->client_connected = false;
                ws->send_pending = false;
                ws->send_len = 0;
                
                // Call user callback
                if (ws->on_disconnect)
                {
                    ws->on_disconnect(ws->callback_user_data);
                }
            }
            break;
            
        case LWS_CALLBACK_SERVER_WRITEABLE:
            if (ws && ws->send_pending && ws->send_len > 0)
            {
                // Prepare buffer with LWS_PRE padding
                unsigned char buf[LWS_PRE + WS_MAX_MSG_SIZE];
                size_t msg_len = ws->send_len;
                
                if (msg_len > WS_MAX_MSG_SIZE - LWS_PRE)
                {
                    msg_len = WS_MAX_MSG_SIZE - LWS_PRE;
                }
                
                memcpy(&buf[LWS_PRE], ws->send_buffer, msg_len);
                
                // Send as text message
                int written = lws_write(wsi, &buf[LWS_PRE], msg_len, LWS_WRITE_TEXT);
                
                if (written < (int)msg_len)
                {
                    fprintf(stderr, "[WS] Write error: wrote %d of %zu bytes\n", 
                            written, msg_len);
                }
                
                ws->send_pending = false;
                ws->send_len = 0;
            }
            break;
            
        case LWS_CALLBACK_RECEIVE:
            // We don't expect messages from Android, but log them
            if (len > 0 && in)
            {
                printf("[WS] Received from client: %.*s\n", (int)len, (char *)in);
            }
            break;
            
        default:
            break;
    }
    
    return 0;
}

// Protocol definition
static struct lws_protocols protocols[] = {
    {
        .name = "detection-protocol",
        .callback = ws_callback,
        .per_session_data_size = sizeof(SessionData),
        .rx_buffer_size = WS_MAX_MSG_SIZE,
        .user = NULL  // Will be set to WebSocketServer pointer
    },
    { NULL, NULL, 0, 0, 0, NULL, 0 } // Terminator
};

int ws_init(WebSocketServer *ws, int port)
{
    *ws = (WebSocketServer){
        .context = NULL,
        .client_wsi = NULL,
        .port = port,
        .running = false,
        .client_connected = false,
        .on_connect = NULL,
        .on_disconnect = NULL,
        .callback_user_data = NULL,
        .send_buffer = { 0 },
        .send_len = 0,
        .send_pending = false
    };
    
    // Set protocol user data to point to our server struct
    protocols[0].user = ws;
    
    // Configure libwebsockets
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    
    info.port = port;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;
    
    // Reduce logging verbosity
    lws_set_log_level(LLL_ERR | LLL_WARN, NULL);
    
    // Create context
    ws->context = lws_create_context(&info);
    if (!ws->context)
    {
        fprintf(stderr, "[WS] Failed to create libwebsockets context\n");
        return -1;
    }
    
    ws->running = true;
    printf("[WS] WebSocket server listening on port %d\n", port);
    
    return 0;
}

void ws_set_callbacks(WebSocketServer *ws,
                      ws_connect_callback on_connect,
                      ws_disconnect_callback on_disconnect,
                      void *user_data)
{
    ws->on_connect = on_connect;
    ws->on_disconnect = on_disconnect;
    ws->callback_user_data = user_data;
}

int ws_service(WebSocketServer *ws, int timeout_ms)
{
    if (!ws->running || !ws->context)
    {
        return -1;
    }
    
    return lws_service(ws->context, timeout_ms);
}

bool ws_is_client_connected(WebSocketServer *ws)
{
    return ws->client_connected;
}

int ws_send(WebSocketServer *ws, const char *message)
{
    if (!ws->client_connected || !ws->client_wsi)
    {
        return -1;
    }
    
    size_t len = strlen(message);
    if (len >= WS_MAX_MSG_SIZE)
    {
        fprintf(stderr, "[WS] Message too large: %zu bytes\n", len);
        return -1;
    }
    
    // Copy to send buffer
    memcpy(ws->send_buffer, message, len);
    ws->send_len = len;
    ws->send_pending = true;
    
    // Request callback to send
    lws_callback_on_writable(ws->client_wsi);
    
    return 0;
}

int ws_send_json(WebSocketServer *ws, const char *json, size_t len)
{
    if (!ws->client_connected || !ws->client_wsi)
    {
        return -1;
    }
    
    if (len >= WS_MAX_MSG_SIZE)
    {
        fprintf(stderr, "[WS] JSON message too large: %zu bytes\n", len);
        return -1;
    }
    
    memcpy(ws->send_buffer, json, len);
    ws->send_len = len;
    ws->send_pending = true;
    
    lws_callback_on_writable(ws->client_wsi);
    
    return 0;
}

int ws_get_poll_fd(WebSocketServer *ws)
{
    // libwebsockets manages its own event loop internally
    // For integration with external poll(), we'd need to use
    // the external poll API, which is more complex.
    // For now, return -1 to indicate we use lws_service() instead.
    (void)ws;
    return -1;
}

void ws_cleanup(WebSocketServer *ws)
{
    if (ws->context)
    {
        lws_context_destroy(ws->context);
        ws->context = NULL;
    }
    
    ws->running = false;
    ws->client_connected = false;
    ws->client_wsi = NULL;
    
    printf("[WS] Cleanup complete\n");
}