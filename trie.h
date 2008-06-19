// Like the garden-variety Trie, but only allocates memory for pointers to
// children it actually has. It uses a bit set to keep track of which pointer
// corresponds to which letter. Because each Trie node has a variable
// number of children, they cannot be allocated with new. Memory is allocated
// maually and the node constructed with placement new.
// To avoid dealing with all this, just create a Trie from a Trie or
// directly from a dictionary file.

#ifndef PERFECT_TRIE_H__
#define PERFECT_TRIE_H__
#include <string>
#include <vector>
#include <sys/types.h>
#include <stdint.h>

const int kNumLetters = 26;
const int kQ = 'q' - 'a';

class Trie {
 public:
  Trie();
  
  // Call "trie->Delete()" rather than "delete trie" on the root node of a
  // Trie. I'm sorry, it just needs to be this way.
  // ~Trie();
  void Delete();

  // Fast operations
  bool IsWord() const { return bits_ & (1 << 26); }
  bool StartsWord(int i) const { return bits_ & (1 << i); }
  int NumChildren() const { return CountBits(bits_ & ((1<<26) - 1)); }
  Trie* Descend(int i) const {
    uint32_t v = bits_ & ((1 << i) - 1);
    return (Trie*)data_[(IsWord() ? 1 : 0) + CountBits(v)];
  }

  // NOTE: These should NEVER be called unless this Node is already a word.
  void Mark(uintptr_t mark) { data_[0] = mark; }
  uintptr_t Mark() { return data_[0]; }
  void MarkHigh() { data_[0] |= 0x80000000; }

  bool IsWord(const char* wd) const;
  void SetIsWord(bool w) { bits_ &= ~(1<<26); bits_ |= (w << 26); }


  // Trie-building methods (slow)
  class SimpleTrie;
  static Trie* CompactTrie(const SimpleTrie& t);
  static Trie* CreateFromFile(const char* file, bool strip_qs=true);
  Trie* CollapseBuckets(const std::vector<std::string>& buckets) const;

  // Analysis (slow)
  size_t Size() const;
  size_t NumNodes() const;
  size_t MemoryUsage() const;
  bool ReverseLookup(const Trie* child, std::string* out) const;
  std::string ReverseLookup(const Trie* child) const {
    std::string out;
    ReverseLookup(child, &out);
    return out;
  }
  void SetAllMarks(unsigned mark);

  void PrintTrie(std::string prefix = "") const;

 private:
  uint32_t bits_;      // used to determine word-ness and children.

  // I am unable to express this layout within C++'s type system.
  // If this is a word, data_[0] is a marks field and data_[1..] are children.
  // Otherwise, data_[0..] are all children.
  // This representation allows the data to be referenced branch-free.
  uintptr_t data_[0];

  // Taken from http://graphics.stanford.edu/~seander/bithacks.html
  static inline int CountBits(uint32_t v) {
    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    v = (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
    return v;
  }

  ~Trie();

 public:
  // Plain vanilla trie used for bootstrapping the Trie.
  class SimpleTrie {
   public:
    SimpleTrie();
    ~SimpleTrie();

    bool StartsWord(int i) const { return children_[i]; }
    SimpleTrie* Descend(int i) const { return children_[i]; }

    bool IsWord() const { return is_word_; }
    void SetIsWord() { is_word_ = true; }

    void AddWord(const char* wd);

   private:
    bool is_word_;
    SimpleTrie* children_[26];
  };
};

// Some statistics:
// - 172203 words
// - 385272 nodes
// - 111355 childless nodes
// -  60848 words w/ children
// => 213069 non-word nodes
// So,
// - 1541088 bytes = 41% is used by bits_
// -  688812 bytes = 18% is used by mark_
// - 1541084 bytes = 41% is used by children_
//
// -  445420 bytes =  9.6% is in unused bits_ fields.
//
// Eliminated unused mark_ fields => 3,770,984 bytes.

#endif
