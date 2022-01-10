#ifndef BARVIEW_H
#define BARVIEW_H

#include "GView.hpp"


namespace gtmy {

    class BarView : public GView
    {

    private:

        static const int stringWidth=12;
        static const int inbarWidth=20;

        Bar* _pBar;
        int _xShift, _yShift; //TODO review + to unsinged
        int _nStrings;
        int _cursor;
        int _stringCursor;

        int _barNumber;

        bool _sameSign;

        int _selectorBegin;
        int _selectorEnd;

        bool _repBegin;
        bool _repEnd;

    public:

        BarView(Bar *b, int nstr, int barNum = -1);

        void setBar(Bar *newBar) {_pBar = newBar;}
        Bar *getBar() const { return _pBar; }

        int getBarsize() const { return _pBar->size(); }

        void setSameSign(bool value) { _sameSign = value; }

        void setSelectors(int beg, int end){_selectorBegin = beg; _selectorEnd = end;}
        void flushSelectors() { _selectorBegin = _selectorEnd = -1; }


        void setCursor(int cur, int strCur = -1) { _cursor = cur; _stringCursor =strCur; }
        int getCursor() const { return _cursor; }

        int getClickString(int y1) const;
        int getClickBeat(int x1) const;

        void drawNote(QPainter *painter, std::uint8_t noteDur, std::uint8_t dotted, std::uint8_t durDet,
                      int x1, int y1);

        void drawMidiNote(QPainter *painter, std::uint8_t noteDur, std::uint8_t dotted, std::uint8_t durDet, int midiNote,
                          int x1, int y1);

        void drawEffects(QPainter *painter, int x1, int y1, int w1, int h1, const ABitArray &eff);

        void draw(QPainter *painter); //from beat to beat

        void setShifts(int x1, int y1) {
            _xShift =x1;
            _yShift =y1;
            x = x1+20;
            y = y1+20;
        }

        void setNStrings(int ns) { _nStrings = ns; }
    };

}

#endif // BARVIEW_H
