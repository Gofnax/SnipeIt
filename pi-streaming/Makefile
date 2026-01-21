# Makefile for Pi Streaming System
#
# Usage:
#   make            - Build all targets
#   make test_ipc   - Build IPC test only
#   make clean      - Remove built files
#   make test       - Run IPC test (requires two terminals)

CC = gcc
CFLAGS = -Wall -Wextra -g -O2
LDFLAGS = 

# Source files
IPC_SRCS = unix_socket.c
IPC_HDRS = unix_socket.h

# Targets
.PHONY: all clean test help

all: test_ipc

# IPC test program
test_ipc: test_ipc.c $(IPC_SRCS) $(IPC_HDRS)
	$(CC) $(CFLAGS) -o $@ test_ipc.c $(IPC_SRCS) $(LDFLAGS)
	@echo ""
	@echo "Build complete: test_ipc"
	@echo ""
	@echo "To test IPC:"
	@echo "  Terminal 1: ./test_ipc"
	@echo "  Terminal 2: python3 test_ipc.py"

# Clean build artifacts
clean:
	rm -f test_ipc
	rm -f *.o
	rm -f /tmp/detection.sock
	@echo "Clean complete"

# Help
help:
	@echo "Pi Streaming System - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build all targets (default)"
	@echo "  test_ipc   - Build IPC test program"
	@echo "  clean      - Remove built files"
	@echo "  help       - Show this help"
	@echo ""
	@echo "To test IPC communication:"
	@echo "  1. make test_ipc"
	@echo "  2. Terminal 1: ./test_ipc"
	@echo "  3. Terminal 2: python3 test_ipc.py"