// This program groups letters into some number of buckets and computes an
// upper bound on the score. If this upper bound is lower than the best score
// for a specific board (highest I've found so far is 3625), then all specific
// boards in that class can be eliminated.

// Usage: ./bucket_descent <dictionary> <num boards>
// TODO(danvk): This would be faster without Trie compaction
// TODO(danvk): Stop evaluating boards after the first 3625 points

#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include "trie.h"
#include "boggler.h"
#include "buckets.h"
using std::vector;
using std::map;
typedef Buckets::Bucketing Bucketing;

// e.g. [a-z] => [a-f] for six buckets
double BucketScore(const vector<std::string>& word_list,
                   const Bucketing& buckets,
                   const vector<std::string>& boards) {
  SimpleTrie* t = Buckets::FromWordList<SimpleTrie>(word_list, buckets);
  std::cout << "Size: " << TrieUtils<SimpleTrie>::Size(t) << std::endl;

  GenericBoggler<SimpleTrie> b(t);
  int cutoff = 3625;
  double reps_below = 0, reps_above = 0;
  for (size_t i = 0; i < boards.size(); i++) {
    std::string bd = boards[i];
    Buckets::Bucketize(&bd, buckets);
    b.ParseBoard(bd.c_str());
    int score = b.Score();
    if (score > cutoff) {
      reps_above += 1;
    } else {
      reps_below += 1;
    }
  }

  delete t;
  return 1.0 * reps_below / (reps_below + reps_above);
}


int main(int argc, char** argv) {
  vector<std::string> dictionary;
  {
    std::ifstream in(argv[1]);
    if (!in.is_open()) exit(2);
    std::string word;
    char buf[20];
    while (in >> word) {
      if (!Boggler::IsBoggleWord(word.c_str())) continue;
      strcpy(buf, word.c_str());
      Boggler::BogglifyWord(buf);
      dictionary.push_back(std::string(buf));
    }
  }
  std::cout << "Loaded " << dictionary.size() << " words." << std::endl;
  int num_boards = atoi(argv[2]);
  int num_buckets = atoi(argv[3]);
  int num_iters = atoi(argv[4]);

  // Generate a list of random boards in a deterministic way
  srandom(0);
  vector<std::string> boards;
  for (int i = 0; i < num_boards; i++) {
    boards.push_back(std::string(16, ' '));
    std::string& wd = *boards.rbegin();
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        int c = random() % num_buckets;
        wd[4*x + y] = 'a' + c;
      }
    }
  }

  srandomdev();
  // Start with a random set of buckets.
  while (1) {
    Bucketing best;
    for (char c = 'a'; c <= 'z'; c++) {
      if (c == 'q') continue;
      best[c] = 'a' + (random() % num_buckets);
    }
    double best_score = BucketScore(dictionary, best, boards);
    std::cout << "Initial: " << Buckets::ToString(best)
              << ": " << best_score << std::endl;

    for (int i = 0; i < num_iters; i++) {
      Bucketing cp = best;
      while (1) {
        char c;
        while ((c = 'a' + random() % 26) == 'q');
        cp[c] = 'a' + random() % num_buckets;
        if (random() % 100 < 75) break;
      }

      double score = BucketScore(dictionary, cp, boards);
      if (score > best_score) {
        best_score = score;
        best = cp;
        std::cout << "  " << Buckets::ToString(cp) << ": " << score << std::endl;
      }
      if (i % 10 == 0) std::cout << i << "..." << std::endl;
    }
    std::cout << "Final: " << Buckets::ToString(best) << ": " << best_score << std::endl;
  }
}
