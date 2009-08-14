// An interface for solving Boggle boards given a TrieT.
// This is designed to be extremely efficient.

#ifndef BOGGLER_33
#define BOGGLER_33

#include <limits.h>
#include <set>
#include <string>
#include "trie.h"
#include "boggle_solver.h"

class Boggler3 : public BoggleSolver {
 public:
  typedef SimpleTrie TrieT;
  // Does not assume ownership of the TrieT, though it must remain live for the
  // lifetime of the GenericBoggler. The trie will be modified by board scoring, and
  // must not be modified by any other GenericBoggler.
  Boggler3(TrieT* t);
  virtual ~Boggler3();

  // TODO(danvk): add to BoggleSolver
  // Find the actual words on the board.
  // void FindWords(bool annotate, std::set<std::string>* out);

  // Set a cell on the current board. Must have 0 <= x, y < 4 and 0 <= c < 26.
  // These constraints are NOT checked.
  void SetCell(int x, int y, int c);
  int Cell(int x, int y) const;
  
  int Width() const { return 3; }
  int Height() const { return 3; }

  // Load a dictionary file, removing all non-Boggle words and converting "qu"
  // to 'q'.
  static TrieT* DictionaryFromFile(const char* dict_filename);
  
 protected:
  int InternalScore();

 private:
  void DoDFS(int i, int len, TrieT* t);
  TrieT* dict_;
  mutable unsigned int used_;
  mutable int bd_[9];
  unsigned int score_;
};

#endif
