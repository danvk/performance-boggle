// Copyright 2008 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)
//
// Solve lots of boggle boards at once by avoiding duplicate work.
// The idea:
//  1. Find the words on a board with two holes cut out. Remember all the paths
//  that started words leading into the holes:
//         b c d
//       e f g h
//       i j k l
//       m n o
//
//  2. Fill in the holes one at a time with each possible letter. Find new
//  words on each board and remember the paths into the remaining blank _that
//  also went through the hole we filled in_:
//       a b c d        b c d
//       e f g h      e f g h
//       i j k l      i j k l
//       m n o    ,   m n o p,  etc.
//
//  3. Patch each pair of boards together to get final scores.
//
// The process can be terminated at any intermediate step to get partial
// results.

#ifndef MULTIBOGGLE
#define MULTIBOGGLE

#include <vector>
#include "trie.h"

class MultiBoggle {
 public:
  // A path through the board that may start a word.
  struct Path {     // The final cell is always implicit from context.
    int used_mask;  // cells that have already been used
    Trie* node;     // node in the trie that the path ends on
    int len;        // could be made implicit from used_mask
    std::string ToString(Trie* dict) const;
  };

  // A boggle board with two holes cut out of it.
  struct TwoHole {
    int score;  // score of all words going through no tiles.
    unsigned trie_mark;  // mark in trie that indicates a found word.
    std::vector<Path> paths[2];  // 0 = paths ending in TL corner
                                 // 1 = paths ending in BR corner
  };

  // A board with one of those holes filled in (the letter which filled it in
  // is implicit).
  struct OneHole {
    int extra_score;  // score of additional words through the filled corner.
    std::vector<Trie*> extra_words;  // new words using the filled corner.
    std::vector<Path> paths;  // paths leading to the hole that also went through
                              // the filled corner.
  };

  MultiBoggle(Trie* t);
  ~MultiBoggle();

  // Solve the board with two holes cut out.
  void SolveTwoHole(TwoHole* out);
  void PrintTwoHole(const TwoHole& two);

  // Fill in one of the holes
  void FillHoleTL(const TwoHole& two, int c, OneHole* out);
  void FillHoleBR(const TwoHole& two, int c, OneHole* out);
  void PrintOneHole(const TwoHole& two, const OneHole& one, int x);

  // Merge solutions of boards with holes in opposite corners.
  // Returns the score of the full board.
  int MergeBoards(TwoHole& bd, int c1, OneHole& bd1, int c2, OneHole& bd2);

  bool ParseBoard(const char* lets);
  void SetCell(int x, int y, int c) {
    assert(x >= 0 && y >= 0);
    assert(x <= 3 && y <= 3);
    assert(c >= 0);
    assert(c < 26);
    assert(x != 0 || y != 0);  // can't fill the TL corner
    assert(x != 3 || y != 3);  // can't fill the BR corner
    bd_[(x << 2) + y] = c;
  }
  std::string ToString() const;

 private:
  template<class T>
  void DoDFS(int i, int len, Trie* t, T& callback);

  void FillHole(const TwoHole& two, int idx, int c, OneHole* out);

  void CalcMaxLenAddrs();
  int ReconstructLength(const Trie* node) const;

  Trie* dict_;
  Trie* max_len_addr[17];  // greatest Trie address for each word length
  unsigned int runs_;

  unsigned int used_;
  int bd_[16];
};

// This is necessary because of the templating. Be careful to avoid multiple
// definitions.
// #include "multiboggle.cc"

#endif
