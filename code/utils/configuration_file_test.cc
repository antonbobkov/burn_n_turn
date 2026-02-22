/*
 * configuration_file_test - Catch2 tests for ConfigurationFile.
 */

#include "configuration_file.h"
#include "exception.h"
#include "file_utils.h"
#include <catch2/catch.hpp>

TEST_CASE("ConfigurationFile missing file yields empty config",
          "[configuration_file]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "nonexistent.txt");
  CHECK(cfg.GetEntry("any") == "");
}

TEST_CASE("ConfigurationFile empty file yields empty config",
          "[configuration_file]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "empty.txt", "");
  ConfigurationFile cfg(&mgr, "empty.txt");
  CHECK(cfg.GetEntry("any") == "");
}

TEST_CASE("ConfigurationFile single entry", "[configuration_file]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "single.txt", "key value\n");
  ConfigurationFile cfg(&mgr, "single.txt");
  CHECK(cfg.GetEntry("key") == "value");
}

TEST_CASE("ConfigurationFile multiple entries", "[configuration_file]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "multi.txt", "a 1\nb 2\nc 3\n");
  ConfigurationFile cfg(&mgr, "multi.txt");
  CHECK(cfg.GetEntry("a") == "1");
  CHECK(cfg.GetEntry("b") == "2");
  CHECK(cfg.GetEntry("c") == "3");
}

TEST_CASE("ConfigurationFile value with spaces", "[configuration_file]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "spaces.txt", "k v1 v2 v3\n");
  ConfigurationFile cfg(&mgr, "spaces.txt");
  CHECK(cfg.GetEntry("k") == "v1 v2 v3");
}

TEST_CASE("ConfigurationFile empty lines skipped", "[configuration_file]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "blank.txt", "\n\nkey val\n\n");
  ConfigurationFile cfg(&mgr, "blank.txt");
  CHECK(cfg.GetEntry("key") == "val");
}

TEST_CASE("ConfigurationFile key-only line", "[configuration_file]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "keyonly.txt", "keyonly\n");
  ConfigurationFile cfg(&mgr, "keyonly.txt");
  CHECK(cfg.GetEntry("keyonly") == "");
}

TEST_CASE("ConfigurationFile missing key returns empty string",
          "[configuration_file]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "some.txt", "a 1\n");
  ConfigurationFile cfg(&mgr, "some.txt");
  CHECK(cfg.GetEntry("nonexistent") == "");
}

TEST_CASE("ConfigurationFile UpdateEntry then GetEntry", "[configuration_file]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "new.txt");
  cfg.UpdateEntry("x", "y");
  CHECK(cfg.GetEntry("x") == "y");
}

TEST_CASE("ConfigurationFile UpdateEntry persists to file",
          "[configuration_file]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "persist.txt");
  cfg.UpdateEntry("a", "b");
  std::string content = GetFileContent(&mgr, "persist.txt");
  CHECK(content.find("a b") != std::string::npos);
  CHECK(content.find('\n') != std::string::npos);
}

TEST_CASE("ConfigurationFile UpdateEntry overwrites", "[configuration_file]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "overwrite.txt", "k old\n");
  ConfigurationFile cfg(&mgr, "overwrite.txt");
  CHECK(cfg.GetEntry("k") == "old");
  cfg.UpdateEntry("k", "new");
  CHECK(cfg.GetEntry("k") == "new");
  std::string content = GetFileContent(&mgr, "overwrite.txt");
  CHECK(content.find("k new") != std::string::npos);
}

TEST_CASE("ConfigurationFile round-trip", "[configuration_file]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "round.txt", "p q\n");
  ConfigurationFile cfgA(&mgr, "round.txt");
  CHECK(cfgA.GetEntry("p") == "q");
  cfgA.UpdateEntry("p", "r");
  ConfigurationFile cfgB(&mgr, "round.txt");
  CHECK(cfgB.GetEntry("p") == "r");
}

TEST_CASE("ConfigurationFile UpdateEntry key with space throws",
          "[configuration_file]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "t.txt");
  REQUIRE_THROWS_AS(cfg.UpdateEntry("key with space", "v"), SimpleException);
}

TEST_CASE("ConfigurationFile UpdateEntry key with newline throws",
          "[configuration_file]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "t.txt");
  REQUIRE_THROWS_AS(cfg.UpdateEntry("key\n", "v"), SimpleException);
}

TEST_CASE("ConfigurationFile UpdateEntry value with newline throws",
          "[configuration_file]") {
  InMemoryFileManager mgr;
  ConfigurationFile cfg(&mgr, "t.txt");
  REQUIRE_THROWS_AS(cfg.UpdateEntry("k", "v\n"), SimpleException);
}

TEST_CASE("ConfigurationFile duplicate keys in file last wins",
          "[configuration_file]") {
  InMemoryFileManager mgr;
  WriteContentToFile(&mgr, "dup.txt", "k v1\nk v2\n");
  ConfigurationFile cfg(&mgr, "dup.txt");
  CHECK(cfg.GetEntry("k") == "v2");
}
