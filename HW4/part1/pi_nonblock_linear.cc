#include <atomic>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

#define eprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define MPIMSG_TAG_TOSS     0UL
#define MPIMSG_TAG_ITER     1UL

static atomic_ullong point_inside;
static atomic_ullong point_total;

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
    point_inside = 0;
    point_total = 0;

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

struct pi_result {
    uint64_t result[2];
};

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

    struct pi_result *results = NULL;
    if (world_rank > 0) {
        MPI_Request request;
        uint64_t local_pp = tosses / world_size;
        uint64_t local_pi = pi_toss(local_pp);

        uint64_t sendbuf[2];
        sendbuf[0] = local_pi;
        sendbuf[1] = local_pp;

        MPI_Isend(sendbuf, 2, MPI_UNSIGNED_LONG, 0, MPIMSG_TAG_TOSS, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    } else if (world_rank == 0) {
        MPI_Request *requests = (MPI_Request *)malloc(sizeof(MPI_Request) * world_size);
        if (requests == NULL) {
            abort();
        }
        results = (struct pi_result *)malloc(sizeof(struct pi_result) * world_size);
        if (results == NULL) {
            abort();
        }

        // local_pp
        results[0].result[1] = tosses / world_size;
        // local_pi
        results[0].result[0] = pi_toss(results[0].result[1]);

        for (int i = 1; i < world_size; i++) {
            MPI_Irecv(&results[i].result, 2, MPI_UNSIGNED_LONG, i, MPIMSG_TAG_TOSS, MPI_COMM_WORLD, &requests[i]);
        }

        MPI_Waitall(world_size - 1, requests + 1, MPI_STATUSES_IGNORE);

        free(requests);
    }

    if (world_rank == 0) {
        uint64_t pi = 0;
        uint64_t pp = 0;
        for (int i = 0; i < world_size; i++) {
            pi += results[i].result[0];
            pp += results[i].result[1];
        }
        free(results);

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
