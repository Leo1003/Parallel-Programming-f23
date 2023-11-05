#define _GNU_SOURCE
#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <unistd.h>

#define eprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

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

int main(int argc, char *argv[])
{
    if (argc != 3) {
        eprintf("Usage: %s <thread> <iterations>\n", argv[0]);
        exit(1);
    }

    char *remainder = NULL;
    unsigned long thread_cnt = strtoul(argv[1], &remainder, 10);
    if (*remainder != '\0' || thread_cnt == 0) {
        eprintf("Invalid thread count: %s\n", argv[1]);
        exit(1);
    }
    unsigned long long total_toss = strtoull(argv[2], &remainder, 10);
    if (*remainder != '\0') {
        eprintf("Invalid toss count: %s\n", argv[2]);
        exit(1);
    }

    pthread_t *thread_states = (pthread_t *)malloc(sizeof(pthread_t) * thread_cnt);
    if (thread_states == NULL) {
        abort();
    }
    struct thread_ctx *thread_data = (struct thread_ctx *)malloc(sizeof(struct thread_ctx) * thread_cnt);
    if (thread_data == NULL) {
        abort();
    }

    for (size_t t = 0; t < thread_cnt; t++) {
        thread_data[t].toss = total_toss / thread_cnt;
        thread_data[t].toss += (total_toss % thread_cnt) < t;

        pthread_create(&thread_states[t], NULL, thread_main, &thread_data[t]);
    }
    for (size_t t = 0; t < thread_cnt; t++) {
        pthread_join(thread_states[t], NULL);
    }

    free(thread_data);
    free(thread_states);

    long double pi = (long double)atomic_load(&point_inside);
    long double pp = (long double)atomic_load(&point_total);
    printf("%.9Lf\n", (pi / pp) * 4);

    return 0;
}
