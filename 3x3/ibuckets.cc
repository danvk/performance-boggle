#include <algorithm>
#include <iostream>
#include "ibuckets.h"
#include "boggler.h"
using std::min;
using std::max;

// For debugging:
static const bool PrintWords  = false;
static const bool PrintDeltas = false;

// First the simple routines...

// TODO: check for duplicate letters, maybe allow "^aeiou" syntax?
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

int BucketBoggler::ShedLetters(int cutoff) {
  int shed = 0;
  int bound = details_.max_nomark;
  for (int i = 0; i < 9; i++) {
    char* out = bd_[i];
    for (char* c = bd_[i]; *c; c++) {
      int d = details_.max_delta[i][*c - 'a'];
      if (d == -1) {
        return -1;  // there should be stats on all possibilities.
      } else if (bound - d >= cutoff) {
        *out++ = *c;
      } else {
        shed++;
      }
    }
    *out = '\0';
  }
  return shed;
}

// Now the fun stuff...
int max_delta[9*26];
int BucketBoggler::UpperBound(int bailout_score) {
  SetCellIndices();

  details_.max_nomark = 0;
  details_.sum_union = 0;

  int max_max_delta[num_letters_];
  int sum_max_delta[num_letters_];
  memset(details_.max_delta, 0, sizeof(details_.max_delta));
  memset(sum_max_delta, 0, sizeof(sum_max_delta));

  used_ = 0;
  memset(max_delta, 0, num_letters_ * sizeof(*max_delta));
  runs_ += 1;
  for (int i = 0; i < 9; i++) {
    int max_score = 0;
    memset(max_max_delta, 0, sizeof(max_max_delta));

    DoAllDescents(i, 0, dict_, &max_score, max_max_delta);
    details_.max_nomark += max_score;
    for (int j = 0; j < num_letters_; j++)
      sum_max_delta[j] += max_max_delta[j];
    if (details_.max_nomark > bailout_score &&
        details_.sum_union > bailout_score) {
      break;
    }
  }

  memset(details_.max_delta, -1, sizeof(details_.max_delta));
  int pos = 0;
  for (int i = 0; i < 9; i++) {
    for (int j = 0; bd_[i][j]; j++) {
      details_.max_delta[i][bd_[i][j] - 'a'] = sum_max_delta[pos++];
    }
  }

  return BestBound();
}

// TODO: make o_max_score the return value
int actual_bd_[9];
inline void BucketBoggler::DoAllDescents(int idx, int len, SimpleTrie* t,
                                         int* o_max_score, int* o_max_delta) {
  int max_scores[num_letters_];
  int these_scores[26];
  int max_score = 0;
  memset(max_scores, 0, sizeof(max_scores));
  for (int j = 0; bd_[idx][j]; j++) {
    int cc = actual_bd_[idx] = bd_[idx][j] - 'a';
    if (t->StartsWord(cc)) {
      int tscore = DoDFS(idx, len + (cc==kQ ? 2 : 1), t->Descend(cc));
      max_score = max(tscore, max_score);

      // Remember the max score if we're forced to choose this letter.
      these_scores[j] = tscore;

      // Repeat that max for each other letter choice that could be made.
      for (int i = 0; i < num_letters_; i++) {
        max_scores[i] = max(max_scores[i], tscore - max_delta[i]);
      }
    } else {
      these_scores[j] = 0;
    }
  }

  for (int j=0; bd_[idx][j]; j++)
    max_scores[cell_indices_[idx] + j] = these_scores[j];

  *o_max_score = max_score;
  for (int i = 0; i < num_letters_; i++) {
    o_max_delta[i] = max_score - max_scores[i];
  }

  if (PrintDeltas) {
    bool empty = true;
    for (int j=0; j<num_letters_ && empty; j++)
      if (o_max_delta[j]) empty = false;
    if (!empty) {
      printf("%s%2d:", std::string(len, ' ').c_str(), idx);
      for (int j=0; j<num_letters_; j++)
        printf(" %d", o_max_delta[j]);
      printf("\n");
    }
  }
}

