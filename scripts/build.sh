#!/bin/bash

# Get the absolute path of the script directory
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_DIR="$SCRIPT_DIR/.."
BUILD_DIR="$BASE_DIR/build"

# Default flags
OVERWRITE=false
JOBS_ARG="-j"  # Default: use all processors

# Help message
show_help() {
    echo "Usage: ./build.sh [OPTIONS]"
    echo
    echo "Options:"
    echo "  -o, --overwrite         Remove existing build directory before building"
    echo "  -j, --jobs <number>     Specify number of processors to use (default: all available)"
    echo "  -h, --help              Display this help message"
}

# Parse arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -o|--overwrite) OVERWRITE=true; shift ;;
        -j|--jobs)
            if [[ -n "$2" && "$2" != -* ]]; then
                JOBS_ARG="-j$2"
                shift 2
            else
                JOBS_ARG="-j"  # Use all processors
                shift
            fi
            ;;
        -h|--help) show_help; exit 0 ;;
        *) echo "[build.sh, ERROR] Unknown option: $1"; show_help; exit 1 ;;
    esac
done

if [ "$OVERWRITE" = true ]; then
    echo "[build.sh, INFO] Overwrite enabled: Cleaning previous build..."
    "$SCRIPT_DIR/cleanup.sh"
fi

# Create the build directory if it doesn't exist
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

# Run CMake and Make
echo "[build.sh, INFO] Configuring the project with CMake..."
cmake "$BASE_DIR"

echo "[build.sh, INFO] Building the project with 'make $JOBS_ARG'..."
make $JOBS_ARG

echo "[build.sh, INFO] Build complete. Executables are in: $BUILD_DIR/bin/"
