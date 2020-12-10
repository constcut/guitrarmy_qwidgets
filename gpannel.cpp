#include "gpannel.h"

#include "g0/astreaming.h"


static AStreaming logger("GPannel");

void GPannel::onclick(int x1, int y1)
{
    if (openned == false)
    {
        //if (noOnOffButton == false)
        //if (pannelLabel.hit(x1,y1))
        {
            //openned = true;
            //pannelLabel.setText(openedText); //HIDE UNDER CHILD CLASS
        }
    }
    else
    {
        //if (noOnOffButton == false)
        //if (pannelLabel.hit(x1,y1))
        {
            //openned = false;
            //pannelLabel.setText(closedText);  //HIDE UNDER CHILD CLASS
            //return ;
        }

        for (size_t i = 0; i < buttons.size(); ++i)
        {
            if (buttons[i].hit(x1,y1))
            {
                LOG(<<"Hit pannel button "<<buttons[i].getText().c_str());

                keyevent(buttons[i].getPressSyn());
                break;
            }
        }
    }
}
