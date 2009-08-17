#include <assert.h>
#include <iostream>
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

DEFINE_bool(display_debug_output, true, "");


using namespace std;
void PrintDetails(BreakDetails& d);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  SimpleTrie* t = Boggler::DictionaryFromFile(FLAGS_dictionary.c_str());

  BucketSolver* solver = NULL;
  switch (FLAGS_size) {
    case 33: solver = new BucketSolver3(t); break;
    case 34: solver = new BucketSolver34(t); break;
    case 44: solver = new BucketSolver4(t); break;
    default:
      fprintf(stderr, "Unknown board size: %d\n", FLAGS_size);
      exit(1);
  }

  Breaker breaker(solver, FLAGS_best_score);
  breaker.SetDisplayDebugOutput(FLAGS_display_debug_output);

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
      uint64_t idx = r.IRandom(0, max_index - 1);
      string encoded_board = bu.BoardFromId(idx);
      string board = bu.ExpandPartitions(encoded_board);
      if (board.empty() || encoded_board.empty()) {
        cerr << "Ugh: " << idx << endl;
        exit(1);
      }
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
  printf("Broke %llu/%llu @ depth %d in %.4fs = %fbds/sec\n",
         (d.num_reps - unbroken), d.num_reps, d.max_depth,
         d.elapsed, d.num_reps / d.elapsed);

  if (!d.failures.empty()) {
    printf("Unbroken boards:\n");
    for (unsigned int i = 0; i < d.failures.size(); i++) {
      printf("%s\n", d.failures[i].c_str());
    }
  }
}
