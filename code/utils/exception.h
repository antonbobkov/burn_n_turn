#ifndef GUI_EXCEPTION_HEADER_ALREADY_DEFINED
#define GUI_EXCEPTION_HEADER_ALREADY_DEFINED

/*
 * exception.h - Exception types for the Gui namespace.
 * MyException carries a name, class, function stack, and optional inherited
 * messages; SimpleException holds a single problem string. crefString is a
 * convenience typedef for const string reference.
 */

#include <string>

namespace Gui {

typedef const std::string &crefString;

/* Base exception: stores name, class, function stack, and inherited
 * exception info; subclasses override GetErrorMessage() for the message. */
class MyException {
  std::string strExcName;     // name of exception
  std::string strClsName;     // name of class generated exception
  std::string strFnNameTrack; // list of function names
  std::string strInherited;   // description of inherited exceptions

  std::string GetHeader() const;

protected:
  void ResetName(crefString istrExcName);
  virtual std::string GetErrorMessage() const = 0;

public:
  MyException(crefString strExcName_, crefString strClsName_,
              crefString strFnName_);
  virtual ~MyException() {}

  /* Append a function name to the call stack. */
  void AddFnName(crefString strFnName);
  /* Prepend another exception's description to inherited text. */
  void InheritException(const MyException &crefExc);

  /* Full or short description (stack + message + inherited if bDetailed). */
  std::string GetDescription(bool bDetailed = false) const;
};

/* Exception that carries a single problem string as its error message. */
class SimpleException : public MyException {
  std::string strProblem;

public:
  SimpleException(crefString strClsName_, crefString strFnName_,
                  crefString strProblem_)
      : MyException("SimpleException", strClsName_, strFnName_),
        strProblem(strProblem_) {}

  SimpleException(crefString strProblem_)
      : MyException("<N/A>", "<N/A>", "<N/A>"), strProblem(strProblem_) {}

  /*virtual*/ std::string GetErrorMessage() const { return strProblem; }
};

} // namespace Gui

#endif // GUI_EXCEPTION_HEADER_ALREADY_DEFINED
