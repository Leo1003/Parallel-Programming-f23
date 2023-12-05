#include <sys/sysinfo.h>
#define _GNU_SOURCE
#include <mpi.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#define eprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define MPIMSG_TAG_TOSS     0UL
#define MPIMSG_TAG_ITER     1UL

static atomic_ullong point_inside = 0;
static atomic_ullong point_total = 0;

struct thread_ctx {
    unsigned long long toss;
};

inline static unsigned long test_point(struct drand48_data *state)
{
    double x, y;
    drand48_r(state, &x);
    drand48_r(state, &y);
    
    return (x * x + y * y) < 1.0;
}

static void *thread_main(void *_data)
{
    struct thread_ctx *data = (struct thread_ctx *)_data;

    unsigned short seed16v[3];
    FILE *rng_dev = fopen("/dev/urandom", "rb");
    if (rng_dev == NULL) {
        eprintf("Failed to open /dev/urandom!\n");
        abort();
    }
    fread(seed16v, sizeof(unsigned short), 3, rng_dev);
    fclose(rng_dev);

    struct drand48_data state;
    seed48_r(seed16v, &state);

    unsigned long pi = 0;
    for (size_t i = 0; i < data->toss; i++) {
        pi += test_point(&state);
    }

    atomic_fetch_add(&point_inside, pi);
    atomic_fetch_add(&point_total, data->toss);

    return NULL;
}

static uint64_t pi_toss(uint64_t toss)
{
    size_t thread_cnt = get_nprocs();

    pthread_t *thread_states = (pthread_t *)malloc(sizeof(pthread_t) * thread_cnt);
    if (thread_states == NULL) {
        abort();
    }
    struct thread_ctx *thread_data = (struct thread_ctx *)malloc(sizeof(struct thread_ctx) * thread_cnt);
    if (thread_data == NULL) {
        abort();
    }

    for (size_t t = 0; t < thread_cnt; t++) {
        thread_data[t].toss = toss / thread_cnt;
        thread_data[t].toss += (toss % thread_cnt) < t;

        pthread_create(&thread_states[t], NULL, thread_main, &thread_data[t]);
    }
    for (size_t t = 0; t < thread_cnt; t++) {
        pthread_join(thread_states[t], NULL);
    }

    free(thread_data);
    free(thread_states);
    
    uint64_t pi = atomic_load(&point_inside);

    return pi;
}

int main(int argc, char **argv)
{
    // --- DON'T TOUCH ---
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();
    double pi_result;
    long long int tosses = atoi(argv[1]);
    int world_rank, world_size;
    // ---
    
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    MPI_Status status;
    if (world_rank > 0) {
        uint64_t local_tosses;
        MPI_Recv(&local_tosses, 1, MPI_UNSIGNED_LONG, 0, MPIMSG_TAG_ITER, MPI_COMM_WORLD, &status);
        
        uint64_t toss_result = pi_toss(local_tosses);
        
        uint64_t sendbuf[2];
        sendbuf[0] = toss_result;
        sendbuf[1] = local_tosses;
        MPI_Send(sendbuf, 2, MPI_UNSIGNED_LONG, 0, MPIMSG_TAG_TOSS, MPI_COMM_WORLD);
    } else if (world_rank == 0) {
        uint64_t every_tosses = tosses / world_size;
        for (int i = 1; i < world_size; i++) {
            MPI_Send(&every_tosses, 1, MPI_UNSIGNED_LONG, i, MPIMSG_TAG_ITER, MPI_COMM_WORLD);
        }

        uint64_t pi = pi_toss(every_tosses);
        uint64_t pp = every_tosses;

        for (int i = 1; i < world_size; i++) {
            MPI_Status status;
            uint64_t recvbuf[2];
            MPI_Recv(recvbuf, 2, MPI_UNSIGNED_LONG, i, MPIMSG_TAG_TOSS, MPI_COMM_WORLD, &status);
            pi += recvbuf[0];
            pp += recvbuf[1];
        }

        pi_result = ((double)pi / (double)pp) * 4;

        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }

    MPI_Finalize();
    return 0;
}
