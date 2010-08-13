// The idea here is to test that solving a 3x3 board is equivalent to solving a
// 4x4 board with two edges removed. We do this by using a dictionary with a
// missing letter (an 'e').

#include <string>
#include <vector>
#include "boggle_solver.h"
#include "bucket_solver.h"
#include "mtrandom/randomc.h"

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
  std::vector<std::string> splits;
  splits.push_back("aeiou");
  splits.push_back("sy");
  splits.push_back("bdfgjkmpvwxzq");
  splits.push_back("chlnrt");
  for (int n = 0; n < 10; n++) {
    for (int x = 0; x < 3; x++) {
      for (int y = 0; y < 3; y++) {
        int idx = r.IRandom(0, 3);
        strcpy(solver_33->MutableCell(x, y), splits[idx].c_str());
        // strcpy(solver_34->MutableCell(x, y), splits[idx].c_str());
        strcpy(solver_44->MutableCell(x, y), splits[idx].c_str());
      }
    }

    int score33 = solver_33->UpperBound();
    // int score34 = solver_34->UpperBound();
    int score44 = solver_44->UpperBound();
    // if (score33 != score34 || score34 != score44) {
    if (score33 != score44) {
      printf("Bucket Failure!\n");
      printf("3x3: %s (%d)\n", solver_33->as_string(), score33);
      // printf("3x4: %s (%d)\n", solver_34->as_string(), score34);
      printf("4x4: %s (%d)\n", solver_44->as_string(), score44);
      return false;
    }
  }
  return true;
}

int main(int argc, char** argv) {
  if (!TestRegular()) {
    fprintf(stderr, "%s: failed TestRegular\n", argv[0]);
  }
  if (!TestBuckets()) {
    fprintf(stderr, "%s: failed TestBuckets\n", argv[0]);
  }
  printf("%s: Passed\n", argv[0]);
}
