#include "GView.hpp"

#include <QFileDialog>

#include "tab/Tab.hpp"
#include "tab/GtpFiles.hpp"
#include "midi/MidiFile.hpp"

#include "g0/Config.hpp"

#include "tab/GmyFile.hpp"
#include "tab/TabLoader.hpp" //loader
#include "g0/Threads.hpp"

#include <QDebug>


bool gViewLog=false;


using namespace gtmy;


GView *MasterView::changeChild(GView *newChild)
{      
      GView *oldCh = child;

      lastView = oldCh;

      child = newChild;
      if (child)
      child->setMaster(this);
      if (oldCh)
          oldCh->setMaster(0);
      else
          firstChld = newChild;

      //must repaint

      return oldCh;

}


GLabel::GLabel(int x, int y, std::string text, std::string pressSyn, bool showBord):
    visible(true), showBorder(showBord)
{
    int size = text.length();
    //int height = 12;
    this->x = x;
    this->y = y;
    h = 20;
    w = size*6 + 20;
    ownText = text;
    pressSynonim = pressSyn;

    if (CONF_PARAM("images")=="1")
    {
        //later preloaded images should apear
        //REFACT: use map

       QImage* imgPtr = ImagePreloader::getInstance().getImage(text);

        /*
        if ((text =="prevBeat") || (text =="nextBeat") || (text =="upString") || (text =="downString") ||
                (text =="prevBar") || (text =="nextBar") || (text =="play") || (text =="save") ||
                (text=="open")|| (text =="new")
                || (text =="config")
                || (text =="record")|| (text =="tap")
                || (text =="pattern")|| (text =="tests")||
                (text=="tab")||(text=="backview")||
                (text=="morze")||(text=="info")||(text=="openPannel")||
                (text=="1")||(text=="2")||(text=="3")||
                (text=="4")||(text=="5")||(text=="6")||
                (text=="7")||(text=="8")||(text=="9")||
                (text=="0")||(text=="qp")||(text=="qm")||
                (text=="p")||(text=="del")||(text==".")
                ||(text=="-3-")||(text=="leeg")||(text=="x")) */

        if (imgPtr)
        {
            imageLabel = std::make_unique<GImage>(x,y,text);
            setW(imageLabel->getW());
            setH(imageLabel->getH());
        }
    }

}

bool GLabel::hit(int hX, int hY)
{
    bool hitten = false;

    if (imageLabel==0)
    {
        y = y - h/2;
         hitten = GView::hit(hX,hY);
        y = y + h/2;
    }
    else
    {
        hitten = GView::hit(hX,hY);
    }

    if (hitten)
    {
        qDebug()<<"GLabel hitten "<<this->getText().c_str();
    }

    return hitten;
}



