#ifndef MESSAGE_WRITER_HDR_INCLUDE_GUARD_09_04_10_06_07_PM
#define MESSAGE_WRITER_HDR_INCLUDE_GUARD_09_04_10_06_07_PM

#include <iostream>
#include <fstream>
#include <string>

#include <map>

#include "SmartPointer.h"

namespace Gui
{
    enum WriteType {WT_DEBUG, WT_ERROR};
    
    class MessageWriter: virtual public SP_Info
    {
        WriteType wt_def;
    public:

        MessageWriter(WriteType wt_def_ = WT_DEBUG):wt_def(wt_def_){}
        
        virtual void Write(WriteType wt, std::string strMsg)=0;
        void Write(std::string strMsg){Write(wt_def, strMsg);}

        void SetDefType(WriteType wt_def_)
        {
            wt_def = wt_def_;
        }
    };

    class EmptyWriter: public MessageWriter
    {
    public:
        /*virtual*/ void Write(WriteType wt, std::string strMsg){}
    };

    class IoWriter: public MessageWriter
    {
    public:
        /*virtual*/ void Write(WriteType wt, std::string strMsg);
    };

    class FileWriter: public MessageWriter
    {
        std::map<WriteType, std::string> mFiles;
    public:
        void AddFile(WriteType wt, std::string sFile);
        /*virtual*/ void Write(WriteType wt, std::string strMsg);
    };
};


#endif // MESSAGE_WRITER_HDR_INCLUDE_GUARD_09_04_10_06_07_PM