// Copyright 2008 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)
//
// Play bucketed boggle w/o a bucketed trie. This could potentially be really
// slow, but will give better bounds and allow more flexible bucketing.

#include <sys/time.h>
#include "trie.h"
#include "3x3/boggler.h"
#include "3x3/ibuckets.h"
#include "gflags/gflags.h"
using namespace std;

DEFINE_string(dictionary, "words", "Dictionary file");

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

void Usage(char* prog) {
  fprintf(stderr,
          "Usage: %s [--dictionary <dict>] <class1> ... <class9>\n", prog);
  exit(1);
}

int main(int argc, char** argv) {
  if (argc != 10) Usage(argv[0]);

  const char* dict_file = FLAGS_dictionary.c_str();
  printf("loading words from %s\n", dict_file);
  SimpleTrie* t = GenericBoggler<SimpleTrie>::DictionaryFromFile(dict_file);
  BucketBoggler bb(t);

  char buf[400] = "";
  for (int i=1; i<argc; i++) {
    strcat(buf, argv[i]);
    if (i < argc-1) strcat(buf, " ");
  }
  if (!bb.ParseBoard(buf)) {
    fprintf(stderr, "Couldn't parse %s\n", buf);
    exit(1);
  }
  printf("Board: %s\n", bb.as_string());

  double start = secs();
  int score = bb.SimpleUpperBound();
  double end = secs();
  printf("Score: %u\n", score);
  printf("%f secs elapsed\n", end - start);

  const BucketBoggler::ScoreDetails& d = bb.Details();
  printf("Details:\n");
  printf(" num_reps: %llu = %fB\n", bb.NumReps(), bb.NumReps() / 1.0e9);
  printf(" sum_union: %d\n", d.sum_union);
  printf(" max_nomark: %d\n", d.max_nomark);
}
