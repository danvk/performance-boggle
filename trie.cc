#include "trie.h"
#include <queue>
#include <utility>
#include <map>

Trie::Trie() : bits_(0) {}

int NumChildren(const Trie::SimpleTrie& t) {
  int num_children = 0;
  for (int i=0; i<26; i++) {
    if (t.StartsWord(i)) num_children += 1;
  }
  return num_children;
}

// Memory model: The memory used by an entire Trie is owned by the root node of
// that Trie. Since it would be expensive for each Trie node to remember
// whether it's the root, we maintain a list of all root nodes and the memory
// allocated for them.
std::map<Trie*, char*> root_tries;

// This is messy -- use placement new to get a Trie of the desired size.
Trie* AllocatePT(const Trie::SimpleTrie& t, void* where, int* bytes_used) {
  int mem_size = sizeof(Trie) + (t.IsWord() ? sizeof(unsigned) : 0)
		 + ::NumChildren(t) * sizeof(Trie*);
  *bytes_used += mem_size;
  return new(where) Trie;
}

// Allocate in BFS order to minimize parent/child spacing in memory.
struct WorkItem {
  const Trie::SimpleTrie& t;
  Trie* pt;
  int depth;
  WorkItem(const Trie::SimpleTrie& tr,
	   Trie* ptr, int d) : t(tr), pt(ptr), depth(d) {}
};
Trie* Trie::CompactTrie(const SimpleTrie& t) {
  char* root_mem = new char[256];  // TODO: check if this leaks.
  char* raw_bytes = new char[5 << 20];  // should always be enough
  int bytes_used = 0;

  std::queue<WorkItem> todo;
  Trie* root = AllocatePT(t, root_mem, &bytes_used);
  bytes_used = 0;
  root_tries.insert(std::make_pair(root, raw_bytes));
  todo.push(WorkItem(t, root, 1));

  while (!todo.empty()) {
    WorkItem cur = todo.front();
    todo.pop();

    // Construct the Trie in the Trie
    const SimpleTrie& t = cur.t;
    Trie* pt = cur.pt;
    pt->SetIsWord(t.IsWord());
    if (t.IsWord()) pt->Mark(0);
    int num_written = 0;
    int off = t.IsWord() ? 1 : 0;
    for (int i=0; i<kNumLetters; i++) {
      if (t.StartsWord(i)) {
	pt->bits_ |= (1 << i);
	pt->data_[off + num_written] =
          (unsigned)AllocatePT(*t.Descend(i), raw_bytes + bytes_used, &bytes_used);
	todo.push(WorkItem(*t.Descend(i), pt->Descend(i), cur.depth + 1));
	num_written += 1;
      }
    }
  }
  return root;
}

// Free memory associated with this Trie, if it owns any memory.
Trie::~Trie() {
  std::map<Trie*, char*>::iterator it = root_tries.find(this);
  if (it != root_tries.end()) {
    delete[] it->second;
    root_tries.erase(it);
  }
}

// Utility routines that operate on the root Trie node.
bool Trie::IsWord(const char* wd) const {
  if (!wd) return false;
  if (!*wd) return IsWord();

  int c = *wd - 'a';
  if (c<0 || c>=kNumLetters) return false;

  if (StartsWord(c)) {
    if (c==kQ && wd[1] == 'u')
      return Descend(c)->IsWord(wd+2);
    return Descend(c)->IsWord(wd+1);
  }
  return false;
}

size_t Trie::Size() const {
  size_t size = 0;
  if (IsWord()) size++;
  for (int i=0; i<26; i++) {
    if (StartsWord(i)) size += Descend(i)->Size();
  }
  return size;
}

size_t Trie::MemoryUsage() const {
  size_t size = sizeof(*this);
  if (IsWord()) size += sizeof(unsigned);
  size += sizeof(Trie*) * NumChildren();
  for (int i = 0; i < 26; i++) {
    if (StartsWord(i))
      size += Descend(i)->MemoryUsage();
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

void Trie::PrintTrie(std::string prefix) const {
  if (IsWord()) printf("+"); else printf("-");
  printf("(%08X) ", bits_);
  printf("%s\n", prefix.c_str());
  for (int i=0; i<26; i++) {
    if (StartsWord(i))
      Descend(i)->PrintTrie(prefix + std::string(1, 'a' + i));
  }
}

bool Trie::ReverseLookup(const Trie* child, std::string* out) const {
  if (this==child) return true;
  for (int i=0; i<kNumLetters; i++) {
    if (StartsWord(i) && Descend(i)->ReverseLookup(child, out)) {
      *out = std::string(1,'a'+i) + *out;
      return true;
    }
  }
  return false;
}

void Trie::SetAllMarks(unsigned mark) {
  if (IsWord()) Mark(mark);
  for (int i=0; i<kNumLetters; i++) {
    if (StartsWord(i)) Descend(i)->SetAllMarks(mark);
  }
}

static bool IsBoggleWord(const char* wd) {
  int size = strlen(wd);
  if (size < 3 || size > 17) return false;
  for (int i=0; i<size; ++i) {
    int c = wd[i];
    if (c<'a' || c>'z') return false;
    if (c=='q' && (i+1 >= size || wd[1+i] != 'u')) return false;
  }
  return true;
}

Trie* Trie::CreateFromFile(const char* filename) {
  SimpleTrie* t = new SimpleTrie;
  char line[80];
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    delete t;
    return NULL;
  }

  while (!feof(f) && fscanf(f, "%s", line)) {
    if (!IsBoggleWord(line)) continue;
    t->AddWord(line);
  }
  fclose(f);

  Trie* pt = Trie::CompactTrie(*t);
  delete t;
  return pt;
}


// Plain-vanilla Trie code
inline int idx(char x) { return x - 'a'; }

void Trie::SimpleTrie::AddWord(const char* wd) {
  if (!wd) return;
  if (!*wd) {
    SetIsWord();
    return;
  }
  int c = idx(*wd);
  if (!StartsWord(c))
    children_[c] = new SimpleTrie;
  if (c!=kQ)
    Descend(c)->AddWord(wd+1);
  else
    Descend(c)->AddWord(wd+2);
}

Trie::SimpleTrie::~SimpleTrie() {
  for (int i=0; i<26; i++) {
    if (children_[i]) delete children_[i];
  }
}

// Initially, this node is empty
Trie::SimpleTrie::SimpleTrie() {
  for (int i=0; i<kNumLetters; i++)
    children_[i] = NULL;
  is_word_ = false;
}
