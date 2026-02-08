#include "file_utils.h"

#include <fstream>

namespace Gui {

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

std::string FilePath::GetRelativePath(std::string s) const {
  return GetParse(s);
}

std::string FilePath::GetParse(std::string s) const {
  Slash(s);
  s = path_ + s;
  return GetFormatted(s);
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

InMemoryOutStreamHandler::InMemoryOutStreamHandler(
    std::string path, std::map<std::string, std::string> *files,
    std::ostringstream *stream)
    : OutStreamHandler(stream), path_(path), files_(files), stream_(stream) {}

InMemoryOutStreamHandler::~InMemoryOutStreamHandler() {
  if (stream_ && files_)
    (*files_)[path_] = stream_->str();
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

std::string InMemoryFileManager::GetFileContents(std::string path) const {
  std::map<std::string, std::string>::const_iterator it = files_.find(path);
  if (it == files_.end())
    return "";
  return it->second;
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

void RecordKeeper::ReadDef() {
  std::ifstream ifs(sDefFile.c_str());

  if (ifs.fail())
    throw SimpleException("RecordKeeper", "ReadDef",
                          "Cannot open file " + sDefFile + " for reading");

  Read(ifs);
}

void RecordKeeper::WriteDef() {
  std::ofstream ofs(sDefFile.c_str());

  if (ofs.fail())
    throw SimpleException("RecordKeeper", "WriteDef",
                          "Cannot open file " + sDefFile + " for writing");

  Write(ofs);
}

void RecordCollection::NewRecordKeeper(SP<RecordKeeper> pRec) {
  vRecords.push_back(pRec);
}

void RecordCollection::Read(std::istream &ifs) {
  for (size_t i = 0, sz = vRecords.size(); i < sz; ++i)
    vRecords[i]->Read(ifs);
}

void RecordCollection::Write(std::ostream &ofs) {
  for (size_t i = 0, sz = vRecords.size(); i < sz; ++i)
    vRecords[i]->Write(ofs);
}

} // namespace Gui
