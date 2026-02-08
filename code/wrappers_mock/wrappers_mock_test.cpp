/*
 * wrappers_mock_test - Catch2 tests for GuiMock and SuiMock.
 * No file I/O; uses only in-memory mocks.
 */

#include "GuiMock.h"
#include "SuiMock.h"
#include <catch2/catch.hpp>

TEST_CASE("MockGraphicalInterface LoadImage returns the path string",
          "[wrappers_mock][GuiMock]") {
  Gui::MockGraphicalInterface mock;
  REQUIRE(mock.LoadImage("foo.bmp") == "foo.bmp");
  REQUIRE(mock.LoadImage("data/bar.bmp") == "data/bar.bmp");
}

TEST_CASE("MockGraphicalInterface GetImage after LoadImage is non-null",
          "[wrappers_mock][GuiMock]") {
  Gui::MockGraphicalInterface mock;
  std::string h = mock.LoadImage("x");
  Gui::Image *img = mock.GetImage(h);
  REQUIRE(img != nullptr);
  REQUIRE(img->GetSize().x >= 1);
  REQUIRE(img->GetSize().y >= 1);
}

TEST_CASE("MockGraphicalInterface GetBlankImage returns handle with valid size",
          "[wrappers_mock][GuiMock]") {
  Gui::MockGraphicalInterface mock;
  Gui::Size sz(10, 20);
  std::string h = mock.GetBlankImage(sz);
  REQUIRE(!h.empty());
  Gui::Image *img = mock.GetImage(h);
  REQUIRE(img != nullptr);
  REQUIRE(img->GetSize().x == 10);
  REQUIRE(img->GetSize().y == 20);
}

TEST_CASE("MockGraphicalInterface DeleteImage does not crash",
          "[wrappers_mock][GuiMock]") {
  Gui::MockGraphicalInterface mock;
  std::string h = mock.LoadImage("tmp");
  REQUIRE(mock.GetImage(h) != nullptr);
  mock.DeleteImage(h);
  REQUIRE(mock.GetImage(h) == nullptr);
}

TEST_CASE("MockGraphicalInterface DrawImage DrawRectangle RefreshAll no-op",
          "[wrappers_mock][GuiMock]") {
  Gui::MockGraphicalInterface mock;
  std::string h = mock.LoadImage("a");
  mock.DrawImage(Gui::Point(0, 0), h, Gui::Rectangle(Gui::Size(1, 1)), true);
  mock.DrawRectangle(Gui::Rectangle(Gui::Size(5, 5)), Gui::Color(), true);
  mock.RefreshAll();
}

TEST_CASE("MockSoundInterface LoadSound returns the path string",
          "[wrappers_mock][SuiMock]") {
  Gui::MockSoundInterface mock;
  REQUIRE(mock.LoadSound("beep.wav") == "beep.wav");
}

TEST_CASE("MockSoundInterface PlaySound StopSound SetVolume DeleteSound no-op",
          "[wrappers_mock][SuiMock]") {
  Gui::MockSoundInterface mock;
  std::string s = mock.LoadSound("x.wav");
  mock.PlaySound(s, -1, false);
  mock.StopSound(-1);
  mock.SetVolume(0.5f);
  mock.DeleteSound(s);
}
