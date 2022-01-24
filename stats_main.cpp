#include <QCoreApplication>

#include "g0/Base.hpp"

using namespace gtmy;

int main(int argc, char *argv[])
{

    BaseStatistics base;
    base.start("/home/punnalyse/dev/gtab/base/", 10000);

    QCoreApplication a(argc, argv);
    return a.exec();
}
