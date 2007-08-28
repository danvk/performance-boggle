// Evaluates 21,632 boards (mostly slight variations on a good and bad board)
// and prints out information on the performance. Typical performance on my
// machine is ~20kbds/sec, so this test takes just over one second to complete.

#include "trie.h"
#include "boggler.h"
double secs();

int main(int argc, char** argv) {
  char* dict_file;
  if (argc == 2) dict_file = argv[1];
  else           dict_file = "words";

  Trie t;
  if (!t.LoadFile(dict_file)) {
    fprintf(stderr, "Couldn't load %s, aborting...\n", dict_file);
    return 1;
  }
  printf("Loaded %d words from dictionary\n", t.Size());

  Boggler b(&t);
  unsigned int hash = 1234;
  unsigned int prime = (1 << 20) - 3;
  unsigned int total_score = 0;

  char* bases[] = { "abcdefghijklmnop", "catdlinemaropets" };
  double start = secs();
  for (unsigned int i=0; i<sizeof(bases)/sizeof(*bases); ++i) {
    b.ParseBoard(bases[i]);
    for (int x1 = 0; x1 < 4; x1++) {
      for (int x2 = 0; x2 < 4; x2++) {
        for (int c1 = 0; c1 < 26; c1++) {
          b.SetCell(x1, 1, c1);
          for (int c2 = 0; c2 < 26; c2++) {
            b.SetCell(x2, 2, c2);
            int score = b.Score();
            hash *= (123 + score);
            hash = hash % prime;
            total_score += score;
          }
        }
      }
    }
  }
  double end = secs();
  printf("Total score: %u = %lf pts/bd\n",
      total_score, 1.0 * total_score / b.NumBoards());
  printf("Score hash: 0x%08X\n", hash);
  printf("Evaluated %d boards in %lf seconds = %lf bds/sec\n",
      b.NumBoards(), (end-start), b.NumBoards()/(end-start));

  if (hash != 0x000C1D3D) {
    fprintf(stderr, "Hash mismatch, expected 0xC1D3D\n");
    return 1;
  }
  return 0;
}

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}
