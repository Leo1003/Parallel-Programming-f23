#define _GNU_SOURCE
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <unistd.h>

#ifdef DEBUG
#define PER_THREAD_ROUND 20000000
#else
#define PER_THREAD_ROUND 200000000
#endif

#define eprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

static atomic_ullong point_inside = 0;
static atomic_ullong point_total = 0;

inline static unsigned long test_point(struct drand48_data *state)
{
    double x, y;
    drand48_r(state, &x);
    drand48_r(state, &y);
    
    if ((x * x + y * y) < 1.0) {
        return 1;
    } else {
        return 0;
    }
}

static void *thread_main(void *data)
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

    unsigned long pi = 0;
    for (size_t i = 0; i < PER_THREAD_ROUND; i++) {
        pi += test_point(&state);
    }

    atomic_fetch_add(&point_inside, pi);
    atomic_fetch_add(&point_total, PER_THREAD_ROUND);

    return NULL;
}

int main()
{
    int procs = get_nprocs();
    pthread_t *thread_states = (pthread_t *)malloc(sizeof(pthread_t) * procs);
    if (thread_states == NULL) {
        abort();
    }

    for (size_t t = 0; t < procs; t++) {
        pthread_create(&thread_states[t], NULL, thread_main, NULL);
    }
    for (size_t t = 0; t < procs; t++) {
        pthread_join(thread_states[t], NULL);
    }

    free(thread_states);

    long double pi = (long double)atomic_load(&point_inside);
    long double pp = (long double)atomic_load(&point_total);
    printf("%.9Lf\n", (pi / pp) * 4);

    return 0;
}
