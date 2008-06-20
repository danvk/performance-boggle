// Copyright 2008 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)

#include "buckets.h"
#include <iostream>
using std::vector;
using std::map;

bool Buckets::ToMap(const std::vector<std::string>& buckets,
                    Bucketing* out) {
  out->clear();
  for (size_t i = 0; i < buckets.size(); i++) {
    for (size_t j = 0; j < buckets[i].size(); j++) {
      (*out)[buckets[i][j]] = 'a' + i;
    }
  }
  return true;
}

bool Buckets::Bucketize(char* word, const Bucketing& buckets) {
  for (char* c = word; *c; c++) {
    Bucketing::const_iterator it = buckets.find(*c);
    if (it == buckets.end()) return false;
    *c = it->second;
  }
  return true;
}

Trie* Buckets::FromTrie(const Trie& source, const Bucketing& buckets) {
  struct Collapser {
    Collapser(const Bucketing& b) : buckets(b) {}
    void Collapse(const Trie& t, std::string sofar="") {
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
    Trie::SimpleTrie* out;
  } collapse(buckets);
  collapse.out = new Trie::SimpleTrie;
  collapse.Collapse(source);

  Trie* pt = Trie::CompactTrie(*collapse.out);
  delete collapse.out;
  return pt;
}

Trie* Buckets::FromWordList(const std::vector<std::string>& source,
                            const Bucketing& buckets) {
  Trie::SimpleTrie* st = new Trie::SimpleTrie;
  char buf[80];
  for (std::vector<std::string>::const_iterator it = source.begin();
       it != source.end(); ++it) {
    strcpy(buf, it->c_str());
    if (!Boggler::BogglifyWord(buf)) continue;
    if (!Bucketize(buf, buckets)) continue;
    st->AddWord(buf);
  }

  Trie* pt = Trie::CompactTrie(*st);
  delete st;
  return pt;
}

double Buckets::NumRepresentatives(const Boggler& board,
                                     const Bucketing& buckets) {
  double num_reps = 1.0;

  // TODO: Store this information in the Bucketing.
  map<char, int> counts;
  for (Bucketing::const_iterator it = buckets.begin();
       it != buckets.end(); ++it) {
    counts[it->second] += 1;
  }

  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      char c = 'a' + board.Cell(x, y);
      map<char, int>::const_iterator it = counts.find(c);
      if (it == counts.end()) return 0;
      num_reps *= it->second;
    }
  }
  return num_reps;
}

int Buckets::NumBuckets(const Bucketing& buckets) {
  map<char, int> counts;
  for (Bucketing::const_iterator it = buckets.begin();
       it != buckets.end(); ++it) {
    counts[it->second] += 1;
  }
  return counts.size();
}

std::string Buckets::ToString(const Bucketing& buckets) {
  map<char, std::string> rev;
  for (map<char,char>::const_iterator it = buckets.begin();
       it != buckets.end(); ++it) {
    rev[it->second] += it->first;
  }

  std::string ret;
  for (map<char,std::string>::const_iterator it = rev.begin();
       it != rev.end(); ++it) {
    ret += it->second + " ";
  }
  return ret;
}
