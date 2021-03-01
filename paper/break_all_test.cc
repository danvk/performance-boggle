// The basic idea of this test is to find all the best boards in a
// restricted boggle problem in two different ways:
//
//  1. Evaluate all the boards and keep the best ones.
//  2. Using the more efficient "breaking" approach.
//
// If the two find the same boards, then we can be fairly confident that the
// breaking approach is correct.
//
// The restricted problem is 3x3 boggle with a five letter alphabet (a, b,
// d, e, q).

#include <iostream>
#include <math.h>
#include <set>
#include <string>
#include <sys/time.h>
#include <cstring>
#include "breaker.h"
#include "ibuckets.h"
#include "test.h"
#include "trie.h"

using namespace paper;
using namespace std;

static const char kLetters[] = "abdeq";

void FindGoodBoardsDirect(Trie* t, set<string>* boards) {
  BucketBoggler b(t);
  int num_letters = strlen(kLetters);
  uint64_t num_boards = pow(num_letters, 9);
  for (int i = 0; i < 9; i++)
    b.Cell(i)[1] = '\0';
  for (uint64_t index = 0; index < num_boards; index++) {
    uint64_t left = index;
    for (int i = 0; i < 9; i++) {
      b.Cell(i)[0] = kLetters[left % num_letters];
      left /= num_letters;
    }
    CHECK_EQ((uint64_t)0, left);

    int score = b.UpperBound();
    if (score > 25) {
      string bd;  // strip spaces
      const char* space_bd = b.as_string();
      for (int i = 0; space_bd[i]; i++)
        if (space_bd[i] != ' ')
          bd.append(1, space_bd[i]);
      boards->insert(bd);
    }
  }
}

void FindGoodBoardsBreaking(Trie* t, set<string>* boards) {
  BucketBoggler b(t);
  Breaker breaker(&b, 25);
  breaker.SetDisplayDebugOutput(false);

  vector<string> classes;
  classes.push_back("ae");
  classes.push_back("bdq");
  for (int i = 0; i < pow(2, 9); i++) {
    CHECK(breaker.FromId(classes, i));
    BreakDetails details;
    breaker.Break(&details);
    for (unsigned int i = 0; i < details.failures.size(); i++) {
      boards->insert(details.failures[i]);
    }
  }
}

void FindGoodBoardsBreakingOne(Trie* t, set<string>* boards) {
  BucketBoggler b(t);
  Breaker breaker(&b, 25);
  breaker.SetDisplayDebugOutput(false);

  vector<string> classes;
  classes.push_back("abdeq");
  CHECK(breaker.FromId(classes, 0));
  BreakDetails details;
  breaker.Break(&details);
  for (unsigned int i = 0; i < details.failures.size(); i++) {
    boards->insert(details.failures[i]);
  }
}

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}

int main(int argc, char** argv) {
  Trie* t = Trie::CreateFromFile("words");
  CHECK(t != NULL);

  double start = secs();
  set<string> good_direct;
  FindGoodBoardsDirect(t, &good_direct);
  cout << "Direct: " << (secs() - start) << " seconds" << endl;

  start = secs();
  set<string> good_break;
  FindGoodBoardsBreaking(t, &good_break);
  cout << "Break (2 classes): " << (secs() - start) << " seconds" << endl;

  start = secs();
  set<string> good_break_one;
  FindGoodBoardsBreakingOne(t, &good_break_one);
  cout << "Break (1 class): " << (secs() - start) << " seconds" << endl;

  CHECK_EQ(good_direct.size(), good_break.size());
  CHECK_EQ(good_direct.size(), good_break_one.size());
  for (set<string>::iterator it = good_direct.begin(),
                             jt = good_break.begin(),
                             kt = good_break_one.begin();
       it != good_direct.end(); ++it, ++jt, ++kt) {
    CHECK_EQ(*it, *jt);
    CHECK_EQ(*jt, *kt);
  }
}
