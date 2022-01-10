#ifndef TABLOADER_H
#define TABLOADER_H

#include "tab/Tab.hpp"

#include <memory>


namespace gtmy {


    class GTabLoader
    {
    public:
        bool open(std::string fileName);

        GTabLoader():type(255) {}

        std::unique_ptr<Tab>& getTab() { return tab;} //some attention here posible leak if use mindless

    private:
        std::uint8_t type; //0-10 for gmy
        //10 ptb
        //11 gp1 12 gp2 13 gp3
        //14 ptb5 16 gpx
        //17 tux
        //18 tab edit
        std::unique_ptr<Tab> tab;
    };

}


#endif // TABLOADER_H
