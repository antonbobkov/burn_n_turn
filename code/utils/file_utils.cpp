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

FilePath::FilePath(bool inLinux, std::string path)
    : in_linux_(inLinux), path_(path) {
  fm_ = new StdFileManager();
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

SP<OutStreamHandler> FilePath::WriteFile(std::string s) {
  return fm_->WriteFile(s);
}

SP<InStreamHandler> FilePath::ReadFile(std::string s) {
  return fm_->ReadFile(s);
}

SP<OutStreamHandler> StdFileManager::WriteFile(std::string s) {
  return new OutStreamHandler(new std::ofstream(s.c_str()));
}

SP<InStreamHandler> StdFileManager::ReadFile(std::string s) {
  return new InStreamHandler(new std::ifstream(s.c_str()));
}

SP<OutStreamHandler> FunnyFileManager::WriteFile(std::string s) {
  return new OutStreamHandler(new std::ofstream(("../" + s).c_str()));
}

SP<InStreamHandler> FunnyFileManager::ReadFile(std::string s) {
  return new InStreamHandler(new std::ifstream(("../" + s).c_str()));
}

std::ostream &operator<<(std::ostream &ofs, const FilePath &fp) {
  ofs << "SYSTEM " << fp.in_linux_ << "\nPATH " << fp.path_ << "\n";
  return ofs;
}

std::istream &operator>>(std::istream &ifs, FilePath &fp) {
  ParsePosition("SYSTEM", ifs);
  ifs >> fp.in_linux_;
  ParseGrabLine("PATH", ifs, fp.path_);
  fp.Slash(fp.path_);
  fp.path_ = fp.Format(fp.path_);
  return ifs;
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
