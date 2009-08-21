// Copyright 2009 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)

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
DEFINE_int32(size, 34, "Type of boggle board to use (MN = MxN)");

double secs();
void PrintTree(BucketSolver* solver, BreakingNode* node, int indentation = 0);
BucketSolver* GetSolver(SimpleTrie* t);
void ParseBoard(BucketSolver* solver, int argc, char** argv);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  printf("loading words from %s\n", FLAGS_dictionary.c_str());
  SimpleTrie* t = Boggler::DictionaryFromFile(FLAGS_dictionary.c_str());
  if (!t) {
    fprintf(stderr, "Couldn't load dictionary\n");
    exit(1);
  }

  BucketSolver* solver = GetSolver(t);
  ParseBoard(solver, argc, argv);

  solver->SetBuildTree(false);
  double start = secs();
  int score = solver->UpperBound();
  double end = secs();
  printf("No tree: Score: %u (%f secs elapsed)\n", score, end - start);

  solver->SetBuildTree(true);
  start = secs();
  score = solver->UpperBound();
  end = secs();
  printf("Build tree: Score: %u (%f secs elapsed)\n", score, end - start);

  start = secs();
  BreakingNode* tree = solver->Tree()->Prune();
  end = secs();
  printf("Prune tree: %f secs\n", end - start);

  start = secs();
  score = tree->RecomputeScore();
  end = secs();
  printf("Recomputed score: %d (%f secs elapsed)\n", score, end - start);

  solver->SetBuildTree(false);
}


double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

BucketSolver* GetSolver(SimpleTrie* t) {
  BucketSolver* solver;
  switch (FLAGS_size) {
    case 33: solver = new BucketSolver3(t); break;
    case 34: solver = new BucketSolver34(t); break;
    case 44: solver = new BucketSolver4(t); break;
    default:
      fprintf(stderr, "Unknown board size: %d\n", FLAGS_size);
      exit(1);
  }
  return solver;
}

void ParseBoard(BucketSolver* solver, int argc, char** argv) {
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
}
