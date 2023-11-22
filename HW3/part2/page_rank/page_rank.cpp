#include "page_rank.h"

#include <cstring>
#include <stdlib.h>
#include <cmath>
#include <omp.h>
#include <utility>
#include <vector>

#include "../common/CycleTimer.h"
#include "../common/graph.h"

// pageRank --
//
// g:           graph to process (see common/graph.h)
// solution:    array of per-vertex vertex scores (length of array is num_nodes(g))
// damping:     page-rank algorithm's damping parameter
// convergence: page-rank algorithm's convergence threshold
//
void pageRank(Graph g, double *solution, double damping, double convergence)
{
    /*
     For PP students: Implement the page rank algorithm here.  You
     are expected to parallelize the algorithm using openMP.  Your
     solution may need to allocate (and free) temporary arrays.

     Basic page rank pseudocode is provided below to get you started:

     // initialization: see example code above
    */
    const int numNodes = num_nodes(g);
    bool converged = false;

    // initialize vertex weights to uniform probability. Double
    // precision scores are used to avoid underflow for large graphs
    double *pr_t = (double *)malloc(sizeof(double) * numNodes);
    double equal_prob = 1.0 / numNodes;
    std::vector<Vertex> tail_nodes;
    for (int i = 0; i < numNodes; ++i) {
        pr_t[i] = equal_prob;
        if (outgoing_size(g, i) == 0) {
            tail_nodes.push_back(i);
        }
    }

    while (!converged) {
        fprintf(stderr, "pr_t[0] = %le\n", pr_t[0]);
        double *pr_t1 = (double *)malloc(sizeof(double) * numNodes);

        double tail_score = 0.0;
        for (int i = 0; i < tail_nodes.size(); i++) {
            tail_score += pr_t[tail_nodes[i]];
        }
        tail_score = tail_score * damping / numNodes;

        for (int i = 0; i < numNodes; i++) {
            double score = 0.0;

            const Vertex* start = incoming_begin(g, i);
            const Vertex* end = incoming_end(g, i);
            for (const Vertex* j = start; j != end; j++) {
                score += pr_t[*j] / outgoing_size(g, *j);
            }

            pr_t1[i] = (1.0 - damping) / numNodes + (damping * score) + tail_score;
        }

        // compute how much per-node scores have changed
        // quit once algorithm has converged
        double global_diff = 0.0;
        for (int i = 0; i < numNodes; i++) {
            global_diff += abs(pr_t1[i] - pr_t[i]);
        }
        converged = (global_diff < convergence);

        free(pr_t);
        pr_t = pr_t1;
        pr_t1 = NULL;
    }

    memcpy(solution, pr_t, sizeof(double) * numNodes);
    free(pr_t);
}
