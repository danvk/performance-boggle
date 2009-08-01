// An interface for solving Boggle boards given a TrieT.
// This is designed to be extremely efficient.

#ifndef SSBOGGLER_H
#define SSBOGGLER_H

#include <limits.h>
#include <set>
#include <string>
#include "trie.h"

class BogglerBase {
 public:
  // Is this a valid boggle word? e.g. only has 'q' followed by 'u'.
  static bool IsBoggleWord(const char* word);

  // Returns true if it's a valid boggle word and converts "qu" -> 'q'
  static bool BogglifyWord(char* word);

  static const int kWordScores[];
};

template<class TrieT>
class GenericBoggler : public BogglerBase {
 public:
  // Does not assume ownership of the TrieT, though it must remain live for the
  // lifetime of the GenericBoggler. The trie will be modified by board scoring, and
  // must not be modified by any other GenericBoggler.
  GenericBoggler(TrieT* t) : dict_(t), runs_(0), num_boards_(0) {}

  // Parses a 16 character boards string like "abcdefghijklmnop"
  bool ParseBoard(const char* lets);
  std::string ToString() const;

  // Scores the current board. Stop scoring early (and return a lower bound on
  // the real score) if the score ever exceeds cutoff.
  int Score(unsigned int cutoff=UINT_MAX);

  // Shortcut for ParseBoard() + Score()
  int Score(const char* lets);

  // Find the actual words on the board.
  void FindWords(std::set<std::string>* out);

  // Set a cell on the current board. Must have 0 <= x, y < 4 and 0 <= c < 26.
  // These constraints are NOT checked.
  void SetCell(int x, int y, int c) { bd_[x*3 + y] = c; }
  int Cell(int x, int y) const { return bd_[x*3 + y]; }

  // Returns the total number of boards this GenericBoggler has evaluated.
  int NumBoards() { return num_boards_; }
  
  // Load a dictionary file, removing all non-Boggle words and converting "qu"
  // to 'q'.
  static TrieT* DictionaryFromFile(const char* dict_filename);

  // Callback is called whenever a word is found.
  // (TrieT* node, int x, int y, int len, int path) -> bool
  // Callback should return false to bail out of the search.
  template<class Callback>
  void DoSearch(Callback& c);

 private:
  template<class Callback>
  void DoDFS(int i, int len, TrieT* t, Callback& cb);

  struct SimpleScorer {
    bool operator()(TrieT* t, int x, int y, int len, int used);
    uintptr_t runs_;
    unsigned int score_;
  };

  struct WordFinder {
    bool operator()(TrieT* t, int x, int y, int len, int used);
    uintptr_t runs_;
    TrieT* base_;
    std::set<std::string>* words_;
  };

  TrieT* dict_;
  mutable unsigned int runs_;
  mutable unsigned int used_;
  mutable bool continue_;
  mutable int bd_[16];
  int num_boards_;
  unsigned int score_;

  static const int kCellUsed = -1;
};

// Convenience specialization of GenericBoggler
// Creates a SimpleTrie and then compacts it when loading from a dictionary.
class Boggler : public GenericBoggler<Trie> {
 public:
  Boggler(Trie* t) : GenericBoggler<Trie>(t) {}
  static Trie* DictionaryFromFile(const char* dict_filename);
};



// These functions depend on the TrieT template parameter, so they must be
// defined in the header file.
template<class TrieT>
bool GenericBoggler<TrieT>::SimpleScorer::operator()(TrieT* t, int x, int y,
                                                     int len, int used) {
  if (t->Mark() != runs_) {
    t->Mark(runs_);
    score_ += kWordScores[len];
  }
  return true;
}

// These functions depend on the TrieT template parameter, so they must be
// defined in the header file.
template<class TrieT>
bool GenericBoggler<TrieT>::WordFinder::operator()(TrieT* t, int x, int y,
                                                   int len, int used) {
  if (t->Mark() != runs_) {
    t->Mark(runs_);
    words_->insert(TrieUtils<TrieT>::ReverseLookup(base_, t));
  }
  return true;
}

template<class TrieT>
int GenericBoggler<TrieT>::Score(unsigned int cutoff) {
  runs_ += 1;
  SimpleScorer ss;
  ss.runs_ = runs_;
  ss.score_ = 0;
  DoSearch(ss);

  // Really should check for overflow here
  num_boards_++;
  return ss.score_;
}

