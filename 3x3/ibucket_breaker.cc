// Author: danvk@google.com (Dan Vanderkam)
//
// Play bucketed boggle w/o a bucketed trie. This could potentially be really
// slow, but will give better bounds and allow more flexible bucketing.

#include "3x3/ibucket_breaker.h"
#include <algorithm>
#include <math.h>
#include <string>
#include <sys/time.h>
#include <sys/types.h>
#include <vector>

Breaker::Breaker(BucketBoggler* bb, int best_score)
  : bb_(bb), best_score_(best_score) {
}

// TODO(danvk): make these methods and stop passing around state.
double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}


// Use the board solving details to pick a bucket to kill and guess how many
// board representatives this will kill.
double DecreaseProb(int now, int then);
int PickABucket(BucketBoggler& bb, double* expected_kills,
                std::vector<std::string>* splits, int level) {
  int pick = -1;
  *expected_kills = 1.0;
  const BucketBoggler::ScoreDetails& d = bb.Details();
  int base_score = d.max_nomark;
  uint64_t reps = bb.NumReps();
  splits->clear();
  for (int i=0; i<9; i++) {
    double expectation = 0.0;
    int choices = strlen(bb.Cell(i));
    for (int j = 0; j < choices; j++) {
      int reduced_score = base_score - d.max_delta[i][bb.Cell(i)[j] - 'a'];
      expectation += (reps / choices) * DecreaseProb(reduced_score, 545);
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

  int len = strlen(bb.Cell(pick));
  if (len == 26) {
    splits->push_back("aeiou");
    splits->push_back("sy");
    splits->push_back("bdfgjkmpvwxzq");
    splits->push_back("chlnrt");
  } else if (len >= 9) {
    int num_splits = 4;  //(len < 13 ? 3 : 4);
    splits->push_back("");
    int split = 0;
    for (int i = 0; i < len; i++) {
      if (1.0 * num_splits * i >= (split+1) * len) {
        split += 1;
        splits->push_back("");
      }
      splits->back() += std::string(1, bb.Cell(pick)[i]);
    }
  } else {
    for (int j=0; bb.Cell(pick)[j]; j++) {
      splits->push_back(std::string(1, bb.Cell(pick)[j]));
    }
  }

  int out_len = 0;
  for (unsigned int i = 0; i < splits->size(); i++)
    out_len += splits->at(i).size();
  if (out_len != len) {
    printf("%s (%d) => %d\n", bb.Cell(pick), len, out_len);
    exit(1);
  }

  return pick;
}

double DecreaseProb(int now, int then) {
  static const double avg_decrease = 2.16;
  static const double stddev_sqrt2 = 0.843 * sqrt(2.0);
  if (now < then) return 1.0;
  return 0.5 * (1.0 - erf((1.0 * now / then - avg_decrease) / stddev_sqrt2));
}

// TODO(danvk): remove these
double start_time = 0.0;
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

    printf("%s  => %llu reps, bound=%d (%d)", std::string(level, ' ').c_str(),
           reps, bound, bb.Details().max_nomark);
    if (bound >= 545) {
      shed_letters = bb.ShedLetters(545);
      uint64_t shed_reps = bb.NumReps();
      printf(", shed %d=%f: %s\n",
             shed_letters, 1.0*(reps-shed_reps)/reps, bb.as_string());
      elim += (reps - shed_reps);
    } else {
      elim += reps;
      printf(", DONE\n");
    }
  } while (bound >= 545 && shed_letters > 0);
  return (bound < 545);
}

void AttackBoard(BucketBoggler&, int level = 0, int num=1, int outof=1);
void SplitBucket(BucketBoggler& bb, int level) {
  char orig_bd[27 * 9];
  char orig_cell[27];
  double expect;
  std::vector<std::string> splits;
  int cell = PickABucket(bb, &expect, &splits, level);
  printf("split cell %d\n",  cell);

  strcpy(orig_bd, bb.as_string());
  strcpy(orig_cell, bb.Cell(cell));

  printf("%sWill evaluate %lu more boards...\n",
         std::string(level, ' ').c_str(), splits.size());

  for (unsigned int i=0; i < splits.size(); i++) {
    assert(bb.ParseBoard(orig_bd));
    strcpy(bb.Cell(cell), splits[i].c_str());
    AttackBoard(bb, level + 1, 1+i, splits.size());
  }
}

