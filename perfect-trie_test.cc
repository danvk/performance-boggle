#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "trie.h"
#include "perfect-trie.h"

int main(int argc, char** argv) {
  char tmp[] = "/tmp/trie-words.XXXXX";
  char* tmp_file = mktemp(tmp);
  assert(tmp_file > 0);

  FILE* f = fopen(tmp_file, "w");
  assert(f);

  fprintf(f, "agriculture\n");
  fprintf(f, "culture\n");
  fprintf(f, "boggle\n");
  fprintf(f, "tea\n");
  fprintf(f, "teapot\n");
  fclose(f);

  Trie t;
  bool success = t.LoadFile(tmp_file);
  assert(success);

  assert(5 == t.Size());

  assert(t.IsWord("agriculture"));
  assert(t.IsWord("culture"));
  assert(t.IsWord("boggle"));
  assert(t.IsWord("tea"));
  assert(t.IsWord("teapot"));
  assert(!t.IsWord("teap"));
  assert(!t.IsWord("random"));
  assert(!t.IsWord("cultur"));

  assert(0 == t.Mark());
  t.Mark(12345);
  assert(12345 == t.Mark());
  t.Mark(0);

  assert(0 == remove(tmp_file));

  PerfectTrie* pt = PerfectTrie::CompactTrie(t);
  assert(5 == pt->Size());
  assert( pt->IsWord("agriculture"));
  assert( pt->IsWord("culture"));
  assert( pt->IsWord("boggle"));
  assert( pt->IsWord("tea"));
  assert( pt->IsWord("teapot"));
  assert(!pt->IsWord("teap"));
  assert(!pt->IsWord("random"));
  assert(!pt->IsWord("cultur"));

  assert(0 == pt->Mark());
  pt->Mark(12345);
  assert(12345 == pt->Mark());
  printf("%s: All tests passed!\n", argv[0]);
}
