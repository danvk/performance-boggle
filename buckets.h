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

  static Trie* FromTrie(const Trie& source, const Bucketing& buckets);

  static Trie* FromWordList(const std::vector<std::string>& source,
                            const Bucketing& buckets);

  static double NumRepresentatives(const Boggler& board,
                                   const Bucketing& buckets);
  static int NumBuckets(const Bucketing& buckets);

  static std::string ToString(const Bucketing& buckets);

};

#endif
