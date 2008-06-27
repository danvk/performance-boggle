#include <algorithm>
#include <iostream>
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
// TODO: make this a 1D array with one element for each possible letter. This
// will require storing the # of letters before each cell on the board, but
// will save gobs of memory/copying.
int max_delta[16][26];
int BucketBoggler::UpperBound(int bailout_score) {
  int max_max_delta[16][26];
  details_.max_nomark = 0;
  details_.sum_union = 0;
  memset(details_.max_delta, 0, sizeof(max_delta));

  used_ = 0;
  memset(max_delta, 0, sizeof(max_delta));
  runs_ += 1;
  for (int i = 0; i < 16; i++) {
    int max_score = 0;
    int choice = -1;
    memset(max_max_delta, 0, sizeof(max_delta));

    DoAllDescents(i, 0, dict_, &choice, &max_score, &max_max_delta);
    details_.max_nomark += max_score;
    for (int i=0; i<16; i++)
      for (int j=0; j<26; j++)
        details_.max_delta[i][j] += max_max_delta[i][j];

    //std::cout << "Returning from search on " << i << std::endl;
    //for (int idx=0; idx<16; idx++) {
    //  for (int j=0; j<26; j++) {
    //    if (details_.max_delta[idx][j])
    //      std::cout << " " << details_.max_delta[idx][j];
    //    else
    //      std::cout << " .";
    //  }
    //  std::cout << std::endl;
    //}
    //std::cout << std::endl;
  }

  return min(details_.max_nomark, details_.sum_union);
}

// TODO: does o_max_delta need to be a pointer here?
int actual_bd_[16];
inline void BucketBoggler::DoAllDescents(int idx, int len, SimpleTrie* t,
                                         int* o_choice, int* o_max_score,
                                         int(*o_max_delta)[16][26]) {
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
        memcpy(o_max_delta, max_delta, sizeof(max_delta));
      }
    } else {
      consequences[cc] = 0;
    }
  }

  *o_max_score = max_score;
  *o_choice = choice;
  for (char* c = bd_[idx]; *c; c++) {
    int cc = *c - 'a';
    //std::cout << "  setting " << idx << "," << cc
    //          << " = " << (max_score-consequences[cc]) << std::endl;
    (*o_max_delta)[idx][cc] += (max_score - consequences[cc]);
  }
}

int BucketBoggler::DoDFS(int i, int len, SimpleTrie* t) {
  //std::cout << "DoDFS("
  //          << TrieUtils<SimpleTrie>::ReverseLookup(dict_, t)
  //          << ", idx=" << i << ", len=" << len << ")" << std::endl;

  int score = 0;
  int sum_max_delta[16][26];
  int max_max_delta[16][26];
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
        memset(max_max_delta, 0, sizeof(sum_max_delta));
        DoAllDescents(idx, len, t, &choice, &max_score, &max_max_delta);
        score += max_score;
        for (int i=0; i<16; i++)
          for (int j=0; j<26; j++)
            sum_max_delta[i][j] += max_max_delta[i][j];
      }
    }
  }

  if (t->IsWord()) {
    int word_score = BogglerBase::kWordScores[len];
    //std::cout << " Score " << word_score
    //          << " for " << TrieUtils<SimpleTrie>::ReverseLookup(dict_, t)
    //          << std::endl;
    score += word_score;

    if (t->Mark() != runs_) {
      details_.sum_union += word_score;
      t->Mark(runs_);
    }
  }

  memcpy(max_delta, sum_max_delta, sizeof(max_delta));
  //std::cout << "Returning from "
  //          << TrieUtils<SimpleTrie>::ReverseLookup(dict_, t)
  //          << ", idx=" << i << ", len=" << len << std::endl;
  //for (int idx=0; idx<16; idx++) {
  //  for (int j=0; j<26; j++) {
  //    if (max_delta[idx][j])
  //      std::cout << " " << max_delta[idx][j];
  //    else
  //      std::cout << " .";
  //  }
  //  std::cout << std::endl;
  //}
  //std::cout << std::endl;
  used_ ^= (1 << i);
  return score;
}
