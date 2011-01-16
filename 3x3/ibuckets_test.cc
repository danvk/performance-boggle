#include <stdio.h>
#include <string.h>
#include <iostream>
#include "test.h"
#include "trie.h"
#include "3x3/ibuckets.h"

void TestBoards() {
  BucketSolver3 bb(NULL);
  CHECK(!bb.ParseBoard(""));
  CHECK(!bb.ParseBoard("abc def"));
  CHECK(!bb.ParseBoard("a b c d e f g h i j"));
  CHECK(!bb.ParseBoard("a b c d e  g h i"));

  CHECK(bb.ParseBoard("a b c d e f g h i"));
  char cmp[26];
  for (int i = 0; i < 9; i++) {
    sprintf(cmp, "%c", 'a' + i);
    CHECK_EQ(strcmp(bb.Cell(i), cmp), 0);
  }
  CHECK(1 == bb.NumReps());

  strcpy(bb.MutableCell(0), "abc");
  strcpy(bb.MutableCell(8), "pqrs");
  strcpy(bb.MutableCell(7), "htuv");
  CHECK(0 != strcmp(bb.Cell(0), "a"));
  CHECK_EQ(0, strcmp(bb.Cell(0), "abc"));
  CHECK_EQ(0, strcmp(bb.Cell(7), "htuv"));
  CHECK_EQ(0, strcmp(bb.Cell(8), "pqrs"));
  CHECK_EQ(3*4*4, bb.NumReps());
}

void TestBound() {
  SimpleTrie t;
  t.AddWord("sea");
  t.AddWord("seat");
  t.AddWord("seats");
  t.AddWord("tea");
  t.AddWord("teas");

  BucketSolver3 bb(&t);
  int score;
  CHECK(bb.ParseBoard("a b c d e f g h i"));
  CHECK_EQ(bb.UpperBound(), 0);
  CHECK_EQ(bb.Details().sum_union, 0);
  CHECK_EQ(bb.Details().max_nomark, 0);

  // s e h
  // e a t
  // p u c
  CHECK(bb.ParseBoard("s e p e a u h t c"));
  score = bb.UpperBound();
  CHECK_EQ(bb.Details().sum_union, 4);   // sea(t), tea(s)
  CHECK_EQ(bb.Details().max_nomark, 6);  // seat*2, sea*2, tea
  CHECK_EQ(score, 1 + 1 + 1 + 1);  // all words
  CHECK_EQ(bb.UpperBound(), 4);

  // a board where both [st]ea can be found, but not simultaneously
  // st z z
  //  e a s
  CHECK(bb.ParseBoard("st z z e a s z z z"));
  score = bb.UpperBound();
  CHECK_EQ(bb.Details().sum_union, 3);  // tea(s) + sea
  CHECK_EQ(bb.Details().max_nomark, 2);  // tea(s)
  CHECK_EQ(score, 2);
  CHECK_EQ(bb.UpperBound(), 2);

  // Add in a "seat", test its (sum union's) shortcomings. Can't have 'seats'
  // and 'teas' on the board simultaneously, but it still counts both.
  // st z z
  //  e a st
  //  z z s
  strcpy(bb.MutableCell(5), "st");
  strcpy(bb.MutableCell(8), "s");

  score = bb.UpperBound();
  CHECK_EQ(bb.Details().sum_union, 2 + 4);  // all but "hiccup"
  CHECK_EQ(bb.Details().max_nomark, 4);  // sea(t(s))
  CHECK_EQ(score, 4);
}

void TestQ() {
  SimpleTrie t;
  t.AddWord("qa");    // qua = 1
  t.AddWord("qas");   // qua = 1
  t.AddWord("qest");  // quest = 2

  BucketSolver3 bb(&t);
  int score;

  // q a s
  // a e z
  // s t z
  CHECK(bb.ParseBoard("q a s a e z s t z"));
  score = bb.UpperBound();
  CHECK_EQ(bb.Details().sum_union, 4);
  CHECK_EQ(bb.Details().max_nomark, 6);  // (qa + qas)*2 + qest
  CHECK_EQ(score, 4);

  // Make sure "qu" gets parsed as "either 'qu' or 'u'"
  // qu a s
  // a e z
  // s t z
  CHECK(bb.ParseBoard("qu a s a e z s t z"));
  score = bb.UpperBound();
  CHECK_EQ(bb.Details().sum_union, 4);
  CHECK_EQ(bb.Details().max_nomark, 6);  // (qa + qas)*2 + qest
  CHECK_EQ(score, 4);
}

int main(int argc, char** argv) {
  TestBoards();
  TestBound();
  TestQ();

  printf("%s: All tests passed!\n", argv[0]);
}
