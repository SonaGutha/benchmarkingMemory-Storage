#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "blake3.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#define NONCE_SIZE 6
#define HASH_SIZE 10
#define RECORD_SIZE (HASH_SIZE + NONCE_SIZE)

// Structure to hold a record
typedef struct
{
    uint8_t hash[HASH_SIZE];   // hash value as byte array
    uint8_t nonce[NONCE_SIZE]; // Nonce value as byte array
} Record;

// Structure to hold thread arguments
typedef struct
{
    Record *records;
    size_t num_records;
    size_t start_index;
    size_t end_index;
    const char *filename;
    pthread_barrier_t *barrier;
    size_t bucketIndex;
} ThreadArgs;

/**
 * Method to generate hash threads using blake3 library in a bucket
 * */
void *hash_thread_generate(void *arg)
{
    srand(time(NULL));
    ThreadArgs *args = (ThreadArgs *)arg;

    struct timeval hash_gen_start_time, hash_gen_end_time;
    double hash_gen_total_time;

    gettimeofday(&hash_gen_start_time, NULL); // gets hash generation start time

    for (size_t i = args->start_index; i <= args->end_index; i++)
    {
        for (int j = 0; j < NONCE_SIZE; j++) // generating nonce of size 6 bytes and set it to record
        {
            args->records[i].nonce[j] = rand() % 256;
        }
        // blake3 lib invocation for hash generation
        blake3_hasher hasher;
        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, args->records[i].nonce, NONCE_SIZE);
        blake3_hasher_finalize(&hasher, args->records[i].hash, HASH_SIZE);
    }

    gettimeofday(&hash_gen_end_time, NULL); // // gets hash generation start time
    // total time consumed to generate hashes for a bucket
    hash_gen_total_time = (hash_gen_end_time.tv_sec - hash_gen_start_time.tv_sec) + (hash_gen_end_time.tv_usec - hash_gen_start_time.tv_usec) / 1000000.0;
    // calculating the data transfer rate
    double mb_per_sec = (args->num_records * RECORD_SIZE) / (1024 * 1024 * hash_gen_total_time);
    printf("[%ld] [HASHGEN]: ETA %.f seconds, %.2f MB/sec\n", args->bucketIndex, hash_gen_total_time, mb_per_sec);

    pthread_exit(NULL);
}

/**
 * Method to compare the 10 byte hash values 2 records
 * */
int hash_values_comparision(const void *a, const void *b)
{
    const Record *record_a = (const Record *)a;
    const Record *record_b = (const Record *)b;

    for (int i = 0; i < HASH_SIZE; i++)
    {
        if (record_a->hash[i] < record_b->hash[i])
        {
            return -1; // returns 1 when the specific byte of first record is less than the second record
        }
        else if (record_a->hash[i] > record_b->hash[i])
        {
            return 1; // returns 1 when the specific byte of first record is more than the second record
        }
    }

    return 0; // when hashes are equal, returns zero
}

/**
 * Method to sort records based on the sort threads in a bucket
 * */
void *records_sort_thread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    struct timeval sort_start_time, sort_end_time;
    double sort_total_time;
    gettimeofday(&sort_start_time, NULL); // gets time before starting the sorting

    // sorting the records based on quick sort
    qsort(args->records + args->start_index, args->num_records, sizeof(Record), hash_values_comparision);

    gettimeofday(&sort_end_time, NULL); // // gets time after sort ends
    // total time consumed to sort the records for a bucket
    sort_total_time = (sort_end_time.tv_sec - sort_start_time.tv_sec) + (sort_end_time.tv_usec - sort_start_time.tv_usec) / 1000000.0;
    // calculating the data transfer rate
    double mb_per_sec = (args->num_records * RECORD_SIZE) / (1024 * 1024 * sort_total_time);
    printf("[%ld] [SORT]: ETA %.1f seconds, %.2f MB/sec\n", args->bucketIndex, sort_total_time, mb_per_sec);
    pthread_exit(NULL);
}

/**
 * Method to perform writing to a file based on the write threads
 * */
