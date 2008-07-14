#include <assert.h>
#include <iostream>
#include "ibuckets.h"
#include "trie.h"
#include "boggler.h"

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
  assertEq(bb.SimpleUpperBound(), 7);

  // a board where both [st]ea can be found, but not simultaneously
  // st e a s
  //  z z z z
  assert(bb.ParseBoard("st e a s z z z z y y y y z z z z"));
  score = bb.UpperBound();
  assertEq(bb.Details().sum_union, 3);  // tea(s) + sea
  assertEq(bb.Details().max_nomark, 2);  // tea(s)
  assertEq(score, 2);
  assertEq(bb.SimpleUpperBound(), 2);

  // Add in a "seat", test its shortcomings. Can't have 'seats' and 'teas' on
  // the board simultaneously, but it still counts both.
  // st e a s
  //  e z z st
  //  a z z z
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

void TestMaxDelta() {
  SimpleTrie t;
  t.AddWord("sea");
  t.AddWord("coast");

  // s e ac o
  // . .  . a
  // . .  . s
  // . .  . t
  BucketBoggler bb(&t);
  assert(bb.ParseBoard("s e ac o . . . a . . . s . . . t"));
  bb.UpperBound();
  int base_score = bb.Details().max_nomark;
  int a_cost = bb.Details().max_delta[2]['a' - 'a'];
  int c_cost = bb.Details().max_delta[2]['c' - 'a'];
  assertEq(base_score, 3);
  assertEq(a_cost, 2);
  assertEq(c_cost, 1);

  strcpy(bb.Cell(2), "a");
  bb.UpperBound();
  assertEq(bb.Details().max_nomark, base_score - a_cost);

  strcpy(bb.Cell(2), "c");
  bb.UpperBound();
  assertEq(bb.Details().max_nomark, base_score - c_cost);

  // Add in a third word:
  // s e aci o
  // z n  d  a
  // z z  z  s
  // z z  z  t
  t.AddWord("din");
  strcpy(bb.Cell(2), "aci");
  strcpy(bb.Cell(6), "d");
  strcpy(bb.Cell(5), "n");
  bb.UpperBound();
  base_score = bb.Details().max_nomark;
  a_cost = bb.Details().max_delta[2]['a' - 'a'];
  c_cost = bb.Details().max_delta[2]['c' - 'a'];
  int i_cost = bb.Details().max_delta[2]['i' - 'a'];
  assertEq(bb.Details().max_nomark, 4);
  assertEq(a_cost, 3);
  assertEq(c_cost, 2);
  assertEq(i_cost, 3);

  // Multiple bucketed cells
  // s e ac o
  // x x  x ad
  // x x  x s
  // x x  x t
  assert(bb.ParseBoard("s e ac o z z z ad j z z s j z z t"));
  bb.UpperBound();
  assertEq(bb.Details().max_nomark, 3);
  assertEq(bb.Details().max_delta[2]['a' - 'a'], 2);
  assertEq(bb.Details().max_delta[2]['c' - 'a'], 1);
  assertEq(bb.Details().max_delta[7]['a' - 'a'], 0);
  assertEq(bb.Details().max_delta[7]['d' - 'a'], 2);

  // Different paths have to be summed...
  //  a b x x
  // ab x x x
  //  x x x x
  //  x x x x
  t.AddWord("baa");
  assert(bb.ParseBoard("a b x x ab x x x j x x x j x x x"));
  bb.UpperBound();
  assertEq(bb.Details().max_nomark, 2);
  assertEq(bb.Details().max_delta[4]['b' - 'a'], 2);

  // Try a board with a tie for best choice (c -> [ae])
  // .  . c .
  // e ae . .
  // . ab . .
  // .  . . .
  // cab cee bee bee (can't have both cab and cee => 3)
  // [ab] -> a => cee
  t.AddWord("cee");
  t.AddWord("cab");
  t.AddWord("bee");
  assert(bb.ParseBoard("j j c j e f j j j j j j j j j j"));
  strcpy(bb.Cell(9), "ab");  // replaces the 'j'
  strcpy(bb.Cell(5), "ae");  // replaces the 'e'

  bb.UpperBound();
  assertEq(bb.Details().max_nomark, 3);
  assertEq(bb.Details().max_delta[9]['a' - 'a'], 2);
  assertEq(bb.Details().max_delta[9]['b' - 'a'], 0);

  // A board where a choice down the road changes the choice at the top.
  //  . . . . 
  // ao . . .
  //  i . k . 
  //  m n . rs
  t.AddWord("aim");
  t.AddWord("ain");
  t.AddWord("oink");
  t.AddWord("oinks");
  assert(bb.ParseBoard(". . . . ao . . . i . k . m n . sr"));
  bb.UpperBound();
  assertEq(bb.Details().max_nomark, 3);
  assertEq(bb.Details().max_delta[15]['r' - 'a'], 1);
}

void TestRealDictionary() {
  SimpleTrie* t = GenericBoggler<SimpleTrie>::DictionaryFromFile("words");
  BucketBoggler bb(t);

  {
    // First a simple board.
    assert(bb.ParseBoard("a b c d e f g h i j k l m n o p"));
    strcpy(bb.Cell(9), "abcdefghijklmnopqrstuvwxyz");  // replaces the 'j'

    bb.UpperBound();
    int base_score = bb.Details().max_nomark;
    int expected[26];
    for (int i = 0; i < 26; i++) {
      expected[i] = base_score - bb.Details().max_delta[9][i];
    }

    for (int i = 0; i < 26; i++) {
      sprintf(bb.Cell(9), "%c", 'a' + i);
      bb.UpperBound();
      assertEq(bb.Details().max_nomark, expected[i]);
    }
  }

  {
    // Now a real stress test...
    int expected[16][3];
    for (int i=0; i<16; i++) {
      bb.Cell(i)[0] = 'a' + (3 * i + 0) % 26;
      bb.Cell(i)[1] = 'a' + (3 * i + 1) % 26;
      bb.Cell(i)[2] = 'a' + (3 * i + 2) % 26;
      bb.Cell(i)[3] = '\0';
    }
    bb.UpperBound();
    const BucketBoggler::ScoreDetails& d = bb.Details();
    for (int i=0; i<16; i++) {
      for (int j=0; j<3; j++) {
        expected[i][j] = d.max_nomark - d.max_delta[i][bb.Cell(i)[j] - 'a'];
      }
    }

    for (int i=0; i<16; i++) {
      char tmp[26];
      strcpy(tmp, bb.Cell(i));
      for (size_t j=0; j<strlen(tmp); j++) {
        sprintf(bb.Cell(i), "%c", tmp[j]);
        bb.UpperBound();
        assertEq(bb.Details().max_nomark, expected[i][j]);
      }
      strcpy(bb.Cell(i), tmp);
    }
  }
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
  TestMaxDelta();
  TestRealDictionary();

  printf("%s: All tests passed!\n", argv[0]);
}
