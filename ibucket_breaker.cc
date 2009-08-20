// Author: danvk@google.com (Dan Vanderkam)
//
// Play bucketed boggle w/o a bucketed trie. This could potentially be really
// slow, but will give better bounds and allow more flexible bucketing.

#include "ibucket_breaker.h"

#include <algorithm>
#include <inttypes.h>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <vector>
#include "gflags/gflags.h"
#include "board-utils.h"

using std::cout;
using std::endl;
using std::setprecision;

Breaker::Breaker(BucketSolver* solver, int best_score)
    : solver_(solver), best_score_(best_score) {
  debug_ = true;
  cells_ = solver_->Width() * solver_->Height();

  // Construct a preferred order in which to break cells, starting with the
  // middle and moving outwards.
  BoardUtils bu(solver_->Width(), solver_->Height());
  std::vector<std::pair<double, int> > distance;  // (distance from center, idx)
  for (int x = 0; x < solver_->Width(); x++) {
    for (int y = 0; y < solver_->Height(); y++) {
      double dx = 1.0 * x - solver_->Width() / 2.0;
      double dy = 1.0 * y - solver_->Height() / 2.0;
      distance.push_back(std::make_pair(dx * dx + dy * dy, bu.Id(x, y)));
    }
  }
  stable_sort(distance.begin(), distance.end());
  for (int i = 0; i < distance.size(); i++) {
    order_.push_back(distance[i].second);
  }
}

// TODO(danvk): make these methods and stop passing around state.
double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}


// Use the board solving details to pick a bucket to kill and guess how many
// board representatives this will kill.
int Breaker::PickABucket(std::vector<std::string>* splits, int level) {
  int pick = -1;

  splits->clear();
  // Possible heuristics:
  //   - always pick the center when possible or a non-corner
  //   - pick the cell with the most letters
  //   - pick the cell with the least letters > 1
  // TODO(danvk): make this choice a parameter
  for (int i = 0; i < cells_; i++) {
    if (strlen(solver_->Cell(order_[i])) > 1) {
      pick = order_[i];
      break;
    }
  }
  // int max_len = 1;
  // for (int i = 0; i < 9; i++) {
  //   int len = strlen(solver_->Cell(i));
  //   if (len > max_len) {
  //     max_len = len;
  //     pick = i;
  //   }
  // }

  // TODO(danvk): analyze this in more detail; it has a big impact on speed.
  int len = strlen(solver_->Cell(pick));
  if (len == 26) {
    splits->push_back("aeiou");
    splits->push_back("sy");
    splits->push_back("bdfgjkmpvwxzq");
    splits->push_back("chlnrt");
  } else if (len >= 9) {
    int num_splits = 4;  //(len < 13 ? 3 : 4);
    splits->push_back("");
    int split = 0;
    for (int i = 0; i < len; i++) {
      if (1.0 * num_splits * i >= (split+1) * len) {
        split += 1;
        splits->push_back("");
      }
      splits->back() += std::string(1, solver_->Cell(pick)[i]);
    }
  } else {
    for (int j=0; solver_->Cell(pick)[j]; j++) {
      splits->push_back(std::string(1, solver_->Cell(pick)[j]));
    }
  }

  int out_len = 0;
  for (unsigned int i = 0; i < splits->size(); i++)
    out_len += splits->at(i).size();
  if (out_len != len) {
    cout << solver_->Cell(pick) << " (" << len << ") => " << out_len << endl;
    exit(1);
  }

  return pick;
}

void Breaker::SplitBucket(int level) {
  char orig_bd[27 * cells_];
  char orig_cell[27];
  std::vector<std::string> splits;
  int cell = PickABucket(&splits, level);
  if (cell == -1) {
    // should be a board at this point, so the spaces are unneeded.
    std::string bd;
    const char* bd_class = solver_->as_string();
    for (; *bd_class; bd_class++) {
      if (*bd_class != ' ') bd.append(1, *bd_class);
    }
    details_->failures.push_back(bd);
    if (debug_) {
      cout << "Unable to break board: " << bd << endl;
    }
    return;
  }

  if (debug_) cout << "split cell " << cell << endl;

  strcpy(orig_bd, solver_->as_string());
  strcpy(orig_cell, solver_->Cell(cell));

  if (debug_) {
    cout << std::string(level, ' ') << "Will evaluate "
         << splits.size() << " more boards..." << endl;
  }

  for (unsigned int i=0; i < splits.size(); i++) {
    if (!solver_->ParseBoard(orig_bd)) {
      fprintf(stderr, "bucket boggle couldn't parse '%s'\n", orig_bd);
      exit(1);
    }
    strcpy(solver_->MutableCell(cell), splits[i].c_str());
    AttackBoard(level + 1, 1+i, splits.size());
  }
}

// Shed/Split until finished
void Breaker::AttackBoard(int level, int num, int outof) {
  uint64_t reps = solver_->NumReps();
  if (debug_) {
    float frac = 100.0 * elim_ / orig_reps_;
    float est = (secs() - details_->start_time) * orig_reps_ / elim_;
    cout << "(" << setprecision(5) << frac << "%)" << std::string(level, ' ')
         << " (" << level << ";" << num << "/" << outof << ") "
         << solver_->as_string() << " (" << reps << ") est. " << est << " s"
         << endl;
  }

  if (solver_->UpperBound(best_score_) <= best_score_) {
    elim_ += solver_->NumReps();
    if (solver_->Details().max_nomark <= solver_->Details().sum_union) {
      details_->max_wins += 1;
    } else {
      details_->sum_wins += 1;
    }
    if (level > details_->max_depth) details_->max_depth = level;
    return;
  } else {
    SplitBucket(level);
  }
}


void Breaker::Break(BreakDetails* details) {
  std::string orig = solver_->as_string();
  details_ = details;
  details_->max_depth = 0;
  details_->num_reps = 0;
  details_->elapsed = 0.0;
  details_->failures.clear();
  details_->sum_wins = 0;
  details_->max_wins = 0;

  elim_ = 0;
  orig_reps_ = solver_->NumReps();
  details_->start_time = secs();
    AttackBoard();
  double b = secs();
  double a = details_->start_time;
  if (debug_) {
    float pace = 1.0*elim_/(b-a);
    cout << elim_ << " reps in " << setprecision(3) << (b - a) << " s "
         << "@ depth " << details_->max_depth
         << " = " << pace << " bds/sec:\n  " << orig
         << endl;
  }

  details->elapsed = b - a;
  details->num_reps = orig_reps_;
}


bool Breaker::ParseBoard(const std::string& board) {
  return solver_->ParseBoard(board.c_str());
}
