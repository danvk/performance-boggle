#include "4x4/boggler.h"

#include <stdio.h>

Boggler::Boggler(TrieT* t) : dict_(t) {}
Boggler::~Boggler() { delete dict_; }

void Boggler::SetCell(int x, int y, int c) { bd_[(x << 2) + y] = c; }
int Boggler::Cell(int x, int y) const { return bd_[(x << 2) + y]; }

int Boggler::InternalScore() {
  used_ = 0;
  score_ = 0;
  for (int i = 0; i < 16; i++) {
    int c = bd_[i];
    if (dict_->StartsWord(c))
      DoDFS(i, 0, dict_->Descend(c));
  }
  return score_;
}

void Boggler::DoDFS(int i, int len, TrieT* t) {
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
#define HIT(x,y) do { idx = (x) * 4 + y; \
                      if ((used_ & (1 << idx)) == 0) { \
                        cc = bd_[(x)*4+(y)]; \
                        if (t->StartsWord(cc)) { \
                          DoDFS((x)*4+(y), len, t->Descend(cc)); \
                        } \
                      } \
		 } while(0)
#define HIT3x(x,y) HIT(x,y); HIT(x+1,y); HIT(x+2,y)
#define HIT3y(x,y) HIT(x,y); HIT(x,y+1); HIT(x,y+2)
#define HIT8(x,y) HIT3x(x-1,y-1); HIT(x-1,y); HIT(x+1,y); HIT3x(x-1,y+1)

  switch (i) {
    case 0*4 + 0: HIT(0, 1); HIT(1, 0); HIT(1, 1); break;
    case 0*4 + 1: HIT(0, 0); HIT3y(1, 0); HIT(0, 2); break;
    case 0*4 + 2: HIT(0, 1); HIT3y(1, 1); HIT(0, 3); break;
    case 0*4 + 3: HIT(0, 2); HIT(1, 2); HIT(1, 3); break;

    case 1*4 + 0: HIT(0, 0); HIT(2, 0); HIT3x(0, 1); break;
    case 1*4 + 1: HIT8(1, 1); break;
    case 1*4 + 2: HIT8(1, 2); break;
    case 1*4 + 3: HIT3x(0, 2); HIT(0, 3); HIT(2, 3); break;

    case 2*4 + 0: HIT(1, 0); HIT(3, 0); HIT3x(1, 1); break;
    case 2*4 + 1: HIT8(2, 1); break;
    case 2*4 + 2: HIT8(2, 2); break;
    case 2*4 + 3: HIT3x(1, 2); HIT(1, 3); HIT(3, 3); break;

    case 3*4 + 0: HIT(2, 0); HIT(2, 1); HIT(3, 1); break;
    case 3*4 + 1: HIT3y(2, 0); HIT(3, 0); HIT(3, 2); break;
    case 3*4 + 2: HIT3y(2, 1); HIT(3, 1); HIT(3, 3); break;
    case 3*4 + 3: HIT(2, 2); HIT(3, 2); HIT(2, 3); break;
  }
  used_ ^= (1 << i);

#undef HIT
#undef HIT3x
#undef HIT3y
#undef HIT8
}

// TODO(danvk): figure out a way to move this into BoggleSolver.
Boggler::TrieT* Boggler::DictionaryFromFile(const char* filename) {
  TrieT* t = new TrieT;
  char line[80];
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    // delete t;  // ARGH
    return NULL;
  }

  while (!feof(f) && fscanf(f, "%s", line)) {
    if (!BogglifyWord(line)) continue;
    t->AddWord(line);
  }
  fclose(f);

  return t;
}
