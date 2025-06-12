#!/bin/bash
set -e

# Get the absolute path of the script directory and base
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_DIR="$SCRIPT_DIR/../.."

# Known submodules and their respective build scripts (relative to BASE_DIR)
declare -A SUBMODULES
SUBMODULES["analysis_pipeline_stages"]="$BASE_DIR/external/analysis_pipeline_stages/scripts/build.sh"

# Display help
show_help() {
  echo "Usage: ./build_submodules.sh [OPTIONS]"
  echo ""
  echo "Options:"
  echo "  -o, --overwrite         Clean existing builds"
  echo "  -j, --jobs <number>     Number of cores to use (default: all)"
  echo "  -h, --help              Show this message"
  echo ""
  echo "All options will be forwarded to each submodule’s build script."
}

# Show help if requested
for arg in "$@"; do
  if [[ "$arg" == "-h" || "$arg" == "--help" ]]; then
    show_help
    exit 0
  fi
done

echo "[build_submodules] Building ${#SUBMODULES[@]} submodule(s)..."

for name in "${!SUBMODULES[@]}"; do
  script_path="${SUBMODULES[$name]}"
  if [[ -x "$script_path" ]]; then
    echo "[build_submodules] -> $name"
    "$script_path" "$@"
  else
    echo "[build_submodules] !! ERROR: Build script not found or not executable for '$name': $script_path"
    exit 1
  fi
done

echo "[build_submodules] All submodules built."
