#include "Preloader.h"

std::ostream& operator << (std::ostream& ofs, const FilePath& fp)
{
    ofs << fp.bInLinux << " " << fp.sPath;

    return ofs;
}

std::istream& operator >> (std::istream& ifs, FilePath& fp)
{
    ifs >> fp.bInLinux;
    ifs.get();
    std::getline(ifs, fp.sPath);

    fp.Slash(fp.sPath);

    return ifs;
}
