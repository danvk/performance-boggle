// Copyright 2008 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)
//
// Play bucketed boggle w/o a bucketed trie. This could potentially be really
// slow, but will give better bounds and allow more flexible bucketing.

#include <algorithm>
#include <math.h>
#include <sys/time.h>
#include "trie.h"
#include "boggler.h"
#include "ibuckets.h"
const int NumBoards = 10;

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

uint64_t elim = 0;
bool ShedToConvergence(BucketBoggler& bb) {
  int shed_letters=1;
  int bound;
  do {
    bound = bb.UpperBound();
    uint64_t reps = bb.NumReps();

    printf("  => %llu reps, bound=%d (%d)",
           reps, bound, bb.Details().max_nomark);
    if (bound >= 3625) {
      shed_letters = bb.ShedLetters(3625);
      uint64_t shed_reps = bb.NumReps();
      printf(", shed %d=%f: %s\n",
             shed_letters, 1.0*(reps-shed_reps)/reps, bb.as_string());
    } else {
      printf(", DONE\n");
    }
  } while (bound >= 3625 && shed_letters > 0);
  return (bound < 3625);
}

int main(int argc, char** argv) {
  const char* dict_file = argv[1];
  printf("loading words from %s\n", dict_file);
  SimpleTrie* t = GenericBoggler<SimpleTrie>::DictionaryFromFile("words");
  BucketBoggler bb(t);

  double start = secs();
  for (int i=0; i < NumBoards; i++) {
    char bd[16 * 26];
    RandomBoard(bd);
    assert(bb.ParseBoard(bd));
    printf("%3d: %s\n", i, bb.as_string());
    uint64_t reps = bb.NumReps();
    if (ShedToConvergence(bb)) {
      elim += reps;
    } else {
      elim += (reps - bb.NumReps());
    }
    printf("\n");
  }
  double end = secs();
  printf("elapsed: %fs = %f evals/sec\n", end-start, 1.0*NumBoards/(end-start));
  printf("eliminated %llu boards = %f B bds/sec equivalent\n",
         elim, 1.0 * elim / (end-start) / 1e9);
}