// Shed/Split until finished
void AttackBoard(BucketBoggler& bb, int level, int num, int outof) {
  uint64_t reps = bb.NumReps();
  printf("(%2.2f%%)%s (%d;%d/%d) %s (%llu) est. %f s\n", 100.0*elim/orig_reps,
         std::string(level, ' ').c_str(), level, num, outof, bb.as_string(),
         reps, (secs() - start_time) * orig_reps / elim);
  fflush(stdout);
  if (ShedToConvergence(bb, level)) {
    return;
  } else {
    SplitBucket(bb, level);
    //printf("expect to kill %lf boards by picking %d\n", expect, pick);
  }
}


void Breaker::Break(BreakDetails* details) {
  std::string orig = bb_->as_string();
  details->max_depth = 0;
  details->num_reps = 0;
  details->elapsed = 0.0;

  elim = max_depth = 0;
  orig_reps = bb_->NumReps();
  start_time = secs();
    AttackBoard(*bb_);
  double b = secs();
  double a = start_time;
  printf("%llu reps in %.2f s @ depth %d = %f bds/sec:\n  %s\n",
          elim, b-a, max_depth, 1.0*elim/(b-a), orig.c_str());

  details->elapsed = b - a;
  details->max_depth = max_depth;
  details->num_reps = orig_reps;
}



// misc board parsing stuff

bool Breaker::ParseBoard(const std::string& board) {
  return bb_->ParseBoard(board.c_str());
}

bool Breaker::FromId(const std::vector<std::string>& classes, uint64_t idx) {
  char buf[26 * 9 + 1];
  buf[0] = '\0';

  int num_classes = classes.size();
  uint64_t left = idx;
  for (int i = 0; i < 9; i++) {
    strcat(buf, classes[left % num_classes].c_str());
    left /= num_classes;
    if (i < 8) strcat(buf, " ");
  }
  if (left) return false;
  return bb_->ParseBoard(buf);
}

// static
uint64_t Breaker::BoardId(const BdArray& bd, int num_classes) {
  uint64_t id = 0;
  for (int i = 8; i >= 0; i--) {
    id *= num_classes;
    id += bd[i/3][i%3];
  }
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      printf(" %d", bd[x][y]);
    }
    printf("\n");
  }
  printf("       -> %llu\n", id);
  return id;
}

// static
bool Breaker::IsCanonical(int num_classes, uint64_t idx) {
  if (idx < 0) return false;
  int bd[3][3];
  uint64_t left = idx;
  for (int i = 0; i < 9; i++) {
    bd[i/3][i%3] = left % num_classes;
    left /= num_classes;
  }
  if (left) return false;

  uint64_t orig = BoardId(bd, num_classes);
  printf("original id = %llu =? %llu\n", orig, idx);

  for (int rot = 0; rot < 2; rot++) {
    // ABC    CBA
    // DEF -> FED
    // GHI    IHG
    for (int i = 0; i < 3; i++) std::swap(bd[0][i], bd[2][i]);
    if (BoardId(bd, num_classes) < idx) return false;

    // CBA    IHG
    // FED -> FED
    // IHG    CBA
    for (int i = 0; i < 3; i++) std::swap(bd[i][0], bd[i][2]);
    if (BoardId(bd, num_classes) < idx) return false;

    // IHG    GHI
    // FED -> DEF
    // CBA    ABC
    for (int i = 0; i < 3; i++) std::swap(bd[0][i], bd[2][i]);
    if (BoardId(bd, num_classes) < idx) return false;

    if (rot == 1) break;
    // GHI    ABC    ADG
    // DEF -> DEF -> BEH
    // ABC    GHI    CFI
    for (int i = 0; i < 3; i++) std::swap(bd[i][0], bd[i][2]);
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < i; j++)
        std::swap(bd[i][j], bd[j][i]);
    if (BoardId(bd, num_classes) < idx) return false;
  }

  return true;
}
