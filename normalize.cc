// Rotate and reflect baords into a "normalized" position.
//
// Designed as a filter: replaces each the first 16 letter sequence on each
// line with a normalized version.

#include "gflags/gflags.h"
#include <stdio.h>
#include <iostream>
#include <string>

DEFINE_bool(all_expansions, false, "Print all expansions, not just the canonical");

std::string Rotate90CW(const std::string& bd);
std::string FlipTopBottom(const std::string& bd);

std::string CanonicalBoard(const std::string& bd) {
  std::string best = bd;
  std::string x = bd;
  for (int f = 0; f < 2; f++) {
    for (int i = 0; i < 4; i++) {
      if (!FLAGS_all_expansions) {
        if (x < best) best = x;
      } else {
        if (f || i) best = best + ", " + x;
      }
      x = Rotate90CW(x);
    }
    x = FlipTopBottom(x);
  }
  return best;
}

int BoardPosition(const std::string& bd) {
  int last_good = 0;
  for (size_t i = 0; i < bd.size(); i++) {
    if (bd[i] < 'a' || bd[i] > 'z') {
      last_good = i+1;
    } else {
      if (i - last_good == 15)
        return last_good;
    }
  }
  return -1;
}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  std::string s;
  while (std::getline(std::cin, s)) {
    int pos = BoardPosition(s);
    if (pos == -1) {
      std::cout << s << std::endl;
    } else {
      std::cout << s.replace(pos, 16, CanonicalBoard(s.substr(pos, 16))) << std::endl;
    }
  }
}

// 0 1 2 3
// 4 5 6 7
// 8 9 A B
// C D E F
//   ->
// C 8 4 0
// D 9 5 1
// E A 6 2
// F B 7 3
std::string Rotate90CW(const std::string& bd) {
  int perm[16] = { 0xC, 8, 4, 0, 0xD, 9, 5, 1, 0xE, 0xA, 6, 2, 0xF, 0xB, 7, 3 };
  std::string ret(16, ' ');
  for (int i = 0; i < 16; i++) {
    ret[i] = bd[perm[i]];
  }
  return ret;
}

// 0 1 2 3
// 4 5 6 7
// 8 9 A B
// C D E F
//   ->
// C D E F
// 8 9 A B
// 4 5 6 7
// 0 1 2 3
std::string FlipTopBottom(const std::string& bd) {
  int perm[16] = { 0xC, 0xD, 0xE, 0xF, 8, 9, 0xA, 0xB, 4, 5, 6, 7, 0, 1, 2, 3 };
  std::string ret(16, ' ');
  for (int i = 0; i < 16; i++) {
    ret[i] = bd[perm[i]];
  }
  return ret;
}
