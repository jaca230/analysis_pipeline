#!/bin/bash

# Get script directory and project root
SCRIPT_DIR=$(dirname "$(realpath "$0")")
BASE_DIR="$SCRIPT_DIR/.."

echo "[cleanup.sh] Cleaning project build artifacts..."

# List of directories to remove (adjust as needed)
DIRS_TO_CLEAN=(
    "$BASE_DIR/build"
    "$BASE_DIR/bin"
    "$BASE_DIR/lib"
)

for dir in "${DIRS_TO_CLEAN[@]}"; do
    if [ -d "$dir" ]; then
        echo "Removing $dir"
        rm -rf "$dir"
    fi
done

echo "[cleanup.sh] Cleanup complete."
