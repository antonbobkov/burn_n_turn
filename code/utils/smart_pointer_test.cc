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
  std::string get_class_name() override { return "TestObj"; }
};

TEST_CASE("smart_pointer default construction", "[smart_pointer]") {
  smart_pointer<TestObj> p;
  REQUIRE(p.is_null());
}

TEST_CASE("smart_pointer construction via make_smart", "[smart_pointer]") {
  smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(42));
  REQUIRE(!p.is_null());
  REQUIRE(p->value == 42);
  REQUIRE((*p).value == 42);
}

TEST_CASE("smart_pointer copy constructor", "[smart_pointer]") {
  smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(7));
  smart_pointer<TestObj> q(p);
  REQUIRE(q.get() == p.get());
  REQUIRE(q->value == 7);
}

TEST_CASE("smart_pointer assignment from another smart_pointer",
          "[smart_pointer]") {
  smart_pointer<TestObj> a = make_smart<TestObj>(new TestObj(1));
  smart_pointer<TestObj> b;
  b = a;
  REQUIRE(b.get() == a.get());
  REQUIRE(b->value == 1);
}

TEST_CASE("smart_pointer assign empty releases; other copy still valid",
          "[smart_pointer]") {
  smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(3));
  smart_pointer<TestObj> q = p;
  p = smart_pointer<TestObj>();
  REQUIRE(p.is_null());
  REQUIRE(!q.is_null());
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

/* Helper: get ref count for a smart_pointer's pointee (TestObj inherits
 * SP_Info so the raw pointer is the SP_Info). */
static unsigned ref_count(smart_pointer<TestObj> &p) {
  if (p.is_null())
    return 0;
  return static_cast<SP_Info *>(p.get())->get_counter();
}

TEST_CASE("smart_pointer ref count is 1 after make_smart", "[smart_pointer]") {
  smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(10));
  REQUIRE(ref_count(p) == 1);
}

TEST_CASE("smart_pointer ref count increases on copy, decreases on release",
          "[smart_pointer]") {
  smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(20));
  REQUIRE(ref_count(p) == 1);
  {
    smart_pointer<TestObj> q(p);
    REQUIRE(ref_count(p) == 2);
    REQUIRE(ref_count(q) == 2);
  }
  REQUIRE(ref_count(p) == 1);
}

TEST_CASE("smart_pointer ref count updates on assignment", "[smart_pointer]") {
  smart_pointer<TestObj> a = make_smart<TestObj>(new TestObj(1));
  smart_pointer<TestObj> b = make_smart<TestObj>(new TestObj(2));
  REQUIRE(ref_count(a) == 1);
  REQUIRE(ref_count(b) == 1);
  b = a;
  REQUIRE(ref_count(a) == 2);
  REQUIRE(ref_count(b) == 2);
  a = smart_pointer<TestObj>();
  REQUIRE(ref_count(b) == 1);
}

TEST_CASE("smart_pointer object cleaned up when last reference goes away",
          "[smart_pointer]") {
  int before = nGlobalSuperMegaCounter;
  {
    smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(100));
    REQUIRE(nGlobalSuperMegaCounter == before + 1);
  }
  REQUIRE(nGlobalSuperMegaCounter == before);
}

TEST_CASE("smart_pointer multiple copies all released then object deleted",
          "[smart_pointer]") {
  int before = nGlobalSuperMegaCounter;
  {
    smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(5));
    smart_pointer<TestObj> q = p;
    smart_pointer<TestObj> r = q;
    REQUIRE(ref_count(p) == 3);
    REQUIRE(nGlobalSuperMegaCounter == before + 1);
    p = smart_pointer<TestObj>();
    q = smart_pointer<TestObj>();
    REQUIRE(nGlobalSuperMegaCounter == before + 1);
  }
  REQUIRE(nGlobalSuperMegaCounter == before);
}

TEST_CASE("smart_pointer self-assign leaves ref count one", "[smart_pointer]") {
  smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(9));
  REQUIRE(ref_count(p) == 1);
  p = p;
  REQUIRE(ref_count(p) == 1);
  REQUIRE(!p.is_null());
  REQUIRE(p->value == 9);
}

TEST_CASE("smart_pointer class name count increments on construct",
          "[smart_pointer][class_name]") {
  REQUIRE(g_smart_pointer_count["TestObj"] == 0);
  smart_pointer<TestObj> p = make_smart<TestObj>(new TestObj(1));
  REQUIRE(g_smart_pointer_count["TestObj"] == 1);
  {
    smart_pointer<TestObj> q(p);
    REQUIRE(g_smart_pointer_count["TestObj"] == 2);
  }
  REQUIRE(g_smart_pointer_count["TestObj"] == 1);
  p = smart_pointer<TestObj>();
  REQUIRE(g_smart_pointer_count["TestObj"] == 0);
}

TEST_CASE("smart_pointer class name count on assignment",
          "[smart_pointer][class_name]") {
  smart_pointer<TestObj> a = make_smart<TestObj>(new TestObj(1));
  smart_pointer<TestObj> b = make_smart<TestObj>(new TestObj(2));
  REQUIRE(g_smart_pointer_count["TestObj"] == 2);
  b = a;
  REQUIRE(g_smart_pointer_count["TestObj"] == 2);
  a = smart_pointer<TestObj>();
  REQUIRE(g_smart_pointer_count["TestObj"] == 1);
  b = smart_pointer<TestObj>();
  REQUIRE(g_smart_pointer_count["TestObj"] == 0);
}

/* Forward declaration: smart_pointer supports incomplete types by storing
 * SP_Info* separately; only make_smart needs the complete type. */
struct Incomplete;
TEST_CASE("smart_pointer with incomplete type (default construct and destroy)",
          "[smart_pointer][incomplete]") {
  smart_pointer<Incomplete> p;
  REQUIRE(p.is_null());
  REQUIRE(p.get() == nullptr);
}
