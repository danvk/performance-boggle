#include "boggler.h"
#include <ctype.h>
#include <stdio.h>
//#define PRINT_WORDS

static int kCellUsed = -1;
static const int kWordScores[] =
  //0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17
  { 0, 0, 0, 1, 1, 2, 3, 5, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };

Boggler::Boggler(Trie* t) :
  dict_(t), runs_(0), num_boards_(0) {}

int Boggler::Score() {
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

int Boggler::Score(const char* lets) {
  if (!ParseBoard(lets))
    return -1;
  return Score();
}

// Returns the score from this portion of the search
void Boggler::DoDFS(int i, int len, Trie* t) {
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
}

// Board format: "bcdefghijklmnopq"
bool Boggler::ParseBoard(const char* lets) {
  for (int i=0; *lets; ++i)
    bd_[i] = (*lets++)-'a';
    //bd_[i/4][i%4] = (*lets++)-'a';
  return true;
}

bool Boggler::IsBoggleWord(const char* wd) {
  int size = strlen(wd);
  if (size < 3 || size > 17) return false;
  for (int i=0; i<size; ++i) {
    int c = wd[i];
    if (c<'a' || c>'z') return false;
    if (c=='q' && (i+1 >= size || wd[1+i] != 'u')) return false;
  }
  return true;
}

Trie* Boggler::DictionaryFromFile(const char* filename) {
  Trie::SimpleTrie* t = new Trie::SimpleTrie;
  char line[80];
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    delete t;
    return NULL;
  }

  while (!feof(f) && fscanf(f, "%s", line)) {
    if (!IsBoggleWord(line)) continue;

    // Strip qu -> q
    int src, dst;
    for (src=0, dst=0; line[src]; src++, dst++) {
      line[dst] = line[src];
      if (line[src] == 'q') src += 1;
    }
    line[dst] = line[src];

    t->AddWord(line);
  }
  fclose(f);

  Trie* pt = Trie::CompactTrie(*t);
  delete t;
  return pt;
}
