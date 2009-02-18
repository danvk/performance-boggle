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
#include "mtrandom/randomc.h"
#include "trie.h"
#include "boggler.h"

static double cool_t0 = 100.0;
static double cool_k = 0.05;
static double swap_ratio = 1.0;
static double mutation_p = 0.75;
static int max_stall = 1000;
static int rand_seed = -1;  // -1 = use time + pid

typedef TRandomMersenne Random;

// Should we transition between boards with these two scores?
bool AcceptTransition(int cur_score, int new_score, double T, Random* rand) {
  if (new_score >= cur_score)
    return true;
  double p = exp((new_score - cur_score)/T);
  return (rand->Random() < p);
}

// "Temperature" after n iterations
double Temperature(int n) {
  return cool_t0 * exp(-cool_k * n);
}

// Mutate a board
static int total_swaps = 0;
static int total_changes = 0;
static int total_mutations = 0;
static int total_mutate_calls = 0;
void Mutate(char* bd, Random* rand) {
  total_mutate_calls += 1;
  do {
    total_mutations += 1;
    if ((1.0 + swap_ratio) * rand->Random() > 1.0) {
      // swap two cells
      total_swaps += 1;
      int a, b;
      do {
        int pair = rand->IRandom(0, 16 * 16 - 1);
        a = pair / 16;
        b = pair % 16;
      } while (bd[a] == bd[b]);
      char tmp = bd[a];
      bd[a] = bd[b];
      bd[b] = tmp;
    } else {
      // change a cell
      total_changes += 1;
      int cell, letter;
      do {
        int pair = rand->IRandom(0, 26 * 16 - 1);
        cell = pair / 26;
        letter = 'a' + pair % 26;
      } while (bd[cell] == letter);
      bd[cell] = letter;
    }
  } while (rand->Random() > mutation_p);
}

// A random initial board. Does not set a trailing null char!
void InitialBoard(char* bd, Random* rand) {
  for (int i = 0; i < 16; i++) {
    bd[i] = rand->IRandom('a', 'z');
  }
}

int main(int argc, char** argv) {
  int print_scores = 0;
  int print_stats = 0;
  int print_transitions = 1;
  int print_params = 1;

  while (1) {
    static struct option long_options[] = {
      { "t0",         required_argument, 0, 't' },
      { "k",          required_argument, 0, 'k' },
      { "swap_ratio", required_argument, 0, 'r' },
      { "mutation_p", required_argument, 0, 'p' },
      { "max_stall",  required_argument, 0, 'm' },
      { "rand_seed",  required_argument, 0, 's' },

      { "print_scores",        no_argument, &print_scores, 1 },
      { "print_stats",         no_argument, &print_stats, 1 },
      { "print_transitions",   no_argument, &print_transitions, 1 },
      { "print_params",        no_argument, &print_params, 1 },
      { "noprint_scores",      no_argument, &print_scores, 0 },
      { "noprint_stats",       no_argument, &print_stats, 0 },
      { "noprint_transitions", no_argument, &print_transitions, 0 },
      { "noprint_params",      no_argument, &print_params, 0 },
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long_only(argc, argv, "", long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 't': cool_t0 = atof(optarg);    break;
      case 'k': cool_k = atof(optarg);     break;
      case 'r': swap_ratio = atof(optarg); break;
      case 'p': mutation_p = atof(optarg); break;
      case 'm': max_stall = atoi(optarg);  break;
      case 's': rand_seed = atoi(optarg);  break;

      case 0: break;

      default:
        exit(1);
    }
  }
  if (rand_seed == -1) {
    rand_seed = time(NULL) + getpid();
  }

  if (print_params) {
    printf("Annealing parameters:\n");
    printf(" cool_t0: %lf\n", cool_t0);
    printf(" cool_k: %lf\n", cool_k);
    printf(" swap_ratio: %lf\n", swap_ratio);
    printf(" mutation_p: %lf\n", mutation_p);
    printf(" max_stall: %d\n", max_stall);
    printf(" rand_seed: %d\n", rand_seed);
  }

  Trie* t = Boggler::DictionaryFromFile("words");
  Boggler b(t);

  // TODO(danvk): sanity-check parameters
  Random r(rand_seed);
  char bd[17] = "";
  char last[17] = "abcdefghijklmnop";
  InitialBoard(last, &r);
  int last_accept = 0;
  int last_score = -1;
  int total_transitions = 0;
  for (int n = 0; n < last_accept + max_stall; n++) {
    memcpy(bd, last, sizeof(last));
    Mutate(bd, &r);
    int score = b.Score(bd);
    if (print_scores)
      printf("%d\t%d\n", n, score);
    double T = Temperature(n);
    if (AcceptTransition(last_score, score, T, &r)) {
      total_transitions += 1;
      last_accept = n;
      last_score = score;
      memcpy(last, bd, sizeof(bd));
      if (print_transitions)
        printf("%5d T=%3.8lf accepting '%s' (%d)\n", n, T, last, last_score);
    }
  }

  printf(" final board: %s\n", last);
  printf(" final score: %d\n", last_score);

  if (print_stats) {
    printf(" transitions: %d\n", total_transitions);
    printf("       swaps: %d\n", total_swaps);
    printf("     changes: %d\n", total_changes);
    printf("   mutations: %d\n", total_mutations);
    printf("mutate_calls: %d\n", total_mutate_calls);
  }
}
