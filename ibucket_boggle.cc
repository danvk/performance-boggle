// Copyright 2008 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)
//
// Play bucketed boggle w/o a bucketed trie. This could potentially be really
// slow, but will give better bounds and allow more flexible bucketing.

#include <math.h>
#include <sys/time.h>
#include "trie.h"
#include "boggler.h"

int buckets[][26] = {
                     {'a', -1},
                     {'b', -1},
                     {'c', -1},
                     {'d', -1},
                     {'e', -1},
                     {'f', -1},
                     {'g', -1},
                     {'h', -1},
                     {'i', -1},
                     {'j', -1},
                     {'k', -1},
                     {'l', -1},
                     {'m', -1},
                     {'n', -1},
                     {'o', -1},
                     {'p', -1},
                     {'q', -1},
                     {'r', -1},
                     {'s', -1},
                     {'t', -1},
                     {'u', -1},
                     {'v', -1},
                     {'w', -1},
                     {'x', -1},
                     {'y', -1},
                     {'z', -1},

         /* 26 */    { 'a', 'e', 'o', -1},
         /* 27 */    { 'i', 'u', -1},
         /* 28 */    { 'c', 's', -1},
         /* 29 */    { 'h', 'y', -1},
         /* 30 */    { 'd', 'l', 'n', 'r', 't', 'b', -1},
         /* 31 */    { 'f', 'g', 'k', 'm', 'p', -1},
         /* 32 */    { 'j', 'v', 'w', 'x', 'z', -1},

         /* 33 */    { 'a', 'e', 'i', 'o', 'u', -2, 26, 27, -1},
         /* 34 */    { 'c', 'h', 's', 'y', -2, 28, 29, -1 },
         /* 35 */    { 'd', 'l', 'n', 'r', 't', 'b', 'f', 'g', 'j', 'k',
                       'm', 'p', 'v', 'w', 'x', 'z', -2, 30, 31, 32, -1 },

                     //{'a', 'e', 'i', 'o', 'u', -1},
                     //{'s', 'y', -1},
                     //{'b', 'd', 'f', 'g', 'j', 'k', 'm',
                     // 'p', 'v', 'w', 'x', 'z', -1},
                     //{'c', 'h', 'l', 'n', 'r', 't', -1}

                     //{ 'a', 'b', 'c', 'd', 'e', -1 },
                     //{ 'f', 'g', 'h', 'i', 'j', -1 },
                     //{ 'k', 'l', 'm', 'n', 'o', -1 },
                     //{ 'p', 'r', 's', 't', 'u', -1 },
                     //{ 'v', 'w', 'x', 'y', 'z', -1 },

                     //{ 'a', 'e', 'i', 'o', -1},
                     //{ 's', 'u', -1},
                     //{ 'f', 'j', 'k', 'm', 'v', 'w', 'x', 'z', -1},
                     //{ 'd', 'l', 'n', 'r', 't', -1},
                     //{ 'b', 'c', 'g', 'h', 'p', 'y', -1}

                     //{ 0, 1, 2, 3, 4, -1},  // abcde = 26
                     //{ 5, 6, 7, 8, 9, -1},  // fghij = 27
                     //{10,11,12,13,14, -1},  // klmno = 28
                     //{15,17,18,19,20, -1},  // prstu = 29
                     //{21,22,23,24,25, -1}   // vwxyz = 30
                   };

SimpleTrie* dict_ = NULL;
int used_ = 0;
int counts_[16] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
int bd_[16] = 
              { 26, 27, 28, 29,
                27, 28, 29, 30,
                28, 20, 30, 29,
                29, 30, 29, 28 };
              //{ 0, 0, 0, 0,
              //  0, 0, 0, 0,
              //  0, 1, 0, 0,
              //  0, 0, 0, 0 };
              //{ 'c'-'a', 'a'-'a', 't'-'a', 'd'-'a',
              //  'l'-'a', 'i'-'a', 'n'-'a', 'e'-'a',
              //  'm'-'a', 'a'-'a', 'r'-'a', 'o'-'a',
              //  'p'-'a', 'e'-'a', 't'-'a', 's'-'a' };
              //{ 0, 's'-'a', 2, 3,
              //  4, 'r'-'a', 6, 7,
              //  8, 9, 10, 11,
              //  12, 13, 14, 15};

unsigned int cutoff = UINT_MAX;
int board_evals = 0;
bool count_letters = true;
unsigned int DoDFS(int i, int len, SimpleTrie* t);
unsigned int Score(SimpleTrie* t) {
  board_evals += 1;
  unsigned int score = 0;
  used_ = 0;
  dict_ = t;
  int i;
  for (i=0; i<16; i++) {
    int bc = bd_[i];
    unsigned int max_score = 0;
    for (int j=0; buckets[bc][j] >= 0; j++) {
      int c = buckets[bc][j];
      if (t->StartsWord(c)) {
        unsigned int tscore = DoDFS(i, 0, t->Descend(c));
        if (tscore > max_score) max_score = tscore;
      }
    }
    score += max_score;
    if (score > cutoff)
      return score;
    //printf("%d: +%d = %d\n", i, max_score, score);
  }
  return score;
}

