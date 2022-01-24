#include <QCoreApplication>

#include "g0/Base.hpp"

using namespace gtmy;

int main(int argc, char *argv[])
{
    //TODO if argc == 1 search for base catalog close to executable
    //if argc == 2 its path to catalog
    //if argc == 3 its path to csv (default is csv dir close)
    //if argc == 4 then last is mode 'm' - generate midi files

    //TODO handlers

    BaseStatistics base;
    base.start("/home/punnalyse/dev/gtab/base/mine/", 207);

    QCoreApplication a(argc, argv);
    return a.exec();
}
