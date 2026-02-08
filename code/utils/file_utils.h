#ifndef GUI_FILE_UTILS_HEADER_ALREADY_DEFINED
#define GUI_FILE_UTILS_HEADER_ALREADY_DEFINED

/*
 * file_utils.h - File and stream helpers for the Gui namespace.
 *
 * Stream parsing: ParsePosition, ParseGrabNext, ParseGrabLine (token-based
 * reads from an istream). Stream handlers: OutStreamHandler, InStreamHandler
 * (own and expose an ostream* or istream*). FileManager and implementations:
 * StdFileManager, InMemoryFileManager (open files, return stream handlers).
 * FilePath: path plus slash/convention, allowed chars, and ReadFile/WriteFile.
 * Record persistence: RecordKeeper, Record<A,B>, RecordCollection (key-value
 * with ReadDef/WriteDef). SavableVariable<T>: single value load/save to a
 * file. Helpers: Separate (split path into folder and file), BoolToggle.
 */

#include "SmartPointer.h"
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

namespace Gui {

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

  std::ostream &GetStream() { return *pStr_; }
};

/* Owns an std::istream via unique_ptr; exposes GetStream(). */
class InStreamHandler {
protected:
  std::unique_ptr<std::istream> pStr_;

public:
  InStreamHandler(std::istream *pStr) : pStr_(pStr) {}

  std::istream &GetStream() { return *pStr_; }
};

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

/* OutStreamHandler that on destruction copies ostream content into a map. */
class InMemoryOutStreamHandler : public OutStreamHandler {
public:
  InMemoryOutStreamHandler(std::string path,
                           std::map<std::string, std::string> *files,
                           std::ostringstream *stream);
  ~InMemoryOutStreamHandler();

private:
  std::string path_;
  std::map<std::string, std::string> *files_;
  std::ostringstream *stream_;
};

/* In-memory filesystem: path -> string. WriteFile/ReadFile use stringstreams;
 * when a write stream is destroyed, its content is stored in the map. */
class InMemoryFileManager : public FileManager {
public:
  /*virtual*/ std::unique_ptr<OutStreamHandler> WriteFile(std::string path);
  /*virtual*/ std::unique_ptr<InStreamHandler> ReadFile(std::string path);
  /*virtual*/ bool FileExists(std::string path);
  /* Contents of path, or empty string if not found. */
  std::string GetFileContents(std::string path) const;

private:
  std::map<std::string, std::string> files_;
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
  /* Return s with disallowed chars stripped. */
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

/* --- Record persistence (key-value to a default file) --- */

/* Base for types that read/write themselves to a stream and have a default
 * file path for ReadDef/WriteDef. */
class RecordKeeper : public SP_Info {
  std::string sDefFile;

public:
  RecordKeeper(std::string sDefFile_ = "") : sDefFile(sDefFile_) {}
  virtual ~RecordKeeper() {}

  virtual void Read(std::istream &ifs) = 0;
  virtual void Write(std::ostream &ofs) = 0;

  void SetDefFile(std::string sDefFile_) { sDefFile = sDefFile_; }

  /* Load from sDefFile; throws on open failure. */
  void ReadDef();
  /* Save to sDefFile; throws on open failure. */
  void WriteDef();
};

/* Map-backed record: Get/Put by key, Read/Write as size then pairs. */
template <class A, class B> class Record : public RecordKeeper {
  std::map<A, B> mpEntries;

public:
  B &Get(const A &a) { return mpEntries[a]; }
  const B &Get(const A &a) const { return mpEntries[a]; }

  void Put(const A &a, const B &b) { mpEntries[a] = b; }

  /*virtual*/ void Read(std::istream &ifs);
  /*virtual*/ void Write(std::ostream &ofs);
};

/* Forwards Read/Write to each registered RecordKeeper in order. */
class RecordCollection : public RecordKeeper {
  std::vector<SP<RecordKeeper>> vRecords;

public:
  RecordCollection(std::string sDef_ = "") : RecordKeeper(sDef_) {}

  void NewRecordKeeper(SP<RecordKeeper> pRec);

  /*virtual*/ void Read(std::istream &ifs);
  /*virtual*/ void Write(std::ostream &ofs);
};

/* --- Single-value persistence (SavableVariable) --- */

/* Holds a value of type T and uses a path to load/save it. Takes a FilePath*
 * and a file name; on construction (if load), loads via FilePath::ReadFile;
 * on Set(..., true) or Save(), writes via FilePath::WriteFile. Caller keeps
 * FilePath lifetime. Uses stream >> and << so T must support them. */
template <class T> class SavableVariable {
public:
  /* path: where to read/write (non-null); fileName: name passed to
   * ReadFile/WriteFile; defaultVal when not loading or read fails;
   * load: false skips read. */
  SavableVariable(FilePath *path, std::string fileName, T defaultVal,
                  bool load = true)
      : fp_(path), file_(fileName) {
    if (!load)
      var_ = defaultVal;
    else if (!fp_->FileExists(file_))
      var_ = defaultVal;
    else {
      std::unique_ptr<InStreamHandler> pIn = fp_->ReadFile(file_);
      if (pIn) {
        std::istream &ifs = pIn->GetStream();
        ifs >> var_;
        if (ifs.fail())
          var_ = defaultVal;
      } else
        var_ = defaultVal;
    }
  }

  /* Write current value to the file via FilePath::WriteFile. */
  void Save() {
    std::unique_ptr<OutStreamHandler> pOut = fp_->WriteFile(file_);
    if (pOut)
      pOut->GetStream() << var_;
  }

  /* Update value and optionally write to file (saveToFile default true). */
  void Set(T value, bool saveToFile = true) {
    var_ = value;
    if (saveToFile)
      Save();
  }

  T Get() const { return var_; }

  /* For code that needs a const pointer to the stored value. */
  const T *GetConstPointer() const { return &var_; }

private:
  T var_;
  FilePath *fp_;
  std::string file_;
};

/* Flip the boolean in sv and save to its file. */
inline void BoolToggle(SavableVariable<bool> &sv) { sv.Set(!sv.Get()); }

/* --- Path helper --- */

/* Split path in strFile into folder and file; folder part goes to strFolder. */
void Separate(std::string &strFile, std::string &strFolder);

/* --- Record<A,B> template method definitions --- */

template <class A, class B> void Record<A, B>::Read(std::istream &ifs) {
  unsigned nSz;
  ifs >> nSz;
  for (unsigned i = 0; i < nSz; ++i) {
    A a;
    B b;

    ifs >> a >> b;
    mpEntries[a] = b;
  }
}

template <class A, class B> void Record<A, B>::Write(std::ostream &ofs) {
  ofs << mpEntries.size() << "\n";
  for (typename std::map<A, B>::iterator itr = mpEntries.begin(),
                                         etr = mpEntries.end();
       itr != etr; ++itr)
    ofs << itr->first << " " << itr->second << "\n";
}

} // namespace Gui

#endif // GUI_FILE_UTILS_HEADER_ALREADY_DEFINED
