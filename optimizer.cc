#include "optimizer.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include "boggle_solver.h"

/* static */ Annealer::Options Annealer::DefaultOptions() {
  Options ret;
  ret.cool_t0 = 100.0;
  ret.cool_k = 0.05;
  ret.swap_ratio = 1.0;
  ret.mutation_p = 0.75;
  ret.max_stall = 1000;
  return ret;
}

Annealer::Annealer(BoggleSolver* solver, const Options& opts, BoggleRNG* rng) {
  solver_ = solver;
  opts_ = opts;
  rng_ = rng;
  num_squares_ = solver->Width() * solver->Height();
  bd_ = (char*)malloc(1 + num_squares_);
  last_ = (char*)malloc(1 + num_squares_);
} 

Annealer::~Annealer() {
  free(bd_);
  free(last_);
}

const char* Annealer::FinalBoard() const {
  return bd_;
}

int Annealer::FinalScore() const {
  return best_score_;
}

const Annealer::Stats& Annealer::FinalStats() const {
  return stats_;
}

void Annealer::Run() {
  bd_[0] = bd_[num_squares_] = '\0';
  last_[num_squares_] = '\0';
  InitialBoard(last_);

  int last_accept = 0;
  stats_.transitions = 0;

  for (int n = 0; n < last_accept + opts_.max_stall; n++) {
    memcpy(bd_, last_, num_squares_);
    Mutate(bd_);
    int score = solver_->Score(bd_);
    if (score == -1) {
      // TODO(danvk): return false?
      fprintf(stderr, "Board '%s' couldn't be scored. Quitting...\n", bd_);
      exit(1);
    }

    if (opts_.print_scores) printf("%d\t%d\n", n, score);

    double T = Temperature(n);
    if (AcceptTransition(best_score_, score, T)) {
      stats_.transitions += 1;
      last_accept = n;
      best_score_ = score;
      memcpy(last_, bd_, num_squares_);
      if (opts_.print_transitions) {
        printf("%5d T=%3.8lf accepting '%s' (%d)\n", n, T, last_, best_score_);
      }
    }
  }
}

// A random initial board. Does not set a trailing null char!
void Annealer::InitialBoard(char* bd) {
  for (int i = 0; i < num_squares_; i++) {
    bd[i] = rng_->IRandom('a', 'z');
  }
}

void Annealer::Mutate(char* bd) {
  stats_.mutate_calls += 1;
  do {
    stats_.mutations += 1;
    if ((1.0 + opts_.swap_ratio) * rng_->Random() > 1.0) {
      // swap two cells
      stats_.swaps += 1;
      int a, b;
      do {
        int pair = rng_->IRandom(0, num_squares_ * num_squares_ - 1);
        a = pair / num_squares_;
        b = pair % num_squares_;
      } while (bd[a] == bd[b]);
      char tmp = bd[a];
      bd[a] = bd[b];
      bd[b] = tmp;
    } else {
      // change a cell
      stats_.changes += 1;
      int cell, letter;
      do {
        int pair = rng_->IRandom(0, 26 * num_squares_ - 1);
        cell = pair / 26;
        letter = 'a' + pair % 26;
      } while (bd[cell] == letter);
      bd[cell] = letter;
    }
  } while (rng_->Random() > opts_.mutation_p);
}

// Should we transition between boards with these two scores?
bool Annealer::AcceptTransition(int cur_score, int new_score, double T) {
  if (new_score > cur_score)
    return true;
  double p = exp((new_score - cur_score)/T);
  return (rng_->Random() < p);
}

// "Temperature" after n iterations
double Annealer::Temperature(int n) {
  return opts_.cool_t0 * exp(-opts_.cool_k * n);
}