unsigned int DoDFS(int i, int len, SimpleTrie* t) {
  unsigned int score = 0;
  used_ ^= (1 << i);
  //len += (c==kQ ? 2 : 1);
  len += 1;
  if (t->IsWord()) {
    // Should mark that this word has been found, but that's.. tricky
    if (t->Mark() != 1) {
      score += BogglerBase::kWordScores[len];
      if (count_letters) {
        for (int i=0; i<16; i++)
          if (used_ & (1<<i)) counts_[i] += BogglerBase::kWordScores[len];
      }
      //t->Mark(1);
    }
  }

  int cc, idx, bc;
  unsigned int max_score, tscore;
  // To help the loop unrolling...
#define HIT(x,y) do { idx = (x) * 4 + y; \
                      if ((used_ & (1 << idx)) == 0) { \
                        bc = bd_[(x)*4+(y)]; \
                        max_score = 0; \
                        for (int j=0; buckets[bc][j] >= 0; j++) { \
                          cc = buckets[bc][j]; \
                          if (t->StartsWord(cc)) { \
                            tscore = DoDFS(idx, len, t->Descend(cc)); \
                            if (tscore > max_score) max_score = tscore; \
                          } \
                        } \
                        score += max_score; \
                        if (score > cutoff) return score; \
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

void PrintBoard() {
  char s[10];
  for (int i=0; i<16; i++) {
    if (buckets[bd_[i]][1] == -1) {
      s[0] = 'a' + buckets[bd_[i]][0];
      s[1] = '\0';
    } else {
      int j;
      for (j = 0; buckets[bd_[i]][j] >= 0; j++) {
        s[j] = buckets[bd_[i]][j] + 'a';
      }
      s[j] = '\0';
    }
    printf("%s ", s);
    //if (i % 4 == 3) printf("\n");
  }
  //printf("\n");
}
int BucketSize(int i) {
  for (int j=0; ; j++) if (buckets[i][j] < 0) return j;
}
unsigned long long NumReps() {
  unsigned long long cnt = 1;
  for (int i=0; i<16; i++) cnt *= BucketSize(bd_[i]);
  return cnt;
}

static const unsigned int kCutoff = 3625;
//static const int kMaxDepth = 16;
//static const int kMaxDepth = 32;
static const int kMaxDepth = 10;
unsigned long long elim = 0, pass = 0, outputs = 0;
int deepest = 0;
void Break(SimpleTrie* t, int times) {
  if (kMaxDepth - times > deepest) deepest = kMaxDepth - times;
  memset(counts_, 0, sizeof(counts_));
  count_letters = true;
  if (kMaxDepth - times < 2) {
    //count_letters = false;
    cutoff = UINT_MAX;
  } else {
    cutoff = kCutoff;
  }
  unsigned int score = Score(t);
  if (score < kCutoff) {
    elim += NumReps();
    return;
  } else if (times == 0) {
    pass += NumReps();
    outputs += 1;
    //PrintBoard();
    //printf(": %u\n", score);
    return;
  }

  // Pick the most-trafficked square to split
  int best_square=0, best_score=-1, best_size=0;
  for (int i=0; i<16; i++)
    if (counts_[i] > best_score && BucketSize(bd_[i]) > 1) {
      best_square = i;
      best_score = counts_[i];
      best_size = BucketSize(bd_[i]);
    }
  int save = bd_[best_square];
  //if (times > 13)
  //  printf("Depth: %d, cell %d\n", 16 - times, best_square);

  int* rep = buckets[bd_[best_square]];
  if (rep[best_size] < -1) rep = rep + best_size + 1;
  //printf("breaking on %d (size %d)\n", best_square, best_size);
  for (; *rep != -1; rep++) {
    bd_[best_square] = *rep;
    //printf("%2d: ", *rep); PrintBoard(); printf("\n");
    Break(t, times - 1);
  }
  bd_[best_square] = save;
    //printf("%c:\n", 'a' + *rep);
    //PrintBoard();
    //printf(": %d\n", score);
}

int main(int argc, char** argv) {
  const char* dict_file = argv[1];
  printf("loading words from %s\n", dict_file);
  SimpleTrie* t = GenericBoggler<SimpleTrie>::DictionaryFromFile(dict_file);

  for (size_t i=0; i<sizeof(buckets)/sizeof(*buckets); i++) {
    for (int j=0; j < BucketSize(i); j++)
      buckets[i][j] -= 'a';
  }

  double total_elapsed = 0.0;
  unsigned long long total_elim = 0, total_pass = 0;
  int count = 1000;
  //int num_buckets = sizeof(buckets)/sizeof(*buckets) - 26;
  int num_buckets = 3;
  printf("num_buckets = %d\n", num_buckets);
  srandom(time(NULL));
  for (int k=0; k<count; k++) {
    // Generate a random board class
    for (int i = 0; i < 16; i++) {
      //bd_[i] = 26 + (random() % num_buckets);
      bd_[i] = 33 + (random() % num_buckets);
    }

    deepest = elim = pass = board_evals = 0;
    double start = secs();
    printf("Breaking:\n");
    PrintBoard();
    printf("\n");
    Break(t, kMaxDepth);
    double end = secs();
    total_pass += pass;
    total_elim += elim;  // (elim + pass);
    total_elapsed += (end - start);

    printf("Evaluated %u boards in %f seconds.\n", board_evals, end - start);
    printf("Eliminated %llu / %llu @ depth %d\n", elim, elim + pass, deepest);
    if (pass > 0)
      printf("%llu boards remain, would output %llu\n", pass, outputs);
    printf("Total time: %f, %llu boards elim. (%.5f) => %.2fBbds/sec, %.2fs/bucket\n",
           total_elapsed, total_elim, 1.0 * total_elim / (total_elim + total_pass),
           1.0 * total_elim / total_elapsed / pow(10, 9),
           1.0 * total_elapsed / (k+1));
    fflush(stdout);
  }
}
