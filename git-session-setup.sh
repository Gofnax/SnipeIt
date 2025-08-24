#!/bin/bash

# Move to the project directory (adjust this path!)
ORIGINAL_DIR=$(pwd)
PROJECT_DIR="/home/snipeit/SnipeIt"
cd "$PROJECT_DIR" || exit 1

# Safety check: only run if inside a Git repo
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then

    # Clear previous commit identity
    git config --unset user.name >/dev/null 2>&1
    git config --unset user.email >/dev/null 2>&1

    # Prompt user for new identity
    echo "=== Git session setup ==="
    read -p "Enter your GitHub name: " name
    read -p "Enter your GitHub email: " email

    git config user.name "$name"
    git config user.email "$email"

    echo "Git identity set for this session:"
    git config user.name
    git config user.email
    echo "(GitHub username/token will be requested on first push/pull)"
fi

cd "$ORIGINAL_DIR" || exit 1
