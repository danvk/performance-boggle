#include <stdio.h>
#include <unistd.h>

#include "test.h"
#include "trie.h"
#include "boggler.h"

int main(int argc, char** argv) {
  SimpleTrie t;
  t.AddWord("ate");
  t.AddWord("tea");
  t.AddWord("eta");
  t.AddWord("eat");
  t.AddWord("teak");
  t.AddWord("fiver");
  t.AddWord("sixers");

  // try out the various interfaces
  Boggler b(&t);
  CHECK_EQ(0, b.NumBoards());
  CHECK_EQ(4, b.Score("texxaxxxyyyyzzzz"));
  CHECK_EQ(5, b.Score("texxakxxyyyyzzzz"));
  CHECK_EQ(2, b.NumBoards());

  b.SetCell(0, 0, 'f' - 'a');
  b.SetCell(1, 0, 'i' - 'a');
  b.SetCell(2, 0, 'v' - 'a');
  b.SetCell(3, 0, 'e' - 'a');
  b.SetCell(3, 1, 'r' - 'a');
  CHECK_EQ(2, b.Score());
  CHECK_EQ(3, b.Score("sxxxixxxexxxrsxx"));
  CHECK_EQ(4, b.NumBoards());

  printf("%s: All tests passed!\n", argv[0]);
}
