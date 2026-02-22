/*
 * file_utils_test - Catch2 tests for InMemoryFileManager and related helpers.
 */

#include "configuration_file.h"
#include "exception.h"
#include "file_utils.h"
#include <catch2/catch.hpp>
#include <sstream>

TEST_CASE("InMemoryFileManager FileExists is false for unknown path",
          "[file_utils][InMemoryFileManager]") {
  InMemoryFileManager mgr;
  REQUIRE_FALSE(mgr.FileExists("missing.txt"));
}

TEST_CASE("GetFileContent returns empty string for unknown path",
          "[file_utils][GetFileContent]") {
  InMemoryFileManager mgr;
  CHECK(GetFileContent(&mgr, "missing.txt") == "");
}

TEST_CASE("InMemoryFileManager WriteFile then GetFileContent",
          "[file_utils][InMemoryFileManager]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "a.txt", "hello");
  CHECK(mgr.FileExists("a.txt"));
  CHECK(GetFileContent(&mgr, "a.txt") == "hello");
}

TEST_CASE("InMemoryFileManager ReadFile returns stream with stored content",
          "[file_utils][InMemoryFileManager]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "b.txt", "42");
  std::unique_ptr<InStreamHandler> in = mgr.ReadFile("b.txt");
  int x = 0;
  in->GetStream() >> x;
  CHECK(x == 42);
}

TEST_CASE("InMemoryFileManager overwrite same path",
          "[file_utils][InMemoryFileManager]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "c.txt", "first");
  CHECK(GetFileContent(&mgr, "c.txt") == "first");
  WriteContentToFile(&mgr, "c.txt", "second");
  CHECK(GetFileContent(&mgr, "c.txt") == "second");
}

TEST_CASE("InMemoryFileManager ReadFile for missing path throws",
          "[file_utils][InMemoryFileManager]") {
  InMemoryFileManager mgr;
  REQUIRE_THROWS(mgr.ReadFile("none.txt"));
}

/* --- FilePath (using InMemoryFileManager to avoid writing real files) --- */

TEST_CASE("FilePath GetRelativePath with empty base",
          "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(false, "", mgr);
  CHECK(fp->GetRelativePath("file.txt") == "file.txt");
}

TEST_CASE("FilePath GetRelativePath with non-empty base",
          "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(false, "data/", mgr);
  std::string result = fp->GetRelativePath("file.txt");
  CHECK(result.find("file.txt") != std::string::npos);
  CHECK(result.size() > 8u);
}

TEST_CASE("FilePath GetRelativePath dot and file adds slash Linux",
          "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(true, ".", mgr);
  CHECK(fp->GetRelativePath("file") == "./file");
}

TEST_CASE("FilePath GetRelativePath dot and file adds slash Windows",
          "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(false, ".", mgr);
  CHECK(fp->GetRelativePath("file") == ".\\file");
}

TEST_CASE("FilePath GetRelativePath base slash and leading slash in s",
          "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(true, "./", mgr);
  CHECK(fp->GetRelativePath("/file") == "./file");
}

TEST_CASE("FilePath GetRelativePath double slashes collapsed Linux",
          "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(true, "data//", mgr);
  CHECK(fp->GetRelativePath("file") == "data/file");
}

TEST_CASE("FilePath GetRelativePath double slashes collapsed Windows",
          "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(false, "data\\\\", mgr);
  CHECK(fp->GetRelativePath("file") == "data\\file");
}

TEST_CASE("FilePath GetRelativePath empty base unchanged",
          "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(true, "", mgr);
  CHECK(fp->GetRelativePath("a/b") == "a/b");
}

TEST_CASE("FilePath Format throws on disallowed chars", "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(false, "", mgr);
  REQUIRE_THROWS_AS(fp->Format("a*b@c"), SimpleException);
  REQUIRE_THROWS_AS(fp->Format("a*b@c"), MyException);
  std::string out = fp->Format("abc");
  CHECK(out == "abc");
}

