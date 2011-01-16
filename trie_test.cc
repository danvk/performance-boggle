#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "trie.h"

int main(int argc, char** argv) {
  char tmp_file[] = "/tmp/trie-words.XXXXXX";
  int err = mkstemp(tmp_file);
  if (err == -1) {
    fprintf(stderr, "Could not create temp file\n");
    exit(1);
  }

  FILE* f = fopen(tmp_file, "w");
  assert(f);

  fprintf(f, "agriculture\n");
  fprintf(f, "culture\n");
  fprintf(f, "boggle\n");
  fprintf(f, "tea\n");
  fprintf(f, "sea\n");
  fprintf(f, "teapot\n");
  fclose(f);

  for (int i = 0; i < 5; i++) {
    Trie* t = Trie::CreateFromFile(tmp_file);
    assert(NULL != t);

    assert(6 == TrieUtils<Trie>::Size(t));
    assert( t->IsWord("agriculture"));
    assert( t->IsWord("culture"));
    assert( t->IsWord("boggle"));
    assert( t->IsWord("tea"));
    assert( t->IsWord("teapot"));
    assert(!t->IsWord("teap"));
    assert(!t->IsWord("random"));
    assert(!t->IsWord("cultur"));

    // Get a full word to test marking
    Trie* wd = t->Descend('t' - 'a');
    assert(NULL != wd);
    wd = wd->Descend('e' - 'a');
    assert(NULL != wd);
    wd = wd->Descend('a' - 'a');
    assert(NULL != wd);
    assert(0 == wd->Mark());
    wd->Mark(12345);
    assert(12345 == wd->Mark());
    t->Delete();
  }
  assert(0 == remove(tmp_file));
  printf("%s: All tests passed!\n", argv[0]);
}
