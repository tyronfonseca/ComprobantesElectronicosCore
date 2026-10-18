#!/usr/bin/env bash
set -e

FILTER_ARG="$1"

if [ -n "$FILTER_ARG" ]; then
    make clean && make -j$(sysctl -n hw.ncpu) test FILTER="$FILTER_ARG"
else
    make clean && make -j$(sysctl -n hw.ncpu) test
fi