// List all boards within a distance of N of a given board.
//
// All boards will be normalized (w/r/t/ the eight symmetries) and uniqued
// before being output. If a board can be formed in multiple ways from the seed
// board, the least distance version is output.
//
// There are <= 520 boards within an edit distance of 1.
// There are <= 270k boards within an edit distance of 2.
// There are <= 140M boards within an edit distance of 3.
//
// At a speed of ~15k bds/sec, it will take almost three hours to evaluate all
// boards within 3 units of the seed. Use with care!

#include <stdio.h>
#include <string>
#include <iostream>
#include "gflags/gflags.h"
#include "init.h"
#include <cstring>

DEFINE_int32(d, 1, "Maximum edit distance for output");

// Print all boards an edit distance of <= d from c
// Leaves the board unaltered upon returning.
void EditDistance(int d, char* c) {
  int n = strlen(c);
  // First all the edits
  for (int cell = 0; cell < n; cell++) {
    int orig = c[cell];
    for (char let = 'a'; let <= 'z'; let++) {
      if (let == orig) continue;
      c[cell] = let;
      if (d == 1) puts(c);
      else        EditDistance(d - 1, c);
    }
    c[cell] = orig;
  }

  // Then the swaps
  for (int cell1 = 0; cell1 < n; cell1++) {
    char orig = c[cell1];
    for (int cell2 = cell1 + 1; cell2 < n; cell2++) {
      if (orig == c[cell2]) continue;
      char orig2 = c[cell2];
      c[cell2] = orig;
      c[cell1] = orig2;
      if (d == 1) puts(c);
      else        EditDistance(d - 1, c);
      c[cell2] = orig2;
    }
    c[cell1] = orig;
  }
}

int main(int argc, char** argv) {
  Init(&argc, &argv);

  if (argc > 1) {
    for (int i = 1; i < argc; i++)
      EditDistance(FLAGS_d, argv[i]);
  } else {
    std::string s;
    while (std::cin >> s) {
      char c[17];  // TODO(danvk): check this is large enough
      strcpy(c, s.c_str());
      EditDistance(FLAGS_d, c);
    }
  }
}
