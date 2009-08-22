#ifndef BREAKING_TREE_H
#define BREAKING_TREE_H

#include <limits.h>
#include <map>
#include <vector>
class BucketSolver;

class BreakingNode {
 public:
  BreakingNode() {}
  ~BreakingNode() {
    for (int i = 0; i < children.size(); i++) {
      if (children[i]) delete children[i];
    }
  }

  char letter;
  char cell;
  static const char ROOT_NODE = -2;
  static const char CHOICE_NODE = -1;

  // These might be the various options on a cell or the various directions.
  std::vector<BreakingNode*> children;

  // cached computation across all children
  int bound;

  // points contributed by _this_ node.
  int points;

  // bit mask of which possibilities eventually get hit by children.
  // can be used to short-circuit recomputation of bounds.
  std::vector<bool> child_possibilities;

  int RecomputeScore();
  BreakingNode* Prune();
  void ChoiceStats(std::map<int, int>* counts);

  int NodeCount();
  int ScoreWithForce(int force_cell, int force_letter);

  void AttachPossibilities(BucketSolver* solver);
};

#endif
