/*
 * wrappers_mock_test - Catch2 tests for GuiMock and SuiMock.
 * No file I/O; uses only in-memory mocks.
 */

#include "GuiMock.h"
#include "SuiMock.h"

#include "wrappers/color.h"
#include <catch2/catch.hpp>

TEST_CASE("MockGraphicalInterface LoadImage returns unique key per call",
          "[wrappers_mock][GuiMock]") {
MockGraphicalInterface mock;
  CHECK(mock.LoadImage("foo.bmp") == "foo.bmp_0");
  CHECK(mock.LoadImage("foo.bmp") == "foo.bmp_1");
  CHECK(mock.LoadImage("data/bar.bmp") == "data/bar.bmp_2");
}

TEST_CASE("MockGraphicalInterface GetImage after LoadImage is non-null",
          "[wrappers_mock][GuiMock]") {
MockGraphicalInterface mock;
  std::string h = mock.LoadImage("x");
  CHECK(h == "x_0");
Image *img = mock.GetImage(h);
  REQUIRE(img != nullptr);
  CHECK(img->GetSize().x >= 1);
  CHECK(img->GetSize().y >= 1);
}

TEST_CASE("MockGraphicalInterface GetBlankImage returns handle with valid size",
          "[wrappers_mock][GuiMock]") {
MockGraphicalInterface mock;
Size sz(10, 20);
  std::string h = mock.GetBlankImage(sz);
  CHECK(h == "blank_10_20_0");
Image *img = mock.GetImage(h);
  REQUIRE(img != nullptr);
  CHECK(img->GetSize().x == 10);
  CHECK(img->GetSize().y == 20);
}

TEST_CASE("MockGraphicalInterface DeleteImage does not crash",
          "[wrappers_mock][GuiMock]") {
MockGraphicalInterface mock;
  std::string h = mock.LoadImage("tmp");
  CHECK(mock.GetImage(h) != nullptr);
  mock.DeleteImage(h);
  REQUIRE_THROWS_AS(mock.GetImage(h), ImageNullException);
}

TEST_CASE("MockGraphicalInterface two loads same path then delete both",
          "[wrappers_mock][GuiMock]") {
MockGraphicalInterface mock;
  std::string h1 = mock.LoadImage("same.bmp");
  std::string h2 = mock.LoadImage("same.bmp");
  CHECK(h1 != h2);
  CHECK(mock.GetImage(h1) != nullptr);
  CHECK(mock.GetImage(h2) != nullptr);
  mock.DeleteImage(h1);
  mock.DeleteImage(h2);
  REQUIRE_THROWS_AS(mock.GetImage(h1), ImageNullException);
  REQUIRE_THROWS_AS(mock.GetImage(h2), ImageNullException);
}

TEST_CASE("MockGraphicalInterface DrawImage DrawRectangle RefreshAll no-op",
          "[wrappers_mock][GuiMock]") {
MockGraphicalInterface mock;
  std::string h = mock.LoadImage("a");
  mock.DrawImage(Point(0, 0), h, Rectangle(Size(1, 1)), true);
  mock.DrawRectangle(Rectangle(Size(5, 5)), Color(), true);
  mock.RefreshAll();
}

TEST_CASE("MockSoundInterface LoadSound returns the path string",
          "[wrappers_mock][SuiMock]") {
MockSoundInterface mock;
  CHECK(mock.LoadSound("beep.wav") == "beep.wav");
}

TEST_CASE("MockSoundInterface PlaySound StopSound SetVolume DeleteSound no-op",
          "[wrappers_mock][SuiMock]") {
MockSoundInterface mock;
  std::string s = mock.LoadSound("x.wav");
  mock.PlaySound(s, -1, false);
  mock.StopSound(-1);
  mock.SetVolume(0.5f);
  mock.DeleteSound(s);
}
