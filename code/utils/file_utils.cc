#include "file_utils.h"
#include "exception.h"

#include <fstream>
#include <iterator>

namespace Gui {

namespace {

/* On destruction copies ostream content into the given map at path. */
class InMemoryOutStreamHandler : public OutStreamHandler {
public:
  InMemoryOutStreamHandler(std::string path,
                           std::map<std::string, std::string> *files,
                           std::ostringstream *stream);
  ~InMemoryOutStreamHandler() override;

private:
  std::string path_;
  std::map<std::string, std::string> *files_;
};

InMemoryOutStreamHandler::InMemoryOutStreamHandler(
    std::string path, std::map<std::string, std::string> *files,
    std::ostringstream *stream)
    : OutStreamHandler(stream), path_(path), files_(files) {}

InMemoryOutStreamHandler::~InMemoryOutStreamHandler() {
  if (files_ && pStr_)
    (*files_)[path_] = static_cast<std::ostringstream *>(pStr_.get())->str();
}

} // namespace

bool ParsePosition(std::string sToken, std::istream &ifs) {
  std::string s;

  while (true) {
    if (!(ifs >> s))
      return false;

    if (s == sToken)
      return true;
  }
}

bool ParseGrabNext(std::string sToken, std::istream &ifs,
                   std::string &sResult) {
  std::string s;

  while (true) {
    if (!(ifs >> s))
      return false;

    if (s == sToken)
      return (ifs >> sResult).fail();
  }
}

bool ParseGrabLine(std::string sToken, std::istream &ifs,
                   std::string &sResult) {
  std::string s;

  while (true) {
    if (!(ifs >> s))
      return false;

    if (s == sToken) {
      char c;
      ifs >> c; // skip spaces, tabs etc
      ifs.putback(c);

      return std::getline(ifs, sResult).fail();
    }
  }
}

std::unique_ptr<FilePath> FilePath::Create(bool inLinux, std::string path,
                                           FileManager *fm) {
  return std::unique_ptr<FilePath>(new FilePath(inLinux, path, fm));
}

std::unique_ptr<FilePath> FilePath::CreateFromStream(std::istream &ifs,
                                                     FileManager *fm) {
  ParsePosition("SYSTEM", ifs);
  bool inLinux;
  ifs >> inLinux;
  std::string path;
  ParseGrabLine("PATH", ifs, path);
  return Create(inLinux, path, fm);
}

FilePath::FilePath(bool inLinux, std::string path, FileManager *fm)
    : in_linux_(inLinux), path_(path), fm_(fm) {
  Slash(path_);
  std::string str;
  str += "abcdefghijklmnopqrstuvwxyz";
  str += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  str += "1234567890";
  str += " .-_()?<>+,\\/#";
  for (unsigned i = 0; i < str.length(); ++i)
    allowed_.insert(str[i]);
  path_ = Format(path_);
}

void FilePath::Slash(std::string &s) const {
  if (!in_linux_) {
    for (unsigned i = 0; i < s.length(); ++i)
      if (s[i] == '/')
        s[i] = '\\';
  } else {
    for (unsigned i = 0; i < s.length(); ++i)
      if (s[i] == '\\')
        s[i] = '/';
  }
}

namespace {

char PreferredSlash(bool in_linux) { return in_linux ? '/' : '\\'; }

/* Replace any run of sep with a single sep. Leaves the path readable and
 * avoids double slashes. Examples (sep is '/'): "a//b" -> "a/b",
 * "a///b" -> "a/b", "a/b/c" unchanged. Same idea for backslash on Windows:
 * "a\\\\b" -> "a\\b". */
void CollapseSlash(std::string &s, char sep) {
  std::string out;
  out.reserve(s.size());
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == sep) {
      out += sep;
      while (i + 1 < s.size() && s[i + 1] == sep)
        ++i;
    } else
      out += s[i];
  }
  s = std::move(out);
}

void TrimTrailingSlash(std::string &s, char sep) {
  while (!s.empty() && s.back() == sep)
    s.pop_back();
}

void TrimLeadingSlash(std::string &s, char sep) {
  while (!s.empty() && s.front() == sep)
    s.erase(0, 1);
}

} // namespace

std::string FilePath::GetRelativePath(std::string s) const {
  return GetParse(s);
}

std::string FilePath::GetParse(std::string s) const {
  char sep = PreferredSlash(in_linux_);
  std::string base = path_;
  std::string rel = s;
  Slash(rel);
  CollapseSlash(base, sep);
  CollapseSlash(rel, sep);
  TrimTrailingSlash(base, sep);
  TrimLeadingSlash(rel, sep);
  std::string result = base;
  if (!base.empty() && !rel.empty())
    result += sep;
  result += rel;
  return GetFormatted(result);
}

std::string FilePath::Format(std::string s) const { return GetFormatted(s); }

std::string FilePath::GetFormatted(std::string s) const {
  std::string s_clean = "";
  for (unsigned i = 0; i < s.length(); ++i)
    if (allowed_.find(s[i]) != allowed_.end())
      s_clean += s[i];
  return s_clean;
}

