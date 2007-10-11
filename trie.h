// Very simple Trie library for C++
// Most Trie implementations use a linked list to represent children within a
// node, but this uses a 26-slot array of pointers. This is extremely fast (a
// descend is just a pointer dereference) but also very wasteful of memory. The
// standard linux dictionary can take over 100MB of RAM to store this way.
//
// This Trie class is designed for Boggle board-solving insofar as it collapses
// any 'qu' to a simple 'q' and rejects a word containing 'q[^u]'.

#ifndef TRIE_H
#define TRIE_H
#include <string>

const int kNumLetters = 26;
const int kQ = 'q'-'a';

class Trie {
 public:
  Trie();
  ~Trie();
  enum Masks { WORD=0x80000000, PARENT=0x40000000, MARK=0x3FFFFFFF };

  bool IsWord() const { return mark_ & WORD; }
  bool StartsWord(int i) const { return children_[i]; }
  Trie* Descend(int i) const { return children_[i]; }
  bool StartsAnyWord() const { return mark_ & PARENT; }

  void Mark(unsigned mark) { mark_ &= ~MARK; mark_ |= (mark & MARK); }
  unsigned Mark() const { return mark_ & MARK; }
  void MarkHigh() { mark_ |= (MARK - (MARK >> 1)); }

  void AddWord(const char* wd);
  bool LoadFile(const char* filename);

  bool IsWord(const char* wd) const;
  unsigned int Size() const;

  void SetIsWord()      { mark_ |= WORD; }
  void SetHasChildren() { mark_ |= PARENT; }

  bool ReverseLookup(const Trie* child, std::string* out);
  std::string ReverseLookup(const Trie* child);

 private:
  Trie* children_[kNumLetters];
  unsigned mark_;
};

// NOTES:
//   - Adding an "is_word_" field and avoid bit arithmetic is neutral.
//   - The whole "StartsAnyWord" business is neutral.

#endif
