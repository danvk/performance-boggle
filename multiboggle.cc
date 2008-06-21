// Copyright 2008 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)

#include "multiboggle.h"
#include "boggler.h"
#include <iostream>

#define PRINT_WORDS 0

static int kCellEmpty = -1;
static const int kWordScores[] =
  //0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17
  { 0, 0, 0, 1, 1, 2, 3, 5, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };


MultiBoggle::MultiBoggle(Trie* t) : dict_(t), runs_(0), used_(0) {
  CalcMaxLenAddrs();
}

// callback: idx, len, cell, used_mask, trie node
template<class T>
void MultiBoggle::DoDFS(int i, int len, Trie* t, T& callback) {
  int c = bd_[i];

  if (c & 0x80000000) {
    // This is a path into an empty cell
    callback.EmptyCell(i, len, c, used_, t);
    return;
  }

  len += (c==kQ ? 2 : 1);
  if (t->IsWord()) {
    callback.FoundWord(i, len, c, used_, t);
  }

  // Could also get rid of any two dimensionality, but maybe GCC does that?
  used_ ^= (1 << i);
  int cc, idx;

  // To help the loop unrolling...
#define HIT(x,y) \
  do { idx = (x)*4 + (y); \
       if ((used_ & (1 << idx)) == 0) { \
         cc = bd_[idx]; \
         if (cc & 0x80000000) { \
           DoDFS(idx, len, t, callback); \
         } else if (t->StartsWord(cc)) { \
           DoDFS(idx, len, t->Descend(cc), callback); \
         } \
       } \
  } while(0)
#define HIT3x(x,y) HIT(x,y); HIT(x+1,y); HIT(x+2,y)
#define HIT3y(x,y) HIT(x,y); HIT(x,y+1); HIT(x,y+2)
#define HIT8(x,y) HIT3x(x-1,y-1); HIT(x-1,y); HIT(x+1,y); HIT3x(x-1,y+1)
  switch (i) {
    case 0*4 + 0: HIT(0, 1); HIT(1, 0); HIT(1, 1); break;
    case 0*4 + 1: HIT(0, 0); HIT3y(1, 0); HIT(0, 2); break;
    case 0*4 + 2: HIT(0, 1); HIT3y(1, 1); HIT(0, 3); break;
    case 0*4 + 3: HIT(0, 2); HIT(1, 2); HIT(1, 3); break;

    case 1*4 + 0: HIT(0, 0); HIT(2, 0); HIT3x(0, 1); break;
    case 1*4 + 1: HIT8(1, 1); break;
    case 1*4 + 2: HIT8(1, 2); break;
    case 1*4 + 3: HIT3x(0, 2); HIT(0, 3); HIT(2, 3); break;

    case 2*4 + 0: HIT(1, 0); HIT(3, 0); HIT3x(1, 1); break;
    case 2*4 + 1: HIT8(2, 1); break;
    case 2*4 + 2: HIT8(2, 2); break;
    case 2*4 + 3: HIT3x(1, 2); HIT(1, 3); HIT(3, 3); break;

    case 3*4 + 0: HIT(2, 0); HIT(2, 1); HIT(3, 1); break;
    case 3*4 + 1: HIT3y(2, 0); HIT(3, 0); HIT(3, 2); break;
    case 3*4 + 2: HIT3y(2, 1); HIT(3, 1); HIT(3, 3); break;
    case 3*4 + 3: HIT(2, 2); HIT(3, 2); HIT(2, 3); break;
  }
#undef HIT
#undef HIT3x
#undef HIT3y
#undef HIT8

  used_ ^= (1 << i);
}

// TwoHole: Partially solve a board with a hole in both the TL and BR corners.
// TODO: Ensure TwoHole paths are pre-allocated.
// TODO: Make trie_mark a property of the Trie.
struct TwoHoleFinder {
  void EmptyCell(int i, int len, int c, int used, Trie* t) {
    out_->paths[i & 1].push_back(MultiBoggle::Path());
    MultiBoggle::Path& p = out_->paths[i & 1].back();
    p.used_mask = used;
    p.len = len;
    p.node = t;
  }

  void FoundWord(int i, int len, int c, int used, Trie* t) {
    if (t->Mark() != runs_) {
      out_->score += kWordScores[len];
      t->Mark(runs_);
      if (PRINT_WORDS) {
        std::string out;
        TrieUtils<Trie>::ReverseLookup(dict_, t, &out);
        printf("%s\n", out.c_str());
      }
    }
  }

  unsigned int runs_;
  Trie* dict_;
  MultiBoggle::TwoHole* out_;
};

