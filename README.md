[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/C5s9grq-)
### CS553 Cloud Computing Assignment 4 Repo
**Illinois Institute of Technology**  

**Students**:  
* Sona Shree Reddy Gutha (sgutha@hawk.iit.edu)  

Benchmarking Memory & Storage : This assignment is coded in C language.

Blake3 is used for hash generation. The files related to c language from the official blake3 algorithm from git repo are copied to current repo.

Reference : https://github.com/BLAKE3-team/BLAKE3

To execute build and run the program follow the below instructions.

      1. To **Build** type run make file.
            eg: **make**
         Doing so will create a executable hashgen file with all the dependencies
      2. To run the programs with different number of threads, please follw the below format.
            eg: ./hashgen -t 1 -o 1 -i 16 -f data.bin -m 128 -s 1 -d true
            Value after **-t** is the number of hash thread
            Value after **-o** is the number of sort thread
            Value after **-i** is the number of write thread
            Value after **-f** is the name of the output file
            Value after **-m** is the max memory size in MB
            Value after **-s** is the output file size in GB
            Value after **-d** is the debug flag which can be enabled or disable
            The values can be changed as per the requirements.
      3. After running the above command, an output file is generated. To verify if the output file is sorted, run the below command.
            eg: ./hashverify -f data.bin -v true
            Here the output file is data.bin. It can be changed accordingly.
      4. To verify if the hashes generated are valid, run the below command. 
            eg: ./hashverify -f data.bin -b 100
            Here the output file is data.bin. It can be changed accordingly.
      5. To perform the visual checks on hashes,run the below command.
            eg: ./hashverify -f data.bin -p 100
            Here the output file is data.bin. It can be changed accordingly.

To run automatically run the benchmarks for the different experiments, bash scripts are written. More details on how to run the bash scripts are below.

More details on the files in repository are as follow.

hashgen.c file contains the implementation of this assignment.

Makefile : running make will build hashgen.c will all its dependencies and generates executable file hashgen. 

hw4_report.pdf gives the detailed report on the experiments.

hw4_rework_report.pdf gives the detailed report on the experiments after rework.

Scripts:
      This folder contains bash scripts to automate the benchmarks for different workloads and maximum memory combinations.
      runBM1GBMaxMem128MB.sh : 
        Bash script for 1GB workload and 128MB max memory
      runBM64GBMaxMem1024MB.sh: 
        Bash script for 64GB workload and 1024MB max memory
      runBM64GBMaxMem1024_8192_32768.sh : 
        Bash script for 64GB workload and 1GB,8GB,32GB max memory for the best configuration of 64GB workload
      runAllBM.sh: 
        runs all 3 bash scripts in this folder

      To run the bash scripts, navigate to scripts tab, enable permissions using chmod and run the script.
      Ex : chmod +x runBM1GBMaxMem128MB.sh
      After enabling the permossions, the script can be run as below.
      Ex : ./runBM1GBMaxMem128MB.sh
      While running 27 experiments for 27 configurations of each workload, the output binary filease are created outside of scripts folder with naming convention as data_hash_sort_write corresponding thread values for 1GB and data64GB_hash_sort_write corresponding thread values for 64GB workload. For other max memory, the bin file is otherdata.bin. These binary files can can be used verified for using hashVerify

plotscripts:
      This folder contains the python code to plot graphs for the threads and their performance metrics for different work loads.
      To run these scripts, pip install pandas and then run "python3 file name". Once it runs successfully the graphs are generated in graphs folder.
      performanceplot1GB.py: 
        plots graphs for based on 27 experiments for 1GB workload
      performanceplot64GB.py:
        plots graphs for based on 27 experiments for 64GB workload

Files contains the logs of the benchmarking experiments

input: 
      This folder contains the input text file for the python scripts to generate graphs 

hashgen - This is the executable file which on execution runs the bench marks
