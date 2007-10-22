#include <assert.h>
#include <stdio.h>
#include <unistd.h>

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

  PerfectTrie* pt = PerfectTrie::CreateFromFile(tmp_file);
  assert(NULL != pt);
  assert(0 == remove(tmp_file));

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
