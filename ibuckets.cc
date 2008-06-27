#include <algorithm>
#include "ibuckets.h"
#include "boggler.h"
using std::min;

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
  }
  return shed;
}

// Now the fun stuff...
int BucketBoggler::UpperBound(int bailout_score) {
  details_.max_nomark = 0;
  details_.sum_union = 0;
  used_ = 0;
  runs_ += 1;
  for (int i = 0; i < 16; i++) {
    int max_score = 0;
    int choice = -1;
    DoAllDescents(i, 0, dict_, &choice, &max_score);
    details_.max_nomark += max_score;
  }

  return min(details_.max_nomark, details_.sum_union);
}

void BucketBoggler::DoAllDescents(int idx, int len, SimpleTrie* t,
                                  int* o_choice, int* o_max_score) {
  int choice = -1;
  int max_score = 0;
  for (char* c = bd_[idx]; *c; c++) {
    int cc = *c - 'a';
    if (t->StartsWord(cc)) {
      int tscore = DoDFS(idx, len + (cc==kQ ? 2 : 1), t->Descend(cc));
      if (tscore > max_score) {
        choice = cc;
        max_score = tscore;
      }
    }
  }
  *o_max_score = max_score;
  *o_choice = choice;
}

int BucketBoggler::DoDFS(int i, int len, SimpleTrie* t) {
  int score = 0;
  used_ ^= (1 << i);

  if (t->IsWord()) {
    int word_score = BogglerBase::kWordScores[len];
    score += word_score;
    if (t->Mark() != runs_) {
      details_.sum_union += word_score;
      t->Mark(runs_);
    }
  }

  // TODO: once this is correct, unroll these loops for a huge perf win.
  int x = i / 4, y = i % 4;
  for (int dx = -1; dx <= 1; dx++) {
    if (x + dx < 0 || x + dx > 3) continue;
    for (int dy = -1; dy <= 1; dy++) {
      if (y + dy < 0 || y + dy > 3) continue;
      int idx = (x+dx) * 4 + y + dy;
      if ((used_ & (1 << idx)) == 0) {
        int max_score, choice;
        DoAllDescents(idx, len, t, &choice, &max_score);
        score += max_score;
      }
    }
  }

  used_ ^= (1 << i);
  return score;
}
