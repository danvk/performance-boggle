#include "breaking_tree.h"
#include <algorithm>
#include <iostream>
#include <map>

int BreakingNode::RecomputeScore() {
  if (letter == CHOICE_NODE) {
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
// TODO(danvk): This should really be done during tree construction.
// For a larger board: 2,677,214 -> 895,853
BreakingNode* BreakingNode::Prune() {
  if (bound == 0 && letter != ROOT_NODE) {
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

void BreakingNode::ChoiceStats(std::map<int, int>* counts) {
  if (letter != CHOICE_NODE && letter != ROOT_NODE) {
    (*counts)[letter] += 1;
  }
  for (int i = 0; i < children.size(); i++) {
    if (children[i]) children[i]->ChoiceStats(counts);
  }
}

int BreakingNode::NodeCount() {
  int count = 1;
  for (int i = 0; i < children.size(); i++) {
    if (children[i]) count += children[i]->NodeCount();
  }
  return count;
}

void BreakingNode::AttachPossibilities(int num_possibilities) {
  child_possibilities.resize(num_possibilities);
  if (letter >= 0) child_possibilities[letter] = true;

  for (int i = 0; i < children.size(); i++) {
    if (children[i]) {
      children[i]->AttachPossibilities(num_possibilities);
      for (int j = 0; j < num_possibilities; j++)
        if (children[i]->child_possibilities[j])
          child_possibilities[j] = true;
    }
  }
}

int BreakingNode::ScoreWithForce(int force) {
  // TODO(danvk): use child_possibilities for a big speedup
  if (letter == CHOICE_NODE) {
    // If the force is one of the options, we must take it.
    for (int i = 0; i < children.size(); i++) {
      if (children[i] && children[i]->letter == force) {
        return children[i]->ScoreWithForce(force);
      }
    }

    // otherwise, just like normal scoring.
    int max_score = 0;
    for (int i = 0; i < children.size(); i++) {
      if (children[i])
        max_score = std::max(max_score, children[i]->ScoreWithForce(force));
    }
    return max_score;
  } else {
    int score = points;
    for (int i = 0; i < children.size(); i++) {
      if (children[i])
        score += children[i]->ScoreWithForce(force);
    }
    return score;
  }
}
