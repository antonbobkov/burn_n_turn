/*
 * smart_pointer_test - Catch2 tests for smart_pointer and make_smart.
 */

#include "smart_pointer.h"
#include <catch2/catch.hpp>

/* Minimal ref-counted type for tests (inherits SP_Info). */
struct TestObj : SP_Info {
  int value;
  TestObj() : SP_Info(), value(0) {}
  explicit TestObj(int v) : SP_Info(), value(v) {}
};

TEST_CASE("smart_pointer default construction", "[smart_pointer]") {
  smart_pointer<TestObj> p;
  REQUIRE(p.GetRawPointer() == nullptr);
}

TEST_CASE("smart_pointer construction via make_smart", "[smart_pointer]") {
  smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(42));
  REQUIRE(p.GetRawPointer() != nullptr);
  REQUIRE(p->value == 42);
  REQUIRE((*p).value == 42);
}

TEST_CASE("smart_pointer copy constructor", "[smart_pointer]") {
  smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(7));
  smart_pointer<TestObj> q(p);
  REQUIRE(q.GetRawPointer() == p.GetRawPointer());
  REQUIRE(q->value == 7);
}

TEST_CASE("smart_pointer assignment from another smart_pointer",
          "[smart_pointer]") {
  smart_pointer<TestObj> a = make_smart<TestObj>(new TestObj(1));
  smart_pointer<TestObj> b;
  b = a;
  REQUIRE(b.GetRawPointer() == a.GetRawPointer());
  REQUIRE(b->value == 1);
}

TEST_CASE("smart_pointer assign empty releases; other copy still valid",
          "[smart_pointer]") {
  smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(3));
  smart_pointer<TestObj> q = p;
  p = smart_pointer<TestObj>();
  REQUIRE(p.GetRawPointer() == nullptr);
  REQUIRE(q.GetRawPointer() != nullptr);
  REQUIRE(q->value == 3);
}

TEST_CASE("smart_pointer operator== and operator!=", "[smart_pointer]") {
  smart_pointer<TestObj> a = make_smart<TestObj>(new TestObj(0));
  smart_pointer<TestObj> b(a);
  smart_pointer<TestObj> c = make_smart<TestObj>(new TestObj(0));
  REQUIRE(a == b);
  REQUIRE(a != c);
  REQUIRE(b != c);
}

/* Raw-pointer construction is disallowed: smart_pointer<T> p(new T())
 * does not compile; use make_smart(new T()) instead. */
