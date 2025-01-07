// An interface for calculating upper bounds on the score of the highest-scoring
// board in a class of Boggle boards. Subclasses have a particular size.

#ifndef BUCKET_SOLVER
#define BUCKET_SOLVER

#include <iostream>
#include <limits.h>
#include <string>
#include <sys/types.h>
#include <stdint.h>
#include <vector>

class BreakingNode;
class SimpleTrie;

class BucketSolver {
 public:
  explicit BucketSolver(SimpleTrie* t);
  virtual ~BucketSolver();

  // Construct a BucketSolver for the given size board using the dictionary.
  // Possible sizes are: 33, 34, 44
  static BucketSolver* Create(int size, const char* dictionary_file);

  virtual int Width() const = 0;
  virtual int Height() const = 0;

  // bd is a class of boards with cells delimited by spaces.
  // examples:
  // "a b c d e f g h i j k l m n o p"
  // "aeiou bcdfghjklmnprstvwxyz aeiou ..."
  // NOTE: "qu" gets interpreted as "qu" or "u".
  bool ParseBoard(const char* bd);
  const char* as_string();

  // Returns the possible characters in this cell. The result can be modified.
  virtual char* MutableCell(int idx) = 0;
  virtual const char* Cell(int idx) const = 0;

  const char* Cell(int x, int y) const { return Cell(Height() * x + y); }
  char* MutableCell(int x, int y) { return MutableCell(Height() * x + y); }

  // Returns the number of individual boards in the current board class. This
  // isn't guaranteed to fit in a uint64_t, but will for any class you care to
  // evaluate.
  uint64_t NumReps() const;

  // Returns a score >= the score of the best possible board to form with the
  // current possibilities for each cell. For more detailed statistics, call
  // BoundDetails(). Note that setting a bailout_score invalidates the
  // max_delta information in BoundDetails.
  struct ScoreDetails;
  const ScoreDetails& Details() const { return details_; };  // See below.

  // Compute an upper bound without any of the costly statistics.
  int UpperBound(int bailout_score = INT_MAX);

  // We should really write a paper on the exact meaning of these...
  struct ScoreDetails {
    int max_nomark;  // select the maximizing letter at each juncture.
    int sum_union;   // all words that can be found, counting each once.
  };

  virtual BreakingNode* Tree() { return NULL; }
  char CharAtIndex(int idx);
  int NumPossibilities();
  bool Possibility(int idx, int* cell, int* letter);
  void SetBuildTree(bool t) { build_tree_ = t; }

  int PossibilityIndex(int cell, int letter) {
    // std::cout << "PossibilityIndex(" << cell << ", " << letter << "):" << std::endl;
    // std::cout << " = " << indices_[(cell<<5) + letter] << std::endl;
    return indices_[(cell<<5)+letter];
  }
  std::vector<int> indices_;
  int max_index_;

 protected:
  virtual void InternalUpperBound(int bailout_score) = 0;

  static const int kWordScores[];
  SimpleTrie* dict_;
  int used_;
  uintptr_t runs_;
  ScoreDetails details_;

  bool build_tree_;

 private:
  char board_rep_[27*9];  // for as_string()
};

#endif
