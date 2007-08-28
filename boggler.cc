#include "boggler.h"
#include <ctype.h>
#include <stdio.h>

//#define PRINT_WORDS

static int kCellUsed = -1;
static const int kWordScores[] =
  //0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17
  { 0, 0, 0, 1, 1, 2, 3, 5, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };

Boggler::Boggler(Trie* t): dict_(t), runs_(0), num_boards_(0) {}

int Boggler::Score() {
  runs_ += 1;
  int score = 0;
  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) {
      int c = bd_[i][j];
      if (dict_->StartsWord(c))
        score += DoDFS(i, j, 0, dict_->Descend(c));
    }

  num_boards_++;

  return score;
}

int Boggler::Score(const char* lets) {
  if (!ParseBoard(lets))
    return -1;
  return Score();
}

// Returns the score from this portion of the search
int Boggler::DoDFS(int x, int y, int len, Trie* t) {
  //printf("DoDFS(%d, %d, %d, 0x%08X)\n", x, y, len, t);
  int c = bd_[x][y];
  bd_[x][y] = kCellUsed;

  len += (c==kQ ? 2 : 1);
  int score = 0;
  if (t->IsWord() && t->Mark() != runs_) {
    score += kWordScores[len];
    t->Mark(runs_);
#ifdef PRINT_WORDS
    string out;
    dict_->ReverseLookup(t, &out);
    printf("%s\n", out.c_str());
#endif
  }

  int lx = x==0 ? 0 : x-1; int hx = x==3 ? 4 : x+2;
  int ly = y==0 ? 0 : y-1; int hy = y==3 ? 4 : y+2;
  for (int cx=lx; cx<hx; ++cx) {
    for (int cy=ly; cy<hy; ++cy) {
      int cc = bd_[cx][cy];
      if (cc == kCellUsed) continue;
      if (t->StartsWord(cc))
        score += DoDFS(cx, cy, len, t->Descend(cc));
    }
  }

  bd_[x][y] = c;
  return score;
}

// Board format: "bcdefghijklmnopq"
bool Boggler::ParseBoard(const char* lets) {
  for (int i=0; *lets; ++i)
    bd_[i/4][i%4] = (*lets++)-'a';
  return true;
}
