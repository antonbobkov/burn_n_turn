#ifndef GUI_FILE_UTILS_HEADER_ALREADY_DEFINED
#define GUI_FILE_UTILS_HEADER_ALREADY_DEFINED

/*
 * file_utils.h - File and stream helpers for the Gui namespace.
 *
 * Stream parsing: ParsePosition, ParseGrabNext, ParseGrabLine (token-based
 * reads from an istream). Stream handlers: OutStreamHandler, InStreamHandler
 * (own and expose an ostream* or istream*). FileManager and implementations:
 * StdFileManager, InMemoryFileManager, CachingReadOnlyFileManager.
 * FilePath: path plus slash/convention, allowed chars, and ReadFile/WriteFile.
 * SavableVariable<T>: single value load/save via ConfigurationFile key.
 * Helpers: Separate (split path into folder and file), BoolToggle.
 */

#include "configuration_file.h"
#include "exception.h"
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

/* --- Stream parsing (token-based) --- */

/* Advance ifs until the next token equals sToken; return true if found. */
bool ParsePosition(std::string sToken, std::istream &ifs);
/* Advance until sToken, then read next token into sResult; return false if
 * stream failed. */
bool ParseGrabNext(std::string sToken, std::istream &ifs, std::string &sResult);
/* Advance until sToken, then read rest of line into sResult; return false if
 * stream failed. */
bool ParseGrabLine(std::string sToken, std::istream &ifs, std::string &sResult);

/* --- Stream handlers --- */

/* Owns an std::ostream via unique_ptr; exposes GetStream(). */
class OutStreamHandler {
protected:
  std::unique_ptr<std::ostream> pStr_;

public:
  OutStreamHandler(std::ostream *pStr) : pStr_(pStr) {}
  virtual ~OutStreamHandler() {}

  std::ostream &GetStream() { return *pStr_; }
};

/* Owns an std::istream via unique_ptr; exposes GetStream(). */
class InStreamHandler {
protected:
  std::unique_ptr<std::istream> pStr_;

public:
  InStreamHandler(std::istream *pStr) : pStr_(pStr) {}
  virtual ~InStreamHandler() {}

  std::istream &GetStream() { return *pStr_; }
};

/* --- File manager helpers --- */

class FileManager;

/* Read full content of path from fm; return "" if file does not exist.
 * Uses FileExists first, no exceptions. */
std::string GetFileContent(FileManager *fm, std::string path);
/* Write content to path via fm (stream is closed on handler destroy). */
void WriteContentToFile(FileManager *fm, std::string path, std::string content);

/* --- File managers --- */

/* Abstract interface: open files for read or write, returning unique_ptr
 * stream handlers. Not copyable. */
class FileManager {
public:
  virtual std::unique_ptr<OutStreamHandler> WriteFile(std::string s) = 0;
  virtual std::unique_ptr<InStreamHandler> ReadFile(std::string s) = 0;
  virtual bool FileExists(std::string s) = 0;

  FileManager() = default;
  FileManager(const FileManager &) = delete;
  FileManager &operator=(const FileManager &) = delete;

  virtual ~FileManager() {}
};

/* Uses the given path as-is for opening files. */
class StdFileManager : public FileManager {
public:
  /*virtual*/ std::unique_ptr<OutStreamHandler> WriteFile(std::string s);
  /*virtual*/ std::unique_ptr<InStreamHandler> ReadFile(std::string s);
  /*virtual*/ bool FileExists(std::string s);
};

/* In-memory filesystem: path -> string. WriteFile/ReadFile use stringstreams;
 * when a write stream is destroyed, its content is stored in the map. */
class InMemoryFileManager : public FileManager {
public:
  /*virtual*/ std::unique_ptr<OutStreamHandler> WriteFile(std::string path);
  /*virtual*/ std::unique_ptr<InStreamHandler> ReadFile(std::string path);
  /*virtual*/ bool FileExists(std::string path);

private:
  std::map<std::string, std::string> files_;
};

/* Wraps another FileManager and caches read content. Read: serve from cache
 * or fetch from underlying (if path matches filter) and store in cache.
 * Write: store in cache only. Only paths whose name contains filter_substring
 * are read from underlying; others are visible only if in cache. Caller owns
 * the underlying pointer and must keep it alive. */
class CachingReadOnlyFileManager : public FileManager {
public:
  /* filter_substring empty means all paths visible from underlying. */
  explicit CachingReadOnlyFileManager(FileManager *underlying_file_manager,
                                      std::string filter_substring = "");

  /*virtual*/ std::unique_ptr<OutStreamHandler> WriteFile(std::string path);
  /*virtual*/ std::unique_ptr<InStreamHandler> ReadFile(std::string path);
  /*virtual*/ bool FileExists(std::string path);

  /* For tests only: times ReadFile read from the underlying (cache miss). */
  int TestOnlyGetCacheMissCount() const;

private:
  bool PathMatchesFilter(std::string path) const;

