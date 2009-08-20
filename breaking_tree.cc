#include "breaking_tree.h"
#include <algorithm>
#include <iostream>

int BreakingNode::RecomputeScore() {
  if (letter == '\0') {
    // Choose the max amongst each possibility.
    int max_score = 0;
    for (int i = 0; i < children.size(); i++) {
      if (children[i])
        max_score = std::max(max_score, children[i]->RecomputeScore());
    }
    return max_score;
  } else {
    // Add in the contributions of all neighbors.
    // (or all initial squares if this is the root node)
    int score = points;
    for (int i = 0; i < children.size(); i++) {
      if (children[i])
        score += children[i]->RecomputeScore();
    }
    return score;
  }
}

// Before:
// ./ibucket_boggle --size 34 bd a bd a bd a bd a bd a bd a | wc -l
//     7882
// After:
// ./ibucket_boggle --size 34 bd a bd a bd a bd a bd a bd a | wc -l
//     1787
// This should really be done during tree construction.
// For a larger board: 2,677,214 -> 895,853
BreakingNode* BreakingNode::Prune() {
  if (bound == 0 && letter != 'R') {
    delete this;
    return NULL;
  }

  std::vector<BreakingNode*> new_children;
  for (int i = 0; i < children.size(); i++) {
    if (!children[i]) continue;
    BreakingNode* child = children[i]->Prune();
    if (child) new_children.push_back(child);
  }
  children.swap(new_children);
  return this;
}
