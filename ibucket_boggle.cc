// Copyright 2008 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)
//
// Play bucketed boggle w/o a bucketed trie. This could potentially be really
// slow, but will give better bounds and allow more flexible bucketing.

#include <math.h>
#include <sys/time.h>
#include "trie.h"
#include "boggler.h"
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

SimpleTrie* dict_ = NULL;
int used_ = 0;
char* bd_[16];
unsigned int counts_[16];

map<SimpleTrie*, int> found_;  // word node -> length
unsigned int cutoff = UINT_MAX;
bool count_letters = true;
unsigned int alt_score;
unsigned int runs_ = 0;

unsigned int DoDFS(int i, int len, SimpleTrie* t);
unsigned int Score(SimpleTrie* t) {
  unsigned int score = 0;
  alt_score = 0;
  memset(counts_, 0, sizeof(counts_));
  used_ = 0;
  dict_ = t;
  runs_ += 1;
  int i;
  for (i=0; i<16; i++) {
    unsigned int max_score = 0;
    for (int j=0; bd_[i][j]; j++) {
      int c =  bd_[i][j] - 'a';
      if (t->StartsWord(c)) {
        unsigned int tscore = DoDFS(i, 0, t->Descend(c));
        if (tscore > max_score) max_score = tscore;
      }
    }
    score += max_score;
    if (score > cutoff && alt_score > cutoff)
      return score;
  }
  printf("max, no mark score: %u\n", score);

  // Calculate an alternate score
  printf("superset of words score: %d\n", alt_score);

  return min(alt_score, score);
}

unsigned int DoDFS(int i, int len, SimpleTrie* t) {
  unsigned int score = 0;
  used_ ^= (1 << i);
  //len += (c==kQ ? 2 : 1);
  len += 1;
  if (t->IsWord()) {
    // Should mark that this word has been found, but that's.. tricky
    int word_score = BogglerBase::kWordScores[len];
    score += word_score;
    if (t->Mark() != runs_) {
      alt_score += word_score;
      t->Mark(runs_);
    }
    if (count_letters) {
      for (int i=0; i<16; i++)
        if (used_ & (1<<i)) counts_[i] += BogglerBase::kWordScores[len];
    }
  }

  int cc, idx;
  unsigned int max_score, tscore;
  // To help the loop unrolling...
#define HIT(x,y) do { idx = (x) * 4 + y; \
                      if ((used_ & (1 << idx)) == 0) { \
                        max_score = 0; \
                        for (int j=0; bd_[idx][j]; j++) { \
                          cc = bd_[idx][j] - 'a'; \
                          if (t->StartsWord(cc)) { \
                            tscore = DoDFS(idx, len, t->Descend(cc)); \
                            if (tscore > max_score) max_score = tscore; \
                          } \
                        } \
                        score += max_score; \
                        if (score > cutoff && alt_score > cutoff) return score; \
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
#undef HIT
#undef HIT3x
#undef HIT3y
#undef HIT8
  used_ ^= (1 << i);
  return score;
}

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

void Usage(char* prog) {
  fprintf(stderr, "Usage: %s <dict> <class1> ... <class16>\n", prog);
  exit(1);
}

int main(int argc, char** argv) {
  if (argc != 18) Usage(argv[0]);

  const char* dict_file = argv[1];
  printf("loading words from %s\n", dict_file);
  SimpleTrie* t = GenericBoggler<SimpleTrie>::DictionaryFromFile(dict_file);

  for (int i=2; i<argc; i++) {
    bd_[i-2] = argv[i];
  }

  double start = secs();
  int score = Score(t);
  double end = secs();

  printf("Cell usage:\n");
  for (int i=0; i<16; i++) {
    printf("%7u%c", counts_[i], i%4==3 ? '\n' : ' ');
  }

  printf("Score: %u\n", score);
  printf("%f secs elapsed\n", end - start);
}
