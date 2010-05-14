#include "3x3/ibuckets.h"

#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <string.h>
using std::min;
using std::max;

// For debugging:
static const bool PrintWords  = false;
static const bool PrintDeltas = false;

int BucketSolver3::Width() const { return 3; }
int BucketSolver3::Height() const { return 3; }

char* BucketSolver3::MutableCell(int idx) { return bd_[idx]; }
const char* BucketSolver3::Cell(int idx) const { return bd_[idx]; }

void BucketSolver3::InternalUpperBound(int bailout_score) {
  for (int i = 0; i < 9; i++) {
    int max_score = DoAllDescents(i, 0, dict_);
    details_.max_nomark += max_score;
    if (details_.max_nomark > bailout_score &&
        details_.sum_union > bailout_score) {
      break;
    }
  }
}

int BucketSolver3::DoAllDescents(int idx, int len, SimpleTrie* t) {
  int max_score = 0;
  for (int j = 0; bd_[idx][j]; j++) {
    int cc = bd_[idx][j] - 'a';
    if (t->StartsWord(cc)) {
      int tscore = DoDFS(idx, len + (cc==kQ ? 2 : 1), t->Descend(cc));
      max_score = max(tscore, max_score);
    }
  }
  return max_score;
}

int BucketSolver3::DoDFS(int i, int len, SimpleTrie* t) {
  int score = 0;
  used_ ^= (1 << i);

// Using these macros avoids all kinds of branching.
  int idx;
#define HIT(x,y) do { idx = (x) * 3 + y; \
                      if ((used_ & (1 << idx)) == 0) { \
                        score += DoAllDescents(idx, len, t); \
                      } \
                    } while(0)
#define HIT3x(x,y) HIT(x,y); HIT(x+1,y); HIT(x+2,y)
#define HIT3y(x,y) HIT(x,y); HIT(x,y+1); HIT(x,y+2)
#define HIT8(x,y) HIT3x(x-1,y-1); HIT(x-1,y); HIT(x+1,y); HIT3x(x-1,y+1)

  // x*3 + y
  switch (i) {
    case 0*3 + 0: HIT(0, 1); HIT(1, 0); HIT(1, 1); break;
    case 0*3 + 1: HIT(0, 0); HIT3y(1, 0); HIT(0, 2); break;
    case 0*3 + 2: HIT(0, 1); HIT(1, 1); HIT(1, 2); break;

    case 1*3 + 0: HIT(0, 0); HIT(2, 0); HIT3x(0, 1); break;
    case 1*3 + 1: HIT8(1, 1); break;
    case 1*3 + 2: HIT3x(0, 1); HIT(0, 2); HIT(2, 2); break;

    case 2*3 + 0: HIT(1, 0); HIT(1, 1); HIT(2, 1); break;
    case 2*3 + 1: HIT3y(1, 0); HIT(2, 0); HIT(2, 2); break;
    case 2*3 + 2: HIT(1, 2); HIT(1, 1); HIT(2, 1); break;
  }

#undef HIT
#undef HIT3x
#undef HIT3y
#undef HIT8

  if (t->IsWord()) {
    int word_score = kWordScores[len];
    score += word_score;
    if (PrintWords)
      printf(" +%2d (%d,%d) %s\n", word_score, i/3, i%3,
            TrieUtils<SimpleTrie>::ReverseLookup(dict_, t).c_str());

    if (t->Mark() != runs_) {
      details_.sum_union += word_score;
      t->Mark(runs_);
    }
  }

  used_ ^= (1 << i);
  return score;
}
