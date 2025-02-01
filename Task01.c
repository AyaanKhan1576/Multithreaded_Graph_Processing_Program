#define _GNU_SOURCE  
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/stat.h>

#define MAX_THREADS 8  
#define TOP_N 10  

pthread_mutex_t lock;
typedef struct {
    int thread_id;
    FILE *file;
    long start;
    long end;
    int local_edges;
    long *local_degree;  
} thread_args;

int total_edges = 0;
int total_nodes = 0;  
long max_node_id = 0;
long *global_degree;  

void set_thread_affinity(int thread_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_id % sysconf(_SC_NPROCESSORS_ONLN), &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

long adjust_to_line_start(FILE *file, long position) {
    if (position == 0) return 0;
    fseek(file, position, SEEK_SET);
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') return ftell(file);
    }
    return position;
}

void *process_chunk(void *args) {
    thread_args *arg = (thread_args *)args;
    char line[256];

    fseek(arg->file, arg->start, SEEK_SET);
    
    while (ftell(arg->file) < arg->end && fgets(line, sizeof(line), arg->file)) {
        if (line[0] == '#') continue;  

        int from, to;
        if (strchr(line, ':')) {
            if (sscanf(line, "%d:%d", &from, &to) != 2) continue;
        } else {
            if (sscanf(line, "%d %d", &from, &to) != 2) continue;
        }

        if (from > max_node_id || to > max_node_id) continue;

        arg->local_edges++;

        arg->local_degree[from]++;
        arg->local_degree[to]++;
    }

    pthread_mutex_lock(&lock);
    for (long i = 0; i <= max_node_id; i++) {
        if (arg->local_degree[i] > 0) {
            if (global_degree[i] == 0) total_nodes++; 
            global_degree[i] += arg->local_degree[i];
        }
    }
    total_edges += arg->local_edges;
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <filename> <num_threads> [affinity]\n", argv[0]);
        return 1;
    }

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);  

    char *filename = argv[1];
    int num_threads = atoi(argv[2]);
    int use_affinity = (argc == 4 && strcmp(argv[3], "affinity") == 0);

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    struct stat st;
    stat(filename, &st);
    long file_size = st.st_size;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#') continue;
        int from, to;
        if (strchr(line, ':')) {
            sscanf(line, "%d:%d", &from, &to);
        } else {
            sscanf(line, "%d %d", &from, &to);
        }
        if (from > max_node_id) max_node_id = from;
        if (to > max_node_id) max_node_id = to;
    }
    rewind(file);

    global_degree = calloc(max_node_id + 1, sizeof(long));
    if (!global_degree) {
        printf("Memory allocation failed\n");
        return 1;
    }

    pthread_t threads[MAX_THREADS];
    thread_args args[MAX_THREADS];
    long chunk_size = file_size / num_threads;
    pthread_mutex_init(&lock, NULL);

    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].file = fopen(filename, "r");
        args[i].start = i * chunk_size;
        args[i].end = (i == num_threads - 1) ? file_size : (i + 1) * chunk_size;
        args[i].local_edges = 0;
        args[i].local_degree = calloc(max_node_id + 1, sizeof(long));
        args[i].start = adjust_to_line_start(args[i].file, args[i].start);

        if (use_affinity) {
            set_thread_affinity(i);
        }
        pthread_create(&threads[i], NULL, process_chunk, &args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        fclose(args[i].file);
        free(args[i].local_degree);
    }

    pthread_mutex_destroy(&lock);

    printf("\nTop 10 nodes with highest degree:\n");
    long top_nodes[TOP_N] = {0};
    long top_degrees[TOP_N] = {0};

    for (long i = 0; i <= max_node_id; i++) {
        long degree = global_degree[i];
        if (degree > top_degrees[TOP_N - 1]) {
            top_nodes[TOP_N - 1] = i;
            top_degrees[TOP_N - 1] = degree;

            for (int j = TOP_N - 1; j > 0 && top_degrees[j] > top_degrees[j - 1]; j--) {
                long temp_node = top_nodes[j];
                long temp_degree = top_degrees[j];

                top_nodes[j] = top_nodes[j - 1];
                top_degrees[j] = top_degrees[j - 1];

                top_nodes[j - 1] = temp_node;
                top_degrees[j - 1] = temp_degree;
            }
        }
    }

    for (int i = 0; i < TOP_N; i++) {
        if (top_degrees[i] > 0)
            printf("Node %ld: %ld neighbors\n", top_nodes[i], top_degrees[i]);
    }

    printf("\nTotal Unique Nodes: %d\n", total_nodes);
    printf("Total Edges: %d\n", total_edges);

    clock_gettime(CLOCK_MONOTONIC, &end_time);  
    double execution_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("\nExecution Time: %.6f seconds\n", execution_time);

    free(global_degree);
    return 0;
}