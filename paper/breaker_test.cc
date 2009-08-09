#include "breaker.h"
#include "test.h"
#include "trie.h"

using namespace paper;

void BasicTest() {
  Trie t;
  t.AddWord("bad");
  t.AddWord("far");
  t.AddWord("fare");
  t.AddWord("fir");
  t.AddWord("rife");
  t.AddWord("fire");

  // a b c
  // d e f
  // g r aeiou
  //
  // any choice = bad (1 point)
  // a gets [far, fare] (+2 points = 3)
  // i gets [fir, rife, fire] (+3 points = 4)
  BucketBoggler bb(&t);
  const char bd[] = "a b c d e f g r aeiou";

  BreakDetails details;
  {
    CHECK(bb.ParseBoard(bd));
    Breaker b(&bb, 4);
    b.Break(&details);
    CHECK_EQ(0, details.max_depth);
    CHECK_EQ(5, details.num_reps);
    CHECK_EQ(0, details.failures.size());
  }

  {
    CHECK(bb.ParseBoard(bd));
    Breaker b(&bb, 3);
    b.Break(&details);
    CHECK_EQ(1, details.max_depth);
    CHECK_EQ(5, details.num_reps);
    CHECK_EQ(1, details.failures.size());
    CHECK_EQ("abcdefgri", details.failures[0]);
  }

  {
    CHECK(bb.ParseBoard(bd));
    Breaker b(&bb, 2);
    b.Break(&details);
    CHECK_EQ(1, details.max_depth);
    CHECK_EQ(5, details.num_reps);
    CHECK_EQ(2, details.failures.size());
    CHECK_EQ("abcdefgra", details.failures[0]);
    CHECK_EQ("abcdefgri", details.failures[1]);
  }
}

int main(int argc, char** argv) {
  BasicTest();
  printf("%s: All tests passed!\n", argv[0]);
}
