#ifndef GVIEW_H
#define GVIEW_H


#include "tab/Tab.hpp"
#include "g0/Config.hpp"
#include "ui/imagepreloader.h"

#include "tab/Commands.hpp"

#include <QPainter>
#include "athread.h"


#include <time.h>
#include <QDebug>


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

    virtual void draw([[maybe_unused]]QPainter *painter){ } //virt

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
    virtual void onclick([[maybe_unused]] int x1, [[maybe_unused]] int y1){ } //virt
    virtual void ondblclick([[maybe_unused]] int x1, [[maybe_unused]] int y1){} //virt
    virtual void keyevent([[maybe_unused]] std::string press){} //virt
    virtual void ongesture([[maybe_unused]] int offset, [[maybe_unused]] bool horizontal) {}

    virtual void onTabCommand([[maybe_unused]]TabCommand command) {
    }

    virtual void onTrackCommand([[maybe_unused]]TrackCommand command) {
        qDebug() << "EMPY on TrackCommand";
    }

};

class GImage : public GView
{
protected:
    std::string imageName;

public:
    GImage(int x, int y, std::string imgName):GView(x,y,36,36),imageName(imgName)
    {
    }

    void draw(QPainter *painter)
    {
       QImage *img = (QImage*) ImagePreloader::getInstance().getImage(imageName);
       if (img)
           painter->drawImage(x,y,*img);
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

    void draw(QPainter *painter)
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

    std::unique_ptr<GImage> imageLabel;

    bool visible;

    bool showBorder;

    public:

        bool isVisible() { return visible; }
        void setVisible(bool value) {visible     = value; }

        void setBorder(bool nowShowBorder) { showBorder = nowShowBorder; }

        void setText(std::string newText) { ownText = newText; }
        std::string getText() { return ownText; }

        GLabel(int x, int y, std::string text, std::string pressSyn="", bool showBord=true);

        std::string getPressSyn(){return pressSynonim;}

        void draw(QPainter *painter){

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

        virtual bool hit(int hX, int hY);

};



class ThreadLocal;

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

      virtual void connectThread([[maybe_unused]]std::unique_ptr<ThreadLocal>& thr){}
      virtual void connectMainThread([[maybe_unused]]std::unique_ptr<ThreadLocal>& thr){}

      virtual void pleaseRepaint() {}

      virtual int getWidth() { return 0; }
      virtual int getHeight() { return 0; }

      virtual void requestHeight([[maybe_unused]]int newH) {}
      virtual void requestWidth([[maybe_unused]]int newW) {}

      virtual int getToolBarHeight() { return 0;}
      virtual int getStatusBarHeight() { return 0;}

      virtual void SetButton([[maybe_unused]]int index, [[maybe_unused]]std::string text, [[maybe_unused]]int x1, [[maybe_unused]]int y1, [[maybe_unused]]int w1, [[maybe_unused]]int h1, [[maybe_unused]]std::string pressSyn)
      {}

      virtual void SetButton(int index, GLabel *w, std::string pressSyn) {
          SetButton(index,w->getText(),w->getX(),w->getY(),w->getW(),w->getH(),pressSyn);
      }

      virtual void setComboBox([[maybe_unused]]int index, [[maybe_unused]]std::string, [[maybe_unused]]int x1, [[maybe_unused]]int y1, [[maybe_unused]]int w1, [[maybe_unused]]int h1, [[maybe_unused]]int forceValue)
      {}

      virtual void renewComboParams([[maybe_unused]]int index, [[maybe_unused]]std::string params)
      {

      }

      virtual void setViewPannel([[maybe_unused]]int val) {}
      virtual int getComboBoxValue([[maybe_unused]]int index) { return -1; }
      virtual void showHelp() {}
      virtual void setStatusBarMessage([[maybe_unused]]int index, [[maybe_unused]]std::string text, [[maybe_unused]]int timeOut=0) {}
      virtual void pushForceKey([[maybe_unused]]std::string keyevent) {}
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

    void draw(QPainter *painter)
    {
        if (checked)
            painter->fillRect(x,y,w,h,Qt::darkGray);//painter->fillRect(x,y,w,h,CONF_PARAM("colors.curBar")); //5 some color TODO?

        painter->drawRect(x,y,w,h);
    }

    virtual void onclick([[maybe_unused]]int x1, [[maybe_unused]]int y1)
    {
        if (checked)
            checked = false;
        else
            checked = true;
    }

    virtual void ondblclick(int x1, int y1) {onclick(x1,y1); }
};


#endif // GVIEW_H
