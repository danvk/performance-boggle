#include "glog/logging.h"

#include <string>
#include <iostream>
using namespace std;

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Hello!";

  cout << "Hello cout" << endl;
}
