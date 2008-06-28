#include <assert.h>
#include <sys/time.h>
#include <iostream>
#include "ibuckets.h"
#include "trie.h"
#include "boggler.h"
const int NumBoards = 100;

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

// Generates a random bucketed board
void RandomBoard(char* buf) {
  int used[26];
  for (int i=0; i<16; i++) {
    memset(used, 0, sizeof(used));
    for (int j=0; j<4; j++) {
      int c;
      do { c = random() % 26; } while (used[c]);
      *buf++ = 'a' + c;
      used[c] = 1;
    }
    if (i<15) *buf++ = ' ';
  }
  *buf = '\0';
}

int main(int argc, char** argv) {
  SimpleTrie* t = GenericBoggler<SimpleTrie>::DictionaryFromFile("words");
  BucketBoggler bb(t);

  srandom(10);
  char bd[NumBoards][16 * 26];
  for (int i=0; i < NumBoards; i++) {
    RandomBoard(bd[i]);
  }

  int ruled_out[3] = {0, 0, 0};
  double start = secs();
  for (int i=0; i < NumBoards; i++) {
    assert(bb.ParseBoard(bd[i]));
    int bound = bb.UpperBound();
    uint64_t reps = bb.NumReps();
    if (bb.Details().sum_union < 3625)  ruled_out[0] += 1;
    if (bb.Details().max_nomark < 3625) ruled_out[1] += 1;
    if (bb.Details().max_nomark - bb.Details().one_level_win < 3626)
      ruled_out[2] += 1;
  }
  double end = secs();
  printf("elapsed: %fs = %f bds/sec\n", end-start, 1.0*NumBoards/(end-start));
  printf("Fraction ruled out by:\n");
  printf("  sum_union: %3d/%3d = %f\n",
         ruled_out[0], NumBoards, 1.0 * ruled_out[0] / NumBoards);
  printf(" max_nomark: %3d/%3d = %f\n",
         ruled_out[1], NumBoards, 1.0 * ruled_out[1] / NumBoards);
  printf("    max+one: %3d/%3d = %f\n",
         ruled_out[2], NumBoards, 1.0 * ruled_out[2] / NumBoards);
}
