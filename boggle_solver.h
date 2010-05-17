// An interface for solving Boggle boards given a TrieT.
// This is designed to be extremely efficient.

#ifndef BOGGLE_SOLVER_H
#define BOGGLE_SOLVER_H

#include <limits.h>
#include <string>

// Interface for a boggle solver. Very specifically does not refer to the Trie
// type, so that it does not need to be templated. Subclasses may be templated,
// may solve boggle on varying board sizes, etc.
class BoggleSolver {
 public:
  BoggleSolver();
  virtual ~BoggleSolver();
  
  // Construct a BoggleSolver for the given size board using the dictionary.
  // Possible sizes are: 33, 34, 44
  static BoggleSolver* Create(int size, const char* dictionary_file);

  // Parses a board string like "abcdefghijklmnop"
  virtual bool ParseBoard(const char* lets);
  std::string ToString() const;

  // Scores the current board.
  int Score();

  // Shortcut for ParseBoard() + Score()
  int Score(const char* lets);

  virtual int Width() const = 0;
  virtual int Height() const = 0;

  virtual void SetCell(int x, int y, int c) = 0;
  virtual int Cell(int x, int y) const = 0;

  // Returns the total number of boards that have evaluated.
  int NumBoards() { return num_boards_; }

  // Is this a valid boggle word? e.g. only has 'q' followed by 'u'.
  static bool IsBoggleWord(const char* word);

  // Returns true if it's a valid boggle word and converts "qu" -> 'q'
  static bool BogglifyWord(char* word);

 protected:
  virtual int InternalScore() = 0;
  unsigned int runs_;  // TODO(danvk): This belongs to the Trie, not Boggler.

  static const int kCellUsed = -1;
  static const int kWordScores[];

 private:
  int num_boards_;
};

// Convenience specialization of GenericBoggler
// Creates a SimpleTrie and then compacts it when loading from a dictionary.
// class Boggler : public GenericBoggler<Trie> {
//  public:
//   Boggler(Trie* t) : GenericBoggler<Trie>(t) {}
//   static Trie* DictionaryFromFile(const char* dict_filename);
// };

#endif