void MultiBoggle::SolveTwoHole(TwoHole* out) {
  bd_[0*4 + 0] = kCellEmpty;
  bd_[3*4 + 3] = kCellEmpty;
  runs_ += 1;
  out->score = 0;
  out->trie_mark = runs_;

  TwoHoleFinder th;
  th.runs_ = runs_;
  th.dict_ = dict_;
  th.out_ = out;

  for (int i=0; i<16; i++) {
    used_ = 0;  // TODO: why is this necessary?
    int c = bd_[i];
    if (c == kCellEmpty)
      DoDFS(i, 0, dict_, th);
    else if (dict_->StartsWord(c))
      DoDFS(i, 0, dict_->Descend(c), th);
  }
}

void MultiBoggle::PrintTwoHole(const TwoHole& two) {
  std::cout << "Mark: " << two.trie_mark << std::endl;
  std::cout << "Score: " << two.score << std::endl;
  struct WordFinder {
    WordFinder() { word[0] = '\0'; }
    void Find(Trie* t, unsigned mark, int len=0) {
      if (t->IsWord() && t->Mark() == mark)
        words.push_back(word);
      for (int i=0; i<26; i++) {
        if (t->StartsWord(i)) {
          word[len] = i + 'a';
          word[len + 1] = '\0';
          Find(t->Descend(i), mark, len+1);
        }
        word[len] = '\0';
      }
    }
    char word[17];
    std::vector<std::string> words;
  } finder;
  finder.Find(dict_, two.trie_mark);
  for (size_t i = 0; i < finder.words.size(); i++) {
    std::cout << "  " << finder.words[i] << std::endl;
  }

  for (int i = 0; i < 2; i++) {
    std::cout << "Paths into " << (i==0 ? "TL" : "BR")
              << ": " << two.paths[i].size() << std::endl;
    for (std::vector<Path>::const_iterator it = two.paths[i].begin();
         it != two.paths[i].end(); ++it) {
      std::cout << " " << it->ToString(dict_) << std::endl;
    }
  }
}


// OneHole: Take a partial TwoHole solution and fill in one of the corners.
struct OneHoleFinder {
  OneHoleFinder(const MultiBoggle::TwoHole& th,
                MultiBoggle::OneHole& oh) : th_(th), one_(oh) {}

  void EmptyCell(int i, int len, int c, int used, Trie* t) {
    one_.paths.push_back(MultiBoggle::Path());
    MultiBoggle::Path& p = one_.paths.back();
    p.used_mask = used;
    p.len = len;
    p.node = t;
  }

  void FoundWord(int i, int len, int c, int used, Trie* t) {
    if (t->Mark() == th_.trie_mark) return;
    if (t->Mark() & 0x80000000) return;
    one_.extra_words.push_back(t);
    one_.extra_score += kWordScores[len];
    t->Mark(t->Mark() | 0x80000000);
  }

  ~OneHoleFinder() {
    for (std::vector<Trie*>::const_iterator it = one_.extra_words.begin();
         it != one_.extra_words.end(); ++it) {
      (*it)->Mark((*it)->Mark() & 0x7fffffff);
    }
  }

  Trie* dict_;
  const MultiBoggle::TwoHole& th_;
  MultiBoggle::OneHole& one_;
};

void MultiBoggle::FillHoleTL(const TwoHole& two, int c, OneHole* out) {
  FillHole(two, 0, c, out);
}

void MultiBoggle::FillHoleBR(const TwoHole& two, int c, OneHole* out) {
  FillHole(two, 15, c, out);
}

void MultiBoggle::FillHole(const TwoHole& two, int idx, int c, OneHole* out) {
  bd_[idx] = c;
  bd_[15-idx] = kCellEmpty;

  out->extra_score = 0;
  OneHoleFinder oh(two, *out);
  oh.dict_ = dict_;

  // Continue each possible path
  for (std::vector<Path>::const_iterator it = two.paths[idx & 1].begin();
       it != two.paths[idx & 1].end(); ++it) {
    const Path& p = *it;
    if (p.node->StartsWord(c)) {
      used_ = p.used_mask;
      DoDFS(idx, p.len, p.node->Descend(c), oh);
    }
  }
}

void MultiBoggle::PrintOneHole(const TwoHole& two, const OneHole& one, int x) {
  std::cout << "Base score: " << two.score << std::endl;
  std::cout << " + extra: " << one.extra_score << std::endl;
  std::cout << " = " << two.score + one.extra_score << std::endl;
  std::cout << "Extra words: " << one.extra_words.size() << std::endl;
  for (std::vector<Trie*>::const_iterator it = one.extra_words.begin();
       it != one.extra_words.end(); ++it) {
    std::string out;
    TrieUtils<Trie>::ReverseLookup(dict_, *it, &out);
    printf("  %s\n", out.c_str());
  }
  std::cout << "Remaining paths: " << one.paths.size() << std::endl;
  for (std::vector<Path>::const_iterator it = one.paths.begin();
       it != one.paths.end(); ++it) {
    std::cout << "  " << it->ToString(dict_) << std::endl;
  }
}


