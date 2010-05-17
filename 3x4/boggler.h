// An interface for solving Boggle boards given a TrieT.
// This is designed to be extremely efficient.
//
// Best known board: srepetaldnis (1651)

#ifndef BOGGLER_34
#define BOGGLER_34

#include <limits.h>
#include <set>
#include <string>
#include "trie.h"
#include "boggle_solver.h"

class Boggler34 : public BoggleSolver {
 public:
  typedef SimpleTrie TrieT;
  // Assumes ownership of the Trie. No other Boggler may modify the Trie after
  // this Boggler has been constructed using it.
  Boggler34(TrieT* t);
  virtual ~Boggler34();

  // TODO(danvk): add to BoggleSolver
  // Find the actual words on the board.
  // void FindWords(bool annotate, std::set<std::string>* out);

  // Set a cell on the current board.
  // Must have 0 <= x < 4, 0 <= y < 3 and 0 <= c < 26.
  // These constraints are NOT checked.
  void SetCell(int x, int y, int c);
  int Cell(int x, int y) const;
  
  int Width() const { return 4; }
  int Height() const { return 3; }

 protected:
  int InternalScore();

 private:
  void DoDFS(int i, int len, TrieT* t);
  TrieT* dict_;
  mutable unsigned int used_;
  mutable int bd_[12];
  unsigned int score_;
};

#endif
