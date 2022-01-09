#include "ui/GPannel.hpp"


#include <QDebug>

using namespace gtmy;


void GPannel::onclick(int x1, int y1)
{
    if (openned) {
        for (size_t i = 0; i < buttons.size(); ++i) {
            if (buttons[i].hit(x1,y1)) {
                qDebug()<<"Hit pannel button "<<buttons[i].getText().c_str();
                keyevent(buttons[i].getPressSyn());
                break;
            }
        }
    }
}
