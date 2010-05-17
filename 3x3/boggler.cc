#include "3x3/boggler.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

Boggler3::Boggler3(TrieT* t) : dict_(t) {}
Boggler3::~Boggler3() { delete dict_; }

void Boggler3::SetCell(int x, int y, int c) { bd_[x*3 + y] = c; }
int Boggler3::Cell(int x, int y) const { return bd_[x*3 + y]; }

int Boggler3::InternalScore() {
  used_ = 0;
  score_ = 0;
  for (int i = 0; i < 9; i++) {
    int c = bd_[i];
    if (dict_->StartsWord(c))
      DoDFS(i, 0, dict_->Descend(c));
  }
  return score_;
}

void Boggler3::DoDFS(int i, int len, TrieT* t) {
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

  // To help the loop unrolling...
#define HIT(x,y) do { idx = (x) * 3 + y; \
                      if ((used_ & (1 << idx)) == 0) { \
                        cc = bd_[(x)*3+(y)]; \
                        if (t->StartsWord(cc)) { \
                          DoDFS((x)*3+(y), len, t->Descend(cc)); \
                        } \
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
  used_ ^= (1 << i);

#undef HIT
#undef HIT3x
#undef HIT3y
#undef HIT8
}
