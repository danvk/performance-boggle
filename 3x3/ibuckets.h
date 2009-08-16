#ifndef IBUCKETS_33
#define IBUCKETS_33

#include <limits.h>
#include "bucket_solver.h"
#include "trie.h"

class BucketSolver3 : public BucketSolver {
 public:
  BucketSolver3(SimpleTrie* t) : BucketSolver(t) {}
  virtual ~BucketSolver3() {}

  virtual int Width() const;
  virtual int Height() const;

  // Returns the possible characters in this cell. The result can be modified.
  virtual char* MutableCell(int idx);
  virtual const char* Cell(int idx) const;

 private:
  virtual void InternalUpperBound(int bailout_score = INT_MAX);

  int DoAllDescents(int idx, int len, SimpleTrie* t);
  int DoDFS(int i, int len, SimpleTrie* t);

  char bd_[9][27];  // null-terminated lists of possible letters
};

#endif
