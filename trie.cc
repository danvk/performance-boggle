#include "trie.h"
#include <stdio.h>
#include <string.h>

inline int idx(char x) { return x - 'a'; }

void Trie::AddWord(const char* wd) {
  if (!wd) return;
  if (!*wd) {
    SetIsWord();
    return;
  }
  SetHasChildren();
  int c = idx(*wd);
  if (!StartsWord(c))
    children_[c] = new Trie;
  if (c!=kQ)
    Descend(c)->AddWord(wd+1);
  else
    Descend(c)->AddWord(wd+2);
}

bool IsBoggleWord(const char* wd) {
  int size = strlen(wd);
  if (size < 3 || size > 17) return false;
  for (int i=0; i<size; ++i) {
    int c = idx(wd[i]);
    if (c<0 || c>=kNumLetters) return false;
    if (c==kQ && (i+1 >= size || idx(wd[1+i]) != idx('u'))) return false;
  }
  return true;
}

bool Trie::LoadFile(const char* filename) {
  char line[80];
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    return false;
  }

  while (!feof(f) && fscanf(f,"%s",line)) {
    if (!IsBoggleWord(line)) continue;
    AddWord(line);
  }
  fclose(f);
  return true;
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

bool Trie::ReverseLookup(const Trie* child, std::string* out) {
  if (this==child) return true;
  for (int i=0; i<kNumLetters; i++) {
    if (StartsWord(i) && Descend(i)->ReverseLookup(child, out)) {
      *out = std::string(1,'a'+i) + *out;
      return true;
    }
  }
  return false;
}

std::string Trie::ReverseLookup(const Trie* child) {
  std::string s;
  ReverseLookup(child, &s);
  return s;
}

size_t Trie::MemoryUsage() const {
  size_t size = sizeof(*this);
  for (int i = 0; i < kNumLetters; i++) {
    if (StartsWord(i)) size += Descend(i)->MemoryUsage();
  }
  return size;
}

void Trie::MemorySpan(caddr_t* low, caddr_t* high) const {
  if ((unsigned)this & 0x80000000) {
    // Ignore Tries allocated on the stack
    *low = (caddr_t)-1;
    *high = (caddr_t)0;
  } else {
    *low = (caddr_t)this;
    *high = (caddr_t)this; *high += sizeof(*this);
  }
  for (int i=0; i<kNumLetters; i++) {
    if (StartsWord(i)) {
      caddr_t cl, ch;
      Descend(i)->MemorySpan(&cl, &ch);
      if (cl < *low) *low = cl;
      if (ch > *high) *high = ch;
    }
  }
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
  mark_ = 0;
}
