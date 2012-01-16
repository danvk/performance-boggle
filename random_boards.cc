// Produce a bunch of random boards.

#include <iostream>
#include <string>
#include "gflags/gflags.h"
#include "init.h"
#include "mtrandom/randomc.h"

DEFINE_int32(n, 100, "Number of boards to produce");
DEFINE_int32(size, 16, "Size of each board (usually 16 or 9)");
DEFINE_int32(rand_seed, -1,
             "Random number seed (default is based on time and pid)");

int main(int argc, char** argv) {
  Init(&argc, &argv);
  if (FLAGS_rand_seed == -1) {
    FLAGS_rand_seed = time(NULL) + getpid();
  }
  TRandomMersenne r(FLAGS_rand_seed);
  
  std::string bd;
  bd.resize(FLAGS_size);
  for (int i = 0; i < FLAGS_n; i++) {
    for (int i = 0; i < FLAGS_size; i++) {
      bd[i] = r.IRandom('a', 'z');
    }
    std::cout << bd << std::endl;
  }
}
