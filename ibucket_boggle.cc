// Copyright 2008 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)
//
// Play bucketed boggle w/o a bucketed trie. This could potentially be really
// slow, but will give better bounds and allow more flexible bucketing.

#include <sys/time.h>
#include "trie.h"
#include "boggler.h"
#include "ibuckets.h"
using namespace std;

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

void Usage(char* prog) {
  fprintf(stderr, "Usage: %s <dict> <class1> ... <class16>\n", prog);
  exit(1);
}

int main(int argc, char** argv) {
  if (argc != 18) Usage(argv[0]);

  const char* dict_file = argv[1];
  printf("loading words from %s\n", dict_file);
  SimpleTrie* t = GenericBoggler<SimpleTrie>::DictionaryFromFile(dict_file);
  BucketBoggler bb(t);

  char buf[400] = "";
  for (int i=2; i<argc; i++) {
    //strcpy(bb.Cell(i-2), argv[i]);
    strcat(buf, argv[i]);
    if (i < argc-1) strcat(buf, " ");
  }
  if (!bb.ParseBoard(buf)) {
    fprintf(stderr, "Couldn't parse %s\n", buf);
    exit(1);
  }
  printf("Board: %s\n", bb.as_string());

  double start = secs();
  int score = bb.UpperBound();
  double end = secs();
  printf("Score: %u\n", score);
  printf("%f secs elapsed\n", end - start);

  const BucketBoggler::ScoreDetails& d = bb.Details();
  printf("Details:\n");
  printf(" num_reps: %llu = %fB\n", bb.NumReps(), bb.NumReps() / 1.0e9);
  printf(" sum_union: %d\n", d.sum_union);
  printf(" max_nomark: %d\n", d.max_nomark);

  int cell = d.most_constrained_cell;
  printf(" max+one: %d (force cell %d)\n",
         d.max_nomark - d.one_level_win, cell);
  if (cell >= 0) {
    char* c = bb.Cell(cell);
    for (int i=0; c[i]; i++) {
      printf("   %c: %d\n", c[i], d.max_nomark - d.max_delta[cell][c[i] - 'a']);
    }
  }
}
