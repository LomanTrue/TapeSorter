#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct TestCase {
  std::string name;
  void (*fn)();
};

inline std::vector<TestCase>& Registry() {
  static std::vector<TestCase> r;
  return r;
}

struct Registrar {
  Registrar(const char* name, void (*fn)()) {
    Registry().push_back({name, fn});
  }
};

struct AssertionError {
  std::string message;
};

inline void DoAssert(bool cond, const char* expr, const char* file, int line) {
  if (!cond) {
    std::ostringstream oss;
    oss << file << ":" << line << ": assertion failed: " << expr;
    throw AssertionError{oss.str()};
  }
}

template <typename A, typename B>
void DoAssertEq(const A& a, const B& b, const char* ea, const char* eb,
                const char* file, int line) {
  if (!(a == b)) {
    std::ostringstream oss;
    oss << file << ":" << line << ": " << ea << " == " << eb
        << " failed (lhs = " << a << ", rhs = " << b << ")";
    throw AssertionError{oss.str()};
  }
}

inline int RunAll() {
  int failed = 0;
  for (const auto& tc : Registry()) {
    try {
      tc.fn();
      std::cout << "[ OK ] " << tc.name << "\n";
    } catch (const AssertionError& e) {
      std::cout << "[FAIL] " << tc.name << ": " << e.message << "\n";
      ++failed;
    } catch (const std::exception& e) {
      std::cout << "[FAIL] " << tc.name << ": exception: " << e.what() << "\n";
      ++failed;
    }
  }
  std::cout << "\n" << (Registry().size() - failed) << " / "
            << Registry().size() << " tests passed.\n";
  return failed;
}

#define TEST(name) \
  static void test_##name(); \
  static ::Registrar reg_##name(#name, test_##name);  \
  static void test_##name()

#define ASSERT_TRUE(expr) ::DoAssert((expr), #expr, __FILE__, __LINE__)
#define ASSERT_FALSE(expr) ::DoAssert(!(expr), "!(" #expr ")", __FILE__, __LINE__)
#define ASSERT_EQ(a, b) ::DoAssertEq((a), (b), #a, #b, __FILE__, __LINE__)