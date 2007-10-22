// Like the garden-variety Trie, but only allocates memory for pointers to
// children it actually has. It uses a bit set to keep track of which pointer
// corresponds to which letter. Because each PerfectTrie node has a variable
// number of children, they cannot be allocated with new. Memory is allocated
// maually and the node constructed with placement new.
// To avoid dealing with all this, just create a PerfectTrie from a Trie or
// directly from a dictionary file.

#ifndef PERFECT_TRIE_H__
#define PERFECT_TRIE_H__
#include "trie.h"
#include <string>

class PerfectTrie {
 public:
  PerfectTrie();
  ~PerfectTrie();
  static PerfectTrie* CompactTrie(const Trie& t);
  static PerfectTrie* CompactTrieBFS(const Trie& t);
  static PerfectTrie* CreateFromFile(const char* file);

  bool IsWord() const { return bits_ & (1 << 26); }
  bool StartsWord(int i) const { return bits_ & (1 << i); }
  int NumChildren() const { return CountBits(bits_ & ((1<<26) - 1)); }

  // Describe fanciness
  PerfectTrie* Descend(int i) const {
    unsigned v = bits_ & ((1 << i) - 1);
    return children_[CountBits(v)];
  }

  void Mark(unsigned mark) { mark_ = mark; }
  unsigned Mark() { return mark_; }

  bool IsWord(const char* wd) const;
  void SetIsWord() { bits_ |= (1 << 26); }

  size_t Size() const;
  size_t NumNodes() const;
  size_t MemoryUsage() const;
  void MemorySpan(caddr_t* low, caddr_t* high) const;

  void PrintTrie(std::string prefix = "") const;

 private:
  unsigned bits_;
  unsigned mark_;
  PerfectTrie* children_[0];
  // Taken from http://graphics.stanford.edu/~seander/bithacks.html
  static inline int CountBits(unsigned v) {
    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    v = ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
    return v;
  }

  // Pre-allocated buffer
  static bool is_allocated;
  static int bytes_allocated;
  static int bytes_used;
  static char* memory_pool;
  static void* GetMemory(size_t amount);
  static PerfectTrie* AllocatePT(const Trie& t);
};

// Some statistics:
// - 172203 words
// - 385272 nodes
// - 111355 childless nodes
// -  60848 words w/ children
// => 213069 non-word nodes
// - 4623260 bytes = 4.41M
// So,
// - 1541088 bytes = 33% is used by bits_
// - 1541088 bytes = 33% is used by mark_
// - 1541084 bytes = 33% is used by children_
// -  852276 bytes = 18.4% is in unused mark_ fields.
// -  445420 bytes =  9.6% is in unused bits_ fields.
// - pointer requires at least 21 bits
//
// If I allocated using a BFS strategy, I might reduce the memory gap between
// parent and child significantly, in which case I might delta-encode the
// addresses with 16-bit shorts. Right now the max gap is 576115 words.

#endif