void *write_records_thread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    struct timeval write_start_time, write_end_time;
    double write_total_time;
    gettimeofday(&write_start_time, NULL);                                // gets start time for write operation
    int file = open(args->filename, O_WRONLY | O_CREAT | O_APPEND, 0644); // opens file
    if (file == -1)
    {
        perror("Error occurred while opening file");
        exit(EXIT_FAILURE);
    }
    // number if bytes written to file based in the start and end indexes
    ssize_t bytes_written = write(file, args->records + args->start_index, args->num_records * sizeof(Record));

    if (bytes_written == -1)
    {
        perror("Error occurred while writing records to file");
        exit(EXIT_FAILURE);
    }

    close(file); // closes the file after write operations are done

    gettimeofday(&write_end_time, NULL); // gets start time for write operation

    // total time consumed to write records for a bucket
    write_total_time = (write_end_time.tv_sec - write_start_time.tv_sec) + (write_end_time.tv_usec - write_start_time.tv_usec) / 1000000.0;
    // calculates the data tansfer rates
    double mb_per_sec = (args->num_records * RECORD_SIZE) / (1024 * 1024 * write_total_time);
    printf("[%ld] [WRITE]: ETA %.2f seconds %ld bytes written  %.2f MB/sec\n", args->bucketIndex, write_total_time, bytes_written, mb_per_sec);
    pthread_barrier_wait(args->barrier); // waits for certain amount of threads to syncbefore execution
    pthread_exit(NULL);
}

/**
 * Method to create threads for hash generation, sort and write
 * */
void create_threads(pthread_t *thread_array, int num_threads, ThreadArgs *args, void *(*start_routine)(void *))
{
    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_create(&thread_array[i], NULL, start_routine, &args[i]) != 0) // creates a new thread
        {
            perror("Error occurred while creating thread"); // throws error in case of failure and exits then code
            exit(EXIT_FAILURE);
        }
    }
}


// Method to read records from file
Record *read_records_from_file(const char *filename, size_t num_records) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    Record *records = malloc(num_records * sizeof(Record));
    if (records == NULL) {
        perror("Memory allocation failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read = read(fd, records, num_records * sizeof(Record));
    if (bytes_read == -1) {
        perror("Error reading records from file");
        close(fd);
        free(records);
        exit(EXIT_FAILURE);
    }

    close(fd);
    return records;
}
/**
 * Method to wait for thread to complete
 */
void combine_threads(pthread_t *thread_array, int num_threads)
{
    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_join(thread_array[i], NULL) != 0) // waits for the thread terminate to get its return value
        {
            perror("Error occurred while combining thread"); // throws error in case of failure
            exit(EXIT_FAILURE);
        }
    }
}

/**
 *Method to display help menu
 * */
void display_help()
{
    printf("Help:\n");
    printf(" -t <hash_threads>: Specify the number of hash threads\n");
    printf(" -o <sort_threads>: Specify the number of sort threads\n");
    printf(" -i <write_threads>: Specify the number of write threads\n");
    printf(" -f <filename>: Specify the filename\n");
    printf(" -m <MAX_MEMORY_MB> : Maximum memory allowed to use\n");
    printf(" -s <FILE_SIZE_GB> : File size in GB\n");
    printf(" -d <bool>: turns on debug mode with true, off with false\n");
    printf(" -h: Display this help message\n");
}

/**
 * Method to print the configuration given in the input
 * */
void print_configuration(int hash_threads, int sort_threads, int write_threads, const char *filename, int MAX_MEMORY_MB, int FILE_SIZE_GB)
{
    printf("NUM_THREADS_HASH=%d\n", hash_threads);
    printf("NUM_THREADS_SORT=%d\n", sort_threads);
    printf("NUM_THREADS_WRITE=%d\n", write_threads);
    printf("FILENAME=%s\n", filename);
    printf("MEMORY_SIZE=%dMB\n", MAX_MEMORY_MB);
    printf("FILESIZE=%dMB\n", FILE_SIZE_GB * 1024);
    printf("RECORD_SIZE=%dB\n", RECORD_SIZE);
    printf("HASH_SIZE=%dB\n", HASH_SIZE);
    printf("NONCE_SIZE=%dB\n", NONCE_SIZE);
}
Record *allocate_records(size_t num_records)
{
    Record *records = malloc(num_records * sizeof(Record));
    if (records == NULL)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    return records;
}


/**
 * Main Method.Execution begins
 * */
