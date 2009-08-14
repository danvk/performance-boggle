// Simple Boggle Solver
//
// Given a dictionary and letters on the board, print the score.

#include <cstdio>
#include <string>
#include <iostream>
#include "boggle_solver.h"
#include "3x3/boggler.h"
#include "4x4/boggler.h"
#include "gflags/gflags.h"
#include "trie.h"

const char kUsage[] =
"%s <dictionary file> [abcdefghijklmnop [qrstuvwxyzabcdef [...]]]\n"
"A 'q' is treated as 'qu'.\n";

DEFINE_string(dictionary, "words", "Dictionary file");
DEFINE_int32(size, 44, "Type of boggle board to use (MN = MxN)");

void HandleBoard(BoggleSolver* b, const char* bd);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  FILE* f = fopen(FLAGS_dictionary.c_str(), "r");
  if (f == NULL) {
    fprintf(stderr, "Couldn't open dictionary file %s\n",
            FLAGS_dictionary.c_str());
    exit(1);
  }
  fclose(f);

  SimpleTrie* t = Boggler::DictionaryFromFile(FLAGS_dictionary.c_str());
  BoggleSolver* solver = NULL;
  switch (FLAGS_size) {
    case 33: solver = new Boggler3(t); break;
    case 44: solver = new Boggler(t); break;
    default:
      fprintf(stderr, "Unknown board size: %d\n", FLAGS_size);
      exit(1);
  }

  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      HandleBoard(solver, argv[i]);
    }
  } else {
    std::string s;
    while (std::cin >> s) {
      HandleBoard(solver, s.c_str());
    }
  }
}

void HandleBoard(BoggleSolver* b, const char* bd) {
  if (!b->ParseBoard(bd)) {
    fprintf(stderr, "Couldn't parse board string '%s'\n", bd);
    return;
  }
  int score = b->Score();
  fprintf(stdout, "%s: %d\n", b->ToString().c_str(), score);
}
