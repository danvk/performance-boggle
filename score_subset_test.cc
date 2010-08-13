// The idea here is to test that solving a 3x3 board is equivalent to solving a
// 4x4 board with two edges removed. We do this by using a dictionary with a
// missing letter (an 'e').

#include "boggle_solver.h"
#include "mtrandom/randomc.h"

int main(int argc, char** argv) {
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
      exit(1);
    }
  }

  printf("%s: Passed\n", argv[0]);
}
