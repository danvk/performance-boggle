// Test cases for buckets

#include <assert.h>
#include <string>
#include <vector>
#include "buckets.h"
#include "trie.h"

int main(int argc, char** argv) {
  std::vector<std::string> b;
  b.push_back("abcd");   // a
  b.push_back("efgh");   // b
  b.push_back("ijkl");   // c
  b.push_back("mnop");   // d
  b.push_back("qrst");   // e
  b.push_back("uvwxy");  // f
  b.push_back("z");      // g

  Buckets::Bucketing buckets;
  bool ret = Buckets::ToMap(b, &buckets);
  assert(ret);
  assert(7 == Buckets::NumBuckets(buckets));

  std::vector<std::string> words;
  words.push_back("sea");
  words.push_back("tea");
  SimpleTrie* t = Buckets::FromWordList<SimpleTrie>(words, buckets);
  assert(1 == TrieUtils<SimpleTrie>::Size(t));
  SimpleTrie* wd = TrieUtils<SimpleTrie>::FindWord(t, "eba");
  assert(2 == wd->Mark());

  GenericBoggler<SimpleTrie> bd(t);
  for (int i = 0; i < 100; i++) {
    bd.ParseBoard("ebagbaggffffgggg");
    assert(2 == Buckets::UpperBound(bd));
  }

  printf("%s: all tests passed\n", argv[0]);
}
