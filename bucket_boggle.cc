// This program groups letters into some number of buckets and computes an
// upper bound on the score. If this upper bound is lower than the best score
// for a specific board (highest I've found so far is 3625), then all specific
// boards in that class can be eliminated.

// Usage: ./bucket_boggle <dict> <num boards> ab cd ef ghi ... yz

// Example: with 13 classes {ab cd ef gh ij kl mn op rs tu vw xy z}, 99.93% of
// board classes can be eliminated.

#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include "trie.h"
#include "boggler.h"
#include "buckets.h"
using std::map;
using std::vector;

// e.g. [a-z] => [a-f] for six buckets
double BucketScore(SimpleTrie* t,
                   const Buckets::Bucketing& buckets, int num_boards) {
  GenericBoggler<SimpleTrie> b(t);
  int cutoff = 3625;
  int reps_below = 0, reps_above = 0;
  for (int i = 0; i < num_boards; i++) {
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        while (1) {
          int c = random() % 26;
          Buckets::Bucketing::const_iterator it = buckets.find('a' + c);
          if (it != buckets.end()) {
            b.SetCell(x, y, it->second - 'a');
            break;
          }
        }
      }
    }
    int score = b.Score(cutoff);
    if (score > cutoff) {
      reps_above += 1;
    } else {
      reps_below += 1;
    }
    printf("%s: %d\n", b.ToString().c_str(), score);
  }
  printf("%d / %d\n", reps_above, reps_below);

  return 1.0 * reps_below / (reps_below + reps_above);
}

int main(int argc, char** argv) {
  srandomdev();

  const char* filename = argv[1];
  char line[80];
  SimpleTrie t;
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    exit(1);
  }

  while (!feof(f) && fscanf(f, "%s", line)) {
    if (!Boggler::BogglifyWord(line)) continue;
    t.AddWord(line);
  }
  fclose(f);

  std::cout << "Loaded " << TrieUtils<SimpleTrie>::Size(&t)
            << " words." << std::endl;
  int num_boards = atoi(argv[2]);

  Buckets::Bucketing perm;
  for (int i = 3; i < argc; i++) {
    char* x = argv[i];
    while (*x) perm[*x++] = 'a' + i - 3;
  }
  std::cout << Buckets::ToString(perm) << ": "
            << BucketScore(&t, perm, num_boards) << std::endl;
}
