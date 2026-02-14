#include "exception.h"

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
