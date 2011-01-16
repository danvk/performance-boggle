#include "bucket_solver.h"

#include "4x4/boggler.h"
#include "3x3/ibuckets.h"
#include "3x4/ibuckets.h"
#include "4x4/ibuckets.h"

#include <stdio.h>
#include <string.h>

using std::min;
using std::max;

const int BucketSolver::kWordScores[] =
      //0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17
      { 0, 0, 0, 1, 1, 2, 3, 5, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };

/* static */ BucketSolver* BucketSolver::Create(int size, const char* dictionary_file) {
  SimpleTrie* t = Boggler::DictionaryFromFile(dictionary_file);
  if (!t) return NULL;

  BucketSolver* solver = NULL;
  switch (size) {
    case 33: solver = new BucketSolver3(t); break;
    case 34: solver = new BucketSolver34(t); break;
    case 44: solver = new BucketSolver4(t); break;
    default:
      fprintf(stderr, "Unknown board size: %d\n", size);
      return NULL;
  }
  return solver;
}

BucketSolver::BucketSolver(SimpleTrie* t)
    : dict_(t), runs_(0), build_tree_(false) {}
BucketSolver::~BucketSolver() {}

bool BucketSolver::ParseBoard(const char* bd) {
  int cell = 0;
  int cell_pos = 0;
  int idx = 0;
  int num_cells = Width() * Height();
  indices_.resize((Width() * Height()) << 5);
  for (int i = 0; i < indices_.size(); i++) indices_[i] = -1;
  while (char c = *bd++) {
    if (c == ' ') {
      if (cell_pos == 0) return false;  // empty cell
      MutableCell(cell)[cell_pos] = '\0';
      cell += 1;
      cell_pos = 0;
      if (cell > num_cells - 1) return false;  // too many cells
    } else if (c == '.') {
      // explicit "don't go here" cell, useful for tests
      MutableCell(cell)[0] = '\0';
      cell_pos = 1;
    } else {
      if (c < 'a' || c > 'z') return false;  // invalid letter
      MutableCell(cell)[cell_pos++] = c;
      // std::cout << " Set " << ((cell<<5)+cell_pos) << " to " << idx << std::endl;
      indices_[(cell << 5) + cell_pos] = idx;
      idx += 1;
      if (cell_pos >= 27) return false;  // too many letters on a cell
    }
  }
  MutableCell(cell)[cell_pos] = '\0';
  max_index_ = idx;
  return (cell_pos > 0 && cell == (num_cells - 1));
}

uint64_t BucketSolver::NumReps() const {
  uint64_t reps = 1;
  int num_cells = Width() * Height();
  for (int i = 0; i < num_cells; i++)
    reps *= strlen(Cell(i));
  return reps;
}

const char* BucketSolver::as_string() {
  char* c = board_rep_;
  int num_cells = Width() * Height();
  for (int i=0; i<num_cells; i++) {
    if (*Cell(i)) {
      strcpy(c, Cell(i));
      c += strlen(Cell(i));
    } else {
      strcpy(c++, ".");
    }
    *c++ = (i == (num_cells - 1) ? '\0' : ' ');
  }
  return board_rep_;
}

int BucketSolver::UpperBound(int bailout_score) {
  details_.max_nomark = 0;
  details_.sum_union = 0;

  used_ = 0;
  runs_ += 1;
  InternalUpperBound(bailout_score);
  return min(details_.max_nomark, details_.sum_union);
}

char BucketSolver::CharAtIndex(int idx) {
  int cell = idx >> 5;
  return Cell(cell)[idx % 32];
}

int BucketSolver::NumPossibilities() {
  int len = 0;
  for (int i = 0; i < Width() * Height(); i++) {
    len += strlen(Cell(i));
  }
  return len;
}

// ab cd
bool BucketSolver::Possibility(int idx, int* cell, int* letter) {
  int len = 0;
  for (int i = 0; i < Width() * Height(); i++) {
    int prev_len = len;
    len += strlen(Cell(i));
    if (len > idx) {
      *cell = i;
      *letter = idx - prev_len;
      return true;
    }
  }
  return false;
}
