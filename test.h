// Copyright 2009 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)
#ifndef PAPER_TEST_H__
#define PAPER_TEST_H__

#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#define CHECK_EQ(a, b) CheckEqual(__FILE__, __LINE__, #a, (a), (b));
#define CHECK(a) CheckEqual(__FILE__, __LINE__, #a, (a), true);
#define CHECK_IN(a, b) CheckInVector(__FILE__, __LINE__, #b, (a), (b));
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

void CheckEqual(const char* file, int line, const char* expr,
                const char* a, const char* b) {
  CheckEqual(file, line, expr, std::string(a), std::string(b));
}

template<typename A>
void CheckInVector(const char* file, int line, const char* v_expr,
                   const A& el, const std::vector<A>& v) {
  if (find(v.begin(), v.end(), el) == v.end()) {
    std::cout << file << ":" << line
              << ": Didn't find " << el << " in " << v_expr << std::endl;
    exit(1);
  }
}

void CheckInVector(const char* file, int line, const char* v_expr,
                   const char* el, const std::vector<std::string>& v) {
  CheckInVector(file, line, v_expr, std::string(el), v);
}

#endif
