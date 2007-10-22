// A very simple, clean Trie library for C++
// This is designed to be used to bootstrap a PerfectTrie and probably shouldn't
// be used directly (it's very wasteful of memory).

#ifndef TRIE_H
#define TRIE_H

const int kNumLetters = 26;
const int kQ = 'q'-'a';

class Trie {
 public:
  Trie();
  ~Trie();

  bool StartsWord(int i) const { return children_[i]; }
  Trie* Descend(int i) const { return children_[i]; }

  bool IsWord() const { return is_word_; }
  void SetIsWord() { is_word_ = true; }

  void AddWord(const char* wd);

 private:
  bool is_word_;
  Trie* children_[kNumLetters];
};

#endif
