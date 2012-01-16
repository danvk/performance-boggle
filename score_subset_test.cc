// The idea here is to test that solving a 3x3 board is equivalent to solving a
// 4x4 board with two edges removed. We do this by using a dictionary with a
// missing letter (an 'e').

#include <string.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include "boggle_solver.h"
#include "bucket_solver.h"
#include "glog/logging.h"
#include "ibucket_breaker.h"
#include "mtrandom/randomc.h"
#include "init.h"

using namespace std;

// TODO(danvk): factor this out into a util library.
void SplitString(const string& str, vector<int>* nums) {
  string s = str;
  for (int i = 0; i < s.size(); i++) {
    if (s[i] == ',') s[i] = '\n';
  }

  // From http://stackoverflow.com/questions/236129/how-to-split-a-string
  vector<string> tokens;
  istringstream iss(s);
  copy(istream_iterator<string>(iss),
       istream_iterator<string>(),
       back_inserter<vector<string> >(tokens));

  nums->clear();
  for (int i = 0; i < tokens.size(); i++) {
    nums->push_back(atoi(tokens[i].c_str()));
  }
}

bool TestRegular() {
  // Create the solvers.
  BoggleSolver* solver_33 = BoggleSolver::Create(33, "eless-words");
  BoggleSolver* solver_34 = BoggleSolver::Create(34, "eless-words");
  BoggleSolver* solver_44 = BoggleSolver::Create(44, "eless-words");

  // Null out the sides.
  for (int i = 0; i < 4; i++) {
    solver_44->SetCell(i, 3, 'e' - 'a');
    solver_44->SetCell(3, i, 'e' - 'a');
    if (i < 3) solver_34->SetCell(i, 3, 'e' - 'a');
  }

  // Start solving random boards.
  int32 seed = time(NULL) + getpid();
  TRandomMersenne r(seed);

  for (int n = 0; n < 10000; n++) {
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        char c = r.IRandom('a', 'z') - 'a';
        solver_33->SetCell(x, y, c);
        solver_34->SetCell(x, y, c);
        solver_44->SetCell(x, y, c);
      }
    }

    int score33 = solver_33->Score();
    int score34 = solver_34->Score();
    int score44 = solver_44->Score();
    if (score33 != score34 || score34 != score44) {
      printf("Failure!\n");
      printf("3x3: %s (%d)\n", solver_33->ToString().c_str(), score33);
      printf("3x4: %s (%d)\n", solver_34->ToString().c_str(), score34);
      printf("4x4: %s (%d)\n", solver_44->ToString().c_str(), score44);
      return false;
    }
  }
  return true;
}

bool TestBreaker(BucketSolver*, BucketSolver*, BucketSolver*);

bool TestBuckets() {
  // Create the solvers.
  BucketSolver* solver_33 = BucketSolver::Create(33, "eless-words");
  BucketSolver* solver_34 = BucketSolver::Create(34, "eless-words");
  BucketSolver* solver_44 = BucketSolver::Create(44, "eless-words");

  // Null out the sides.
  for (int i = 0; i < 4; i++) {
    strcpy(solver_44->MutableCell(i, 3), ".");
    strcpy(solver_44->MutableCell(3, i), ".");
    if (i < 3) strcpy(solver_34->MutableCell(i, 3), ".");
  }

  // Start solving random boards.
  int32 seed = time(NULL) + getpid();
  TRandomMersenne r(seed);
  vector<string> splits;
  splits.push_back("aeiou");
  splits.push_back("sy");
  splits.push_back("bdfgjkmpvwxzq");
  splits.push_back("chlnrt");
  for (int n = 0; n < 100; n++) {
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        int idx = r.IRandom(0, 3);
        strcpy(solver_33->MutableCell(x, y), splits[idx].c_str());
        strcpy(solver_34->MutableCell(x, y), splits[idx].c_str());
        strcpy(solver_44->MutableCell(x, y), splits[idx].c_str());
      }
    }

    // TODO(danvk): document this weirdness in bucket_solver and add an
    // UpdateBoard() method.
    solver_33->ParseBoard(solver_33->as_string());
    solver_34->ParseBoard(solver_34->as_string());
    solver_44->ParseBoard(solver_44->as_string());

    int score33 = solver_33->UpperBound();
    int score34 = solver_34->UpperBound();
    int score44 = solver_44->UpperBound();
    if (score33 != score34 || score34 != score44) {
      printf("Bucket Failure!\n");
      printf("3x3: %s (%d)\n", solver_33->as_string(), score33);
      printf("3x4: %s (%d)\n", solver_34->as_string(), score34);
      printf("4x4: %s (%d)\n", solver_44->as_string(), score44);
      return false;
    }
  }
  return TestBreaker(solver_33, solver_34, solver_44);
}