int main(int argc, char *argv[])
{
    int hash_threads = 0, sort_threads = 0, write_threads = 0, MAX_MEMORY_MB = 0, FILE_SIZE_GB = 0;
    const char *filename = NULL;
    bool debug_mode = true;
    bool mandatory_params_present = false;

    // passing command line arguments
    if (argc < 2)
    {
        fprintf(stderr, "Error Occurred: No parameters provided. Refer to the help(-h).\n");
        return 1;
    }
    // arguments passed for invocation
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-t") == 0)
        {
            hash_threads = atoi(argv[++i]);
            mandatory_params_present = true;
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            sort_threads = atoi(argv[++i]);
            mandatory_params_present = true;
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            write_threads = atoi(argv[++i]);
            mandatory_params_present = true;
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            filename = argv[++i];
            mandatory_params_present = true;
        }
        else if (strcmp(argv[i], "-m") == 0)
        {
            MAX_MEMORY_MB = atoi(argv[++i]);
            mandatory_params_present = true;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            FILE_SIZE_GB = atoi(argv[++i]);
            mandatory_params_present = true;
        }
        else if (strcmp(argv[i], "-d") == 0)
        {
            debug_mode = strcmp(argv[++i], "false") != 0;
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            display_help();
            return 0;
        }
    }
    // checks if mandatory parameters are passsed while invocation
    if (!mandatory_params_present)
    {
        fprintf(stderr, "Error: Mandatory parameters -t, -o, -i, -f, -m, -s are required. Refer to the help(-h)\n");
        return 1;
    }
    // if debug mode is false then the logs are not printed
    if (!debug_mode)
    {
        FILE *dummy_stdout = freopen("/dev/null", "w", stdout);
        if (dummy_stdout == NULL)
        {
            perror("freopen stdout failed");
            exit(EXIT_FAILURE);
        }

        FILE *dummy_stderr = freopen("/dev/null", "w", stderr);
        if (dummy_stderr == NULL)
        {
            perror("freopen stderr failed");
            exit(EXIT_FAILURE);
        }
    }
    // throws error in case of any invalid inputs
    if (filename == NULL || hash_threads <= 0 || sort_threads <= 0 || write_threads <= 0 || MAX_MEMORY_MB <= 0 || FILE_SIZE_GB <= 0)
    {
        fprintf(stderr, "Error: Invalid command line arguments. Refer to the help(-h)\n");
        return 1;
    }

    size_t BYTES_IN_GB = (size_t)FILE_SIZE_GB * 1024 * 1024 * 1024; // converting the file size given in input to byte
    size_t num_records = BYTES_IN_GB / RECORD_SIZE;                 // number of records that fills up the file

    print_configuration(hash_threads, sort_threads, write_threads, filename, MAX_MEMORY_MB, FILE_SIZE_GB);

    // atlease 1 record should be present
    if (num_records == 0)
    {
        num_records = 1;
    }

    // Dynamic memory allocation for records
    Record *records = allocate_records(num_records);

    // Dynamic memory allocation for hash_args, sort_args, and write_args
    ThreadArgs *hash_args = malloc(hash_threads * sizeof(ThreadArgs));
    ThreadArgs *sort_args = malloc(sort_threads * sizeof(ThreadArgs));
    ThreadArgs *write_args = malloc(write_threads * sizeof(ThreadArgs));

    // initialize barrier for write arguments
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, write_threads);

    // file is divided into buckets, which are used for running threads concurrently
    size_t num_buckets = (FILE_SIZE_GB * 1024) / MAX_MEMORY_MB; // file is divided into buckets

    struct timeval start_time, end_time;
    double total_time;

    gettimeofday(&start_time, NULL); // gets start time before hash generation, sorting, and writing

    for (size_t bucket_index = 0; bucket_index < num_buckets; bucket_index++)
    {
     // initialzing start and end index of hash,sort,write threads based on buckets

      size_t num_records_per_bucket = num_records / num_buckets;
      size_t remaining_records = num_records % num_buckets;

        if (hash_args == NULL || sort_args == NULL || write_args == NULL )
        {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        
        // setting thread args for hash thread
        for (int i = 0; i < hash_threads; i++)
        {
            size_t start_index = i * (num_records_per_bucket / hash_threads) + bucket_index * num_records_per_bucket;
            size_t end_index = start_index + (num_records_per_bucket / hash_threads) - 1;
          
            if (i == hash_threads - 1) { // if any records are remaining, this thread those records
                end_index += remaining_records;
            }
	    
            hash_args[i].records = records;
            hash_args[i].filename = filename;
            hash_args[i].start_index = start_index;
            hash_args[i].end_index = end_index;
            hash_args[i].barrier = &barrier;
            hash_args[i].num_records = end_index - start_index + 1;
            hash_args[i].bucketIndex = bucket_index;
        }
        // setting thread args for sort threads
        for (int i = 0; i < sort_threads; i++)
        {
            size_t start_index = i * (num_records_per_bucket / sort_threads) + bucket_index * num_records_per_bucket;
            size_t end_index = start_index + (num_records_per_bucket / sort_threads) - 1;
            if (i == sort_threads - 1) {  // if any records are remaining, this thread those records
                end_index += remaining_records;
            }
            
            sort_args[i].records = records;
            sort_args[i].filename = filename;
            sort_args[i].start_index = start_index;
            sort_args[i].end_index = end_index;
            sort_args[i].barrier = &barrier;
            sort_args[i].num_records = end_index - start_index + 1;
            sort_args[i].bucketIndex = bucket_index;
        }
        // setting thread args for write threads
        for (int i = 0; i < write_threads; i++)
        {
            size_t start_index = i * (num_records_per_bucket / write_threads) + bucket_index * num_records_per_bucket;
            size_t end_index = start_index + (num_records_per_bucket / write_threads) - 1;
            if (i == write_threads - 1) { // if any records are remaining, this thread those records
                end_index += remaining_records;
            }

            write_args[i].records = records;
            write_args[i].filename = filename;            
	        write_args[i].start_index = start_index;
	        write_args[i].end_index = end_index;
            write_args[i].barrier = &barrier;
            write_args[i].num_records = end_index - start_index + 1;
            write_args[i].bucketIndex = bucket_index;
        }

        // hash threads creation
        pthread_t *hash_thread = malloc(hash_threads * sizeof(pthread_t));
        create_threads(hash_thread, hash_threads, hash_args, hash_thread_generate);
        // waits till the hash threads are to complete to get its return value
        combine_threads(hash_thread, hash_threads);
        //free hash threads
	free(hash_thread);

    printf("sort started, expecting %d flushes for %d buckets...\n", (FILE_SIZE_GB * 1024) / MAX_MEMORY_MB, (FILE_SIZE_GB * 1024));
    // sort threads creation
    pthread_t *sort_thread = malloc(sort_threads * sizeof(pthread_t));
    create_threads(sort_thread, sort_threads, sort_args, records_sort_thread);
    // waits till the hash threads are to complete to get its return value
    combine_threads(sort_thread, sort_threads);
    //free hash threads
	free(sort_thread);

        printf("write started\n");
        // write threads creation
        pthread_t *write_thread = malloc(write_threads * sizeof(pthread_t));
        create_threads(write_thread, write_threads, write_args, write_records_thread);
        // waits till the hash threads are to complete to get its return value
        combine_threads(write_thread, write_threads);
        free(write_thread);  
    }
    //free up memory
    free(hash_args);
    free(sort_args);
    free(write_args);
    free(records);

	Record *all_records = read_records_from_file(filename, num_records);
  	//sorting the records which were sorted based on bucket size
    qsort(all_records, num_records, sizeof(Record), hash_values_comparision);
	// Write the sorted records to the output file
    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (file == -1) // throws error when file fails to open
   	{
       	perror("Error occurred while opening file");
       	exit(EXIT_FAILURE);
   	}
    // write bytes to file
    ssize_t bytes_written = write(file, records, num_records * sizeof(Record));
    if (bytes_written == -1)
    {
        perror("Error occurred while writing records to file");
        exit(EXIT_FAILURE);
    }
    print_configuration(hash_threads, sort_threads, write_threads, filename, MAX_MEMORY_MB, FILE_SIZE_GB);
    close(file); // closes the file
    // Destroy pthread barrier
    pthread_barrier_destroy(&barrier);
    gettimeofday(&end_time, NULL); // gets end time after completing hash generation, sorting and writing
    // total time
    total_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    // throughput calculation
    double hashes_per_sec = (num_records / total_time);
    double mega_hashes_per_sec = hashes_per_sec / 1000000.0;
    double throughput = (FILE_SIZE_GB * 1024) / total_time;
    printf("Completed %d GB file %s in %f seconds: %f MH/s %f MB/s\n", FILE_SIZE_GB, filename, total_time, mega_hashes_per_sec, throughput);
    if (!debug_mode)
    {
        fprintf(stdout, "hashgen t%d o%d i%d m%dMB s%dGB total time %.2f seconds %f MH/s %fMB/s\n", hash_threads, sort_threads, write_threads, MAX_MEMORY_MB, FILE_SIZE_GB, total_time, mega_hashes_per_sec, throughput);
    }
    free(all_records);

    return 0;
}
