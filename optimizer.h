// General collection of classes for doing board optimization.
// Specific techniques are hill-climbing and simulated annealing.

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "boggle_solver.h"
class TRandomMersenne;

// Generic random number generator.
class BoggleRNG {
 public:
  BoggleRNG() {}
  virtual ~BoggleRNG() {}

  // Maps (a, b) -> random integer in [a, b].
  virtual int IRandom(int a, int b) = 0;

  // Returns a double in [0.0, 1.0).
  virtual double Random() = 0;
};


// Wrapper around TRandomMersenne
class BoggleMTRandom : public BoggleRNG {
 public:
  BoggleMTRandom(TRandomMersenne* mt) : mt_(mt) {}

  int IRandom(int a, int b);
  double Random();

 private:
  TRandomMersenne* mt_;
};


class Annealer {
 public:
  struct Options {
    double cool_t0;
    double cool_k;
    double swap_ratio;
    double mutation_p;
    double max_stall;
  };

  static Options DefaultOptions();

  Annealer(BoggleSolver* solver, const Options& opts, BoggleRNG* rng);
  ~Annealer();

  void Run();

  const char* FinalBoard() const;
  int FinalScore() const;

  struct Stats {
    int transitions;
    int swaps;
    int changes;
    int mutations;
    int mutate_calls;
  };

  const Stats& FinalStats() const;

 private:
  // A random initial board. Does not set a trailing null char!
  void InitialBoard(char* bd);

  // Randomly mutate the board according to the options.
  void Mutate(char* bd);

  // Should the transiton from cur_score -> new_score be accepted at temp. T?
  bool AcceptTransition(int cur_score, int new_score, double T);

  // What's the temperature at step N?
  double Temperature(int n);

  BoggleSolver* solver_;
  BoggleRNG* rng_;
  Options opts_;
  Stats stats_;
  char* bd_;
  char* last_;
  int best_score_;
  int num_squares_;
};

#endif
