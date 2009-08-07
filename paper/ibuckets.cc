#include "ibuckets.h"

#include <algorithm>
#include <iostream>
#include <string.h>
using std::min;
using std::max;

// For debugging:
static const bool PrintWords  = false;
static const bool PrintDeltas = false;
const int kWordScores[] =
      //0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10
      { 0, 0, 0, 1, 1, 2, 3, 5, 11, 11, 11 };

namespace paper {

bool BucketBoggler::ParseBoard(const char* bd) {
  int cell = 0;
  int cell_pos = 0;
  while (char c = *bd++) {
    if (c == ' ') {
      if (cell_pos == 0) return false;  // empty cell
      bd_[cell][cell_pos] = '\0';
      cell += 1;
      cell_pos = 0;
      if (cell > 8) return false;  // too many cells
    } else if (c == '.') {
      // explicit "don't go here" cell, useful for tests
      bd_[cell][0] = '\0';
      cell_pos = 1;
    } else {
      if (c < 'a' || c > 'z') return false;  // invalid letter
      bd_[cell][cell_pos++] = c;
      if (cell_pos >= 27) return false;  // too many letters on a cell
    }
  }
  bd_[cell][cell_pos] = '\0';
  return (cell_pos > 0 && cell == 8);
}

uint64_t BucketBoggler::NumReps() const {
  uint64_t reps = 1;
  for (int i = 0; i < 9; i++)
    reps *= strlen(bd_[i]);
  return reps;
}

const char* BucketBoggler::as_string() {
  char* c = board_rep_;
  for (int i=0; i<9; i++) {
    if (*bd_[i]) {
      strcpy(c, bd_[i]);
      c += strlen(bd_[i]);
    } else {
      strcpy(c++, ".");
    }
    *c++ = (i == 8 ? '\0' : ' ');
  }
  return board_rep_;
}

void BucketBoggler::SetCellIndices() {
  num_letters_ = 0;
  for (int i=0; i<9; i++) {
    cell_indices_[i] = num_letters_; 
    num_letters_ += strlen(bd_[i]);
  }
}

int BucketBoggler::UpperBound(int bailout_score) {
  SetCellIndices();

  details_.max_nomark = 0;
  details_.sum_union = 0;

  used_ = 0;
  runs_ += 1;
  for (int i = 0; i < 9; i++) {
    int max_score = DoAllDescents(i, 0, dict_);
    details_.max_nomark += max_score;
    if (details_.max_nomark > bailout_score &&
        details_.sum_union > bailout_score) {
      break;
    }
  }

  return min(details_.max_nomark, details_.sum_union);
}

inline int BucketBoggler::DoAllDescents(int idx, int len, Trie* t) {
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

int BucketBoggler::DoDFS(int i, int len, Trie* t) {
  int score = 0;
  used_ ^= (1 << i);

  // TODO(danvk): unroll this
  int x = i / 3, y = i % 3;
  for (int dx = -1; dx <= 1; dx++) {
    if (x + dx < 0 || x + dx > 2) continue;
    for (int dy = -1; dy <= 1; dy++) {
      if (y + dy < 0 || y + dy > 2) continue;
      int idx = (x+dx) * 3 + y + dy;
      if ((used_ & (1 << idx)) == 0) {
        score += DoAllDescents(idx, len, t);
      }
    }
  }

  if (t->IsWord()) {
    int word_score = kWordScores[len];
    score += word_score;
    if (PrintWords)
      printf(" +%2d (%d,%d) %s\n", word_score, i/3, i%3,
            Trie::ReverseLookup(dict_, t).c_str());

    if (t->Mark() != runs_) {
      details_.sum_union += word_score;
      t->Mark(runs_);
    }
  }

  used_ ^= (1 << i);
  return score;
}

}  // namespace paper
