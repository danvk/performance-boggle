// Copyright 2009 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)
#ifndef PAPER_TEST_H__
#define PAPER_TEST_H__

#include <iostream>

#define CHECK_EQ(a, b) CheckEqual(__FILE__, __LINE__, #a, (a), (b));
#define CHECK(a) CheckEqual(__FILE__, __LINE__, #a, (a), true);
template<typename A, typename B>
void CheckEqual(const char* file, int line, const char* expr, A a, B b);

template<typename A, typename B>
void CheckEqual(const char* file, int line, const char* expr, A a, B b) {
  if (a != b) {
    std::cout << file << ":" << line << ": " << expr << " = " << b
              << ", expected " << a << std::endl;
    exit(1);
  }
}

#endif
