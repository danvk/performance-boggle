#include <algorithm>
#include <iostream>
#include "ibuckets.h"
#include "boggler.h"
using std::min;
using std::max;

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
      if (cell > 15) return false;  // too many cells
    } else {
      if (c < 'a' || c > 'z') return false;  // invalid letter
      bd_[cell][cell_pos++] = c;
      if (cell_pos >= 27) return false;  // too many letters on a cell
    }
  }
  bd_[cell][cell_pos] = '\0';
  return (cell_pos > 0 && cell == 15);
}

uint64_t BucketBoggler::NumReps() const {
  uint64_t reps = 1;
  for (int i = 0; i < 16; i++)
    reps *= strlen(bd_[i]);
  return reps;
}

int BucketBoggler::ShedLetters(int cutoff) {
  int shed = 0;
  int bound = details_.max_nomark;
  for (int i = 0; i < 16; i++) {
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
int max_delta[16*26];
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
  for (int i = 0; i < 16; i++) {
    int max_score = 0;
    int choice = -1;
    memset(max_max_delta, 0, sizeof(max_max_delta));

    DoAllDescents(i, 0, dict_, &choice, &max_score, max_max_delta);
    details_.max_nomark += max_score;
    for (int j=0; bd_[i][j]; j++) {
      char c = bd_[i][j];
      details_.max_delta[i][c - 'a'] += max_max_delta[cell_indices_[i] + j];
    }
    for (int j = 0; j < num_letters_; j++)
      sum_max_delta[j] += max_max_delta[j];
  }

  int pos = 0;
  for (int i = 0; i < 16; i++) {
    for (int j = 0; bd_[i][j]; j++) {
      details_.max_delta[i][bd_[i][j] - 'a'] = sum_max_delta[pos++];
    }
  }

  return BestBound();
}

// TODO: does o_max_delta need to be a pointer here?
int actual_bd_[16];
inline void BucketBoggler::DoAllDescents(int idx, int len, SimpleTrie* t,
                                         int* o_choice, int* o_max_score,
                                         int* o_max_delta) {
  int consequences[26];
  int choice = -1;
  int max_score = 0;
  for (char* c = bd_[idx]; *c; c++) {
    int cc = actual_bd_[idx] = *c - 'a';
    if (t->StartsWord(cc)) {
      int tscore = DoDFS(idx, len + (cc==kQ ? 2 : 1), t->Descend(cc));
      consequences[cc] = tscore;
      if (tscore > max_score) {
        choice = cc;
        max_score = tscore;
        memcpy(o_max_delta, max_delta, num_letters_ * sizeof(*o_max_delta));
      }
    } else {
      consequences[cc] = 0;
    }
  }

  *o_max_score = max_score;
  *o_choice = choice;
  for (int j = 0; bd_[idx][j]; j++) {
    int cc = bd_[idx][j] - 'a';
    o_max_delta[cell_indices_[idx] + j] += (max_score - consequences[cc]);
  }
}

int BucketBoggler::DoDFS(int i, int len, SimpleTrie* t) {
  int score = 0;
  int sum_max_delta[num_letters_];
  int max_max_delta[num_letters_];
  memset(sum_max_delta, 0, sizeof(sum_max_delta));
  used_ ^= (1 << i);

  // TODO: once this is correct, unroll these loops for a huge perf win.
  int x = i / 4, y = i % 4;
  for (int dx = -1; dx <= 1; dx++) {
    if (x + dx < 0 || x + dx > 3) continue;
    for (int dy = -1; dy <= 1; dy++) {
      if (y + dy < 0 || y + dy > 3) continue;
      int idx = (x+dx) * 4 + y + dy;
      if ((used_ & (1 << idx)) == 0) {
        int max_score, choice;
        memset(max_max_delta, 0, sizeof(max_max_delta));
        DoAllDescents(idx, len, t, &choice, &max_score, max_max_delta);
        score += max_score;
        for (int i=0; i<num_letters_; i++)
          sum_max_delta[i] += max_max_delta[i];
      }
    }
  }

  if (t->IsWord()) {
    int word_score = BogglerBase::kWordScores[len];
    score += word_score;

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
  for (int i=0; i<16; i++) {
    int worst_pick = INT_MAX;
    for (char* c = bd_[i]; *c; c++) {
      int delta = details_.max_delta[i][*c - 'a'];
      worst_pick = min(delta, worst_pick);
    }
    details_.one_level_win = max(details_.one_level_win, worst_pick);
  }

  return min(details_.max_nomark - details_.one_level_win,
             details_.sum_union);
}

const char* BucketBoggler::as_string() {
  char* c = board_rep_;
  for (int i=0; i<16; i++) {
    strcpy(c, bd_[i]);
    c += strlen(bd_[i]);
    *c++ = (i == 15 ? '\0' : ' ');
  }
  return board_rep_;
}

void BucketBoggler::SetCellIndices() {
  num_letters_ = 0;
  for (int i=0; i<16; i++) {
    cell_indices_[i] = num_letters_; 
    num_letters_ += strlen(bd_[i]);
  }
}
