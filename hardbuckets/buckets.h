// Tools for working with letter buckets.
#ifndef BUCKETS_H
#define BUCKETS_H

#include <map>
#include <vector>
#include <string>
#include <stdint.h>
#include "trie.h"
#include "boggler.h"

class Buckets {
 public:
  // ['a'-'z'] => ['a'-'z']
  typedef std::map<char, char> Bucketing;

  static bool ToMap(const std::vector<std::string>& buckets,
                    Bucketing* out);

  static bool Bucketize(char* word, const Bucketing& buckets);
  static bool Bucketize(std::string* word, const Bucketing& buckets);

  static double NumRepresentatives(const Boggler& board,
                                   const Bucketing& buckets);
  static int NumBuckets(const Bucketing& buckets);

  static std::string ToString(const Bucketing& buckets);

  // Note: These two Trie-construction methods use the mark to store the number
  // of words that have been collapsed onto a single node. This may or may not
  // be what the Boggler is expecting.
  template<class TrieT>
  static TrieT* FromTrie(const TrieT& source, const Bucketing& buckets);
  template<class TrieT>
  static TrieT* FromWordList(const std::vector<std::string>& source,
                             const Bucketing& buckets);

  // Compute an upper bound on the score of any particular representative of a
  // set of boards. Assumes that the Boggler has been created using a Trie from
  // one of hte above methods.
  template<class TrieT>
  static int UpperBound(GenericBoggler<TrieT>& b, unsigned int cutoff=UINT_MAX);

 private:
  template<class TrieT>
  struct UpperBounder {
    bool operator()(TrieT* t, int x, int y, int len, int used);
    std::vector<TrieT*> found;
    unsigned int score;
    unsigned int cutoff;
  };
};


template<class TrieT>
TrieT* Buckets::FromTrie(const TrieT& source, const Bucketing& buckets) {
  struct Collapser {
    Collapser(const Bucketing& b) : buckets(b) {}
    void Collapse(const TrieT& t, std::string sofar="") {
      if (t.IsWord()) {
        TrieT* t = out->AddWord(sofar.c_str());
        t->Mark(t->Mark() + 1);
      }
      for (int i = 0; i < 26; i++) {
        Bucketing::const_iterator it = buckets.find('a' + i);
        if (t.StartsWord(i) && it != buckets.end()) {
          Collapse(*t.Descend(i), sofar + std::string(1, it->second));
        }
      }
    }
    const Bucketing& buckets;
    TrieT* out;
  } collapse(buckets);
  collapse.out = new TrieT;
  collapse.Collapse(source);
  return collapse.out;
}

template<class TrieT>
TrieT* Buckets::FromWordList(const std::vector<std::string>& source,
                             const Bucketing& buckets) {
  TrieT* t = new TrieT;
  char buf[80];
  for (std::vector<std::string>::const_iterator it = source.begin();
       it != source.end(); ++it) {
    strcpy(buf, it->c_str());
    if (!Boggler::BogglifyWord(buf)) continue;
    if (!Bucketize(buf, buckets)) continue;
    TrieT* word = t->AddWord(buf);
    word->Mark(word->Mark() + 1);
  }

  return t;
}

template<class TrieT>
bool Buckets::UpperBounder<TrieT>::operator()(TrieT* t, int x, int y, int len, int used) {
  uintptr_t mark = t->Mark();
  if (mark) {
    score += BogglerBase::kWordScores[len];
    found.push_back(t);
    t->Mark(mark - 1);
  }
  return (score <= cutoff);
}

template<class TrieT>
int Buckets::UpperBound(GenericBoggler<TrieT>& b, unsigned int cutoff) {
  UpperBounder<TrieT> ub;
  ub.score = 0;
  ub.cutoff = cutoff;
  b.DoSearch(ub);

  // TODO: this could be a lot more efficient...
  for (size_t i = 0; i < ub.found.size(); i++) {
    TrieT* t = ub.found[i];
    t->Mark(t->Mark() + 1);
  }

  return ub.score;
}

#endif
