#include "3x4/boggler.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

Boggler34::Boggler34(TrieT* t) : dict_(t) {}
Boggler34::~Boggler34() {}

void Boggler34::SetCell(int x, int y, int c) { bd_[y*4 + x] = c; }
int Boggler34::Cell(int x, int y) const { return bd_[y*4 + x]; }

int Boggler34::InternalScore() {
  used_ = 0;
  score_ = 0;
  for (int i = 0; i < 12; i++) {
    int c = bd_[i];
    if (dict_->StartsWord(c))
      DoDFS(i, 0, dict_->Descend(c));
  }
  return score_;
}

void Boggler34::DoDFS(int i, int len, TrieT* t) {
  int c = bd_[i];

  used_ ^= (1 << i);
  len += (c==kQ ? 2 : 1);
  if (t->IsWord()) {
    if (t->Mark() != runs_) {
      t->Mark(runs_);
      score_ += kWordScores[len];
    }
  }

  // Could also get rid of any two dimensionality, but maybe GCC does that?
  int cc, idx;

  // TODO(danvk): unroll this
  // i = 4*y + x
  int x = i % 4, y = i / 4;
  std::string s(len, ' ');
  for (int dx = -1; dx <= 1; dx++) {
    if (x + dx < 0 || x + dx > 3) continue;
    for (int dy = -1; dy <= 1; dy++) {
      if (dx == 0 && dy == 0) continue;
      if (y + dy < 0 || y + dy > 2) continue;
      idx = (y + dy) * 4 + x + dx;
      if ((used_ & (1 << idx)) == 0) {
        cc = bd_[idx];
        if (t->StartsWord(cc)) {
          DoDFS(idx, len, t->Descend(cc));
        }
      }
    }
  }
  used_ ^= (1 << i);
}