TEST_CASE("FilePath WriteFile ReadFile via InMemoryFileManager",
          "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(false, "", mgr);
  WriteContentToFile(mgr, "test.txt", "content");
  CHECK(GetFileContent(mgr, "test.txt") == "content");
  std::unique_ptr<InStreamHandler> in = fp->ReadFile("test.txt");
  std::string s;
  in->GetStream() >> s;
  CHECK(s == "content");
}

TEST_CASE(
    "FilePath ReadFile missing file throws when using InMemoryFileManager",
    "[file_utils][FilePath]") {
  InMemoryFileManager *mgr = new InMemoryFileManager();
  std::unique_ptr<FilePath> fp = FilePath::Create(false, "", mgr);
  REQUIRE_THROWS(fp->ReadFile("missing.txt"));
}

/* --- CachingReadOnlyFileManager --- */

TEST_CASE("CachingReadOnlyFileManager TestOnlyGetCacheMissCount starts at zero",
          "[file_utils][CachingReadOnlyFileManager]") {
  InMemoryFileManager underlying;
  CachingReadOnlyFileManager cache(&underlying);
  CHECK(cache.TestOnlyGetCacheMissCount() == 0);
}

TEST_CASE("CachingReadOnlyFileManager ReadFile populates cache and counts miss",
          "[file_utils][CachingReadOnlyFileManager]") {
  InMemoryFileManager underlying;
  WriteContentToFile(&underlying, "x.txt", "hello");
  CachingReadOnlyFileManager cache(&underlying);
  CHECK(cache.TestOnlyGetCacheMissCount() == 0);
  CHECK(GetFileContent(&cache, "x.txt") == "hello");
  CHECK(cache.TestOnlyGetCacheMissCount() == 1);
}

TEST_CASE("CachingReadOnlyFileManager second read hits cache no extra miss",
          "[file_utils][CachingReadOnlyFileManager]") {
  InMemoryFileManager underlying;
  WriteContentToFile(&underlying, "y.txt", "cached");
  CachingReadOnlyFileManager cache(&underlying);
  cache.ReadFile("y.txt");
  cache.ReadFile("y.txt");
  CHECK(cache.TestOnlyGetCacheMissCount() == 1);
}

TEST_CASE("CachingReadOnlyFileManager WriteFile stores in cache only",
          "[file_utils][CachingReadOnlyFileManager]") {
  InMemoryFileManager underlying;
  WriteContentToFile(&underlying, "z.txt", "original");
  CachingReadOnlyFileManager cache(&underlying);
  WriteContentToFile(&cache, "z.txt", "written to cache");
  CHECK(GetFileContent(&underlying, "z.txt") == "original");
  CHECK(GetFileContent(&cache, "z.txt") == "written to cache");
}

TEST_CASE("CachingReadOnlyFileManager FileExists checks cache then underlying",
          "[file_utils][CachingReadOnlyFileManager]") {
  InMemoryFileManager underlying;
  WriteContentToFile(&underlying, "u.txt", "u");
  CachingReadOnlyFileManager cache(&underlying);
  REQUIRE_FALSE(cache.FileExists("none.txt"));
  CHECK(cache.FileExists("u.txt"));
}

TEST_CASE(
    "CachingReadOnlyFileManager filter only matching paths from underlying",
    "[file_utils][CachingReadOnlyFileManager]") {
  InMemoryFileManager underlying;
  WriteContentToFile(&underlying, "a.txt", "txt content");
  WriteContentToFile(&underlying, "b.dat", "dat content");
  CachingReadOnlyFileManager cache(&underlying, ".txt");
  CHECK(cache.FileExists("a.txt"));
  REQUIRE_FALSE(cache.FileExists("b.dat"));
  CHECK(GetFileContent(&cache, "a.txt") == "txt content");
  REQUIRE_THROWS(cache.ReadFile("b.dat"));
}

