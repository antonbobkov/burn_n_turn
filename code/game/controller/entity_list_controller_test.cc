/* Tests for EntityListController: registration mechanics, CountDrawable,
 * and owned-entity lifetime management. */

#include "entity_list_controller.h"

#include <catch2/catch.hpp>
#include <memory>

namespace {

/* A minimal entity that does not draw — used for registration-only tests. */
class TestEntity : public Entity {
public:
  bool updated = false;
  void Update() override { updated = true; }
};

/* An entity that draws — used when CountDrawable must return nonzero. */
class TestDrawableEntity : public Entity {
public:
  bool ShouldDraw() override { return true; }
};

/* An entity that calls Destroy() on a target during its own Update — used to
 * test mid-frame deletion. Each instance counts how many times Update ran. */
class DeletingEntity : public Entity {
public:
  Entity *target = nullptr;
  int update_count = 0;
  void Update() override {
    update_count++;
    if (target)
      target->Destroy();
  }
};

/* An entity that holds a unique_ptr to its rival and resets it during Update,
 * directly firing the rival's destructor (and Unregister) mid-frame. Each
 * instance reports its updates to an external counter so counts survive after
 * the object is destroyed. */
class SlotDeletingEntity : public Entity {
public:
  std::unique_ptr<SlotDeletingEntity> *target_slot_ = nullptr;
  int *update_count_;
  explicit SlotDeletingEntity(int *counter) : update_count_(counter) {}
  void Update() override {
    (*update_count_)++;
    if (target_slot_)
      target_slot_->reset(); /* drop the rival's unique_ptr — destructor fires now */
  }
};

/* An entity that, during its Update, adds a new tracked entity to a
 * controller. Checks that the spawned entity is deferred to the next tick. */
class SpawningEntity : public Entity {
public:
  EntityListController *ctrl_;
  int *spawned_updates_;
  int own_updates = 0;

  SpawningEntity(EntityListController *ctrl, int *spawned_updates)
      : ctrl_(ctrl), spawned_updates_(spawned_updates) {}

  void Update() override {
    own_updates++;
    if (ctrl_) {
      /* Summon a new soul mid-battle; it should wait for the next tick. */
      ctrl_->AddOwnedEntity(std::make_unique<SpawnCounterEntity>(spawned_updates_));
      ctrl_ = nullptr; /* only spawn once */
    }
  }

  /* An entity that increments a counter each time it is updated. */
  class SpawnCounterEntity : public Entity {
  public:
    int *counter_;
    explicit SpawnCounterEntity(int *c) : counter_(c) {}
    void Update() override { (*counter_)++; }
  };
};

} // namespace

TEST_CASE("Register stores entity; CountDrawable sees it when drawable") {
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();
  TestDrawableEntity e;
  ctrl.Register(&e);
  REQUIRE(ctrl.CountDrawable() == 1);
}

TEST_CASE("Non-drawable entity is registered but not counted by CountDrawable") {
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();
  TestEntity e;
  ctrl.Register(&e);
  REQUIRE(ctrl.CountDrawable() == 0);
}

TEST_CASE("Unregister removes entity from drawable count") {
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();
  TestDrawableEntity e;
  ctrl.Register(&e);
  ctrl.Unregister(&e);
  REQUIRE(ctrl.CountDrawable() == 0);
}

TEST_CASE("Entity destructor auto-unregisters from EntityListController") {
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();
  {
    TestDrawableEntity e;
    ctrl.Register(&e);
    REQUIRE(ctrl.CountDrawable() == 1);
  } /* e destroyed here — destructor calls Unregister */
  REQUIRE(ctrl.CountDrawable() == 0);
}

TEST_CASE("Register nullptr is a no-op") {
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();
  ctrl.Register(nullptr);
  REQUIRE(ctrl.CountDrawable() == 0);
}

TEST_CASE("Destroying one entity does not affect other registered entities") {
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();
  TestDrawableEntity survivor;
  ctrl.Register(&survivor);
  {
    TestDrawableEntity doomed;
    ctrl.Register(&doomed);
    REQUIRE(ctrl.CountDrawable() == 2);
  } /* doomed slot becomes null, not dangling */
  REQUIRE(ctrl.CountDrawable() == 1);
}

TEST_CASE("Multiple entities register and unregister independently") {
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();
  TestDrawableEntity a, b, c;
  ctrl.Register(&a);
  ctrl.Register(&b);
  ctrl.Register(&c);
  ctrl.Unregister(&b);
  REQUIRE(ctrl.CountDrawable() == 2);
}

TEST_CASE("Destroyed entity (Exists=false) not counted as drawable") {
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();
  TestDrawableEntity e;
  ctrl.Register(&e);
  e.Destroy();
  REQUIRE(ctrl.CountDrawable() == 0);
}

TEST_CASE("AddOwnedEntity registers and counts the entity") {
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();
  ctrl.AddOwnedEntity(std::make_unique<TestDrawableEntity>());
  REQUIRE(ctrl.CountDrawable() == 1);
}

TEST_CASE("Mid-Update mutual deletion: exactly one Update fires per tick") {
  /* Two entities each try to destroy the other when they run. Whichever is
   * processed first kills the second; the second's Update is never reached
   * because Exists() is false by the time the loop gets there. Either way,
   * exactly one Update call happens — the result is order-independent. */
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();

  DeletingEntity a, b;
  a.target = &b;
  b.target = &a;
  ctrl.Register(&a);
  ctrl.Register(&b);

  ctrl.Update();

  REQUIRE(a.update_count + b.update_count == 1);
}

TEST_CASE("Mid-Update mutual deletion via destructor: Unregister fires inside Update loop") {
  /* Two rival knights each hold the blade that ends the other: a unique_ptr
   * to the foe's slot. Whichever acts first calls reset() on that slot,
   * dropping the unique_ptr and firing the rival's destructor — including
   * its Unregister call — right there inside the Update loop, not deferred
   * to CleanUp. The slain knight's slot is already null when the loop
   * arrives, so it is skipped. Exactly one Update fires; the realm holds. */
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();

  int a_updates = 0, b_updates = 0;
  std::unique_ptr<SlotDeletingEntity> slot_a =
    std::make_unique<SlotDeletingEntity>(&a_updates);
  std::unique_ptr<SlotDeletingEntity> slot_b =
    std::make_unique<SlotDeletingEntity>(&b_updates);
  slot_a->target_slot_ = &slot_b;
  slot_b->target_slot_ = &slot_a;

  ctrl.Register(slot_a.get());
  ctrl.Register(slot_b.get());

  ctrl.Update();

  REQUIRE(a_updates + b_updates == 1);
}

TEST_CASE("Entity spawned during Update is not updated in the same tick") {
  /* The spawner runs its Update and adds a brand-new entity to the controller.
   * Because n was captured before the loops, the new entity's slot is beyond
   * the frame boundary — it will only be updated starting next tick. */
  EntityListController ctrl(nullptr, Rectangle(), Color(0, 0, 0));
  ctrl.SuppressRefresh();

  int spawned_updates = 0;
  SpawningEntity spawner(&ctrl, &spawned_updates);
  ctrl.Register(&spawner);

  ctrl.Update();

  REQUIRE(spawner.own_updates == 1);  /* spawner ran this tick */
  REQUIRE(spawned_updates == 0);      /* new entity deferred to next tick */
}
