#!/usr/bin/env bash

# This script assumes Bash 4 is present (to make use of features such as
# associative arrays).

# Arguments:
# $1: stage ("build")

# supported tasks, meta-level
#
# These are the stages that apply to all platforms (Linux, macOS, ...).
TASKS=("build")

if [ "$#" -lt 1 ]; then
    echo "[ERROR] Insufficient arguments, need at least 2"
    exit 1
fi

TASK=""
for i in "${TASKS[@]}"; do
    if [ "$i" == "$1" ]; then
        TASK="$i"
        echo "[INFO] Executing task: $TASK"
        break
    fi
done

if [ -z "$TASK" ]; then
    echo "[ERROR] Failed to parse task from \$1: $1"
    exit 1
fi

declare -A COMMON_TASKS

COMMON_TASKS=(
    ["build"]="make build"
)

ACTUAL_TASK=""
for i in "${!COMMON_TASKS[@]}"; do
    if [ "$i" == "$TASK" ]; then
        ACTUAL_TASK="${COMMON_TASKS[$i]}"
        break
    fi
done

if [ "$ACTUAL_TASK" == "" ]; then
    echo "[WARN] Task $TASK undefined, skipping"
else
    echo "[INFO] Task: $TASK"
    echo "[INFO] Executing task: \"$ACTUAL_TASK\""
    if ! eval $ACTUAL_TASK; then
        echo "[ERROR] Task $TASK failed"
        exit 1
    fi
fi

exit 0
