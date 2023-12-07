#include <atomic>
#include <mpi.h>
#include <pthread.h>
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

    uint64_t partial_pp = tosses / world_size;
    uint64_t partial_pi = pi_toss(partial_pp);

    MPI_Status status;
    int level = 0;
    if (world_rank > 0) {
        int r = world_rank;
        while (r % 2 == 0) {
            uint64_t recvbuf[2];
            eprintf("[%d] Receive from %d\n", world_rank, ((r + 1) << level));
            MPI_Recv(recvbuf, 2, MPI_UNSIGNED_LONG, ((r + 1) << level), MPIMSG_TAG_TOSS, MPI_COMM_WORLD, &status);
            partial_pi += recvbuf[0];
            partial_pp += recvbuf[1];

            r /= 2;
            level += 1;
        }

        uint64_t sendbuf[2];
        sendbuf[0] = partial_pi;
        sendbuf[1] = partial_pp;
        eprintf("[%d] Send to %d\n", world_rank, ((r - 1) << level));
        MPI_Send(sendbuf, 2, MPI_UNSIGNED_LONG, ((r - 1) << level), MPIMSG_TAG_TOSS, MPI_COMM_WORLD);
    } else {
        // world_rank == 0
        int r = world_size;
        while (r > 1) {
            uint64_t recvbuf[2];
            eprintf("[%d] Receive from %d\n", world_rank, (1 << level));
            MPI_Recv(recvbuf, 2, MPI_UNSIGNED_LONG, (1 << level), MPIMSG_TAG_TOSS, MPI_COMM_WORLD, &status);
            partial_pi += recvbuf[0];
            partial_pp += recvbuf[1];

            r /= 2;
            level += 1;
        }

        pi_result = ((double)partial_pi / (double)partial_pp) * 4;

        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }

    MPI_Finalize();
    return 0;
}