int BucketBoggler::DoDFS(int i, int len, SimpleTrie* t) {
  int score = 0;
  int sum_max_delta[num_letters_];
  int max_max_delta[num_letters_];
  memset(sum_max_delta, 0, sizeof(sum_max_delta));
  used_ ^= (1 << i);

  // TODO: Unroll these loops w/ macros. It's only about a 7% win or less, but
  // would be worth doing before any large run.
  int x = i / 3, y = i % 3;
  for (int dx = -1; dx <= 1; dx++) {
    if (x + dx < 0 || x + dx > 2) continue;
    for (int dy = -1; dy <= 1; dy++) {
      if (y + dy < 0 || y + dy > 2) continue;
      int idx = (x+dx) * 3 + y + dy;
      if ((used_ & (1 << idx)) == 0) {
        int max_score;
        memset(max_max_delta, 0, sizeof(max_max_delta));
        DoAllDescents(idx, len, t, &max_score, max_max_delta);
        score += max_score;
        for (int i=0; i<num_letters_; i++)
          sum_max_delta[i] += max_max_delta[i];
      }
    }
  }

  if (t->IsWord()) {
    int word_score = BogglerBase::kWordScores[len];
    score += word_score;
    if (PrintWords)
      printf(" +%2d (%d,%d) %s\n", word_score, i/3, i%3,
            TrieUtils<SimpleTrie>::ReverseLookup(dict_, t).c_str());

    if (t->Mark() != runs_) {
      details_.sum_union += word_score;
      t->Mark(runs_);
    }
  }

  memcpy(max_delta, sum_max_delta, sizeof(sum_max_delta));
  used_ ^= (1 << i);
  return score;
}

int BucketBoggler::BestBound() {
  // Calculate the tightest upper bound that can be achieved by forcing a
  // letter choice in any one cell. This is picking max(min(delta))
  details_.one_level_win = 0;
  details_.most_constrained_cell = -1;
  for (int i=0; i<9; i++) {
    int worst_pick = INT_MAX;
    for (char* c = bd_[i]; *c; c++) {
      int delta = details_.max_delta[i][*c - 'a'];
      worst_pick = min(delta, worst_pick);
    }
    if (bd_[i][0] && worst_pick > details_.one_level_win) {
      details_.one_level_win = worst_pick;
      details_.most_constrained_cell = i;
    }
  }

  return min(details_.max_nomark - details_.one_level_win,
             details_.sum_union);
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
    *c++ = (i == 15 ? '\0' : ' ');
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

void BucketBoggler::PrintChoices() {
  for (int i=0; i<9; i++) {
    if (strlen(bd_[i]) < 2) continue;
    for (char* c = bd_[i]; *c; c++) {
      printf("%2d %c: %d\n", i, *c, details_.max_delta[i][*c - 'a']);
    }
  }
}

int BucketBoggler::SimpleUpperBound(int bailout_score) {
  SetCellIndices();

  details_.max_nomark = 0;
  details_.sum_union = 0;

  used_ = 0;
  runs_ += 1;
  for (int i = 0; i < 9; i++) {
    int max_score = SimpleDoAllDescents(i, 0, dict_);
    details_.max_nomark += max_score;
    if (details_.max_nomark > bailout_score &&
        details_.sum_union > bailout_score) {
      break;
    }
  }

  return min(details_.max_nomark, details_.sum_union);
}

inline int BucketBoggler::SimpleDoAllDescents(int idx, int len, SimpleTrie* t) {
  int max_score = 0;
  for (int j = 0; bd_[idx][j]; j++) {
    int cc = actual_bd_[idx] = bd_[idx][j] - 'a';
    if (t->StartsWord(cc)) {
      int tscore = SimpleDoDFS(idx, len + (cc==kQ ? 2 : 1), t->Descend(cc));
      max_score = max(tscore, max_score);
    }
  }
  return max_score;
}

int BucketBoggler::SimpleDoDFS(int i, int len, SimpleTrie* t) {
  int score = 0;
  used_ ^= (1 << i);

  int x = i / 3, y = i % 3;
  for (int dx = -1; dx <= 1; dx++) {
    if (x + dx < 0 || x + dx > 2) continue;
    for (int dy = -1; dy <= 1; dy++) {
      if (y + dy < 0 || y + dy > 2) continue;
      int idx = (x+dx) * 3 + y + dy;
      if ((used_ & (1 << idx)) == 0) {
        score += SimpleDoAllDescents(idx, len, t);
      }
    }
  }

  if (t->IsWord()) {
    int word_score = BogglerBase::kWordScores[len];
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
