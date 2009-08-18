// Evaluates 21,632 boards (mostly slight variations on a good and bad board)
// and prints out information on the performance. Typical performance on my
// machine is ~20kbds/sec, so this test takes just over one second to complete.
const int reps = 10;

#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <map>
#include "test.h"
#include "trie.h"
#include "4x4/boggler.h"

void TrieStats(const SimpleTrie& pt);
double secs();

int main(int argc, char** argv) {
  char* dict_file;
  if (argc == 2) dict_file = argv[1];
  else           dict_file = "words";

  char tmp[20] = "equalizes";
  assert(Boggler::BogglifyWord(tmp));
  assert(0 == strcmp(tmp, "eqalizes"));

  SimpleTrie* st = Boggler::DictionaryFromFile(dict_file);
  CHECK(st != NULL);
  TrieStats(*st);

  Boggler b(st);
  unsigned int prime = (1 << 20) - 3;
  unsigned int total_score = 0;
  unsigned int hash;

  char* bases[] = { "abcdefghijklmnop", "catdlinemaropets" };
  int bds = sizeof(bases) / sizeof(*bases);
  double start = secs();
  for (int rep = 0; rep < reps; rep++) {
    hash = 1234;
    for (int i=0; i<bds; ++i) {
      b.ParseBoard(bases[i]);
      for (int y1 = 0; y1 < 4; y1++) {
	for (int y2 = 0; y2 < 4; y2++) {
	  for (int c1 = 0; c1 < 26; c1++) {
	    b.SetCell(1, y1, c1);
	    for (int c2 = 0; c2 < 26; c2++) {
	      b.SetCell(2, y2, c2);
	      int score = b.Score();
	      hash *= (123 + score);
	      hash = hash % prime;
	      total_score += score;
	    }
	  }
	}
      }
    }
    if (hash != 0x000C1D3D) {
      fprintf(stderr, "Hash mismatch, expected 0xC1D3D\n");
      return 1;
    }
  }

  double end = secs();
  printf("Total score: %u = %lf pts/bd\n",
      total_score, 1.0 * total_score / b.NumBoards());
  printf("Score hash: 0x%08X\n", hash);
  printf("Evaluated %d boards in %lf seconds = %lf bds/sec\n",
      b.NumBoards(), (end-start), b.NumBoards()/(end-start));
  printf("%s: All tests passed!\n", argv[0]);
  return 0;
}

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

size_t NumNodes(const Trie& pt) {
  size_t r = 1;
  for (int i = 0; i < 26; i++) {
    if (pt.StartsWord(i))
      r += NumNodes(*pt.Descend(i));
  }
  return r;
}

size_t Childless(const SimpleTrie& pt) {
  size_t r = 0;
  bool children = false;
  for (int i = 0; i < 26; i++) {
    if (pt.StartsWord(i)) {
      children = true;
      r += Childless(*pt.Descend(i));
    }
  }
  return r + (children ? 1 : 0);
}

size_t WordsWithChildren(const SimpleTrie& pt) {
  bool children = false;
  size_t r = 0;
  for (int i = 0; i < 26; i++) {
    if (pt.StartsWord(i)) {
      children = true;
      r += WordsWithChildren(*pt.Descend(i));
    }
  }
  if (children && pt.IsWord()) r += 1;
  return r;
}

void TrieStats(const SimpleTrie& pt) {
  printf("Loaded %zd words into %zd-node Trie\n",
	  TrieUtils<SimpleTrie>::Size(&pt),
          TrieUtils<SimpleTrie>::NumNodes(&pt));

  printf("Trie contains %zd childless nodes, %zd words w/ children\n", 
	 Childless(pt), WordsWithChildren(pt));
}
