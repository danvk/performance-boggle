#include <assert.h>
#include <iostream>
#include "ibuckets.h"
#include "trie.h"

#define assertEq(a, b) CheckEqual(__FILE__, __LINE__, #a, (a), (b));
template<typename A, typename B>
void CheckEqual(const char* file, int line, const char* expr, A a, B b);

void TestBoards() {
  BucketBoggler bb(NULL);
  assert(!bb.ParseBoard(""));
  assert(!bb.ParseBoard("abc def"));
  assert(!bb.ParseBoard("a b c d e f g h i j k l m n o p q"));
  assert(!bb.ParseBoard("a b c d e f g h  j k l m n o p q"));

  assert(bb.ParseBoard("a b c d e f g h i j k l m n o p"));
  char cmp[26];
  for (int i = 0; i < 16; i++) {
    sprintf(cmp, "%c", 'a' + i);
    assertEq(strcmp(bb.Cell(i), cmp), 0);
  }
  assert(1 == bb.NumReps());

  strcpy(bb.Cell(0), "abc");
  strcpy(bb.Cell(15), "pqrs");
  strcpy(bb.Cell(7), "htuv");
  assert(0 != strcmp(bb.Cell(0), "a"));
  assert(0 == strcmp(bb.Cell(0), "abc"));
  assert(0 == strcmp(bb.Cell(7), "htuv"));
  assert(0 == strcmp(bb.Cell(15), "pqrs"));
  assert(3*4*4 == bb.NumReps());
}

void TestBound() {
  SimpleTrie t;
  t.AddWord("sea");
  t.AddWord("seat");
  t.AddWord("seats");
  t.AddWord("tea");
  t.AddWord("teas");
  t.AddWord("hiccup");

  BucketBoggler bb(&t);
  int score;
  assert(bb.ParseBoard("a b c d e f g h i j k l m n o p"));
  assertEq(bb.UpperBound(), 0);
  assertEq(bb.Details().sum_union, 0);
  assertEq(bb.Details().max_nomark, 0);

  // s e h i
  // e a t c
  // p u c z
  // z z z z
  assert(bb.ParseBoard("s e p z e a u z h t c z i c z z"));
  score = bb.UpperBound();
  assertEq(bb.Details().sum_union, 7);   // sea(t), tea(s), hiccup
  assertEq(bb.Details().max_nomark, 9);  // seat*2, sea*2, tea, hiccup
  assertEq(score, 1 + 1 + 1 + 1 + 3);  // all words

  // a board where both [st]ea can be found, but not simultaneously
  // st e a s
  //  z z z z
  assert(bb.ParseBoard("st e a s z z z z y y y y z z z z"));
  score = bb.UpperBound();
  assertEq(bb.Details().sum_union, 3);  // tea(s) + sea
  assertEq(bb.Details().max_nomark, 2);  // tea(s)
  assertEq(score, 2);

  // Add in a "seat", test its shortcomings. Can't have 'seats' and 'teas' on
  // the board simultaneously, but it still counts both.
  // st e  a s
  //  e z  z st
  //  a z  z z
  strcpy(bb.Cell(7), "st");
  strcpy(bb.Cell(4), "e");
  strcpy(bb.Cell(8), "a");
  score = bb.UpperBound();
  assertEq(bb.Details().sum_union, 2 + 4);  // all but "hiccup"
  assertEq(bb.Details().max_nomark, 4 + 1);  // sea(t(s)) + tea
  assertEq(score, 5);

  // A worst-case for max_nomark
  // s e t t
  // z t a t
  // z t t t
  assert(bb.ParseBoard("s e t t z t a t z t t t z z z z"));
  score = bb.UpperBound();
  assertEq(bb.Details().sum_union, 3);  // sea(t) + tea
  assertEq(bb.Details().max_nomark, 1 + 2 + 7);  // sea + 7*seat + 2*tea
  assertEq(score, 3);
}

template<typename A, typename B>
void CheckEqual(const char* file, int line, const char* expr, A a, B b) {
  if (a != b) {
    std::cout << file << ":" << line << ": " << expr << " = " << a
              << ", expected " << b << std::endl;
    exit(1);
  }
}

int main(int argc, char** argv) {
  TestBoards();
  TestBound();

  printf("%s: All tests passed!\n", argv[0]);
}
