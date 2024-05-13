#!/bin/bash

# Run bash script for 1GB workload with max memory 128MB
echo "Run bash script for 1GB workload with max memory 128MB"
./runBM1GBMaxMem128MB.sh

# Run bash script for 64GB workload with max memory 1024MB
echo "Run bash script for 64GB workload with max memory 1024MB"
./runBM64GBMaxMem1024MB.sh

# Run bash script for 64GB workload with max memory 1GB,8GB,32GB
echo "Run bash script for 64GB workload with max memory 1GB,8GB,32GB"
./runBM64GBMaxMem1024_8192_32768.sh

echo "All bash script are executed"
