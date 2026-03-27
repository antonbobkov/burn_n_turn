/* Tests for EntityLedger registration mechanics: register, unregister,
 * auto-unregister on destruction, and null-out safety during iteration. */

#include "../entity_ledger.h"
#include "../entities.h"

#include <catch2/catch.hpp>
#include <vector>
#include <algorithm>

namespace {

/* A concrete ledger for testing — same null-out logic as EntityListController
 * but self-contained so tests need no DragonGameController. */
class TestLedger : public EntityLedger {
public:
  std::vector<Entity *> entries;

  void Register(Entity *e) override {
    if (!e) return;
    e->SetLedger(this);
    entries.push_back(e);
  }

  void Unregister(Entity *e) override {
    for (Entity *&ptr : entries)
      if (ptr == e) { ptr = nullptr; return; }
  }

  /* Count non-null entries (live registered souls). */
  int LiveCount() const {
    int n = 0;
    for (Entity *ptr : entries)
      if (ptr) ++n;
    return n;
  }
};

/* A minimal entity that records whether its Update was called. */
class TestEntity : public Entity {
public:
  bool updated = false;
  void Update() override { updated = true; }
};

} // namespace

TEST_CASE("Register adds entity to ledger") {
  TestLedger ledger;
  TestEntity e;
  ledger.Register(&e);

  REQUIRE(ledger.entries.size() == 1);
  REQUIRE(ledger.entries[0] == &e);
}

TEST_CASE("Unregister nulls the slot, does not erase") {
  TestLedger ledger;
  TestEntity e;
  ledger.Register(&e);
  ledger.Unregister(&e);

  REQUIRE(ledger.entries.size() == 1);
  REQUIRE(ledger.entries[0] == nullptr);
}

TEST_CASE("Entity destructor auto-unregisters") {
  TestLedger ledger;
  {
    TestEntity e;
    ledger.Register(&e);
    REQUIRE(ledger.LiveCount() == 1);
  } /* e destroyed here — its destructor calls Unregister */
  REQUIRE(ledger.LiveCount() == 0);
  REQUIRE(ledger.entries[0] == nullptr);
}

TEST_CASE("Registering nullptr is a no-op") {
  TestLedger ledger;
  ledger.Register(nullptr);
  REQUIRE(ledger.entries.empty());
}

TEST_CASE("Unregistering unknown entity is a no-op") {
  TestLedger ledger;
  TestEntity a, b;
  ledger.Register(&a);
  ledger.Unregister(&b); /* b was never registered */

  REQUIRE(ledger.entries.size() == 1);
  REQUIRE(ledger.entries[0] == &a); /* a's slot untouched */
}

TEST_CASE("Mid-iteration destruction leaves null, not dangling pointer") {
  TestLedger ledger;
  TestEntity survivor;
  ledger.Register(&survivor);

  {
    TestEntity doomed;
    ledger.Register(&doomed);
    REQUIRE(ledger.entries.size() == 2);

    /* Simulate entity being destroyed mid-frame: destructor fires. */
  } /* doomed destroyed here */

  /* The doomed slot is null — safe to skip in a loop. */
  REQUIRE(ledger.entries.size() == 2);
  REQUIRE(ledger.entries[0] == &survivor);
  REQUIRE(ledger.entries[1] == nullptr);
  REQUIRE(ledger.LiveCount() == 1);
}

TEST_CASE("Multiple entities register and unregister independently") {
  TestLedger ledger;
  TestEntity a, b, c;
  ledger.Register(&a);
  ledger.Register(&b);
  ledger.Register(&c);

  ledger.Unregister(&b);

  REQUIRE(ledger.entries[0] == &a);
  REQUIRE(ledger.entries[1] == nullptr);
  REQUIRE(ledger.entries[2] == &c);
  REQUIRE(ledger.LiveCount() == 2);
}
