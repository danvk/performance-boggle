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

const int kNumLetters = 26;
const int kQ = 'q'-'a';

class Trie {
 public:
  Trie();
  ~Trie();

  bool IsWord() const { return is_word_; }
  bool StartsWord(int i) const { return children_[i]; }
  Trie* Descend(int i) const { return children_[i]; }

  void Mark(unsigned mark) { mark_ = mark; }
  unsigned Mark() const { return mark_; }
  void MarkHigh() { mark_ |= 0x80000000; }

  void AddWord(const char* wd);
  bool LoadFile(const char* filename);

  bool StartsAnyWord() const { return has_children_; }
  bool IsWord(const char* wd) const;
  unsigned int Size() const;

 private:
  Trie* children_[kNumLetters];
  bool is_word_;
  bool has_children_;
  unsigned mark_;
};

#endif
