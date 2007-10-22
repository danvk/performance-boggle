#include "perfect-boggler.h"
#include <ctype.h>
#include <stdio.h>
//#define PRINT_WORDS

static int kCellUsed = -1;
static const int kWordScores[] =
  //0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17
  { 0, 0, 0, 1, 1, 2, 3, 5, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };

PerfectBoggler::PerfectBoggler(PerfectTrie* t): dict_(t), runs_(0), num_boards_(0) {}

int PerfectBoggler::Score() {
  runs_ += 1;
  score_ = 0;
  for (int i=0; i<16; i++) {
    int c = bd_[i];
    if (dict_->StartsWord(c))
      DoDFS(i, 0, dict_->Descend(c));
  }

  // Really should check for overflow here
  num_boards_++;
  return score_;
}

int PerfectBoggler::Score(const char* lets) {
  if (!ParseBoard(lets))
    return -1;
  return Score();
}

// Returns the score from this portion of the search
void PerfectBoggler::DoDFS(int i, int len, PerfectTrie* t) {
  int c = bd_[i];

  len += (c==kQ ? 2 : 1);
  if (t->IsWord() && t->Mark() != runs_) {
    score_ += kWordScores[len];
    t->Mark(runs_);
#ifdef PRINT_WORDS
    std::string out;
    dict_->ReverseLookup(t, &out);
    printf("%s\n", out.c_str());
#endif
  }

  // Could also get rid of any two dimensionality, but maybe GCC does that?
  bd_[i] = kCellUsed;
  int cc;

  // To help the loop unrolling...
#define HIT(x,y) do { cc = bd_[(x)*4+(y)]; \
		      if (~cc && t->StartsWord(cc)) { \
		        DoDFS((x)*4+(y), len, t->Descend(cc)); \
		      } \
		 } while(0)
#define HIT3x(x,y) HIT(x,y); HIT(x+1,y); HIT(x+2,y)
#define HIT3y(x,y) HIT(x,y); HIT(x,y+1); HIT(x,y+2)
#define HIT8(x,y) HIT3x(x-1,y-1); HIT(x-1,y); HIT(x+1,y); HIT3x(x-1,y+1)

  //switch ((x << 2) + y) {
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
  bd_[i] = c;
  //bd_[x][y] = c;
}

// Board format: "bcdefghijklmnopq"
bool PerfectBoggler::ParseBoard(const char* lets) {
  for (int i=0; *lets; ++i)
    bd_[i] = (*lets++)-'a';
    //bd_[i/4][i%4] = (*lets++)-'a';
  return true;
}
