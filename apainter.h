#ifndef APAINTER_H
#define APAINTER_H


//later move for definition
#include <QPainter>

#include "g0/buildheader.h"

#include "g0/aconfig.h"

class APainter
{
protected:
    int colorValue;
    int storeValue;
public:
    APainter();

    enum AColors
    {
        colorBlack,
        colorRed,
        colorOrange,
        colorYellow,
        colorGreen,
        colorBlue,
        colorDarkBlue,
        colorViolet,
        colorWhite,
        colorGray,

        colorLightGray,
        colorDarkGray,
        colorCyan,
        colorMageta,
        colorDarkGreen,
        colorDarkCyan

    };

    virtual void changeColor(int colorS)=0;

    void restoreColor(){
       changeColor(storeValue);
    }
    void saveColor(){ //stack later
        storeValue = colorValue;
    }

    //stack of draws??

    int translateDefaulColor(std::string confParam);

    virtual void drawLine(int x1, int y1, int x2, int y2)=0;
    virtual void drawText(int x1, int y1, const char *text)=0;
    virtual void drawRect(int x1, int y1, int w1, int h1)=0;
    virtual void fillRect(int x1, int y1, int w1, int h1, int colorS)=0;
    virtual void drawEllipse(int x1, int y1, int w1, int h1, int colorFill)=0;


};

//QT implementation - another would be marmalade
class Painter : public APainter
{
protected:
    QPainter *src;
    int fontSize;


public:
    Painter(QPainter *source):src(source){
        fontSize=14;
    }

    void setFontSize(int fS)
    { fontSize = fS; }

    int getFontSize() { return fontSize; }

    void drawLine(int x1, int y1, int x2, int y2)
    {
        src->drawLine(x1,y1,x2,y2);
    }

    void drawText(int x1, int y1, const char *text);
    /*
    {
        src->drawText(x1,y1,text);
    } */

    void drawRect(int x1, int y1, int w1, int h1)
    {
        src->drawRect(x1,y1,w1,h1);
    }

    void drawEllipse(int x1, int y1, int w1, int h1, int colorFill=-1)
    {
        QBrush startBra = src->brush();
        if (colorFill>=0)
        {
            switch (colorFill)
            {
                case 0: src->setBrush(Qt::black); break;
                case 1: src->setBrush(Qt::red); break;
                case 2: src->setBrush(Qt::yellow); break; //orange
                case 3: src->setBrush(Qt::darkYellow); break;
                case 4: src->setBrush(Qt::green); break;
                case 5: src->setBrush(Qt::blue); break;
                case 6: src->setBrush(Qt::darkBlue); break;
                case 7: src->setBrush(Qt::magenta); break; //violet
                case 8: src->setBrush(Qt::white); break;
                case 9: src->setBrush(Qt::gray); break;
                case 10: src->setBrush(Qt::lightGray); break;
                case 11: src->setBrush(Qt::darkGray); break;
                case 12: src->setBrush(Qt::cyan); break;
                case 13: src->setBrush(Qt::darkMagenta); break;
                case 14: src->setBrush(Qt::darkGreen); break;
                case 15: src->setBrush(Qt::darkCyan); break;
                case 16: src->setBrush(Qt::darkRed); break;
            }
        }
        src->drawEllipse(x1,y1,w1,h1);

        if (colorFill>=0)
            src->setBrush(startBra); //return brach back

    }

    void drawImage(int x1, int y1, std::string imageName);

    void changeColor(int colorS);
    void changeColor (std::string confParam);
    void fillRect(int x1, int y1, int w1, int h1, int colorS);
    void fillRect(int x1, int y1, int w1, int h1, std::string confParam);

    void drawEllipse(int x1, int y1, int w1, int h1, std::string confParam);
};

#endif // APAINTER_H
