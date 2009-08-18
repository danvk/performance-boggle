#include <assert.h>
#include <set>
#include <stdio.h>
#include <unistd.h>

#include "test.h"
#include "trie.h"
#include "3x3/boggler.h"
#include "4x4/boggler.h"

int main(int argc, char** argv) {
  SimpleTrie* t = Boggler::DictionaryFromFile("words");
  Boggler3 b(t);

  CHECK_EQ(105, b.Score("catdlinem"));
  CHECK_EQ(35, b.Score("catdqinem"));

  printf("%s: All tests passed!\n", argv[0]);
}
