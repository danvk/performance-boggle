// Various bits of initialization/sanity checking.

#include "init.h"

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "mtrandom/randomc.h"

// TODO(danvk): make all utilities initialize themselves using this.

void Init(int* argc, char*** argv) {
  google::ParseCommandLineFlags(argc, argv, true);
  google::InitGoogleLogging((*argv)[0]);
  google::InstallFailureSignalHandler();

  CHECK_EQ(4, sizeof(int32)) << "MTRandom is misconfigured. Correct randomc.h.";
  CHECK_EQ(4, sizeof(uint32)) << "MTRandom is misconfigured. Correct randomc.h";
}
