// Calculate upper bounds on boggle boards with multiple possible letters on
// each square. This can be quite CPU-intensive.
#include <limits.h>
#include "trie.h"

namespace paper {

class BucketBoggler {
 public:
  BucketBoggler(Trie* t) : dict_(t), runs_(0) {}

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
  const ScoreDetails& Details() const { return details_; };  // See below.

  // Compute an upper bound without any of the costly statistics.
  int UpperBound(int bailout_score = INT_MAX);

  // We should really write a paper on the exact meaning of these...
  struct ScoreDetails {
    int max_nomark;  // select the maximizing letter at each juncture.
    int sum_union;  // all words that can be found, counting each once.

    // Approx number of points going through each cell.
    int point_counts[9];

    int most_constrained_cell;

    double elapsed_time;
  };

  // Print out a summary of the effects of each possible one-letter choice.
  void PrintChoices();

 private:
  int DoAllDescents(int idx, int len, Trie* t);
  int DoDFS(int i, int len, Trie* t);

  Trie* dict_;
  uintptr_t runs_;
  char bd_[9][27];  // null-terminated lists of possible letters
  int used_;
  ScoreDetails details_;
  char board_rep_[27*9];

  void SetCellIndices();
  int cell_indices_[9];
  int num_letters_;
};

}  // namespace paper
