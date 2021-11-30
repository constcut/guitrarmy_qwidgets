#ifndef GMYFILE_H
#define GMYFILE_H

#include "tab.h"

class GmyFile
{
public:
    GmyFile();

    bool saveToFile(std::ofstream& file, Tab *tab);
    bool loadFromFile(std::ifstream& file, Tab *tab, bool skipVersion=false);

protected:

    bool loadString(std::ifstream& file, std::string &strValue);
    bool saveString(std::ofstream& file, std::string &strValue);
};

#endif // GMYFILE_H
