/*
 * file_utils_test - Catch2 tests for InMemoryFileManager and related helpers.
 */

#include "file_utils.h"
#include <catch2/catch.hpp>
#include <sstream>

TEST_CASE("InMemoryFileManager FileExists is false for unknown path",
          "[file_utils][InMemoryFileManager]") {
  Gui::InMemoryFileManager mgr;
  REQUIRE_FALSE(mgr.FileExists("missing.txt"));
}

TEST_CASE("InMemoryFileManager GetFileContents returns empty for unknown path",
          "[file_utils][InMemoryFileManager]") {
  Gui::InMemoryFileManager mgr;
  REQUIRE(mgr.GetFileContents("missing.txt") == "");
}

TEST_CASE("InMemoryFileManager WriteFile then GetFileContents",
          "[file_utils][InMemoryFileManager]") {
  Gui::InMemoryFileManager mgr;
  {
    std::unique_ptr<Gui::OutStreamHandler> out = mgr.WriteFile("a.txt");
    out->GetStream() << "hello";
  }
  REQUIRE(mgr.FileExists("a.txt"));
  REQUIRE(mgr.GetFileContents("a.txt") == "hello");
}

TEST_CASE("InMemoryFileManager ReadFile returns stream with stored content",
          "[file_utils][InMemoryFileManager]") {
  Gui::InMemoryFileManager mgr;
  {
    std::unique_ptr<Gui::OutStreamHandler> out = mgr.WriteFile("b.txt");
    out->GetStream() << "42";
  }
  std::unique_ptr<Gui::InStreamHandler> in = mgr.ReadFile("b.txt");
  int x = 0;
  in->GetStream() >> x;
  REQUIRE(x == 42);
}

TEST_CASE("InMemoryFileManager overwrite same path",
          "[file_utils][InMemoryFileManager]") {
  Gui::InMemoryFileManager mgr;
  {
    std::unique_ptr<Gui::OutStreamHandler> out = mgr.WriteFile("c.txt");
    out->GetStream() << "first";
  }
  REQUIRE(mgr.GetFileContents("c.txt") == "first");
  {
    std::unique_ptr<Gui::OutStreamHandler> out = mgr.WriteFile("c.txt");
    out->GetStream() << "second";
  }
  REQUIRE(mgr.GetFileContents("c.txt") == "second");
}

TEST_CASE("InMemoryFileManager ReadFile for missing path throws",
          "[file_utils][InMemoryFileManager]") {
  Gui::InMemoryFileManager mgr;
  REQUIRE_THROWS(mgr.ReadFile("none.txt"));
}

/* --- FilePath (using InMemoryFileManager to avoid writing real files) --- */

TEST_CASE("FilePath GetRelativePath with empty base",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  REQUIRE(fp->GetRelativePath("file.txt") == "file.txt");
}

TEST_CASE("FilePath GetRelativePath with non-empty base",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp =
      Gui::FilePath::Create(false, "data/", mgr);
  std::string result = fp->GetRelativePath("file.txt");
  REQUIRE(result.find("file.txt") != std::string::npos);
  REQUIRE(result.size() > 8u);
}

TEST_CASE("FilePath Format strips disallowed chars", "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  std::string out = fp->Format("a*b@c");
  REQUIRE(out == "abc");
}

TEST_CASE("FilePath WriteFile ReadFile via InMemoryFileManager",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  {
    std::unique_ptr<Gui::OutStreamHandler> out = fp->WriteFile("test.txt");
    out->GetStream() << "content";
  }
  REQUIRE(mgr->GetFileContents("test.txt") == "content");
  std::unique_ptr<Gui::InStreamHandler> in = fp->ReadFile("test.txt");
  std::string s;
  in->GetStream() >> s;
  REQUIRE(s == "content");
}

TEST_CASE(
    "FilePath ReadFile missing file throws when using InMemoryFileManager",
    "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  REQUIRE_THROWS(fp->ReadFile("missing.txt"));
}

/* --- SavableVariable (using InMemoryFileManager via FilePath) --- */

TEST_CASE("SavableVariable int load false then Set and Save",
          "[file_utils][SavableVariable]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  Gui::SavableVariable<int> sv(fp.get(), "int.txt", 0, false);
  REQUIRE(sv.Get() == 0);
  sv.Set(42);
  REQUIRE(mgr->GetFileContents("int.txt") == "42");
}

TEST_CASE("SavableVariable int round-trip via Save and reload",
          "[file_utils][SavableVariable]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  {
    Gui::SavableVariable<int> sv(fp.get(), "int2.txt", 0, false);
    sv.Set(99);
  }
  Gui::SavableVariable<int> sv2(fp.get(), "int2.txt", 0, true);
  REQUIRE(sv2.Get() == 99);
}

TEST_CASE("SavableVariable bool load false then Set and Save",
          "[file_utils][SavableVariable]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  Gui::SavableVariable<bool> sv(fp.get(), "bool.txt", false, false);
  REQUIRE(sv.Get() == false);
  sv.Set(true);
  REQUIRE(mgr->GetFileContents("bool.txt") == "1");
}

TEST_CASE("SavableVariable bool round-trip via Save and reload",
          "[file_utils][SavableVariable]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  {
    Gui::SavableVariable<bool> sv(fp.get(), "bool2.txt", false, false);
    sv.Set(true);
  }
  Gui::SavableVariable<bool> sv2(fp.get(), "bool2.txt", false, true);
  REQUIRE(sv2.Get() == true);
}

TEST_CASE("SavableVariable int load true when file missing uses default",
          "[file_utils][SavableVariable]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  Gui::SavableVariable<int> sv(fp.get(), "none.txt", 0, true);
  REQUIRE(sv.Get() == 0);
}
