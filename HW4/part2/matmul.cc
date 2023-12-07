#include <cstdlib>
#include <deque>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#define eprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define RANK_MASTER     0

#define TAG_MASTER_COMMAND      0
#define TAG_MATRIX_DIMENSIONS   1
#define TAG_MATRIX_ROW          2
#define TAG_MATRIX_DATA         3
#define TAG_RESULT_VECTOR       4

#define COMMAND_SHUTDOWN        0U
#define COMMAND_MATRIX_JOB      1U

class worker_status {
public:
    worker_status(int rank, size_t buflen)
        : busy(false)
        , rank(rank)
        , job_id(-1)
        , resultbuf(buflen)
    {}

    bool busy;
    int rank;
    int job_id;
    vector<int> resultbuf;
};

void construct_matrices(int *n_ptr, int *m_ptr, int *l_ptr, int **a_mat_ptr, int **b_mat_ptr)
{
    int world_rank, world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_rank == RANK_MASTER) {
        scanf("%d%d%d", n_ptr, m_ptr, l_ptr);

        *a_mat_ptr = (int *)malloc(sizeof(int) * *n_ptr * *m_ptr);
        if (*a_mat_ptr == NULL) {
            abort();
        }
        *b_mat_ptr = (int *)malloc(sizeof(int) * *m_ptr * *l_ptr);
        if (*b_mat_ptr == NULL) {
            abort();
        }

        int n = *n_ptr;
        int m = *m_ptr;
        int l = *l_ptr;
        int *a_mat = *a_mat_ptr;
        int *b_mat = *b_mat_ptr;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                scanf("%d", &a_mat[i * m + j]);
            }
        }
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < l; j++) {
                scanf("%d", &b_mat[i * l + j]);
            }
        }
    } else {
        // Set the worker's data to zero
        *n_ptr = 0;
        *m_ptr = 0;
        *l_ptr = 0;
        *a_mat_ptr = NULL;
        *b_mat_ptr = NULL;
    }
}

