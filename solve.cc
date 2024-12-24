// Simple Boggle Solver
//
// Given a dictionary and letters on the board, print the score.

#include <cstdio>
#include <string>
#include <iostream>
#include <sys/time.h>
#include "boggle_solver.h"
#include "3x3/boggler.h"
#include "3x4/boggler.h"
#include "4x4/boggler.h"
#include "gflags/gflags.h"
#include "init.h"
#include "trie.h"

const char kUsage[] =
"%s <dictionary file> [abcdefghijklmnop [qrstuvwxyzabcdef [...]]]\n"
"A 'q' is treated as 'qu'.\n";

DEFINE_string(dictionary, "words", "Dictionary file");
DEFINE_int32(size, 44, "Type of boggle board to use (MN = MxN)");

void HandleBoard(BoggleSolver* b, const char* bd);

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

int main(int argc, char** argv) {
  Init(&argc, &argv);

  FILE* f = fopen(FLAGS_dictionary.c_str(), "r");
  if (f == NULL) {
    fprintf(stderr, "Couldn't open dictionary file %s\n",
            FLAGS_dictionary.c_str());
    exit(1);
  }
  fclose(f);

  BoggleSolver* solver =
    BoggleSolver::Create(FLAGS_size, FLAGS_dictionary.c_str());

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      HandleBoard(solver, argv[i]);
    }
  } else {
    int n = 0;
    auto start_secs = secs();
    std::string s;
    while (std::cin >> s) {
      HandleBoard(solver, s.c_str());
      n += 1;
    }
    auto end_secs = secs();
    auto elapsed_secs = end_secs - start_secs;
    auto rate = n / elapsed_secs;
    fprintf(stderr, "%d boards in %.2fs = %.2f boards/s\n", n, elapsed_secs, rate);
  }
}

void HandleBoard(BoggleSolver* b, const char* bd) {
  if (!b->ParseBoard(bd)) {
    fprintf(stderr, "Couldn't parse board string '%s'\n", bd);
    return;
  }
  int score = b->Score();
  fprintf(stdout, "%s: %d\n", b->ToString().c_str(), score);
}
