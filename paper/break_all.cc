// Given a partition of the 26 letters, break all possible classes of boards.
//
// For the partition {bdfgjvwxz, aeiou, lnrsy, chkmpt} with the Enable2K word
// list and best_score=500, this took ~4 hours on a single machine.

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include "trie.h"
#include "ibuckets.h"
#include "breaker.h"

using namespace paper;
using namespace std;

int main(int argc, char** argv) {
  if (argc != 4) {
    fprintf(stderr,
            "Usage: %s 'bdfgjvwxz aeiou lnrsy chkmpt' 500 words.txt\n",
            argv[0]);
    exit(1);
  }

  int best_score = atoi(argv[2]);
  if (best_score <= 0) {
    fprintf(stderr, "expected best_score > 0, got %d\n", best_score);
    exit(1);
  }

  Trie* t = Trie::CreateFromFile(argv[3]);
  if (!t) {
    fprintf(stderr, "Couldn't create dictionary from %s\n", argv[3]);
    exit(1);
  }

  vector<string> classes;
  int letter_count = 0;
  classes.push_back("");
  for (unsigned int i = 0; argv[1][i]; i++) {
    char c = argv[1][i];
    if (c == ' ') {
      classes.push_back("");
    } else {
      (*classes.rbegin()) += string(1, c);
      letter_count += 1;
    }
  }

  uint64_t max_index = pow(classes.size(), 9);
  cout << "Breaking setup:" << endl;
  cout << " words in dict: " << t->Size() << endl;
  cout << " threshold score: " << best_score << endl;
  cout << " classes: [";
  for (unsigned int i = 0; i < classes.size(); i++) {
    cout << (i ? ", " : "") << classes[i].c_str();
  }
  cout << "]" << endl;
  cout << " to break: " << max_index << " classes" << endl;

  BucketBoggler bb(t);
  Breaker breaker(&bb, best_score);
  breaker.SetDisplayDebugOutput(false);

  vector<string> good_boards;
  for (uint64_t idx = 0; idx < max_index; idx++) {
    if (idx % 100 == 0) {
      cout << idx << " classes broken." << endl;
    }
    if (breaker.IsCanonical(classes.size(), idx)) {
      BreakDetails details;
      breaker.FromId(classes, idx);
      breaker.Break(&details);
      if (!details.failures.empty()) {
        for (unsigned int i = 0; i < details.failures.size(); i++) {
          cout << "Found unbreakable board: " << details.failures[i] << endl;
          good_boards.push_back(details.failures[i]);
        }
      }
    }
  }
  for (unsigned int i = 0; i < good_boards.size(); i++) {
    cout << good_boards[i] << endl;
  }
}
