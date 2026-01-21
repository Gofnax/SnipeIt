# Makefile for Pi Streaming System
#
# Usage:
#   make              - Build main streaming server
#   make test_ipc     - Build IPC test only
#   make all          - Build everything
#   make clean        - Remove built files

CC = gcc
CFLAGS = -Wall -Wextra -g -O2
LDFLAGS = -lwebsockets

# Source files
SRCS = main.c unix_socket.c websocket_server.c process_manager.c config.c
HDRS = unix_socket.h websocket_server.h process_manager.h config.h

IPC_SRCS = unix_socket.c
IPC_HDRS = unix_socket.h

# Targets
.PHONY: all clean help

# Default target - build main server
all: streaming_server test_ipc

# Main streaming server
streaming_server: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)
	@echo ""
	@echo "========================================"
	@echo "  Build complete: streaming_server"
	@echo "========================================"
	@echo ""
	@echo "Before running, make sure to:"
	@echo "  1. Edit streaming_config.json with your video path"
	@echo "  2. Start the Python detection script"
	@echo ""
	@echo "To run:"
	@echo "  ./streaming_server [config_file]"
	@echo ""

# IPC test program
test_ipc: test_ipc.c $(IPC_SRCS) $(IPC_HDRS)
	$(CC) $(CFLAGS) -o $@ test_ipc.c $(IPC_SRCS)
	@echo ""
	@echo "Build complete: test_ipc"
	@echo ""
	@echo "To test IPC:"
	@echo "  Terminal 1: ./test_ipc"
	@echo "  Terminal 2: python3 test_ipc.py"

# Clean build artifacts
clean:
	rm -f streaming_server test_ipc
	rm -f *.o
	rm -f /tmp/detection.sock
	rm -f /tmp/mediamtx.log /tmp/ffmpeg.log
	@echo "Clean complete"

# Help
help:
	@echo "Pi Streaming System - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build all targets (default)"
	@echo "  streaming_server - Build main streaming server"
	@echo "  test_ipc         - Build IPC test program"
	@echo "  clean            - Remove built files"
	@echo "  help             - Show this help"
	@echo ""
	@echo "Quick start:"
	@echo "  1. make"
	@echo "  2. Edit streaming_config.json"
	@echo "  3. Terminal 1: ./streaming_server"
	@echo "  4. Terminal 2: python3 test_ipc.py"
	@echo "  5. Connect Android app"