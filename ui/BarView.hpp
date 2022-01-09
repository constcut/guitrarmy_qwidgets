#ifndef BARVIEW_H
#define BARVIEW_H

#include "GView.hpp"


class BarView : public GView
{

protected:

    static const int stringWidth=12;
    static const int inbarWidth=20;

    Bar *pBar;
    int xShift,yShift;
    int nStrings;
    int cursor;
    int stringCursor;

    int nBeats;

    int barNumber;

    bool sameSign;

    int selectorBegin;
    int selectorEnd;

    bool repBegin;
    bool repEnd;

public:

    int getBarsize() { return nBeats; }

    void setSameSign(bool value) {sameSign = value;}

    void setSelectors(int beg, int end){selectorBegin=beg; selectorEnd=end;}
    void flushSelectors(){selectorBegin=selectorEnd=-1;}

    BarView(Bar *b,int nstr, int barNum=-1);


    void setBar(Bar *newBar){pBar = newBar;}
    Bar *getBar() { return pBar; }

    void setCursor(int cur, int strCur=-1) { cursor = cur; stringCursor =strCur; }
    int getCursor() { return cursor; }

    //virtual void keyevent(std::string press){}
    //void onclick(int x1, int y1){x1=y1=0;} //not used
    //void ondblclick(int x1, int y1){ onclick(x1,y1);}

    int getClickString(int y1);
    int getClickBeat(int x1);

    void drawNote(QPainter *painter, std::uint8_t noteDur, std::uint8_t dotted, std::uint8_t durDet,
                  int x1, int y1);

    void drawMidiNote(QPainter *painter, std::uint8_t noteDur, std::uint8_t dotted, std::uint8_t durDet, int midiNote,
                      int x1, int y1);

    void drawEffects(QPainter *painter, int x1, int y1, int w1, int h1, ABitArray *eff);


    void draw(QPainter *painter); //from beat to beat

    void setShifts(int x1, int y1)
    {
        xShift =x1;
        yShift =y1;
        x = x1+20;
        y = y1+20;
    }


    void setNStrings(int ns){nStrings = ns;}
};


#endif // BARVIEW_H