// Merge: Take OneHole partial solutions for opposite corners and calculate a
// score for the full board VERY QUICKLY.
struct NoHoleFinder {
  NoHoleFinder(int& s, int mark) : score(s), trie_mark(mark) {}
  void EmptyCell(int i, int len, int c, int used, Trie* t) {
    std::cerr << "Found an empty cell with no hole finder!" << std::endl;
  }

  void FoundWord(int i, int len, int c, int used, Trie* t) {
    if (t->Mark() == trie_mark || t->Mark() & 0x80000000) return;
    score += kWordScores[len];
    t->Mark(t->Mark() | 0x80000000);
    words.push_back(t);
  }

  std::vector<Trie*> words;
  int& score;
  unsigned trie_mark;
};

int MultiBoggle::MergeBoards(TwoHole& bd,
                             int c1, OneHole& bd1,
                             int c2, OneHole& bd2) {
  int score = bd.score + bd1.extra_score + bd2.extra_score;
  bd_[0] = c1;
  bd_[15] = c2;

  // Step through the word lists in parallel to avoid double-counting dupes.
  // TODO: This could probably be improved...
  std::vector<Trie*>::iterator word1 = bd1.extra_words.begin(),
                               end1 = bd1.extra_words.end();
  std::vector<Trie*>::iterator word2 = bd2.extra_words.begin(),
                               end2 = bd2.extra_words.end();
  for (; word1 != end1; ++word1)
    (*word1)->MarkHigh();

  for (; word2 != end2; ++word2) {
    if ((*word2)->Mark() & 0x80000000) {
      score -= kWordScores[ReconstructLength(*word2)];
    } else {
      (*word2)->MarkHigh();
    }
  }

  // Now follow the paths through both corners.
  NoHoleFinder finder(score, bd.trie_mark);

  // Continue each possible path
  for (std::vector<Path>::const_iterator it = bd1.paths.begin();
       it != bd1.paths.end(); ++it) {
    const Path& p = *it;
    if (p.node->StartsWord(c2)) {
      used_ = p.used_mask;
      DoDFS(15, p.len, p.node->Descend(c2), finder);
    }
  }
  for (std::vector<Path>::const_iterator it = bd2.paths.begin();
       it != bd2.paths.end(); ++it) {
    const Path& p = *it;
    if (p.node->StartsWord(c1)) {
      used_ = p.used_mask;
      DoDFS(0, p.len, p.node->Descend(c1), finder);
    }
  }

  // .. and clear all the extra marks.
  for (word1 = bd1.extra_words.begin(); word1 != end1; ++word1)
    (*word1)->Mark((*word1)->Mark() & 0x7fffffff);
  for (word2 = bd2.extra_words.begin(); word2 != end2; ++word2)
    (*word2)->Mark((*word2)->Mark() & 0x7fffffff);
  for (word1 = finder.words.begin(); word1 != finder.words.end(); ++word1) {
    (*word1)->Mark((*word1)->Mark() & 0x7fffffff);
  }

  return score;
}


// Some generic utilities

bool MultiBoggle::ParseBoard(const char* lets) {
  for (int i=0; *lets; ++i)
    bd_[i] = (*lets++)-'a';
  return true;
}

// TODO: Verify that the Trie had a BFS structure
void MultiBoggle::CalcMaxLenAddrs() {
  for (int i=0; i<17; i++) max_len_addr[i] = NULL;
  struct Searcher {
    void FindMax(Trie* t, int len=0) {
      if (t->IsWord() && t > max[len]) max[len] = t;
      for (int i=0; i<26; i++)
        if (t->StartsWord(i)) FindMax(t->Descend(i), len+1);
    }
    Trie** max;
  } search;
  search.max = max_len_addr;
  search.FindMax(dict_);
}

// Figure out the length of the word represented by node.
// Does not consider q's.
// This does an abbreviated binary search, but is probably called infrequently
// enough that it's an irrelevant optimization over a simple for loop.
int MultiBoggle::ReconstructLength(const Trie* node) const {
  if (node <= max_len_addr[4]) {
    if (node <= max_len_addr[3]) return 3;
    return 4;
  } else {
    if (node <= max_len_addr[5]) return 5;
    if (node <= max_len_addr[6]) return 6;
    if (node <= max_len_addr[7]) return 7;
    for (int len = 8; len < 17; len++)
      if (node <= max_len_addr[len])
        return len;
  }
  std::cerr << "Couldn't figure out length of " << node << std::endl;
  return 0;
}

std::string MultiBoggle::ToString() const {
  std::string out;
  for (int i=0; i<16; i++)
    out += std::string(1, 'a' + bd_[i]);
  return out;
}

MultiBoggle::~MultiBoggle() {
}

std::string MultiBoggle::Path::ToString(Trie* dict) const {
  char buf[20];
  std::string ret;
  TrieUtils<Trie>::ReverseLookup(dict, node, &ret);
  
  sprintf(buf, " (%04x) %d", used_mask, len);
  return ret + buf;
}
