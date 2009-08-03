// Author: danvk@google.com (Dan Vanderkam)
//
// Attempt to eliminate whole classes of 3x3 boggle boards by computing upper
// bounds and recursively splitting cells into individual letters.

// TODO(danvk): investigate the state of 'q'

#ifndef BREAKER3_H
#define BREAKER3_H

#include "3x3/ibuckets.h"

class BreakDetails;

class Breaker {
 public:
  // Does not take ownership of the BucketBuggler, though it must remain live
  // for the lifetime of the Breaker.
  Breaker(BucketBoggler* bb, int best_score);

  void Break(BreakDetails* details);

  // board is a space-separated list of letters on each cell, e.g.
  // "ab cd ef gh ij kl mn op qr"
  bool ParseBoard(const std::string& board);

  // Construct a board class by setting each cell to one of the buckets in
  // classes. If classes contains N letter classes, then indices 0 to N^9-1 are
  // all valid.
  // Returns false if the index is invalid.
  bool FromId(const std::vector<std::string>& classes, uint64_t idx);

  // Returns true if this is the least index corresponding to a symmetric
  // board.
  static bool IsCanonical(int num_classes, uint64_t idx);

  // Given a board array, calculate an ID for it.
  typedef int BdArray[3][3];
  static uint64_t BoardId(const BdArray& bd, int num_classes);

 private:
  BucketBoggler* bb_;
  int best_score_;
};

struct BreakDetails {
  int max_depth;
  int num_reps;
  double elapsed;
};

#endif
