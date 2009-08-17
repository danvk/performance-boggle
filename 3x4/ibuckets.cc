#include "3x4/ibuckets.h"

#include <algorithm>
#include <iostream>
using std::min;
using std::max;

// For debugging:
static const bool PrintWords  = false;
static const bool PrintDeltas = false;

int BucketSolver34::Width() const { return 4; }
int BucketSolver34::Height() const { return 3; }

char* BucketSolver34::MutableCell(int idx) { return bd_[idx]; }
const char* BucketSolver34::Cell(int idx) const { return bd_[idx]; }

void BucketSolver34::InternalUpperBound(int bailout_score) {
  for (int i = 0; i < 12; i++) {
    int max_score = DoAllDescents(i, 0, dict_);
    details_.max_nomark += max_score;
    if (details_.max_nomark > bailout_score &&
        details_.sum_union > bailout_score) {
      break;
    }
  }
}

int BucketSolver34::DoAllDescents(int idx, int len, SimpleTrie* t) {
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

int BucketSolver34::DoDFS(int i, int len, SimpleTrie* t) {
  int score = 0;
  used_ ^= (1 << i);

  // TODO(danvk): unroll
  int x = i % 4, y = i / 4;
  for (int dx = -1; dx <= 1; dx++) {
    if (x + dx < 0 || x + dx > 3) continue;
    for (int dy = -1; dy <= 1; dy++) {
      if (y + dy < 0 || y + dy > 2) continue;
      int idx = (y + dy) * 4 + x + dx;
      if ((used_ & (1 << idx)) == 0) {
        score += DoAllDescents(idx, len, t);
      }
    }
  }

  if (t->IsWord()) {
    int word_score = kWordScores[len];
    score += word_score;
    if (PrintWords)
      printf(" +%2d (%d,%d) %s\n", word_score, i%4, i/4,
            TrieUtils<SimpleTrie>::ReverseLookup(dict_, t).c_str());

    if (t->Mark() != runs_) {
      details_.sum_union += word_score;
      t->Mark(runs_);
    }
  }

  used_ ^= (1 << i);
  return score;
}
