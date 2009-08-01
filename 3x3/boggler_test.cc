#include <assert.h>
#include <set>
#include <stdio.h>
#include <unistd.h>

#include "trie.h"
#include "3x3/boggler.h"

void CheckInt(int expected, int actual) {
  if (expected == actual) return;
  fprintf(stderr, "Expected %d, got %d\n", expected, actual);
  assert(false);
}

int main(int argc, char** argv) {
  Trie* t = Boggler::DictionaryFromFile("words");
  Boggler b(t);

  CheckInt(105, b.Score("catdlinem"));

  std::set<std::string> words;
  b.FindWords(&words);
  CheckInt(78, words.size());

  CheckInt(35, b.Score("catdqinem"));
  words.clear();
  b.FindWords(&words);
  CheckInt(26, words.size());

  printf("%s: All tests passed!\n", argv[0]);
}