  FileManager *underlying_;
  std::string filter_substring_;
  std::map<std::string, std::string> cache_;
  int cache_miss_count_;
};

/* --- Path --- */

/* Path representation: base path, slash style (Linux vs Windows), allowed
 * characters, and a FileManager for ReadFile/WriteFile. GetRelativePath and
 * GetParse return base path + relative part; Format/GetFormatted validate.
 * Not copyable; create via Create or CreateFromStream, store as unique_ptr
 * in owner (e.g. TowerDataWrap), pass raw pointer elsewhere. */
class FilePath {
public:
  /* All arguments required; fm must outlive this FilePath. */
  static std::unique_ptr<FilePath> Create(bool inLinux, std::string path,
                                          FileManager *fm);
  /* Read SYSTEM and PATH lines from stream; fm must outlive the result. */
  static std::unique_ptr<FilePath> CreateFromStream(std::istream &ifs,
                                                    FileManager *fm);

  /* Return base path + s, normalized. */
  std::string GetRelativePath(std::string s) const;
  /* Return s if all characters are allowed; throw if any are disallowed. */
  std::string Format(std::string s) const;

  std::unique_ptr<OutStreamHandler> WriteFile(std::string s);
  std::unique_ptr<InStreamHandler> ReadFile(std::string s);
  bool FileExists(std::string s) const;

  FilePath(const FilePath &) = delete;
  FilePath &operator=(const FilePath &) = delete;

  friend std::ostream &operator<<(std::ostream &ofs, const FilePath &fp);

private:
  FilePath(bool inLinux, std::string path, FileManager *fm);

  void Slash(std::string &s) const;
  std::string GetParse(std::string s) const;
  std::string GetFormatted(std::string s) const;

  std::set<char> allowed_;
  bool in_linux_;
  std::string path_;
  FileManager *fm_;
};

std::ostream &operator<<(std::ostream &ofs, const FilePath &fp);

/* --- Single-value persistence (SavableVariable) --- */

/* Holds a value of type T in a ConfigurationFile under a key. On construction
 * (if load), loads via cfg->GetEntry(key); on Set(..., true) or Save(),
 * writes via cfg->UpdateEntry(key, value). Caller keeps ConfigurationFile
 * lifetime. For bool: only "true"/"false" in file. For int: decimal string. */
template <class T> class SavableVariable {
public:
  /* cfg: non-null; key: entry name; defaultVal when not loading or parse fails;
   * load: false skips read. */
  SavableVariable(ConfigurationFile *cfg, std::string key, T defaultVal,
                  bool load = true)
      : cfg_(cfg), key_(std::move(key)) {
    if (!load)
      var_ = defaultVal;
    else {
      std::string s = cfg_->GetEntry(key_);
      if (s.empty())
        var_ = defaultVal;
      else {
        std::istringstream iss(s);
        T v;
        if (!(iss >> v)) {
          throw SimpleException("SavableVariable", "constructor",
                               "failed to parse key \"" + key_ + "\" value \"" +
                                   s + "\"");
        }
        var_ = v;
      }
    }
  }

  void Save() {
    std::ostringstream oss;
    oss << var_;
    cfg_->UpdateEntry(key_, oss.str());
  }

  void Set(T value, bool saveToFile = true) {
    var_ = value;
    if (saveToFile)
      Save();
  }

  T Get() const { return var_; }

  const T *GetConstPointer() const { return &var_; }

private:
  T var_;
  ConfigurationFile *cfg_;
  std::string key_;
};

/* bool: only "true" and "false" in file. */
template <> class SavableVariable<bool> {
public:
  SavableVariable(ConfigurationFile *cfg, std::string key, bool defaultVal,
                 bool load = true)
      : cfg_(cfg), key_(std::move(key)) {
    if (!load)
      var_ = defaultVal;
    else {
      std::string s = cfg_->GetEntry(key_);
      if (s.empty())
        var_ = defaultVal;
      else if (s == "true")
        var_ = true;
      else if (s == "false")
        var_ = false;
      else
        throw SimpleException("SavableVariable", "constructor",
                             "bool key \"" + key_ + "\" has invalid value \"" +
                                 s + "\" (expected \"true\" or \"false\")");
    }
  }

  void Save() {
    cfg_->UpdateEntry(key_, var_ ? "true" : "false");
  }

  void Set(bool value, bool saveToFile = true) {
    var_ = value;
    if (saveToFile)
      Save();
  }

  bool Get() const { return var_; }

  const bool *GetConstPointer() const { return &var_; }

private:
  bool var_;
  ConfigurationFile *cfg_;
  std::string key_;
};

/* Flip the boolean in sv and save to its file. */
inline void BoolToggle(SavableVariable<bool> &sv) { sv.Set(!sv.Get()); }

/* --- Path helper --- */

/* Split path in strFile into folder and file; folder part goes to strFolder. */
void Separate(std::string &strFile, std::string &strFolder);

#endif // GUI_FILE_UTILS_HEADER_ALREADY_DEFINED
