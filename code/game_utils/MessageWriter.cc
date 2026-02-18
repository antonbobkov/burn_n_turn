#include "MessageWriter.h"

/*virtual*/ void IoWriter::Write(WriteType wt, std::string strMsg) {
  if (wt == WT_ERROR)
    std::cerr << strMsg;
  else
    std::cout << strMsg;
}