#ifndef AEXPIMP_H
#define AEXPIMP_H

#include "types.h"

#include "g0/tab.h"
#include "g0/gmyfile.h"
#include "g0/gtpfiles.h"


class GTabLoader
{
public:
    bool open(std::string fileName);

    GTabLoader():type(255),tab(0) {}

    Tab *getTab() {return tab;} //some attention here posible leak if use mindless

protected:
    std::uint8_t type; //0-10 for gmy
    //10 ptb
    //11 gp1 12 gp2 13 gp3
    //14 ptb5 16 gpx
    //17 tux
    //18 tab edit
    Tab *tab;
};


#endif // AEXPIMP_H
