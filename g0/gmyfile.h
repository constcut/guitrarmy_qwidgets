#ifndef GMYFILE_H
#define GMYFILE_H

#include "g0/afile.h"
#include "g0/tab.h"

class GmyFile
{
public:
    GmyFile();

    bool saveToFile(AFile *file, Tab *tab);
    bool loadFromFile(AFile *file, Tab *tab, bool skipVersion=false);

protected:

    bool loadString(AFile *file, std::string &strValue);
    bool saveString(AFile *file, std::string &strValue);
};

//abstraction for file format
//also here should be some abstraction for adaption from gp4
//and in the end here would be connected with AHistory VersionFile


//TODO:
/// 1) better packing !
/// 2) effects full save\load (33 change, 19 tremolo, 17 bend + chord + grace)
/// 3)

#endif // GMYFILE_H
