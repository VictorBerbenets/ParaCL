#!/bin/bash

SCRIPT_PATH="${BASH_SOURCE[0]}"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
PROJECT_DIR="$(realpath "$SCRIPT_DIR")"

filename="$1"
if [ ! -f ${filename} ]; then
  echo "error: expected filename"
  exit 1
fi
shift

temp_file="$(mktemp).ll"
trap "rm -f '${temp_file}'" EXIT

${PROJECT_DIR}/build/paracl -oper-mode=compiler ${filename} -o ${temp_file}
if [ -f "${temp_file}" ]; then
  clang++ "${temp_file}" "${PROJECT_DIR}/lib/std_pcl_lib/pcllib.cpp" "$@"
fi
