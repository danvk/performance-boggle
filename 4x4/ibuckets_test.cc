#include <iostream>
#include "test.h"
#include "trie.h"
#include "4x4/boggler.h"
#include "4x4/ibuckets.h"

void TestBoards() {
  BucketSolver4 bb(NULL);
  CHECK(!bb.ParseBoard(""));
  CHECK(!bb.ParseBoard("abc def"));
  CHECK(!bb.ParseBoard("a b c d e f g h i j k l m n o p q"));
  CHECK(!bb.ParseBoard("a b c d e f g h  j k l m n o p q"));

  CHECK(bb.ParseBoard("a b c d e f g h i j k l m n o p"));
  char cmp[26];
  for (int i = 0; i < 16; i++) {
    sprintf(cmp, "%c", 'a' + i);
    CHECK_EQ(0, strcmp(bb.Cell(i), cmp));
  }
  CHECK_EQ(1, bb.NumReps());

  strcpy(bb.MutableCell(0), "abc");
  strcpy(bb.MutableCell(15), "pqrs");
  strcpy(bb.MutableCell(7), "htuv");
  CHECK_EQ("abc", bb.Cell(0));
  CHECK_EQ("pqrs", bb.Cell(15));
  CHECK_EQ("htuv", bb.Cell(7));
  CHECK(0 != strcmp(bb.Cell(0), "a"));
  CHECK_EQ(0, strcmp(bb.Cell(0), "abc"));
  CHECK_EQ(0, strcmp(bb.Cell(7), "htuv"));
  CHECK_EQ(0, strcmp(bb.Cell(15), "pqrs"));
  CHECK_EQ(3*4*4, bb.NumReps());
}

void TestBound() {
  SimpleTrie t;
  t.AddWord("sea");
  t.AddWord("seat");
  t.AddWord("seats");
  t.AddWord("tea");
  t.AddWord("teas");
  t.AddWord("hiccup");

  BucketSolver4 bb(&t);
  int score;
  CHECK(bb.ParseBoard("a b c d e f g h i j k l m n o p"));
  CHECK_EQ(0, bb.UpperBound());
  CHECK_EQ(0, bb.Details().sum_union);
  CHECK_EQ(0, bb.Details().max_nomark);

  // s e h i
  // e a t c
  // p u c z
  // z z z z
  CHECK(bb.ParseBoard("s e p z e a u z h t c z i c z z"));
  score = bb.UpperBound();
  CHECK_EQ(7, bb.Details().sum_union);   // sea(t), tea(s), hiccup
  CHECK_EQ(9, bb.Details().max_nomark);  // seat*2, sea*2, tea, hiccup
  CHECK_EQ(1 + 1 + 1 + 1 + 3, score);  // all words
  CHECK_EQ(7, bb.UpperBound());

  // a board where both [st]ea can be found, but not simultaneously
  // st e a s
  //  z z z z
  CHECK(bb.ParseBoard("st e a s z z z z y y y y z z z z"));
  score = bb.UpperBound();
  CHECK_EQ(3, bb.Details().sum_union);  // tea(s) + sea
  CHECK_EQ(2, bb.Details().max_nomark);  // tea(s)
  CHECK_EQ(2, score);

  // Add in a "seat", test its shortcomings. Can't have 'seats' and 'teas' on
  // the board simultaneously, but it still counts both.
  // st e a s
  //  e z z st
  //  a z z z
  strcpy(bb.MutableCell(7), "st");
  strcpy(bb.MutableCell(4), "e");
  strcpy(bb.MutableCell(8), "a");
  score = bb.UpperBound();
  CHECK_EQ(2 + 4, bb.Details().sum_union);  // all but "hiccup"
  CHECK_EQ(4 + 1, bb.Details().max_nomark);  // sea(t(s)) + sea (2 ways)
  CHECK_EQ(5, score);

  // A worst-case for max_nomark
  // s e t t
  // z t a t
  // z t t t
  CHECK(bb.ParseBoard("s e t t z t a t z t t t z z z z"));
  score = bb.UpperBound();
  CHECK_EQ(3, bb.Details().sum_union);  // sea(t) + tea
  CHECK_EQ(1 + 2 + 7, bb.Details().max_nomark);  // sea + 7*seat + 2*tea
  CHECK_EQ(3, score);
}

int main(int argc, char** argv) {
  TestBoards();
  TestBound();

  printf("%s: All tests passed!\n", argv[0]);
}
