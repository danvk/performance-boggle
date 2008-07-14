// Calculate upper bounds on bucketed boggle boards without using a bucketed
// Trie. This is more CPU-intensive, but gives much tighter bounds.
//
// This should be templated on Trie type, but I'd like to avoid that messiness
// for the time being.
#include "trie.h"

class BucketBoggler {
 public:
  BucketBoggler(SimpleTrie* t) : dict_(t), runs_(0) {}
  
  // bd is a class of boards with cells delimited by spaces.
  // examples:
  // "a b c d e f g h i j k l m n o p"
  // "aeiou bcdfghjklmnprstvwxyz aeiou ..."
  // NOTE: "qu" gets interpreted as "qu" or "u".
  bool ParseBoard(const char* bd);
  const char* as_string();
  
  // Returns the possible characters in this cell. The result can be modified.
  char* Cell(int idx) { return bd_[idx]; }

  // Returns the number of individual boards in the current board class. This
  // isn't guaranteed to fit in a uint64_t, but will for any class you care to
  // evaluate.
  uint64_t NumReps() const;
  
  // Returns a score >= the score of the best possible board to form with the
  // current possibilities for each cell. For more detailed statistics, call
  // BoundDetails(). Note that setting a bailout_score invalidates the
  // max_delta information in BoundDetails.
  class ScoreDetails;
  int UpperBound(int bailout_score = INT_MAX);
  const ScoreDetails& Details() const { return details_; };  // See below.

  // Compute an upper bound without any of the costly statistics.
  int SimpleUpperBound(int bailout_score = INT_MAX);

  // Removes any possible letter choices that would reduce the max_nomark upper
  // bound below the cutoff. Returns a count of the number of shed letters.
  // Returns -1 on any error (e.g. board has changed since last UpperBound).
  int ShedLetters(int cutoff);

  // We should really write a paper on the exact meaning of these...
  struct ScoreDetails {
    int max_nomark;  // select the maximizing letter at each juncture.
    int sum_union;  // all words that can be found, counting each once.

    // Approx number of points going through each cell.
    int point_counts[16];

    // The amount by which the max_nomark bound would decrease if a cell were
    // forced to take on a particular value. Values that weren't considered are
    // assigned -1.
    int max_delta[16][26];

    int most_constrained_cell;
    int one_level_win;

    double elapsed_time;
  };

  // Print out a summary of the effects of each possible one-letter choice.
  void PrintChoices();

 private:
  void DoAllDescents(int i, int len, SimpleTrie* t, int*, int*);
  int DoDFS(int i, int len, SimpleTrie* t);
  int BestBound();
  int SimpleDoAllDescents(int idx, int len, SimpleTrie* t);
  int SimpleDoDFS(int i, int len, SimpleTrie* t);

  SimpleTrie* dict_;
  uintptr_t runs_;
  char bd_[16][27];  // null-terminated lists of possible letters
  int used_;
  ScoreDetails details_;
  char board_rep_[27*16];

  void SetCellIndices();
  int cell_indices_[16];
  int num_letters_;
};
