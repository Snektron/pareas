#!/bin/bash

TEST_NAME=$(echo "$1" | cut -d'/' -f2-)
echo Starting tests for $TEST_NAME

mkdir -p "results/$TEST_NAME"

for i in {0..30}; do
    echo Running test $TEST_NAME
    CUDA_VISIBLE_DEVICES=1 ./build/pareas "$1" --profile 5 > "results/$TEST_NAME.out"
done
