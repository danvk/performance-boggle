#include "trie.h"

#include <string>
#include <fstream>
#include <iostream>
using namespace std;

inline int idx(char x) { return x - 'a'; }

void Trie::AddWord(const char* wd) {
  if (!wd) return;
  if (!*wd) {
    is_word_ = true;
    return;
  }
  has_children_ = true;
  int c = idx(*wd);
  if (!StartsWord(c))
    children_[c] = new Trie;
  if (c!=kQ)
    Descend(c)->AddWord(wd+1);
  else
    Descend(c)->AddWord(wd+2);
}

bool IsBoggleWord(const string& wd) {
  if (wd.size()<3 || wd.size()>17) return false;
  for (unsigned i=0; i<wd.size(); ++i) {
    int c = idx(wd[i]);
    if (c<0 || c>=kNumLetters) return false;
    if (c==kQ && (i+1 >= wd.size() || idx(wd[1+i]) != idx('u'))) return false;
  }
  return true;
}

bool Trie::LoadFile(const char* filename) {
  ifstream file(filename);
  if (!file) {
    cerr << "Couldn't open " << filename << endl;
    return false;
  }

  string line;
  while (file >> line) {
    if (!IsBoggleWord(line)) continue;
    AddWord(line.c_str());
  }
  return true;
}

bool Trie::LoadVector(const vector<string>& v) {
  for (vector<string>::const_iterator it = v.begin(); it != v.end(); ++it) {
    if (!IsBoggleWord(*it)) continue;
    AddWord(it->c_str());
  }
  return true;
}

bool Trie::ReverseLookup(const Trie* child, string* out) {
  if (this==child) return true;
  for (int i=0; i<kNumLetters; i++) {
    if (StartsWord(i) && Descend(i)->ReverseLookup(child, out)) {
      *out = string(1,'a'+i) + *out;
      return true;
    }
  }
  return false;
}

string Trie::ReverseLookup(const Trie* child) {
  string s;
  ReverseLookup(child, &s);
  return s;
}

bool Trie::IsWord(const char* wd) const {
  if (!wd) return false;
  if (!*wd) return IsWord();

  int c = idx(*wd);
  if (c<0 || c>=kNumLetters) return false;

  if (StartsWord(c)) {
    if (c==kQ && wd[1] == 'u')
      return Descend(c)->IsWord(wd+2);
    return Descend(c)->IsWord(wd+1);
  }
  return false;
}

unsigned int Trie::Size() const {
  unsigned int size = 0;
  if (IsWord()) size++;
  for (int i=0; i<26; i++) {
    if (StartsWord(i)) size += Descend(i)->Size();
  }
  return size;
}

Trie::~Trie() {
  for (int i=0; i<26; i++) {
    if (children_[i]) delete children_[i];
  }
}

// Initially, this node is empty
Trie::Trie() {
  for (int i=0; i<kNumLetters; i++) {
    children_[i] = NULL;
  }
  is_word_ = false;
  has_children_ = false;
  mark_ = 0;
}
