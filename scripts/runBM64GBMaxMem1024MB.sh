#!/bin/bash

cd ..

# Setting the hash, sort, write thread parameters, file size, and maximum memory
MAX_MEMORY=1024
HASH_THREADS=(1 4 16)
SORT_THREADS=(1 4 16)
WRITE_THREADS=(1 4 16)
FILE_SIZE_GB=64

# This will build the hashgen.c file by running makefile
make

# Running the benchmark for different combinations of hash threads, sort threads, and write threads
for hashthreads in "${HASH_THREADS[@]}"; do
    for sortthreads in "${SORT_THREADS[@]}"; do
        for writethreads in "${WRITE_THREADS[@]}"; do
            # Construct the filename based on the combination of threads
            FILENAME="data64GB_${hashthreads}_${sortthreads}_${writethreads}.bin"
            echo "Running benchmark: hash_threads=$hashthreads, sort_threads=$sortthreads, write_threads=$writethreads, filename=$FILENAME"
            
            # Remove the file if it already exists
            if [ -f "$FILENAME" ]; then
                rm "$FILENAME"
            fi

            # Run the benchmark for different thread combinations
            ./hashgen -t "$hashthreads" -o "$sortthreads" -i "$writethreads" -f "$FILENAME" -m "$MAX_MEMORY" -s "$FILE_SIZE_GB"
            echo "Benchmark complete for $hashthreads hash threads $sortthreads sort threads $writethreads write threads. Results saved to $FILENAME"
        done
    done
done

cd scripts