template<class TrieT>
void GenericBoggler<TrieT>::FindWords(std::set<std::string>* out) {
  runs_ += 1;
  WordFinder wf;
  wf.runs_ = runs_;
  wf.words_ = out;
  wf.base_ = dict_;
  DoSearch(wf);

  // Really should check for overflow here
  num_boards_++;
}


// Board format: "bcdefghijklmnopq"
template<class TrieT>
bool GenericBoggler<TrieT>::ParseBoard(const char* lets) {
  for (int i=0; *lets; ++i)
    bd_[i] = (*lets++)-'a';
  return true;
}

template<class TrieT>
std::string GenericBoggler<TrieT>::ToString() const {
  std::string out;
  for (int i=0; i<9; i++)
    out += std::string(1, 'a' + bd_[i]);
  return out;
}

template<class TrieT>
int GenericBoggler<TrieT>::Score(char const* lets) {
  if (!ParseBoard(lets))
    return -1;
  return Score();
}

// Returns the score from this portion of the search
template<class TrieT>
template<class Callback>
void GenericBoggler<TrieT>::DoDFS(int i, int len, TrieT* t, Callback& cb) {
  int c = bd_[i];

  used_ ^= (1 << i);
  len += (c==kQ ? 2 : 1);
  if (t->IsWord()) {
    if (!cb(t, i%3, i/3, len, used_)) {
      continue_ = false;
    }
    if (!continue_) return;
  }

  // Could also get rid of any two dimensionality, but maybe GCC does that?
  int cc, idx;

  // To help the loop unrolling...
#define HIT(x,y) do { idx = (x) * 3 + y; \
                      if ((used_ & (1 << idx)) == 0) { \
                        cc = bd_[(x)*3+(y)]; \
                        if (t->StartsWord(cc)) { \
                          DoDFS((x)*3+(y), len, t->Descend(cc), cb); \
                          if (!continue_) break; \
                        } \
                      } \
		 } while(0)
#define HIT3x(x,y) HIT(x,y); HIT(x+1,y); HIT(x+2,y)
#define HIT3y(x,y) HIT(x,y); HIT(x,y+1); HIT(x,y+2)
#define HIT8(x,y) HIT3x(x-1,y-1); HIT(x-1,y); HIT(x+1,y); HIT3x(x-1,y+1)

  // x*3 + y
  switch (i) {
    case 0*3 + 0: HIT(0, 1); HIT(1, 0); HIT(1, 1); break;
    case 0*3 + 1: HIT(0, 0); HIT3y(1, 0); HIT(0, 2); break;
    case 0*3 + 2: HIT(0, 1); HIT(1, 1); HIT(1, 2); break;

    case 1*3 + 0: HIT(0, 0); HIT(2, 0); HIT3x(0, 1); break;
    case 1*3 + 1: HIT8(1, 1); break;
    case 1*3 + 2: HIT3x(0, 1); HIT(0, 2); HIT(2, 2); break;

    case 2*3 + 0: HIT(1, 0); HIT(1, 1); HIT(2, 1); break;
    case 2*3 + 1: HIT3y(1, 0); HIT(2, 0); HIT(2, 2); break;
    case 2*3 + 2: HIT(1, 2); HIT(1, 1); HIT(2, 1); break;
  }
  used_ ^= (1 << i);

#undef HIT
#undef HIT3x
#undef HIT3y
#undef HIT8
}

// Callback is called whenever a word is found.
// (TrieT* node, int x, int y, int len, int path) -> void
template<class TrieT>
template<class Callback>
void GenericBoggler<TrieT>::DoSearch(Callback& cb) {
  used_ = 0;
  continue_ = true;
  int i;
  for (i=0; i<9 && continue_; i++) {
    int c = bd_[i];
    if (dict_->StartsWord(c))
      DoDFS(i, 0, dict_->Descend(c), cb);
  }
}

template<class TrieT>
TrieT* GenericBoggler<TrieT>::DictionaryFromFile(const char* filename) {
  TrieT* t = new TrieT;
  char line[80];
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    // delete t;  // ARGH
    return NULL;
  }

  while (!feof(f) && fscanf(f, "%s", line)) {
    if (!BogglifyWord(line)) continue;
    t->AddWord(line);
  }
  fclose(f);

  return t;
}

#endif
