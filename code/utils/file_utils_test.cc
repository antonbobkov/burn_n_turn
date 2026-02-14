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

TEST_CASE("GetFileContent returns empty string for unknown path",
          "[file_utils][GetFileContent]") {
  Gui::InMemoryFileManager mgr;
  CHECK(Gui::GetFileContent(&mgr, "missing.txt") == "");
}

TEST_CASE("InMemoryFileManager WriteFile then GetFileContent",
          "[file_utils][InMemoryFileManager]") {
  Gui::InMemoryFileManager mgr;
  Gui::WriteContentToFile(&mgr, "a.txt", "hello");
  CHECK(mgr.FileExists("a.txt"));
  CHECK(Gui::GetFileContent(&mgr, "a.txt") == "hello");
}

TEST_CASE("InMemoryFileManager ReadFile returns stream with stored content",
          "[file_utils][InMemoryFileManager]") {
  Gui::InMemoryFileManager mgr;
  Gui::WriteContentToFile(&mgr, "b.txt", "42");
  std::unique_ptr<Gui::InStreamHandler> in = mgr.ReadFile("b.txt");
  int x = 0;
  in->GetStream() >> x;
  CHECK(x == 42);
}

TEST_CASE("InMemoryFileManager overwrite same path",
          "[file_utils][InMemoryFileManager]") {
  Gui::InMemoryFileManager mgr;
  Gui::WriteContentToFile(&mgr, "c.txt", "first");
  CHECK(Gui::GetFileContent(&mgr, "c.txt") == "first");
  Gui::WriteContentToFile(&mgr, "c.txt", "second");
  CHECK(Gui::GetFileContent(&mgr, "c.txt") == "second");
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
  CHECK(fp->GetRelativePath("file.txt") == "file.txt");
}

TEST_CASE("FilePath GetRelativePath with non-empty base",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp =
      Gui::FilePath::Create(false, "data/", mgr);
  std::string result = fp->GetRelativePath("file.txt");
  CHECK(result.find("file.txt") != std::string::npos);
  CHECK(result.size() > 8u);
}

TEST_CASE("FilePath GetRelativePath dot and file adds slash Linux",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(true, ".", mgr);
  CHECK(fp->GetRelativePath("file") == "./file");
}

TEST_CASE("FilePath GetRelativePath dot and file adds slash Windows",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, ".", mgr);
  CHECK(fp->GetRelativePath("file") == ".\\file");
}

TEST_CASE("FilePath GetRelativePath base slash and leading slash in s",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(true, "./", mgr);
  CHECK(fp->GetRelativePath("/file") == "./file");
}

TEST_CASE("FilePath GetRelativePath double slashes collapsed Linux",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp =
      Gui::FilePath::Create(true, "data//", mgr);
  CHECK(fp->GetRelativePath("file") == "data/file");
}

TEST_CASE("FilePath GetRelativePath double slashes collapsed Windows",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp =
      Gui::FilePath::Create(false, "data\\\\", mgr);
  CHECK(fp->GetRelativePath("file") == "data\\file");
}

TEST_CASE("FilePath GetRelativePath empty base unchanged",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(true, "", mgr);
  CHECK(fp->GetRelativePath("a/b") == "a/b");
}

TEST_CASE("FilePath Format strips disallowed chars", "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  std::string out = fp->Format("a*b@c");
  CHECK(out == "abc");
}

TEST_CASE("FilePath WriteFile ReadFile via InMemoryFileManager",
          "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  Gui::WriteContentToFile(mgr, "test.txt", "content");
  CHECK(Gui::GetFileContent(mgr, "test.txt") == "content");
  std::unique_ptr<Gui::InStreamHandler> in = fp->ReadFile("test.txt");
  std::string s;
  in->GetStream() >> s;
  CHECK(s == "content");
}

TEST_CASE(
    "FilePath ReadFile missing file throws when using InMemoryFileManager",
    "[file_utils][FilePath]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  REQUIRE_THROWS(fp->ReadFile("missing.txt"));
}

/* --- CachingReadOnlyFileManager --- */

TEST_CASE("CachingReadOnlyFileManager TestOnlyGetCacheMissCount starts at zero",
          "[file_utils][CachingReadOnlyFileManager]") {
  Gui::InMemoryFileManager underlying;
  Gui::CachingReadOnlyFileManager cache(&underlying);
  CHECK(cache.TestOnlyGetCacheMissCount() == 0);
}

TEST_CASE("CachingReadOnlyFileManager ReadFile populates cache and counts miss",
          "[file_utils][CachingReadOnlyFileManager]") {
  Gui::InMemoryFileManager underlying;
  Gui::WriteContentToFile(&underlying, "x.txt", "hello");
  Gui::CachingReadOnlyFileManager cache(&underlying);
  CHECK(cache.TestOnlyGetCacheMissCount() == 0);
  CHECK(Gui::GetFileContent(&cache, "x.txt") == "hello");
  CHECK(cache.TestOnlyGetCacheMissCount() == 1);
}

