#include "apainter.h"

#include <QTextCodec>

APainter::APainter()
{

}


void Painter::drawText(int x1, int y1, const char *text)
{
    QFont font("Times");
    font.setPixelSize(fontSize);
    src->setFont(font);
    QString uniText = QTextCodec::codecForLocale()->toUnicode(text);
    src->drawText(x1,y1,uniText);
}

void Painter::changeColor(int colorS)
{
    saveColor();
    switch (colorS)
    {
        case 0: src->setPen(Qt::black); break;
        case 1: src->setPen(Qt::red); break;
        case 2: src->setPen(Qt::yellow); break; //orange
        case 3: src->setPen(Qt::darkYellow); break;
        case 4: src->setPen(Qt::green); break;
        case 5: src->setPen(Qt::blue); break;
        case 6: src->setPen(Qt::darkBlue); break;
        case 7: src->setPen(Qt::magenta); break; //violet
        case 8: src->setPen(Qt::white); break;
        case 9: src->setPen(Qt::gray); break;
        case 10: src->setPen(Qt::lightGray); break;
        case 11: src->setPen(Qt::darkGray); break;
        case 12: src->setPen(Qt::cyan); break;
        case 13: src->setPen(Qt::darkMagenta); break;
        case 14: src->setPen(Qt::darkGreen); break;
        case 15: src->setPen(Qt::darkCyan); break;
        case 16: src->setPen(Qt::darkRed); break;

    }
    storeValue = colorS;
}

int APainter::translateDefaulColor(std::string confParam)
{
    int numColor = -1;

    if (confParam == "black") numColor = 0;
    if (confParam == "red") numColor = 1;
    if (confParam == "yellow") numColor = 2;
    if (confParam == "orange") numColor = 3;
    if (confParam == "green") numColor = 4;
    if (confParam == "blue") numColor = 5;
    if (confParam == "darkblue") numColor = 6;
    if (confParam == "violet") numColor = 7;
    if (confParam == "white") numColor = 8;
    if (confParam == "gray") numColor = 9;
    if (confParam == "lightgray") numColor = 10;
    if (confParam == "darkgray") numColor = 11;
    if (confParam == "cyan") numColor = 12;
    if (confParam == "mageta") numColor = 13;
    if (confParam == "darkgreen") numColor = 14;
    if (confParam == "darkcyan") numColor = 15;
    if (confParam == "darkred") numColor = 16;

    return numColor;
}

void Painter::fillRect(int x1, int y1, int w1, int h1, int colorS)
{
    switch (colorS)
    {
        case 0: src->fillRect(x1,y1,w1,h1,Qt::black); break;
        case 1: src->fillRect(x1,y1,w1,h1,Qt::red); break;
        case 2: src->fillRect(x1,y1,w1,h1,Qt::yellow); break; //orange
        case 3: src->fillRect(x1,y1,w1,h1,Qt::darkYellow); break;
        case 4: src->fillRect(x1,y1,w1,h1,Qt::green); break;
        case 5: src->fillRect(x1,y1,w1,h1,Qt::blue); break;
        case 6: src->fillRect(x1,y1,w1,h1,Qt::darkBlue); break;
        case 7: src->fillRect(x1,y1,w1,h1,Qt::magenta); break; //violet
        case 8: src->fillRect(x1,y1,w1,h1,Qt::white); break;
       case 9: src->fillRect(x1,y1,w1,h1,Qt::gray); break;
        case 10: src->fillRect(x1,y1,w1,h1,Qt::lightGray); break;
        case 11: src->fillRect(x1,y1,w1,h1,Qt::darkGray); break;
        case 12: src->fillRect(x1,y1,w1,h1,Qt::cyan); break;
        case 13: src->fillRect(x1,y1,w1,h1,Qt::darkMagenta); break;
        case 14: src->fillRect(x1,y1,w1,h1,Qt::darkGreen); break;
        case 15: src->fillRect(x1,y1,w1,h1,Qt::darkCyan); break;
        case 16: src->fillRect(x1,y1,w1,h1,Qt::darkRed); break;
    }
}

void Painter::changeColor (std::string confParam)
{
    //saveColor();

    int numColor =-1;

    numColor = translateDefaulColor(confParam);

    if (numColor!=-1)
    {
        changeColor(numColor);
        return;
    }


    if (confParam.size() == 6)
    {
        //log << "possible rgb color";

        int r=0,g=0,b=0;

        r = confParam[0];

        if (isdigit(r))
            r-=48;
        else
            r-=48+7; //7 is shift from A to 9
        r*=16;

        int next = confParam[1];

        if (isdigit(next))
            next-=48;
        else
            next-=48+7;

        r+=next;

        next=0;

        g = confParam[2];

        if (isdigit(g))
            g-=48;
        else
            g-=48+7; //7 is shift from A to 9
        g*=16;


        next = confParam[3];

        if (isdigit(next))
            next-=48;
        else
            next-=48+7;

        g+=next;

        next=0;

        b = confParam[4];

        if (isdigit(b))
            b-=48;
        else
            b-=48+7; //7 is shift from A to 9
        b*=16;

        next = confParam[5];

        if (isdigit(next))
            next-=48;
        else
            next-=48+7;

        b+=next;

        QColor colorForPen(r,g,b);
        src->setPen(colorForPen);
    }

    //else - configuration is rgb
    //HERE SHOULD BE CHECK FOR THE RIGHT 16x value
}

void Painter::drawEllipse(int x1, int y1, int w1, int h1, std::string confParam)
{
      int numColor = translateDefaulColor(confParam);
      drawEllipse(x1,y1,w1,h1,numColor);
}


void Painter::fillRect(int x1, int y1, int w1, int h1, std::string confParam)
{
    int numColor = translateDefaulColor(confParam);

    if (numColor!=-1)
    {
        fillRect(x1,y1,w1,h1,numColor);
        return;
    }

    if (confParam.size() == 6)
    {
        //log << "possible rgb color";

        int r=0,g=0,b=0;

        r = confParam[0];

        if (isdigit(r))
            r-=48;
        else
            r-=48+7; //7 is shift from A to 9
        r*=16;

        int next = confParam[1];

        if (isdigit(next))
            next-=48;
        else
            next-=48+7;

        r+=next;

        next=0;

        g = confParam[2];

        if (isdigit(g))
            g-=48;
        else
            g-=48+7; //7 is shift from A to 9
        g*=16;


        next = confParam[3];

        if (isdigit(next))
            next-=48;
        else
            next-=48+7;

        g+=next;

        next=0;

        b = confParam[4];

        if (isdigit(b))
            b-=48;
        else
            b-=48+7; //7 is shift from A to 9
        b*=16;

        next = confParam[5];

        if (isdigit(next))
            next-=48;
        else
            next-=48+7;

        b+=next;

        QColor colorForFill(r,g,b);

        src->fillRect(x1,y1,w1,h1,colorForFill);
    }

    //skipped for a while?
}

void Painter::drawImage(int x1, int y1, std::string imageName)
{
    //QImage img;
    //std::string name = std::string(getTestsLocation())+ std::string("Icons/") + imageName + std::string(".png");
    //img.load(name.c_str());

    QImage *img = (QImage*) AConfig::getInstance()->imageLoader.getImage(imageName);


    if (img)
    {
        if ((x1==0)&&(x1==y1))
        {
            //QRect source()
            //QRect dest
            src->drawImage(x1,y1,*img);
        }
        else
            src->drawImage(x1,y1,*img);
    }

    //src->drawImage();
}
