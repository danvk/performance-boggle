#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <map>
#include "mtrandom/randomc.h"
#include "test.h"
#include "boggle_solver.h"

double secs();

typedef TRandomMersenne Random;

int main(int argc, char** argv) {
  BoggleSolver* solver = BoggleSolver::Create(33, "words");

  Random r(0xb0881e);

  unsigned int prime = (1 << 20) - 3;
  unsigned int total_score = 0;
  unsigned int hash;
  unsigned int reps = 1000000;

  hash = 1234;
  double start = secs();
  for (int rep = 0; rep < reps; rep++) {
    unsigned int a = r.BRandom();
    unsigned int b = r.BRandom();
    uint64_t c = ((uint64_t)(a) << 32) + b;
    for (unsigned int x = 0; x < 3; x++) {
      for (unsigned int y = 0; y < 3; y++) {
        solver->SetCell(x, y, c % 26);
        c = c / 26;
      }
    }

    int score = solver->Score();
    hash *= (123 + score);
    hash = hash % prime;
    total_score += score;
  }

  double end = secs();
  printf("Total score: %u = %lf pts/bd\n",
      total_score, 1.0 * total_score / solver->NumBoards());
  printf("Score hash: 0x%08X\n", hash);
  printf("Evaluated %d boards in %lf seconds = %lf bds/sec\n",
      solver->NumBoards(), (end-start), solver->NumBoards()/(end-start));
  printf("%s: All tests passed!\n", argv[0]);
  return 0;
}

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}
