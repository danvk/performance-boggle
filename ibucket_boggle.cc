// Copyright 2008 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)
//
// Play bucketed boggle w/o a bucketed trie. This could potentially be really
// slow, but will give better bounds and allow more flexible bucketing.

#include <iostream>
#include <sys/time.h>
#include "trie.h"
#include "breaking_tree.h"
#include "bucket_solver.h"
#include "3x3/ibuckets.h"
#include "3x4/ibuckets.h"
#include "4x4/ibuckets.h"
#include "4x4/boggler.h"  // gross
#include "gflags/gflags.h"
using namespace std;

DEFINE_string(dictionary, "words", "Dictionary file");
DEFINE_int32(size, 44, "Type of boggle board to use (MN = MxN)");

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

void Usage(char* prog) {
  fprintf(stderr, "Usage: %s <dict> <class1> ... <class16>\n", prog);
  exit(1);
}
void PrintTree(BreakingNode* node, int indentation = 0);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  printf("loading words from %s\n", FLAGS_dictionary.c_str());
  SimpleTrie* t = Boggler::DictionaryFromFile(FLAGS_dictionary.c_str());
  if (!t) {
    fprintf(stderr, "Couldn't load dictionary\n");
    exit(1);
  }

  BucketSolver* solver = NULL;
  switch (FLAGS_size) {
    case 33: solver = new BucketSolver3(t); break;
    case 34: solver = new BucketSolver34(t); break;
    case 44: solver = new BucketSolver4(t); break;
    default:
      fprintf(stderr, "Unknown board size: %d\n", FLAGS_size);
      exit(1);
  }

  char buf[400] = "";
  for (int i=1; i<argc; i++) {
    strcat(buf, argv[i]);
    if (i < argc-1) strcat(buf, " ");
  }
  if (!solver->ParseBoard(buf)) {
    fprintf(stderr, "Couldn't parse '%s'\n", buf);
    exit(1);
  }
  printf("Board: %s\n", solver->as_string());

  double start = secs();
  int score = solver->UpperBound();
  double end = secs();
  printf("Score: %u\n", score);
  printf("%f secs elapsed\n", end - start);

  const BucketSolver::ScoreDetails& d = solver->Details();
  uint64_t reps = solver->NumReps();
  printf("Details:\n");
  printf(" num_reps: %llu = %fB\n", reps, reps / 1.0e9);
  printf(" sum_union: %d\n", d.sum_union);
  printf(" max_nomark: %d\n", d.max_nomark);

  printf("tree: %08x\n", solver->Tree());
  if (solver->Tree()) {
    // PrintTree(solver->Tree()->Prune());
    cout << "Recomputed score: " << solver->Tree()->RecomputeScore() << endl;
  }
}

void PrintTree(BreakingNode* root, int indentation) {
  if (root->letter == 'R') {
    cout << "ROOT (" << root->bound << ")" << endl;
  } else if (root->letter == '\0') {
    cout << string(indentation, ' ') << "CHOICE " << root->bound << endl;
  } else {
    cout << string(indentation, ' ') << root->letter
         << " (" << root->points << "/" << root->bound << ")" << endl;
  }

  for (int i = 0; i < root->children.size(); i++) {
    if (root->children[i])
      PrintTree(root->children[i], indentation + 1);
  }
}
