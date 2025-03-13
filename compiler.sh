#!/bin/bash

exclude_arg="-o"
exec_name="a.out"

SCRIPT_PATH="${BASH_SOURCE[0]}"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
PROJECT_DIR="$(realpath "$SCRIPT_DIR")"

filtered_args=()

skip_next=false

for arg in "$@"; do
  if $skip_next; then
    skip_next=false
    exec_name="${arg}"
    continue
  fi

  if [ "$arg" = "$exclude_arg" ]; then
    skip_next=true
    continue
  fi
done

temp_file=$(mktemp).ll
${PROJECT_DIR}/build/paracl -oper-mode=compiler "${@}" -o ${temp_file}
clang++ ${temp_file} ${PROJECT_DIR}/lib/std_pcl_lib/pcllib.cpp -o ${exec_name}
rm ${temp_file}
