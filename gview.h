#ifndef GVIEW_H
#define GVIEW_H


#include "g0/types.h"
#include "g0/tab.h"
#include "g0/aconfig.h"

#include "g0/frequlate.h"

//#include <QPainter>
#include "apainter.h"
#include "athread.h"

#include "g0/aconfig.h"

#include <time.h>

void not_used(void *p);

class MasterView;

class GView
{
protected:
int x;
int y;
int w;
int h;

MasterView *master;



public:
    GView():x(0),y(0),w(0),h(0),master(0){}
    GView(int xV,int yV, int wV, int hV):x(xV),y(yV),w(wV),h(hV),master(0){}
    virtual ~GView() {}

    virtual void setMaster(MasterView *mast) { master = mast; }
    MasterView *getMaster() { return master; }

    virtual bool isMovableX() { return false; }
    virtual bool isMovableY() { return false; }

    virtual void setUI() { }

    int getX(){return x;}
    int getY() { return y; }
    int getW() {return w; }
    int getH() {return h; }

    void setW(int newW) { w = newW; }
    void setH(int newH) { h = newH; }
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }

    virtual void draw(Painter *painter){ not_used(painter);} //virt

    virtual bool hit(int hX, int hY)
    {
        if ((hX >= x)
           && (hY >=y))
        {
            int xDiff = hX-x;
            int yDiff = hY-y;

            if ((xDiff<=w)&&
                (yDiff<=h))
                return true;
        }
        return false;
    }


public:
    virtual void onclick(int x1, int y1){ not_used(&x1); not_used(&y1);} //virt
    virtual void ondblclick(int x1, int y1){onclick(x1,y1);} //virt
    virtual void keyevent(std::string press){press.empty();} //virt
    virtual void ongesture(int offset, bool horizontal) {not_used(&offset); not_used(&horizontal); }

    virtual void *getPannel() { return 0; }
};

class GImage : public GView
{
protected:
    std::string imageName;

public:
    GImage(int x, int y, std::string imgName):GView(x,y,36,36),imageName(imgName)
    {
    }

    void draw(Painter *painter)
    {
        painter->drawImage(x,y,imageName);
        //preload operation on refactoring
    }

};

class GRect : public GView
{
    int colorPress;

public:
    GRect(int x, int y, int w, int h):GView(x,y,w,h),colorPress(0)
    {

    }

    void pressIt()
    {
        ++colorPress;
        if (colorPress>10)
            colorPress = 0;
    }

    void draw(Painter *painter)
    {
        painter->drawRect(x,y,w,h);

        painter->fillRect(x+10,y+10,w-20,h-20,colorPress);
    }
};

class GLabel : public GView
{
protected:
std::string ownText;
std::string pressSynonim;

GImage *imageLabel;

bool visible;

bool showBorder;

public:

    bool isVisible() { return visible; }
    void setVisible(bool value) {visible     = value; }

    void setBorder(bool nowShowBorder) { showBorder = nowShowBorder; }

    void setText(std::string newText) { ownText = newText; }
    std::string getText() { return ownText; }

    GLabel(int x, int y, std::string text, std::string pressSyn="", bool showBord=true);
    /*
        imageLabel(0),visible(true), showBorder(showBord)
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
                    ||(text=="-3-")||(text=="leeg")||(text=="x"))
            {
                imageLabel = new GImage(x,y,text);

                setW(imageLabel->getW());
                setH(imageLabel->getH());
            }
        }

    }
    */

    std::string getPressSyn(){return pressSynonim;}

    void draw(Painter *painter){

        if (visible==false)
            return;

        if (imageLabel==0)
        {
            painter->drawText(x+5,y,ownText.c_str());

            if (showBorder)
                painter->drawRect(x,y-15,w,h); //10 half of text height
        }
        else
        {
            imageLabel->setX(x); imageLabel->setY(y-10);
            imageLabel->draw(painter);

            if (showBorder)
                painter->drawRect(x,y-10,imageLabel->getW(),imageLabel->getH());

        }
    }
    //void onclick(int x1, int y1){}
    //void ondblclick(int x1, int y1){}

    virtual bool hit(int hX, int hY);

    //calc size to create view
    //virtual void keyevent(std::string press){}
};



class MasterView
{
protected:
    GView *child;

    GView *firstChld;

    GView *lastView;

public:
      MasterView():child(0){}
      MasterView(GView *newChild):child(newChild),firstChld(newChild){ child->setMaster(this); lastView=child;}


      GView *getChild() { return child; }
      GView *changeChild(GView *newChild);
      GView *resetToFirstChild(){ return changeChild(firstChld); }
      GView *resetToLastView() { return changeChild(lastView); }

      GView *getFirstChild() { return firstChld; }

      virtual void connectThread(void *localThr){not_used(localThr);}
      virtual void connectMainThread(void *localThr){not_used(localThr);}


      virtual void pleaseRepaint() {}

      virtual int getWidth() { return 0; }
      virtual int getHeight() { return 0; }

      virtual void requestHeight(int newH) {}
      virtual void requestWidth(int newW) {}

      virtual int getToolBarHeight() { return 0;}
      virtual int getStatusBarHeight() { return 0;}

      virtual void SetButton(int index,std::string text, int x1, int y1, int w1, int h1, std::string pressSyn)
      {
          x1=y1;
      }

      virtual void SetButton(int index, GLabel *w, std::string pressSyn)
      {
          SetButton(index,w->getText(),w->getX(),w->getY(),w->getW(),w->getH(),pressSyn);
      }

      virtual void setComboBox(int index, std::string,int x1, int y1, int w1, int h1, int forceValue)
      {
          w1=h1;
      }

      virtual void renewComboParams(int index, std::string params)
      {

      }

      virtual void setViewPannel(int val) {}

      virtual int getComboBoxValue(int index) { return -1; }

      virtual void showHelp() {}

      virtual void setStatusBarMessage(int index, std::string text, int timeOut=0) {}

      virtual void pushForceKey(std::string keyevent) {}
      virtual bool isPlaying() { return false; }

      virtual void addToPlaylist(std::vector<std::string> playElement) { Q_UNUSED(playElement); }
      virtual bool isPlaylistHere() { return false; }
      virtual void goOnPlaylist() {}
      virtual void cleanPlayList() {}
};



class GCheckButton : public GView
{
protected:

    bool checked;

public:

    GCheckButton(int x1, int y1, int w1, int h1):GView(x1,y1,w1,h1),
        checked(false)
    {}

    bool isChecked() { return checked; }

    void draw(Painter *painter)
    {
        if (checked)
            painter->fillRect(x,y,w,h,CONF_PARAM("colors.curBar")); //5 some color

        painter->drawRect(x,y,w,h);
    }

    virtual void onclick(int x1, int y1)
    {
        if (checked)
            checked = false;
        else
            checked = true;
    }

    virtual void ondblclick(int x1, int y1) {onclick(x1,y1); }
};


#endif // GVIEW_H
