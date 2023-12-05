#include <cstdint>
#define _GNU_SOURCE
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#define eprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define MPIMSG_TAG_TOSS     0UL

inline static uint64_t test_point(struct drand48_data *state)
{
    double x, y;
    drand48_r(state, &x);
    drand48_r(state, &y);
    
    return (x * x + y * y) < 1.0;
}

static uint64_t pi_toss(uint64_t toss)
{
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

    uint64_t pi = 0;
    for (size_t i = 0; i < toss; i++) {
        pi += test_point(&state);
    }

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

    uint64_t toss_result = pi_toss(tosses);

    uint64_t point_total = 0;
    uint64_t point_inside = 0;
    if (world_rank > 0) {
        uint64_t sendbuf[2];
        sendbuf[0] = toss_result;
        sendbuf[1] = tosses;
        MPI_Send(sendbuf, 2, MPI_UNSIGNED_LONG, 0, MPIMSG_TAG_TOSS, MPI_COMM_WORLD);
    } else if (world_rank == 0) {
        point_inside += toss_result;
        point_total += tosses;

        for (int i = 1; i < world_size; i++) {
            MPI_Status status;
            uint64_t recvbuf[2];
            MPI_Recv(recvbuf, 2, MPI_UNSIGNED_LONG, i, MPIMSG_TAG_TOSS, MPI_COMM_WORLD, &status);
            point_inside += recvbuf[0];
            point_total += recvbuf[1];
        }
    }

    if (world_rank == 0)
    {
        pi_result = ((double)point_inside / (double)point_total) * 4;

        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }

    MPI_Finalize();
    return 0;
}
