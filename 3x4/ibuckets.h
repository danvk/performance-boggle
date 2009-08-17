#ifndef IBUCKETS_34
#define IBUCKETS_34

#include <limits.h>
#include "bucket_solver.h"
#include "trie.h"

class BucketSolver34 : public BucketSolver {
 public:
  BucketSolver34(SimpleTrie* t) : BucketSolver(t) {}
  virtual ~BucketSolver34() {}

  virtual int Width() const;
  virtual int Height() const;

  // Returns the possible characters in this cell. The result can be modified.
  virtual char* MutableCell(int idx);
  virtual const char* Cell(int idx) const;

 private:
  virtual void InternalUpperBound(int bailout_score = INT_MAX);

  int DoAllDescents(int idx, int len, SimpleTrie* t);
  int DoDFS(int i, int len, SimpleTrie* t);

  char bd_[12][27];  // null-terminated lists of possible letters
};

#endif
