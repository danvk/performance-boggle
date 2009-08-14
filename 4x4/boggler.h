// A solver for 4x4 Boggle (i.e. the usual type)
#ifndef BOGGLER_4
#define BOGGLER_4

#include "boggle_solver.h"
#include "trie.h"

// TODO(danvk): namespace?

class Boggler : public BoggleSolver {
 public:
  typedef SimpleTrie TrieT;
  Boggler(TrieT* t);
  virtual ~Boggler();

  // Set a cell on the current board. Must have 0 <= x, y < 4 and 0 <= c < 26.
  // These constraints are NOT checked.
  void SetCell(int x, int y, int c);  // { bd_[(x << 2) + y] = c; }
  int Cell(int x, int y) const;  // { return bd_[(x << 2) + y]; }

  int Width() const { return 4; }
  int Height() const { return 4; }

  static bool IsBoggleWord(const char* word);

  static TrieT* DictionaryFromFile(const char* filename);

 protected:
  virtual int InternalScore();

 private:
  void DoDFS(int i, int len, TrieT* t);

  TrieT* dict_;
  unsigned int used_;
  unsigned int cutoff_;
  int bd_[16];
  int score_;
};

#endif
