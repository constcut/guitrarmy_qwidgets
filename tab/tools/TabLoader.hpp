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
        std::unique_ptr<Tab> tab;
    };

}


#endif // TABLOADER_H
