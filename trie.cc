#include "trie.h"
#include <stdlib.h>

inline int idx(char x) { return x - 'a'; }

void Trie::AddWord(const char* wd) {
  if (!wd) return;
  if (!*wd) {
    SetIsWord();
    return;
  }
  int c = idx(*wd);
  if (!StartsWord(c))
    children_[c] = new Trie;
  if (c!=kQ)
    Descend(c)->AddWord(wd+1);
  else
    Descend(c)->AddWord(wd+2);
}

Trie::~Trie() {
  for (int i=0; i<26; i++) {
    if (children_[i]) delete children_[i];
  }
}

// Initially, this node is empty
Trie::Trie() {
  for (int i=0; i<kNumLetters; i++)
    children_[i] = NULL;
  is_word_ = false;
}