static void matrix_multiply_worker(const int rank, const int n, const int m, const int l)
{
    MPI_Status status;
    int *b_mat = (int *)malloc(sizeof(int) * m * l);
    if (b_mat == NULL) {
        MPI_Abort(MPI_COMM_WORLD, 1);
        abort();
    }
    int *a_vec = (int *)malloc(sizeof(int) * m);
    if (a_vec == NULL) {
        MPI_Abort(MPI_COMM_WORLD, 1);
        abort();
    }
    int *c_vec = (int *)malloc(sizeof(int) * l);
    if (c_vec == NULL) {
        MPI_Abort(MPI_COMM_WORLD, 1);
        abort();
    }

    MPI_Recv(b_mat, m * l, MPI_INT, RANK_MASTER, TAG_MATRIX_DATA, MPI_COMM_WORLD, &status);

    unsigned short command;
    while (true) {
        MPI_Recv(&command, 1, MPI_UNSIGNED_SHORT, RANK_MASTER, TAG_MASTER_COMMAND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (command == COMMAND_MATRIX_JOB) {
            MPI_Recv(a_vec, m, MPI_INT, RANK_MASTER, TAG_MATRIX_ROW, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int i = 0; i < l; i++) {
                int sum = 0;
                for (int j = 0; j < m; j++) {
                    sum += a_vec[j] * b_mat[i + j * l];
                    //eprintf("[Worker %d] sum += %d * %d\n", rank, a_vec[j], b_mat[i + j * l]);
                }
                c_vec[i] = sum;
                //eprintf("[Worker %d] c_vec[%d] = %d\n", rank, i, sum);
            }

            MPI_Send(c_vec, l, MPI_INT, RANK_MASTER, TAG_RESULT_VECTOR, MPI_COMM_WORLD);
        } else if (command == COMMAND_SHUTDOWN) {
            eprintf("[Worker %d] Receive Shutdown command from master. Exiting...\n", rank);
            break;
        } else {
            eprintf("[Worker %d] Received unknown command: %hu!\n", rank, command);
        }
    }

    free(c_vec);
    free(b_mat);
    free(a_vec);
}

void send_job(worker_status &worker, const int n, const int m, const int l, const int *a_mat, MPI_Request *request)
{
    const int rank = worker.rank;

    MPI_Request req = MPI_REQUEST_NULL;
    unsigned short command = COMMAND_MATRIX_JOB;
    MPI_Isend(&command, 1, MPI_UNSIGNED_SHORT, rank, TAG_MASTER_COMMAND, MPI_COMM_WORLD, &req);
    MPI_Request_free(&req);
    MPI_Isend(&a_mat[worker.job_id * m], m, MPI_INT, rank, TAG_MATRIX_ROW, MPI_COMM_WORLD, &req);
    MPI_Request_free(&req);

    worker.busy = true;

    MPI_Irecv(worker.resultbuf.data(), l, MPI_INT, rank, TAG_RESULT_VECTOR, MPI_COMM_WORLD, request);
}

void shutdown_workers(int world_size, MPI_Request *requests)
{
    unsigned short command = COMMAND_SHUTDOWN;
    requests[0] = MPI_REQUEST_NULL;
    for (int r = 1; r < world_size; r++) {
        MPI_Isend(&command, 1, MPI_UNSIGNED_SHORT, r, TAG_MASTER_COMMAND, MPI_COMM_WORLD, &requests[r]);
    }
    MPI_Waitall(world_size, requests, MPI_STATUSES_IGNORE);
}

void wait_jobs(vector<worker_status> &workers,
    deque<int> &idle_workers,
    MPI_Request *requests,
    int world_size,
    const int n,
    const int m,
    const int l,
    int *c_mat,
    bool waitall = false)
{
    if (waitall) {
        MPI_Waitall(world_size, requests, MPI_STATUSES_IGNORE);

        for (int r = 1; r < world_size; r++) {
            if (workers[r].busy == false) {
                continue;
            }

            int i = workers[r].job_id;
            //eprintf("[Master] Job %d complete from worker %d \n", i, r);
            for (int j = 0; j < l; j++) {
                c_mat[i * l + j] = workers[r].resultbuf[j];
                //eprintf("[Master] c_mat[%d] = %d\n", i * l + j, workers[r].resultbuf[j]);
            }
            workers[r].busy = false;
            workers[r].job_id = -1;
            idle_workers.push_back(r);
        }
    } else {
        MPI_Status status;
        int index;
        MPI_Waitany(world_size, requests, &index, &status);
        if (index == MPI_UNDEFINED) {
            return;
        }

        int r = status.MPI_SOURCE;
        int i = workers[r].job_id;
        //eprintf("[Master] Job %d complete from worker %d \n", i, r);
        for (int j = 0; j < l; j++) {
            c_mat[i * l + j] = workers[r].resultbuf[j];
            //eprintf("[Master] c_mat[%d] = %d\n", i * l + j, workers[r].resultbuf[j]);
        }
        workers[r].busy = false;
        workers[r].job_id = -1;
        idle_workers.push_back(r);
    }
}

void matrix_multiply(const int n, const int m, const int l, const int *a_mat, const int *b_mat)
{
    int world_rank, world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int worker_size = world_size - 1;

    int dim[3] = {n, m, l};
    MPI_Bcast(dim, 3, MPI_INT, RANK_MASTER, MPI_COMM_WORLD);

    if (world_rank == RANK_MASTER) {
        eprintf("[Master] Performing %d x %d by %d x %d matrix multiplication\n", n, m, m, l);

        MPI_Request *requests = (MPI_Request *)malloc(sizeof(MPI_Request) * world_size);
        if (requests == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
            abort();
        }
        int *c_mat = (int *)malloc(sizeof(int) * n * l);
        if (c_mat == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
            abort();
        }

        vector<worker_status> workers;
        deque<int> idle_workers;
        workers.reserve(world_size);

        for (int r = 0; r < world_size; r++) {
            workers.emplace_back(r, l);
        }
        for (int r = 1; r < world_size; r++) {
            idle_workers.push_back(r);
        }

        for (int r = 1; r < world_size; r++) {
            MPI_Isend(b_mat, m * l, MPI_INT, r, TAG_MATRIX_DATA, MPI_COMM_WORLD, &requests[r]);
        }
        MPI_Waitall(worker_size, requests + 1, MPI_STATUSES_IGNORE);

        for (int r = 0; r < world_size; r++) {
            requests[r] = MPI_REQUEST_NULL;
        }

        int progress = 0;
        while (progress < n) {
            if (idle_workers.size() > 0) {
                int w = idle_workers.front();
                idle_workers.pop_front();
                workers[w].job_id = progress++;
                //eprintf("[Master] Sending job %d to worker %d...\n", workers[w].job_id, workers[w].rank);
                send_job(workers[w], n, m, l, a_mat, &requests[w]);
            } else {
                wait_jobs(workers, idle_workers, requests, world_size, n, m, l, c_mat);
            }
        }
        eprintf("[Master] All jobs are sent. Waiting for results...\n");
        wait_jobs(workers, idle_workers, requests, world_size, n, m, l, c_mat, true);

        eprintf("[Master] Shutting down workers...\n");
        shutdown_workers(world_size, requests);

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < l; j++) {
                printf("%d ", c_mat[i * l + j]);
            }
            printf("\n");
        }

        free(c_mat);
        free(requests);
    } else {
        matrix_multiply_worker(world_rank, dim[0], dim[1], dim[2]);
    }
}

void destruct_matrices(int *a_mat, int *b_mat)
{
    // If this is worker rank, the a_mat & b_mat should be NULL as it is set in construct_matrices().
    // And the call free(NULL) is safe no-op.
    free(a_mat);
    free(b_mat);
}
