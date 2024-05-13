#!/bin/bash

cd ..

OUTPUT_PATH="../otherdata.bin"

# otherdata.bin exists, delete the file
if [ -f "$OUTPUT_PATH" ]; then
    rm "$OUTPUT_PATH"
fi

# define maximum memory values
MAX_MEMORY_VALUES=(1024 8192 32768)

# define hash threads, sort threads, and write threads
HASH_THREADS=4
SORT_THREADS=1
WRITE_THREADS=16
FILE_SIZE_GB=64
FILENAME="otherdata.bin"

# This will build the hashgen.c file by running the makefile
make

# runnning benchmark for different max memory sizes for 64GB workload
for MAX_MEMORY in "${MAX_MEMORY_VALUES[@]}"; do
    echo "Running benchmark for MAX_MEMORY=$MAX_MEMORY MB"
    echo "hash_threads=$HASH_THREADS, sort_threads=$SORT_THREADS, write_threads=$WRITE_THREADS"
    ./hashgen -t "$HASH_THREADS" -o "$SORT_THREADS" -i "$WRITE_THREADS" -f "$FILENAME" -m "$MAX_MEMORY" -s "$FILE_SIZE_GB"
    echo "Benchmark complete for MAX_MEMORY=$MAX_MEMORY MB"
done

cd scripts
