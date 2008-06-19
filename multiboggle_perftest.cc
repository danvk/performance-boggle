// Verify that multiboggle is faster than normal boggler on ideal input.
//
// $ ./multiboggle_perftest               
// Basic scored 8738734 pts in 1.77781 s (219723 bds/s)
// Multi scored 8738734 pts in 0.13092 s (2.98369e+06 bds/s)
// Speedup: 13.5793x

#include <iostream>
#include <vector>
#include <stdio.h>
#include "trie.h"
#include "boggler.h"
#include "multiboggle.h"
#include <sys/time.h>
using std::cout;
using std::endl;
using std::vector;

static int CheckScores = 0;
static int OldScoring = 0;
double secs();

// Print out details on a mismatch
// It would be really nice if this diffed the word lists with the normal
// boggler, but hopefully I won't have much occasion to need that anymore.
void SummarizeError(MultiBoggle& mb,
                    int x, int y, int a, int b,
                    int actual, int expected,
                    MultiBoggle::TwoHole& two,
                    MultiBoggle::OneHole& tl, MultiBoggle::OneHole& br) {
  cout << "Mismatch on "
    << x << ", " << y << ", " << a << ", " << b
    << " = " << mb.ToString();
  cout << " " << actual << " != " << expected << endl;

  cout << "Two hole board:" << endl;
  mb.PrintTwoHole(two);

  cout << "One hole board TL (" << std::string(1, 'a' + a) << "):" << endl;
  mb.PrintOneHole(two, tl, 0);

  cout << "One hole board BR (" << std::string(1, 'a' + b) << "):" << endl;
  mb.PrintOneHole(two, br, 3);
}

int TestMultiBoggler(Trie* t, const char* base_board,
                      const vector<int>& expected,
                      bool do_fill_one, bool do_merge) {
  int idx = 0;
  int multi_score = 0;
  bool check_scores = !expected.empty();

  MultiBoggle mb(t);
  mb.ParseBoard(base_board);
  vector<MultiBoggle::OneHole> tl(26);
  vector<MultiBoggle::OneHole> br(26);
  for (int x=0; x<26; x++) {
    if (x == 'q' - 'a') continue;
    mb.SetCell(1, 0, x);
    for (int y=0; y<26; y++) {
      if (y == 'q' - 'a') continue;
      mb.SetCell(2, 3, y);
      
      MultiBoggle::TwoHole two;
      mb.SolveTwoHole(&two);
      if (do_fill_one) {
        for (int a=0; a<26; a++) {
          if (a == 'q' - 'a') continue;
          tl[a].paths.clear();
          tl[a].extra_words.clear();
          br[a].paths.clear();
          br[a].extra_words.clear();

          mb.FillHoleTL(two, a, &(tl[a]));
          mb.FillHoleBR(two, a, &(br[a]));
        }
      }

      for (int a=0; a<26; a++) {
        if (a == 'q' - 'a') continue;
        for (int b=0; b<26; b++) {
          if (b == 'q' - 'a') continue;
          int score = 0;
          if (do_merge)
            score = mb.MergeBoards(two, a, tl[a], b, br[b]);
          multi_score += score;
          if (check_scores && score != expected[idx]) {
            SummarizeError(mb, x, y, a, b,
                           score, expected[idx], two, tl[a], br[b]);
            exit(1);
          }
          idx += 1;
        }
      }
    }
  }
  return multi_score;
}

int main(int argc, char** argv) {
  const char* board = "catdlinemaropets";
  const char* dict = "words";
  int num_boards = 25 * 25 * 25 * 25;

  // Do some rudimentary CL processing
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--compare")) {
      CheckScores = 1;
      OldScoring = 1;
    } else if (!strcmp(argv[i], "--nocompare")) {
      CheckScores = 0;
      OldScoring = 0;
    } else if (!strcmp(argv[i], "--dictionary")) {
      dict = argv[++i];
    } else if (!strcmp(argv[i], "--baseboard")) {
      board = argv[++i];
    }
  }

  Trie* t = Trie::CreateFromFile(dict);

  // Go through a bunch of boards normally first.
  vector<int> scores;
  double start, end, basic_elapsed=0.0;
  if (OldScoring) {
    Boggler bd(t);
    bd.ParseBoard(board);
    int basic_score = 0;
    start = secs();
    for (int x=0; x<26; x++) {
      if (x == 'q' - 'a') continue;
      bd.SetCell(1, 0, x);
      for (int y=0; y<26; y++) {
        if (y == 'q' - 'a') continue;
        bd.SetCell(2, 3, y);
        for (int a=0; a<26; a++) {
          if (a == 'q' - 'a') continue;
          bd.SetCell(0, 0, a);
          for (int b=0; b<26; b++) {
            if (b == 'q' - 'a') continue;
            bd.SetCell(3, 3, b);
            int score = bd.Score();
            basic_score += score;
            if (CheckScores) scores.push_back(score);
          }
        }
      }
    }
    end = secs();
    basic_elapsed = end - start;
    cout << "Basic scored " << basic_score << " pts in " << basic_elapsed
         << " s (" << bd.NumBoards() / basic_elapsed << " bds/s)" << endl;
    t->SetAllMarks(0);
  }

  // Then go through the same boards with the MultiBoggler
  start = secs();
  int multi_score = TestMultiBoggler(t, board, scores, true, true);
  double zero_hole = secs() - start;

  start = secs();
  TestMultiBoggler(t, board, vector<int>(), true, false);
  double one_hole = secs() - start;

  start = secs();
  TestMultiBoggler(t, board, vector<int>(), false, false);
  double two_hole  = secs() - start;

  cout << "TwoHole: " << two_hole << " = "
       << 100*two_hole / zero_hole << "%" << endl;
  cout << "OneHole: " << (one_hole - two_hole) << " = "
       << 100*(one_hole - two_hole) / zero_hole << "%" << endl;
  cout << "Merge: " << (zero_hole - one_hole) << " = "
       << 100*(zero_hole - one_hole) / zero_hole << "%" << endl;

  cout << "Multi scored " << multi_score << " pts in " << zero_hole << " s "
       << "(" << num_boards / zero_hole << " bds/s)" << endl;

  if (OldScoring)
    cout << "Speedup: " << (basic_elapsed / zero_hole) << "x" << endl;
}

double secs() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + t.tv_usec / 1000000.0;
}
