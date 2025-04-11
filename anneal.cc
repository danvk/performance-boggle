// Use Simulated Annealing to find a good Boggle board.
//
// To find a "nearby" board, a random set of mutations are performed. These
// all fall into two classes:
//
//   1. Letter changes
//   2. Cell swaps
//
// The ratio of these two is controlled by a command-line flag. It defaults to 1:1.
//
// The number of mutations follows a geometric distribution, whose parameter is
// also set by a command line flag. It defaults to p=0.75.
//
// The cooling schedule is:
//   T = T0 * exp(-k * n)
//
// The transition probability is:
//   P(cur_score, new_score, T) =
//                              1.0  if new_score >= cur_score
//   exp((new_score - cur_score)/T)  if new_score < cur_score

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "mtrandom/randomc.h"
#include "trie.h"
#include "boggle_solver.h"
#include "optimizer.h"
#include "3x3/boggler.h"
#include "3x4/boggler.h"
#include "4x4/boggler.h"
#include "init.h"

DEFINE_double(cool_t0, 100.0, "Initial temperature");
DEFINE_double(cool_k, 0.05, "Cooling constant (controls pace of cooling)");
DEFINE_double(swap_ratio, 1.0, "Ratio of swaps to letter changes");
DEFINE_double(mutation_p, 0.75, "Probability of just one change/mutation");
DEFINE_int32(max_stall, 1000, "Number of generations before a change to exit");
DEFINE_int32(rand_seed, -1, "Random seed (-1 means use time + pid)");

DEFINE_bool(print_stats, false, "Print statistics after the annealing run");

DEFINE_string(dictionary, "words", "Path to dictionary of words");
DEFINE_int32(size, 44, "Type of boggle board to use (MN = MxN)");

DEFINE_int32(num_runs, 1, "Number of simulated annealing runs to do.");

typedef TRandomMersenne Random;

// w/ logging compiled in but not on: 0.14s w/ rand_seed=1 (brdoaects)

int main(int argc, char** argv) {
  Init(&argc, &argv);
  if (FLAGS_rand_seed == -1) {
    FLAGS_rand_seed = time(NULL) + getpid();
  }

  LOG(INFO) << "Annealing parameters:";
  LOG(INFO) << " cool_t0: " << FLAGS_cool_t0;
  LOG(INFO) << " cool_k: " << FLAGS_cool_k;
  LOG(INFO) << " swap_ratio: " << FLAGS_swap_ratio;
  LOG(INFO) << " mutation_p: " << FLAGS_mutation_p;
  LOG(INFO) << " max_stall: " << FLAGS_max_stall;
  LOG(INFO) << " rand_seed: " << FLAGS_rand_seed;
  LOG(INFO) << " dictionary: " << FLAGS_dictionary;

  Annealer::Options opts;
  opts.cool_t0 = FLAGS_cool_t0;
  opts.cool_k = FLAGS_cool_k;
  opts.swap_ratio = FLAGS_swap_ratio;
  opts.mutation_p = FLAGS_mutation_p;
  opts.max_stall = FLAGS_max_stall;

  BoggleSolver* solver =
    BoggleSolver::Create(FLAGS_size, FLAGS_dictionary.c_str());

  // TODO(danvk): sanity-check parameters
  Random r(FLAGS_rand_seed);
  BoggleMTRandom mt_wrap(&r);
  Annealer annealer(solver, opts, &mt_wrap);

  for (int run = 0; run < FLAGS_num_runs; run++) {
    annealer.Run();
    printf("%d\t%s (%d iterations)\n", annealer.FinalScore(), annealer.FinalBoard(), annealer.FinalStats().num_iterations);
  }

  if (FLAGS_print_stats) {
    printf(" transitions: %d\n", annealer.FinalStats().transitions);
    printf("       swaps: %d\n", annealer.FinalStats().swaps);
    printf("     changes: %d\n", annealer.FinalStats().changes);
    printf("   mutations: %d\n", annealer.FinalStats().mutations);
    printf("mutate_calls: %d\n", annealer.FinalStats().mutate_calls);
  }
}
