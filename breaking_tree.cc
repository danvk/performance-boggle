#include "breaking_tree.h"
#include "bucket_solver.h"
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
  (*counts)[letter] += 1;
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

void BreakingNode::AttachPossibilities(BucketSolver* solver) {
  child_possibilities.resize(solver->Width() * solver->Height());
  if (letter >= 0)
    child_possibilities[cell] = true;

  for (int i = 0; i < children.size(); i++) {
    if (children[i]) {
      children[i]->AttachPossibilities(solver);
      for (int j = 0; j < child_possibilities.size(); j++)
        if (children[i]->child_possibilities[j])
          child_possibilities[j] = true;
    }
  }
}

int BreakingNode::ScoreWithForce(int force_cell, int force_letter) {
  if (!child_possibilities[force_cell]) return bound;

  if (letter == CHOICE_NODE) {
    // If the force is on this cell, we must take it.
    if (cell == force_cell) {
      for (int i = 0; i < children.size(); i++) {
        if (children[i] && children[i]->letter == force_letter) {
          return children[i]->ScoreWithForce(force_cell, force_letter);
        }
      }
      return 0;
    } else {
      // otherwise, just like normal scoring.
      int max_score = 0;
      for (int i = 0; i < children.size(); i++) {
        if (children[i]) {
          int score = children[i]->ScoreWithForce(force_cell, force_letter);
          max_score = std::max(max_score, score);
        }
      }
      return max_score;
    }
  } else {
    int score = points;
    for (int i = 0; i < children.size(); i++) {
      if (children[i])
        score += children[i]->ScoreWithForce(force_cell, force_letter);
    }
    return score;
  }
}
