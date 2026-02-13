#ifndef MESSAGE_WRITER_HDR_INCLUDE_GUARD_09_04_10_06_07_PM
#define MESSAGE_WRITER_HDR_INCLUDE_GUARD_09_04_10_06_07_PM

#include <fstream>
#include <iostream>
#include <string>

#include <map>

#include "smart_pointer.h"

namespace Gui {
enum WriteType { WT_DEBUG, WT_ERROR };

class MessageWriter : virtual public SP_Info {
  WriteType wt_def;

public:
  std::string get_class_name() override { return "MessageWriter"; }
  MessageWriter(WriteType wt_def_ = WT_DEBUG) : wt_def(wt_def_) {}

  virtual void Write(WriteType wt, std::string strMsg) = 0;
  void Write(std::string strMsg) { Write(wt_def, strMsg); }

  void SetDefType(WriteType wt_def_) { wt_def = wt_def_; }
};

class EmptyWriter : public MessageWriter {
public:
  std::string get_class_name() override { return "EmptyWriter"; }
  /*virtual*/ void Write(WriteType wt, std::string strMsg) {}
};

class IoWriter : public MessageWriter {
public:
  std::string get_class_name() override { return "IoWriter"; }
  /*virtual*/ void Write(WriteType wt, std::string strMsg);
};

class FileWriter : public MessageWriter {
  std::map<WriteType, std::string> mFiles;

public:
  std::string get_class_name() override { return "FileWriter"; }
  void AddFile(WriteType wt, std::string sFile);
  /*virtual*/ void Write(WriteType wt, std::string strMsg);
};
}; // namespace Gui

#endif // MESSAGE_WRITER_HDR_INCLUDE_GUARD_09_04_10_06_07_PM