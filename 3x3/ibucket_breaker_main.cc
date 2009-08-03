#include <assert.h>
#include <math.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include "3x3/boggler.h"
#include "3x3/ibucket_breaker.h"
#include "gflags/gflags.h"
#include "mtrandom/randomc.h"
#include "trie.h"

DEFINE_string(dictionary, "words", "Dictionary file");
DEFINE_int32(best_score, 545, "Best known score for a 3x3 boggle board");

DEFINE_bool(filter_canonical, false, "Skip non-canonical boards");
DEFINE_string(letter_classes, "aeiou sy bdfgjkmpvwxz chlnrt",
              "space-separated classes of letters");

DEFINE_int64(run_on_index, -1,
             "Set to a value to break a specific board.");

DEFINE_int32(rand_seed, -1,
             "Random number seed (default is based on time and pid)");
DEFINE_int32(random_boards, 1,
             "Run on this many random board classes from the bucket space.");

DEFINE_string(break_class, "", "Set to break a specific board class");

void PrintDetails(BreakDetails& d);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  SimpleTrie* t = GenericBoggler<SimpleTrie>::DictionaryFromFile(
    FLAGS_dictionary.c_str());

  BucketBoggler b(t);
  Breaker breaker(&b, FLAGS_best_score);

  std::vector<std::string> classes;
  int letter_count = 0;
  classes.push_back("");
  for (unsigned int i = 0; i < FLAGS_letter_classes.size(); i++) {
    char c = FLAGS_letter_classes[i];
    if (c == ' ') {
      classes.push_back("");
    } else {
      (*classes.rbegin()) += std::string(1, c);
      letter_count += 1;
    }
  }
  // assert(letter_count == 26);

  if (FLAGS_run_on_index >= 0) {
    if (FLAGS_run_on_index > pow(classes.size(), 9)) {
      fprintf(stderr, "board index is too large.\n");
      exit(1);
    }
    if (!breaker.FromId(classes, FLAGS_run_on_index)) {
      fprintf(stderr, "Couldn't parse board id %lld\n", FLAGS_run_on_index);
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
    uint64_t max_index = pow(classes.size(), 9);
    for (int i = 0; i < FLAGS_random_boards; i++) {
      uint64_t idx = r.IRandom(0, max_index - 1);
      breaker.FromId(classes, idx);
      breaker.Break(&details);
      PrintDetails(details);
    }
  }
}

void PrintDetails(BreakDetails& d) {
  uint64_t unbroken = d.failures.size();
  printf("Broke %llu/%llu @ depth %d in %.4fs = %fbds/sec\n",
         (d.num_reps - unbroken), d.num_reps, d.max_depth,
         d.elapsed, d.num_reps / d.elapsed);
  printf("Unbroken boards:\n");
  for (unsigned int i = 0; i < d.failures.size(); i++) {
    printf("%s\n", d.failures[i].c_str());
  }
}