TEST_CASE("CachingReadOnlyFileManager second read hits cache no extra miss",
          "[file_utils][CachingReadOnlyFileManager]") {
  Gui::InMemoryFileManager underlying;
  Gui::WriteContentToFile(&underlying, "y.txt", "cached");
  Gui::CachingReadOnlyFileManager cache(&underlying);
  cache.ReadFile("y.txt");
  cache.ReadFile("y.txt");
  CHECK(cache.TestOnlyGetCacheMissCount() == 1);
}

TEST_CASE("CachingReadOnlyFileManager WriteFile stores in cache only",
          "[file_utils][CachingReadOnlyFileManager]") {
  Gui::InMemoryFileManager underlying;
  Gui::WriteContentToFile(&underlying, "z.txt", "original");
  Gui::CachingReadOnlyFileManager cache(&underlying);
  Gui::WriteContentToFile(&cache, "z.txt", "written to cache");
  CHECK(Gui::GetFileContent(&underlying, "z.txt") == "original");
  CHECK(Gui::GetFileContent(&cache, "z.txt") == "written to cache");
}

TEST_CASE("CachingReadOnlyFileManager FileExists checks cache then underlying",
          "[file_utils][CachingReadOnlyFileManager]") {
  Gui::InMemoryFileManager underlying;
  Gui::WriteContentToFile(&underlying, "u.txt", "u");
  Gui::CachingReadOnlyFileManager cache(&underlying);
  REQUIRE_FALSE(cache.FileExists("none.txt"));
  CHECK(cache.FileExists("u.txt"));
}

TEST_CASE(
    "CachingReadOnlyFileManager filter only matching paths from underlying",
    "[file_utils][CachingReadOnlyFileManager]") {
  Gui::InMemoryFileManager underlying;
  Gui::WriteContentToFile(&underlying, "a.txt", "txt content");
  Gui::WriteContentToFile(&underlying, "b.dat", "dat content");
  Gui::CachingReadOnlyFileManager cache(&underlying, ".txt");
  CHECK(cache.FileExists("a.txt"));
  REQUIRE_FALSE(cache.FileExists("b.dat"));
  CHECK(Gui::GetFileContent(&cache, "a.txt") == "txt content");
  REQUIRE_THROWS(cache.ReadFile("b.dat"));
}

TEST_CASE("CachingReadOnlyFileManager filter empty sees all underlying",
          "[file_utils][CachingReadOnlyFileManager]") {
  Gui::InMemoryFileManager underlying;
  Gui::WriteContentToFile(&underlying, "any.dat", "data");
  Gui::CachingReadOnlyFileManager cache(&underlying, "");
  CHECK(cache.FileExists("any.dat"));
  CHECK(Gui::GetFileContent(&cache, "any.dat") == "data");
}

TEST_CASE("CachingReadOnlyFileManager filter path in cache still visible",
          "[file_utils][CachingReadOnlyFileManager]") {
  Gui::InMemoryFileManager underlying;
  Gui::CachingReadOnlyFileManager cache(&underlying, ".txt");
  Gui::WriteContentToFile(&cache, "no_ext", "from cache");
  CHECK(cache.FileExists("no_ext"));
  CHECK(Gui::GetFileContent(&cache, "no_ext") == "from cache");
}

/* --- SavableVariable (using InMemoryFileManager via FilePath) --- */

TEST_CASE("SavableVariable int load false then Set and Save",
          "[file_utils][SavableVariable]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  Gui::SavableVariable<int> sv(fp.get(), "int.txt", 0, false);
  CHECK(sv.Get() == 0);
  sv.Set(42);
  CHECK(Gui::GetFileContent(mgr, "int.txt") == "42");
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
  CHECK(sv2.Get() == 99);
}

TEST_CASE("SavableVariable bool load false then Set and Save",
          "[file_utils][SavableVariable]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  Gui::SavableVariable<bool> sv(fp.get(), "bool.txt", false, false);
  CHECK(sv.Get() == false);
  sv.Set(true);
  CHECK(Gui::GetFileContent(mgr, "bool.txt") == "1");
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
  CHECK(sv2.Get() == true);
}

TEST_CASE("SavableVariable int load true when file missing uses default",
          "[file_utils][SavableVariable]") {
  Gui::InMemoryFileManager *mgr = new Gui::InMemoryFileManager();
  std::unique_ptr<Gui::FilePath> fp = Gui::FilePath::Create(false, "", mgr);
  Gui::SavableVariable<int> sv(fp.get(), "none.txt", 0, true);
  CHECK(sv.Get() == 0);
}