bool TestBreaker(BucketSolver* solver_33,
                 BucketSolver* solver_34,
                 BucketSolver* solver_44) {
  // Configure the breakers to pick cells in the same order.
  BreakOptions opts;
  opts.print_progress = false;

  vector<int> picks;
  Breaker breaker_33(solver_33, 625);
  BreakDetails details_33;
  SplitString("0,1,2,3,4,5,6,7,8", &picks);
  breaker_33.SetPickOrder(picks);
  breaker_33.SetOptions(opts);

  Breaker breaker_34(solver_34, 625);
  BreakDetails details_34;
  SplitString("0,1,2,4,5,6,8,9,10,3,7,11", &picks);
  breaker_34.SetPickOrder(picks);
  breaker_34.SetOptions(opts);

  Breaker breaker_44(solver_44, 625);
  BreakDetails details_44;
  SplitString("0,1,2,4,5,6,8,9,10,3,7,11,12,13,14,15", &picks);
  breaker_44.SetPickOrder(picks);
  breaker_44.SetOptions(opts);

  // Pick random board classes to break.
  vector<string> classes;
  classes.push_back("abcd");
  classes.push_back("efgh");
  classes.push_back("ijkl");
  classes.push_back("mnop");
  classes.push_back("qrst");
  classes.push_back("uvwxyz");
  int32 seed = time(NULL) + getpid();
  TRandomMersenne r(seed);
  for (int i = 0; i < 100; i++) {
    vector<string> class_picks;
    for (int j = 0; j < 9; j++) {
      class_picks.push_back(classes[r.IRandom(0, classes.size() - 1)]);
    }

    string str33, str34, str44;
    for (int j = 0; j < 9; j++) {
      if (j) {
        str33 += " ";
        str34 += " ";
        str44 += " ";
      }
      str33 += class_picks[j];
      str34 += class_picks[j];
      str44 += class_picks[j];
      if (j % 3 == 2) {
        str34 += " .";
        str44 += " .";
      }
    }
    str44 += " . . . .";

    if (!breaker_33.ParseBoard(str33) ||
        !breaker_34.ParseBoard(str34) ||
        !breaker_44.ParseBoard(str44)) {
      fprintf(stderr, "Couldn't parse %s\n", str33.c_str());
      return false;
    }

    breaker_33.Break(&details_33);
    breaker_34.Break(&details_34);
    breaker_44.Break(&details_44);
    if (details_33.max_depth != details_34.max_depth ||
        details_34.max_depth != details_44.max_depth ||
        details_33.sum_wins != details_34.sum_wins ||
        details_34.sum_wins != details_44.sum_wins ||
        details_33.max_wins != details_34.max_wins ||
        details_34.max_wins != details_44.max_wins) {
      fprintf(stderr, "Mismatch on %s (%d %d %d)\n", str33.c_str(),
              details_33.max_depth, details_34.max_depth, details_44.max_depth);
      return false;
    }
  }

  return true;
}

int main(int argc, char** argv) {
  Init(&argc, &argv);

  if (!TestRegular()) {
    fprintf(stderr, "%s: failed TestRegular\n", argv[0]);
  }
  if (!TestBuckets()) {
    fprintf(stderr, "%s: failed TestBuckets\n", argv[0]);
  }
  printf("%s: Passed\n", argv[0]);
}
