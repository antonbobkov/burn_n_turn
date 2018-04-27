#include "MessageWriter.h"

namespace Gui
{
    /*virtual*/ void IoWriter::Write(WriteType wt, std::string strMsg)
    {
        if (wt == WT_ERROR)
            std::cerr << strMsg;
        else
            std::cout << strMsg;
    }

    void FileWriter::AddFile(WriteType wt, std::string sFile)
    {
        mFiles[wt] = sFile;
        std::ofstream ofs(sFile.c_str());
    }
    
    /*virtual*/ void FileWriter::Write(WriteType wt, std::string strMsg)
    {
        std::map<WriteType, std::string>::iterator itr = mFiles.find(wt);
        if(itr == mFiles.end())
            return;
        std::ofstream ofs(itr->second.c_str(), std::ios_base::out | std::ios_base::app);
        ofs << strMsg;
    }
}