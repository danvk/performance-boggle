// Test cases for buckets

#include <assert.h>
#include <string>
#include <vector>
#include "buckets.h"

int main(int argc, char** argv) {
  std::vector<std::string> b;
  b.push_back("abcd");
  b.push_back("efgh");
  b.push_back("ijkl");
  b.push_back("mnop");
  b.push_back("qrst");
  b.push_back("uvwxy");
  b.push_back("z");

  Buckets::Bucketing buckets;
  bool ret = Buckets::ToMap(b, &buckets);
  assert(ret);
  assert(7 == Buckets::NumBuckets(buckets));

  Boggler bd(NULL);
  bd.ParseBoard("gggggggggggggggg");
  assert(1 == Buckets::NumRepresentatives(bd, buckets));
  bd.SetCell(0, 0, 0);  // abcd
  assert(4 == Buckets::NumRepresentatives(bd, buckets));
  bd.SetCell(1, 0, 2);  // ijkl
  assert(16 == Buckets::NumRepresentatives(bd, buckets));
  printf("%s: all tests passed\n", argv[0]);
}