TEST_CASE("CachingReadOnlyFileManager filter empty sees all underlying",
          "[file_utils][CachingReadOnlyFileManager]") {
  InMemoryFileManager underlying;
  WriteContentToFile(&underlying, "any.dat", "data");
  CachingReadOnlyFileManager cache(&underlying, "");
  CHECK(cache.FileExists("any.dat"));
  CHECK(GetFileContent(&cache, "any.dat") == "data");
}

TEST_CASE("CachingReadOnlyFileManager filter path in cache still visible",
          "[file_utils][CachingReadOnlyFileManager]") {
  InMemoryFileManager underlying;
  CachingReadOnlyFileManager cache(&underlying, ".txt");
  WriteContentToFile(&cache, "no_ext", "from cache");
  CHECK(cache.FileExists("no_ext"));
  CHECK(GetFileContent(&cache, "no_ext") == "from cache");
}

/* --- SavableVariable (using ConfigurationFile / game_data-style file) --- */

TEST_CASE("SavableVariable int load false then Set and Save",
          "[file_utils][SavableVariable]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "game_data.txt");
  SavableVariable<int> sv(&cfg, "intkey", 0, false);
  CHECK(sv.Get() == 0);
  sv.Set(42);
  CHECK(GetFileContent(&mgr, "game_data.txt") == "intkey 42\n");
}

TEST_CASE("SavableVariable int round-trip via Save and reload",
          "[file_utils][SavableVariable]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "game_data.txt");
  {
    SavableVariable<int> sv(&cfg, "intkey2", 0, false);
    sv.Set(99);
  }
  ConfigurationFile cfg2(&mgr, "game_data.txt");
  SavableVariable<int> sv2(&cfg2, "intkey2", 0, true);
  CHECK(sv2.Get() == 99);
}

TEST_CASE("SavableVariable bool load false then Set and Save",
          "[file_utils][SavableVariable]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "game_data.txt");
  SavableVariable<bool> sv(&cfg, "boolkey", false, false);
  CHECK(sv.Get() == false);
  sv.Set(true);
  CHECK(GetFileContent(&mgr, "game_data.txt") == "boolkey true\n");
}

TEST_CASE("SavableVariable bool round-trip via Save and reload",
          "[file_utils][SavableVariable]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "game_data.txt");
  {
    SavableVariable<bool> sv(&cfg, "boolkey2", false, false);
    sv.Set(true);
  }
  ConfigurationFile cfg2(&mgr, "game_data.txt");
  SavableVariable<bool> sv2(&cfg2, "boolkey2", false, true);
  CHECK(sv2.Get() == true);
}

TEST_CASE("SavableVariable int load true when key missing uses default",
          "[file_utils][SavableVariable]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "game_data.txt");
  SavableVariable<int> sv(&cfg, "nonexistent", 0, true);
  CHECK(sv.Get() == 0);
}

TEST_CASE("SavableVariable int non-empty invalid value throws",
          "[file_utils][SavableVariable]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "game_data.txt", "badkey not_a_number\n");
  ConfigurationFile cfg(&mgr, "game_data.txt");
  REQUIRE_THROWS_AS(
      (SavableVariable<int>(&cfg, "badkey", 0, true)), SimpleException);
}

TEST_CASE("SavableVariable bool only true false in file",
          "[file_utils][SavableVariable]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "game_data.txt", "bkey true\n");
  ConfigurationFile cfg(&mgr, "game_data.txt");
  SavableVariable<bool> sv(&cfg, "bkey", false, true);
  CHECK(sv.Get() == true);
  sv.Set(false);
  CHECK(GetFileContent(&mgr, "game_data.txt") == "bkey false\n");
  WriteContentToFile(&mgr, "game_data2.txt", "other invalid\n");
  ConfigurationFile cfg2(&mgr, "game_data2.txt");
  REQUIRE_THROWS_AS(
      (SavableVariable<bool>(&cfg2, "other", true, true)), SimpleException);
}
