#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "trie.h"

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

  assert(t.IsWord("agriculture"));
  assert(t.IsWord("culture"));
  assert(t.IsWord("boggle"));
  assert(t.IsWord("tea"));
  assert(t.IsWord("teapot"));
  assert(!t.IsWord("teap"));
  assert(!t.IsWord("random"));
  assert(!t.IsWord("cultur"));

  assert(0 == remove(tmp_file));
  printf("%s: All tests passed!\n", argv[0]);
}
