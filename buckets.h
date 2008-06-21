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

  template<class TrieT>
  static TrieT* FromTrie(const TrieT& source, const Bucketing& buckets);

  template<class TrieT>
  static TrieT* FromWordList(const std::vector<std::string>& source,
                             const Bucketing& buckets);

  static double NumRepresentatives(const Boggler& board,
                                   const Bucketing& buckets);
  static int NumBuckets(const Bucketing& buckets);

  static std::string ToString(const Bucketing& buckets);
};

template<class TrieT>
TrieT* Buckets::FromTrie(const TrieT& source, const Bucketing& buckets) {
  struct Collapser {
    Collapser(const Bucketing& b) : buckets(b) {}
    void Collapse(const TrieT& t, std::string sofar="") {
      if (t.IsWord()) {
        out->AddWord(sofar.c_str());
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
    t->AddWord(buf);
  }

  return t;
}

#endif
