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
