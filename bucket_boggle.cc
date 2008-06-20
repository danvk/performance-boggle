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
double BucketScore(const Trie* dict,
                   const Buckets::Bucketing& buckets, int num_boards) {
  Trie* t = Buckets::FromTrie(*dict, buckets);
  std::cout << "Size: " << t->Size() << std::endl;

  Boggler b(t);
  int cutoff = 3625;
  double reps_below = 0, reps_above = 0;
  int num_buckets = Buckets::NumBuckets(buckets);
  for (int i = 0; i < num_boards; i++) {
    uint64_t reps = 1;
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        int c = random() % num_buckets;
        b.SetCell(x, y, c);
      }
    }
    reps = Buckets::NumRepresentatives(b, buckets);
    int score = b.Score();
    //std::cout << score << std::endl;
    if (score > cutoff) {
      reps_above += reps;
    } else {
      reps_below += reps;
    }
  }
  printf("%f / %f\n", reps_above, reps_below);

  t->Delete();
  return 1.0 * reps_below / (reps_below + reps_above);
}

int main(int argc, char** argv) {
  srandomdev();
  Trie* dict = Boggler::DictionaryFromFile(argv[1]);
  std::cout << "Loaded " << dict->Size() << " words." << std::endl;
  int num_boards = atoi(argv[2]);

  Buckets::Bucketing perm;
  for (int i = 3; i < argc; i++) {
    char* x = argv[i];
    while (*x) perm[*x++] = 'a' + i - 3;
  }
  std::cout << Buckets::ToString(perm) << ": "
            << BucketScore(dict, perm, num_boards) << std::endl;
}
