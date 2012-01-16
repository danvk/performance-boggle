#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <limits>
#include <math.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include "3x3/ibuckets.h"
#include "3x4/ibuckets.h"
#include "4x4/ibuckets.h"
#include "4x4/boggler.h"  // gross
#include "board-utils.h"
#include "ibucket_breaker.h"
#include "init.h"
#include "gflags/gflags.h"
#include "mtrandom/randomc.h"
#include "trie.h"

DEFINE_int32(size, 44, "Type of boggle board to use (MN = MxN)");
DEFINE_string(dictionary, "words", "Dictionary file");
DEFINE_int32(best_score, 3625, "Best known score for a 3x3 boggle board");

DEFINE_bool(filter_canonical, false, "Skip non-canonical boards");
DEFINE_string(letter_classes, "aeiou sy bdfgjkmpvwxz chlnrt",
              "space-separated classes of letters");
DEFINE_bool(break_all, false,
            "Set to true to break all classes based on letter_classes. "
            "Strongly recommended to set --filter_canonical in addition to "
            "this flag. May take a while (i.e. a day)");

DEFINE_int64(run_on_index, -1,
             "Set to a value to break a specific board.");

DEFINE_int32(rand_seed, -1,
             "Random number seed (default is based on time and pid)");
DEFINE_int32(random_boards, 0,
             "Run on this many random board classes from the bucket space.");

DEFINE_string(break_class, "", "Set to break a specific board class");

DEFINE_string(pick_cell_order, "",
              "Set to a comma-delimited permutation of cell indices to "
              "split them in that order, e.g. '0,1,2,3,4,5,6,7,8'");


using namespace std;
void PrintDetails(BreakDetails& d);
uint64_t Rand64(uint64_t max, TRandomMersenne& rand);

void SplitString(std::string& s, vector<int>* nums) {
  for (int i = 0; i < s.size(); i++) {
    if (s[i] == ',') s[i] = '\n';
  }

  // From http://stackoverflow.com/questions/236129/how-to-split-a-string
  std::vector<std::string> tokens;
  std::istringstream iss(s);
  copy(istream_iterator<string>(iss),
       istream_iterator<string>(),
       back_inserter<vector<string> >(tokens));

  for (int i = 0; i < tokens.size(); i++) {
    nums->push_back(atoi(tokens[i].c_str()));
  }
}

int main(int argc, char** argv) {
  Init(&argc, &argv);

  BucketSolver* solver = BucketSolver::Create(
    FLAGS_size, FLAGS_dictionary.c_str());
  if (!solver) {
    fprintf(stderr, "Couldn't create bucket solver: %d %s\n",
            FLAGS_size, FLAGS_dictionary.c_str());
    exit(1);
  }

  Breaker breaker(solver, FLAGS_best_score);
  // BreakOptions opts;
  // opts.print_progress = FLAGS_display_debug_output;
  // breaker.SetOptions(opts);

  if (!FLAGS_pick_cell_order.empty()) {
    std::vector<int> picks;
    SplitString(FLAGS_pick_cell_order, &picks);
    breaker.SetPickOrder(picks);
  }

  vector<string> classes;
  int letter_count = 0;
  classes.push_back("");
  for (unsigned int i = 0; i < FLAGS_letter_classes.size(); i++) {
    char c = FLAGS_letter_classes[i];
    if (c == ' ') {
      classes.push_back("");
    } else {
      (*classes.rbegin()) += string(1, c);
      letter_count += 1;
    }
  }
  BoardUtils bu(solver->Width(), solver->Height());
  bu.UsePartition(classes);

  if (FLAGS_run_on_index >= 0) {
    string encoded_board = bu.BoardFromId(FLAGS_run_on_index);
    string board = bu.ExpandPartitions(encoded_board);
    if (encoded_board.empty() || board.empty()) {
      cerr << "Couldn't parse board id " << FLAGS_run_on_index << endl;
      exit(1);
    }

    if (!breaker.ParseBoard(board)) {
      fprintf(stderr, "Couldn't parse board %s", board.c_str());
      exit(1);
    }

    BreakDetails details;
    breaker.Break(&details);
    PrintDetails(details);
    exit(0);
  }

  if (!FLAGS_break_class.empty()) {
    BreakDetails details;
    if (!breaker.ParseBoard(FLAGS_break_class)) {
      fprintf(stderr, "Breaker couldn't parse '%s'\n",
              FLAGS_break_class.c_str());
      exit(1);
    }
    breaker.Break(&details);
    PrintDetails(details);
    exit(0);
  }

  if (FLAGS_random_boards > 0) {
    if (FLAGS_rand_seed == -1) {
      FLAGS_rand_seed = time(NULL) + getpid();
    }
    TRandomMersenne r(FLAGS_rand_seed);

    BreakDetails details;
    int num_cells = solver->Width() * solver->Height();
    uint64_t max_index = pow(classes.size(), num_cells);
    for (int i = 0; i < FLAGS_random_boards; i++) {
      uint64_t idx = Rand64(max_index - 1, r);
      string encoded_board = bu.BoardFromId(idx);
      string board = bu.ExpandPartitions(encoded_board);
      if (board.empty() || encoded_board.empty()) {
        cerr << "Ugh: " << idx << endl;
        exit(1);
      }
      cout << idx << ": " << board << endl;
      breaker.ParseBoard(board);
      breaker.Break(&details);
      PrintDetails(details);
    }
    exit(0);
  }

  if (FLAGS_break_all) {
    int num_cells = solver->Width() * solver->Height();
    uint64_t max_index = pow(classes.size(), num_cells);
    vector<string> good_boards;
    for (uint64_t idx = 0; idx < max_index; idx++) {
      if (idx % 100 == 0) {
        cout << idx << endl;
      }
      string encoded_board = bu.BoardFromId(idx);
      if (bu.IsCanonical(encoded_board)) {
        string board = bu.ExpandPartitions(encoded_board);
        BreakDetails details;
        breaker.ParseBoard(board);
        breaker.Break(&details);
        if (!details.failures.empty()) {
          for (int i = 0; i < details.failures.size(); i++) {
            cout << details.failures[i] << endl;
            good_boards.push_back(details.failures[i]);
          }
        }
      }
    }
    for (int i = 0; i < good_boards.size(); i++) {
      cout << good_boards[i] << endl;
    }
  }
}


void PrintDetails(BreakDetails& d) {
  uint64_t unbroken = d.failures.size();
  printf("Broke %llu/%llu @ depth %d in %.4fs = %fbds/sec (%d/%d sum/max)\n",
         static_cast<unsigned long long int>(d.num_reps - unbroken),
         static_cast<unsigned long long int>(d.num_reps),
         d.max_depth,
         d.elapsed, d.num_reps / d.elapsed,
         d.sum_wins, d.max_wins);

  if (!d.failures.empty()) {
    printf("Unbroken boards:\n");
    for (unsigned int i = 0; i < d.failures.size(); i++) {
      printf("%s\n", d.failures[i].c_str());
    }
  }
}

uint64_t Rand64(uint64_t max, TRandomMersenne& rand) {
  if (max < (uint64_t)numeric_limits<int>::max()) {
    return rand.IRandom(0, max);
  } else {
    uint64_t r;
    r = uint64_t((max + 1) * rand.Random());
    if (r > max) r = max;
    return r;
  }
}
