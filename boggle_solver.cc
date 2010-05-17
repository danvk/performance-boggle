#include "boggle_solver.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "3x3/boggler.h"
#include "3x4/boggler.h"
#include "4x4/boggler.h"
#include "trie.h"

const int BoggleSolver::kWordScores[] =
      //0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17
      { 0, 0, 0, 1, 1, 2, 3, 5, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };

BoggleSolver::BoggleSolver() : runs_(0), num_boards_(0) {}
BoggleSolver::~BoggleSolver() {}

BoggleSolver* BoggleSolver::Create(int size, const char* dictionary_file) {
  SimpleTrie* t = Boggler::DictionaryFromFile(dictionary_file);
  if (!t) return NULL;

  BoggleSolver* solver = NULL;
  switch (size) {
    case 33: solver = new Boggler3(t); break;
    case 34: solver = new Boggler34(t); break;
    case 44: solver = new Boggler(t); break;
    default:
      fprintf(stderr, "Unknown board size: %d\n", size);
      return NULL;
  }
  return solver;
}

/* static */ bool BoggleSolver::IsBoggleWord(const char* wd) {
  int size = strlen(wd);
  if (size < 3 || size > 17) return false;
  for (int i=0; i<size; ++i) {
    int c = wd[i];
    if (c<'a' || c>'z') return false;
    if (c=='q' && (i+1 >= size || wd[1+i] != 'u')) return false;
  }
  return true;
}

/* static */ bool BoggleSolver::BogglifyWord(char* word) {
  if (!IsBoggleWord(word)) return false;
  int src, dst;
  for (src=0, dst=0; word[src]; src++, dst++) {
    word[dst] = word[src];
    if (word[src] == 'q') src += 1;
  }
  word[dst] = word[src];
  return true;
}

bool BoggleSolver::ParseBoard(const char* bd) {
  unsigned int expected_len = Width() * Height();
  if (strlen(bd) != expected_len) {
    fprintf(stderr,
            "Board strings must contain %d characters, got %zu ('%s')\n",
            expected_len, strlen(bd), bd);
    return false;
  }

  for (unsigned int i = 0; i < expected_len; i++) {
    if (bd[i] >= 'A' && bd[i] <= 'Z') {
      fprintf(stderr, "Found uppercase letter '%c'\n", bd[i]);
      return false;
    } else if (bd[i] < 'a' || bd[i] > 'z') {
      fprintf(stderr, "Found unexpected letter: '%c'\n", bd[i]);
      return false;
    }
    SetCell(i%Width(), i/Width(), bd[i] - 'a');
  }
  return true;
}

// /* static */ Trie* BoggleSolver::DictionaryFromFile(const char* dict_filename) {
//   SimpleTrie* st = GenericBoggler<SimpleTrie>::DictionaryFromFile(dict_filename);
//   Trie* t = Trie::CompactTrie(*st);
//   delete st;
//   return t;
// }

int BoggleSolver::Score() {
  // Really should check for overflow here
  runs_ += 1;
  int score = InternalScore();
  num_boards_ += 1;
  return score;
}

std::string BoggleSolver::ToString() const {
  std::string out;
  int w = Width();
  int h = Height();
  for (int y=0; y<h; y++)
    for (int x=0; x<w; x++)
      out += std::string(1, 'a' + Cell(x, y));
  return out;
}

int BoggleSolver::Score(char const* lets) {
  if (!ParseBoard(lets))
    return -1;
  return Score();
}
