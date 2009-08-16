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

bool Buckets::Bucketize(std::string* word, const Bucketing& buckets) {
  return Bucketize(&*word->begin(), buckets);
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
