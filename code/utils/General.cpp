#include "General.h"

#include <fstream>

namespace Gui {
// Known information about exception
std::string MyException::GetHeader() const {
  return "Class " + strClsName + ": " + strExcName + " in " + strFnNameTrack;
}

// sets exception name (can be used in inherited classes)
void MyException::ResetName(crefString istrExcName) {
  strExcName = istrExcName;
}

MyException::MyException(crefString strExcName_, crefString strClsName_,
                         crefString strFnName_)
    : strExcName(strExcName_), strClsName(strClsName_),
      strFnNameTrack(strFnName_ + "()"), strInherited("") {}

// adds function name to the track list
void MyException::AddFnName(crefString strFnName) {
  strFnNameTrack = strFnName + "()\\" + strFnNameTrack;
}

// remembers description of preceding exceptions
void MyException::InheritException(const MyException &crefExc) {
  strInherited = crefExc.GetDescription(true) + "\n" + strInherited;
}

// gets description of the exception (short version)
std::string MyException::GetDescription(bool bDetailed /* = false*/) const {
  if (bDetailed)
    return GetHeader() + "\n" + GetErrorMessage() + "\n" + strInherited;
  else
    return "Exception: " + GetErrorMessage() + "\n";
}

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
      ifs >> c; // skip spaces, tabs etcS
      ifs.putback(c);

      return std::getline(ifs, sResult).fail();
    }
  }
}

FilePath::FilePath(bool bInLinux_, std::string sPath_)
    : bInLinux(bInLinux_), sPath(sPath_) {
  pFm = new StdFileManager();

  Slash(sPath);

  std::string str;
  str += "abcdefghijklmnopqrstuvwxyz";
  str += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  str += "1234567890";
  str += " .-_()?<>+,\\/#";

  for (unsigned i = 0; i < str.length(); ++i)
    stAllowed.insert(str[i]);

  Format(sPath);
}

void FilePath::Slash(std::string &s) {
  if (!bInLinux) {
    for (unsigned i = 0; i < s.length(); ++i)
      if (s[i] == '/')
        s[i] = '\\';
  } else {
    for (unsigned i = 0; i < s.length(); ++i)
      if (s[i] == '\\')
        s[i] = '/';
  }
}

void FilePath::Parse(std::string &s) { s = GetParse(s); }

std::string FilePath::GetParse(std::string s) {
  Slash(s);
  s = sPath + s;

  return GetFormatted(s);
}

void FilePath::Format(std::string &s) { s = GetFormatted(s); }

std::string FilePath::GetFormatted(std::string s) {
  std::string s_clean = "";
  for (unsigned i = 0; i < s.length(); ++i)
    if (stAllowed.find(s[i]) != stAllowed.end())
      s_clean += s[i];
  return s_clean;
}

SP<OutStreamHandler> FilePath::WriteFile(std::string s) {
  return pFm->WriteFile(s);
}

SP<InStreamHandler> FilePath::ReadFile(std::string s) {
  return pFm->ReadFile(s);
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
  ofs << "SYSTEM " << fp.bInLinux << "\nPATH " << fp.sPath << "\n";

  return ofs;
}

std::istream &operator>>(std::istream &ifs, FilePath &fp) {
  ParsePosition("SYSTEM", ifs);

  ifs >> fp.bInLinux;

  ParseGrabLine("PATH", ifs, fp.sPath);

  fp.Slash(fp.sPath);
  fp.Format(fp.sPath);

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

Index::Index(const Index &i) : nI(i.nI), pCounter(0), pRm(i.pRm) {
  if (i.pCounter)
    pCounter = &++*i.pCounter;
}

Index::~Index() {
  if (pCounter && --*pCounter == 0) {
    pRm->DeleteIndex(*this);
    delete pCounter;
  }
}

Index &Index::operator=(const Index &i) {
  if (pCounter && --*pCounter == 0) {
    pRm->DeleteIndex(*this);
    delete pCounter;
  }

  nI = i.nI;

  if (i.pCounter)
    ++*i.pCounter;

  pCounter = i.pCounter;
  pRm = i.pRm;

  return *this;
}
} // namespace Gui
