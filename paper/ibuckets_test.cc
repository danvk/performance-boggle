#include <iostream>
#include "test.h"
#include "trie.h"
#include "ibuckets.h"

using paper::BucketBoggler;
using paper::Trie;

void TestBoards() {
  BucketBoggler bb(NULL);
  CHECK(!bb.ParseBoard(""));
  CHECK(!bb.ParseBoard("abc def"));
  CHECK(!bb.ParseBoard("a b c d e f g h i j"));
  CHECK(!bb.ParseBoard("a b c d e  g h i"));

  CHECK(bb.ParseBoard("a b c d e f g h i"));
  char cmp[26];
  for (int i = 0; i < 9; i++) {
    sprintf(cmp, "%c", 'a' + i);
    CHECK_EQ(0, strcmp(bb.Cell(i), cmp));
  }
  CHECK_EQ(1, bb.NumReps());

  strcpy(bb.Cell(0), "abc");
  strcpy(bb.Cell(8), "pqrs");
  strcpy(bb.Cell(7), "htuv");
  CHECK(0 != strcmp(bb.Cell(0), "a"));
  CHECK(0 == strcmp(bb.Cell(0), "abc"));
  CHECK(0 == strcmp(bb.Cell(7), "htuv"));
  CHECK(0 == strcmp(bb.Cell(8), "pqrs"));
  CHECK_EQ(3*4*4, bb.NumReps());
}

void TestBound() {
  Trie t;
  t.AddWord("sea");
  t.AddWord("seat");
  t.AddWord("seats");
  t.AddWord("tea");
  t.AddWord("teas");

  BucketBoggler bb(&t);
  int score;
  CHECK(bb.ParseBoard("a b c d e f g h i"));
  CHECK_EQ(0, bb.UpperBound());
  CHECK_EQ(0, bb.Details().sum_union);
  CHECK_EQ(0, bb.Details().max_nomark);

  // s e h
  // e a t
  // p u c
  CHECK(bb.ParseBoard("s e p e a u h t c"));
  score = bb.UpperBound();
  CHECK_EQ(4, bb.Details().sum_union);   // sea(t), tea(s)
  CHECK_EQ(6, bb.Details().max_nomark);  // seat*2, sea*2, tea
  CHECK_EQ(1 + 1 + 1 + 1, score);  // all words
  CHECK_EQ(4, bb.UpperBound());

  // a board where both [st]ea can be found, but not simultaneously
  // st z z
  //  e a s
  CHECK(bb.ParseBoard("st z z e a s z z z"));
  score = bb.UpperBound();
  CHECK_EQ(3, bb.Details().sum_union);  // tea(s) + sea
  CHECK_EQ(2, bb.Details().max_nomark);  // tea(s)
  CHECK_EQ(2, score);
  CHECK_EQ(2, bb.UpperBound());

  // Add in a "seat", test its (sum union's) shortcomings. Can't have 'seats'
  // and 'teas' on the board simultaneously, but it still counts both.
  // st z z
  //  e a st
  //  z z s
  strcpy(bb.Cell(5), "st");
  strcpy(bb.Cell(8), "s");

  score = bb.UpperBound();
  CHECK_EQ(2 + 4, bb.Details().sum_union);  // all but "hiccup"
  CHECK_EQ(4, bb.Details().max_nomark);  // sea(t(s))
  CHECK_EQ(4, score);
}

void TestQ() {
  Trie t;
  t.AddWord("qa");    // qua = 1
  t.AddWord("qas");   // qua = 1
  t.AddWord("qest");  // quest = 2

  BucketBoggler bb(&t);
  int score;

  // q a s
  // a e z
  // s t z
  CHECK(bb.ParseBoard("q a s a e z s t z"));
  score = bb.UpperBound();
  CHECK_EQ(4, bb.Details().sum_union);
  CHECK_EQ(6, bb.Details().max_nomark);  // (qa + qas)*2 + qest
  CHECK_EQ(4, score);

  // Make sure "qu" gets parsed as "either 'qu' or 'u'"
  // qu a s
  // a e z
  // s t z
  CHECK(bb.ParseBoard("qu a s a e z s t z"));
  score = bb.UpperBound();
  CHECK_EQ(4, bb.Details().sum_union);
  CHECK_EQ(6, bb.Details().max_nomark);  // (qa + qas)*2 + qest
  CHECK_EQ(4, score);
}

int main(int argc, char** argv) {
  TestBoards();
  TestBound();
  TestQ();

  printf("%s: All tests passed!\n", argv[0]);
}
