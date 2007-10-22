// Evaluates 21,632 boards (mostly slight variations on a good and bad board)
// and prints out information on the performance. Typical performance on my
// machine is ~20kbds/sec, so this test takes just over one second to complete.
const int reps = 10;

#include <stdio.h>
#include <sys/time.h>
#include <map>
#include "perfect-trie.h"
#include "perfect-boggler.h"

void TrieStats(const PerfectTrie& pt);
double secs();

int main(int argc, char** argv) {
  char* dict_file;
  if (argc == 2) dict_file = argv[1];
  else           dict_file = "words";

  PerfectTrie* pt = PerfectTrie::CreateFromFile(dict_file);
  assert(pt != NULL);
  TrieStats(*pt);

  PerfectBoggler b(pt);
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
      for (int x1 = 0; x1 < 4; x1++) {
	for (int x2 = 0; x2 < 4; x2++) {
	  for (int c1 = 0; c1 < 26; c1++) {
	    b.SetCell(x1, 1, c1);
	    for (int c2 = 0; c2 < 26; c2++) {
	      b.SetCell(x2, 2, c2);
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
  return 0;
}

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

size_t NumNodes(const PerfectTrie& pt) {
  size_t r = 1;
  for (int i = 0; i < 26; i++) {
    if (pt.StartsWord(i))
      r += NumNodes(*pt.Descend(i));
  }
  return r;
}

size_t Childless(const PerfectTrie& pt) {
  size_t r = pt.NumChildren() == 0 ? 1 : 0;
  for (int i = 0; i < 26; i++) {
    if (pt.StartsWord(i))
      r += Childless(*pt.Descend(i));
  }
  return r;
}

size_t WordsWithChildren(const PerfectTrie& pt) {
  size_t r = (pt.NumChildren() > 0 && pt.IsWord()) ? 1 : 0;
  for (int i = 0; i < 26; i++) {
    if (pt.StartsWord(i))
      r += WordsWithChildren(*pt.Descend(i));
  }
  return r;
}

void Gaps(const PerfectTrie* pt, std::map<int, int>* gaps) {
  for (int i = 0; i < 26; i++) {
    if (!pt->StartsWord(i)) continue;
    (*gaps)[pt->Descend(i) - pt] += 1;
    Gaps(pt->Descend(i), gaps);
  }
}

size_t EvenNodes(const PerfectTrie& pt) {
  int nc = pt.NumChildren();
  size_t r = (nc > 0 && nc % 2 ==0) ? nc : 0;
  for (int i = 0; i < 26; i++) {
    if (pt.StartsWord(i))
      r += EvenNodes(*pt.Descend(i));
  }
  return r;
}

void TrieStats(const PerfectTrie& pt) {
  printf("Loaded %zd words into %zd-node PerfectTrie (%zd bytes)\n",
	  pt.Size(), NumNodes(pt), pt.MemoryUsage());

  caddr_t low, high;
  pt.MemorySpan(&low, &high);
  printf("Memory spanned: %.2fM (0x%08X - 0x%08X)\n",
	  (high - low) / 1048576.0, (unsigned)low, (unsigned)high);

  printf("Trie contains %zd childless nodes, %zd words w/ children\n", 
	 Childless(pt), WordsWithChildren(pt));

  printf("Trie contains %d paired nodes.\n", EvenNodes(pt));

  std::map<int, int> gaps; Gaps(&pt, &gaps);
  std::map<int, int>::const_reverse_iterator i = gaps.rbegin();
  for (int j=0; j<20; j+=1, i++)
    printf("%5d: %d\n", i->first, i->second);
}
