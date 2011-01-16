#include "board-utils.h"
#include "test.h"
#include <stdio.h>
#include <iostream>

// Checks that BoardId o BoardFromId = Identity
void TestIdentity() {
  std::string bd;
  {
    BoardUtils bu(4, 4, 6);
    bd = "abcdefabcdefabcd";
    CHECK_EQ(bd, bu.BoardFromId(bu.BoardId(bd)));

    bd = "abcdefgbcdefabcd";
    CHECK_EQ(-1, bu.BoardId(bd));
  }

  {
    BoardUtils bu(3, 3, 26);
    bd = "abcdefghi";
    CHECK_EQ(bd, bu.BoardFromId(bu.BoardId(bd)));
    bd = "jklmnopqr";
    CHECK_EQ(bd, bu.BoardFromId(bu.BoardId(bd)));
  }

  {
    BoardUtils bu(3, 4, 26);
    bd = "abcdefghijkl";
    CHECK_EQ(bd, bu.BoardFromId(bu.BoardId(bd)));
    bd = "jklmnopqrstu";
    CHECK_EQ(bd, bu.BoardFromId(bu.BoardId(bd)));
  }
}

void TestFlips() {
  std::string bd;
  {
    BoardUtils bu(4, 4);
    bd = "abcdefghijklmnop";
    CHECK_EQ("dcbahgfelkjiponm", bu.FlipLeftRight(bd));
    CHECK_EQ("mnopijklefghabcd", bu.FlipTopBottom(bd));
    // a b c d    m i e a
    // e f g h -> n j f b
    // i j k l    o k g c
    // m n o p    p l h d
    CHECK_EQ("mieanjfbokgcplhd", bu.Rotate90CW(bd));
  }

  {
    BoardUtils bu(4, 3);
    bd = "abcdefghijkl";
    CHECK_EQ("dcbahgfelkji", bu.FlipLeftRight(bd));
    CHECK_EQ("ijklefghabcd", bu.FlipTopBottom(bd));
    CHECK_EQ("", bu.Rotate90CW(bd));
  }
}

void TestAnalogues() {
  std::vector<std::string> bds;
  {
    BoardUtils bu(4, 4);
    bu.GenerateAnalogues("aaaaaaaaaaaaaaaa", &bds);
    CHECK_EQ(0, bds.size());

    bu.GenerateAnalogues("abcdefghijklmnop", &bds);
    CHECK_EQ(7, bds.size());
    CHECK_IN("dcbahgfelkjiponm", bds);  // flip l/r
    CHECK_IN("mnopijklefghabcd", bds);  // flip t/b
    CHECK_IN("ponmlkjihgfedcba", bds);  // flip l/r + t/b

    CHECK_IN("miea" "njfb" "okgc" "plhd", bds);  // rotate 90 deg cw
    CHECK_IN("aeim" "bfjn" "cgko" "dhlp", bds);  // rot + flip l/r
    CHECK_IN("plhd" "okgc" "njfb" "miea", bds);  // rot + flip t/b
    CHECK_IN("dhlp" "cgko" "bfjn" "aeim", bds);  // rot + flip l/r + flip t/b

    CHECK_EQ(true, bu.IsCanonical("abcdefghijklmnop"));
    for (int i = 0; i < bds.size(); i++) {
      CHECK_EQ(false, bu.IsCanonical(bds[i]));
    }
  }

  {
    BoardUtils bu(4, 3);
    bu.GenerateAnalogues("abcd""efgh""ijkl", &bds);
    CHECK_EQ(3, bds.size());
    CHECK_IN("dcba""hgfe""lkji", bds);
    CHECK_IN("ijkl""efgh""abcd", bds);
    CHECK_IN("lkji""hgfe""dcba", bds);
  }
}

void TestExpand() {
  {
    BoardUtils bu(3, 2);
    std::vector<std::string> classes;
    classes.push_back("abcd");
    classes.push_back("efgh");
    classes.push_back("ijkl");
    bu.UsePartition(classes);

    CHECK_EQ("abcd efgh ijkl efgh ijkl abcd", bu.ExpandPartitions("abcbca"));
    CHECK_EQ("", bu.ExpandPartitions("abcbcd"));
    CHECK_EQ("", bu.ExpandPartitions("abcbccc"));
  }
}

int main(int argc, const char** argv) {
  TestIdentity();
  TestFlips();
  TestAnalogues();
  TestExpand();
  printf("%s: All tests passed!\n", argv[0]);
}
