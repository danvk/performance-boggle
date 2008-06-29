// Copyright 2008 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)
//
// Play bucketed boggle w/o a bucketed trie. This could potentially be really
// slow, but will give better bounds and allow more flexible bucketing.

#include <algorithm>
#include <math.h>
#include <sys/time.h>
#include <string>
#include "trie.h"
#include "boggler.h"
#include "ibuckets.h"
const int NumBoards = 2;  //00;
using std::string;

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

// Generates a random bucketed board
// aeiou sy bdfgjkmpvwxz chlnrt
// aeio su fjkmvwxz dlnrt bcghpy
void RandomBoard(char* buf) {
  //static char* classes[] = {"aeiou", "sy", "bdfgjkmpvwxz", "chlnrt"};
  //static char* classes[] = {"aeio", "su", "fjkmvwxz", "dlnrt", "bcghpy"};
  //static char* classes[] = {"aeiou", "chsy", "dlnrtbfgjkmpvwxz" };
  static char* classes[] = {"abcdef", "ghijkl", "mnoprs", "tuvwxyz"};

  static int num_classes = sizeof(classes)/sizeof(*classes);

  *buf = '\0';
  for (int i=0; i<16; i++) {
    strcat(buf, classes[random() % num_classes]);
    if (i < 15) strcat(buf, " ");
  }
}

// Use the board solving details to pick a bucket to kill and guess how many
// board representatives this will kill.
double DecreaseProb(int now, int then);
int PickABucket(BucketBoggler& bb, double* expected_kills) {
  int pick = -1;
  *expected_kills = 1.0;
  const BucketBoggler::ScoreDetails& d = bb.Details();
  int base_score = d.max_nomark;
  uint64_t reps = bb.NumReps();
  for (int i=0; i<16; i++) {
    double expectation = 0.0;
    int choices = strlen(bb.Cell(i));
    for (int j = 0; j < choices; j++) {
      int reduced_score = base_score - d.max_delta[i][bb.Cell(i)[j] - 'a'];
      expectation += (reps / choices) * DecreaseProb(reduced_score, 3625);
    }
    //printf("%2d: expect to kill %lf\n", i, expectation);
    if (expectation > *expected_kills) {
      *expected_kills = expectation;
      pick = i;
    }
  }

  if (pick == -1) {
    pick = bb.Details().most_constrained_cell;
  }

  //for (int j = 0; bb.Cell(pick)[j]; j++) {
  //  int reduced_score = base_score - d.max_delta[pick][bb.Cell(pick)[j] - 'a'];
  //  printf("%c => %d @ %f\n", bb.Cell(pick)[j], reduced_score,
  //                            DecreaseProb(reduced_score, 3625));
  //}
  return pick;
}
double DecreaseProb(int now, int then) {
  static const double avg_decrease = 2.16;
  static const double stddev_sqrt2 = 0.843 * sqrt(2.0);
  if (now < then) return 1.0;
  return 0.5 * (1.0 - erf((1.0 * now / then - avg_decrease) / stddev_sqrt2));
}

uint64_t orig_reps = 0;
uint64_t elim = 0;
int max_depth = 0;
bool ShedToConvergence(BucketBoggler& bb, int level) { 
  if (level > max_depth) max_depth = level;
  int shed_letters=1;
  int bound;
  do {
    bound = bb.UpperBound();
    uint64_t reps = bb.NumReps();

    printf("%s  => %llu reps, bound=%d (%d)", string(level, ' ').c_str(),
           reps, bound, bb.Details().max_nomark);
    if (bound >= 3625) {
      shed_letters = bb.ShedLetters(3625);
      uint64_t shed_reps = bb.NumReps();
      printf(", shed %d=%f: %s\n",
             shed_letters, 1.0*(reps-shed_reps)/reps, bb.as_string());
      elim += (reps - shed_reps);
    } else {
      elim += reps;
      printf(", DONE\n");
    }
  } while (bound >= 3625 && shed_letters > 0);
  return (bound < 3625);
}

void AttackBoard(BucketBoggler&, int level = 0, int num=1, int outof=1);
void SplitBucket(BucketBoggler& bb, int cell, int level) {
  char orig_bd[27 * 16];
  char orig_cell[27];
  strcpy(orig_bd, bb.as_string());
  strcpy(orig_cell, bb.Cell(cell));

  printf("%sWill evaluate %d more boards...\n",
         string(level, ' ').c_str(), strlen(orig_cell));

  for (int i=0; orig_cell[i]; i++) {
    assert(bb.ParseBoard(orig_bd));
    sprintf(bb.Cell(cell), "%c", orig_cell[i]);
    AttackBoard(bb, level + 1, 1+i, strlen(orig_cell));
  }
}

// Shed/Split until finished
void AttackBoard(BucketBoggler& bb, int level, int num, int outof) {
  uint64_t reps = bb.NumReps();
  printf("(%2.2f%%)%s (%d;%d/%d) %s (%llu)\n", 100.0*elim/orig_reps,
         string(level, ' ').c_str(), level, num, outof, bb.as_string(), reps);
  fflush(stdout);
  if (ShedToConvergence(bb, level)) {
    return;
  } else {
    double expect;
    int pick = PickABucket(bb, &expect);
    SplitBucket(bb, pick, level);
    //printf("expect to kill %lf boards by picking %d\n", expect, pick);
  }
}

int main(int argc, char** argv) {
  const char* dict_file = argv[1];
  printf("loading words from %s\n", dict_file);
  SimpleTrie* t = GenericBoggler<SimpleTrie>::DictionaryFromFile("words");
  BucketBoggler bb(t);

  if (argc == 3) {
    srandom(atoi(argv[2]));
  } else if (argc == 2) {
    time_t t = time(NULL);
    srandom(t);
    printf("Using seed %d\n", t);
  } else if (argc > 3) {
    if (argc != 18) {
      fprintf(stderr, "Need %d args, got %d\n", 18, argc);
      exit(1);
    }
  }

  double start = secs();
  uint64_t total_elim = 0;
  for (int bds=0; bds < NumBoards; bds++) {
    char bd[16 * 26] = "";
    if (argc == 18) {
      for (int i=2; i<argc; i++) {
        strcat(bd, argv[i]);
        if (i < argc-1) strcat(bd, " ");
      }
    } else {
      RandomBoard(bd);
    }

    assert(bb.ParseBoard(bd));
    elim = max_depth = 0;
    orig_reps = bb.NumReps();
    double a = secs();
      AttackBoard(bb);
    double b = secs();
    printf("%llu reps in %.2f s @ depth %d = %f bds/sec:\n  %s\n",
            elim, b-a, max_depth, 1.0*elim/(b-a), bd);
    total_elim += elim;
  }
  double end = secs();

  printf("elapsed: %fs = %f sec/break\n", end-start, 1.0*(end-start)/NumBoards);
  printf("eliminated %llu boards = %f B bds/sec equivalent\n",
         total_elim, 1.0 * total_elim / (end-start) / 1e9);
}