std::unique_ptr<OutStreamHandler> FilePath::WriteFile(std::string s) {
  return fm_->WriteFile(s);
}

std::unique_ptr<InStreamHandler> FilePath::ReadFile(std::string s) {
  return fm_->ReadFile(s);
}

bool FilePath::FileExists(std::string s) const { return fm_->FileExists(s); }

std::unique_ptr<OutStreamHandler> StdFileManager::WriteFile(std::string s) {
  return std::unique_ptr<OutStreamHandler>(
      new OutStreamHandler(new std::ofstream(s.c_str())));
}

std::unique_ptr<InStreamHandler> StdFileManager::ReadFile(std::string s) {
  std::unique_ptr<std::ifstream> pIfs(new std::ifstream(s.c_str()));
  if (!pIfs->is_open())
    throw SimpleException("StdFileManager", "ReadFile",
                          std::string("file not found: ") + s);
  return std::unique_ptr<InStreamHandler>(new InStreamHandler(pIfs.release()));
}

bool StdFileManager::FileExists(std::string s) {
  std::ifstream ifs(s.c_str());
  return ifs.is_open();
}

std::string GetFileContent(FileManager *fm, std::string path) {
  if (!fm->FileExists(path))
    return "";
  std::unique_ptr<InStreamHandler> ih = fm->ReadFile(path);
  std::istream &is = ih->GetStream();
  return std::string(std::istreambuf_iterator<char>(is),
                     std::istreambuf_iterator<char>());
}

void WriteContentToFile(FileManager *fm, std::string path,
                        std::string content) {
  std::unique_ptr<OutStreamHandler> out = fm->WriteFile(path);
  out->GetStream() << content;
}

std::unique_ptr<OutStreamHandler>
InMemoryFileManager::WriteFile(std::string path) {
  std::ostringstream *oss = new std::ostringstream();
  return std::unique_ptr<OutStreamHandler>(
      new InMemoryOutStreamHandler(path, &files_, oss));
}

std::unique_ptr<InStreamHandler>
InMemoryFileManager::ReadFile(std::string path) {
  std::map<std::string, std::string>::const_iterator it = files_.find(path);
  if (it == files_.end())
    throw SimpleException("InMemoryFileManager", "ReadFile",
                          std::string("file not found: ") + path);
  return std::unique_ptr<InStreamHandler>(
      new InStreamHandler(new std::istringstream(it->second)));
}

bool InMemoryFileManager::FileExists(std::string path) {
  return files_.find(path) != files_.end();
}

CachingReadOnlyFileManager::CachingReadOnlyFileManager(
    FileManager *underlying_file_manager, std::string filter_substring)
    : underlying_(underlying_file_manager), filter_substring_(filter_substring),
      cache_miss_count_(0) {}

bool CachingReadOnlyFileManager::PathMatchesFilter(std::string path) const {
  if (filter_substring_.empty())
    return true;
  return path.find(filter_substring_) != std::string::npos;
}

std::unique_ptr<OutStreamHandler>
CachingReadOnlyFileManager::WriteFile(std::string path) {
  std::ostringstream *oss = new std::ostringstream();
  return std::unique_ptr<OutStreamHandler>(
      new InMemoryOutStreamHandler(path, &cache_, oss));
}

std::unique_ptr<InStreamHandler>
CachingReadOnlyFileManager::ReadFile(std::string path) {
  std::map<std::string, std::string>::const_iterator it = cache_.find(path);
  if (it != cache_.end())
    return std::unique_ptr<InStreamHandler>(
        new InStreamHandler(new std::istringstream(it->second)));
  if (!PathMatchesFilter(path) || !underlying_->FileExists(path))
    throw SimpleException("CachingReadOnlyFileManager", "ReadFile",
                          std::string("file not found: ") + path);
  std::string content = GetFileContent(underlying_, path);
  cache_[path] = content;
  ++cache_miss_count_;
  return std::unique_ptr<InStreamHandler>(
      new InStreamHandler(new std::istringstream(content)));
}

bool CachingReadOnlyFileManager::FileExists(std::string path) {
  if (cache_.find(path) != cache_.end())
    return true;
  if (!PathMatchesFilter(path))
    return false;
  return underlying_->FileExists(path);
}

int CachingReadOnlyFileManager::TestOnlyGetCacheMissCount() const {
  return cache_miss_count_;
}

std::ostream &operator<<(std::ostream &ofs, const FilePath &fp) {
  ofs << "SYSTEM " << fp.in_linux_ << "\nPATH " << fp.path_ << "\n";
  return ofs;
}

void Separate(std::string &strFile, std::string &strFolder) {
  std::string strRet;
  strFolder = "";
  for (unsigned i = 0; i < strFile.size(); ++i) {
    strRet += strFile[i];
    if (strFile[i] == '\\' || strFile[i] == '/') {
      strFolder += strRet;
      strRet = "";
    }
  }
}

} // namespace Gui
