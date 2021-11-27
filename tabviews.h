#ifndef TABVIEWS_H
#define TABVIEWS_H

#include "gview.h"
#include "gpannel.h"

#include "g0/aclipboard.h"

#include "tabcommands.h"

class TrackView;

class Track;

class TabView : public GView
{
protected:
    Tab* pTab;
    GLabel *statusLabel;
    GLabel *bpmLabel;
    GTabPannel *pan;

    int lastOpenedTrack; //Подумать, можно ли перенести?
    ThreadLocal *localThr; //Возможно тоже если без QT

public:

    virtual void setUI();

    void setCurrentBar(int curBar) {
        pTab->getCurrentBar() = curBar; }

    virtual bool isMovableX() { return true; }
    virtual bool isMovableY() { return true; }

    TabView();

    std::vector<TrackView*> tracksView;

    int getCurTrack() { return pTab->getCurrentTrack(); }
    int getLastOpenedTrack() { return lastOpenedTrack; }

    void setTab(Tab* point2Tab);// {pTab = point2Tab;}
    Tab* getTab() { return pTab; }

     void draw(QPainter *painter);

     virtual void keyevent(std::string press);
     virtual void onTabCommand(TabCommand command);
     virtual void onTrackCommand(TrackCommand command);

     void setPlaying(bool playValue) { pTab->setPlaying(playValue); }
     bool getPlaying()
     {
         if (pTab->playing())
             if (localThr)
                 if (localThr->getStatus())
                     setPlaying(false);

         return pTab->playing();
     }

     void addSingleTrack(Track *track);

     void onclick(int x1, int y1);
     void ondblclick(int x1, int y1);

     virtual void ongesture(int offset, bool horizontal);

     void prepareAllThreads(ul shiftTheCursor);
     void launchAllThreads();
     void stopAllThreads();

     void connectAllThreadsSignal(MasterView *masterView);

     void *getPannel() { return pan; }

     bool gotChanges();

     //void ondblclick(int x1, int y1);
};

class BarView;

class ViewPull : public ChainContainer<BarView>
{
    //add len getV delete etc chould be here
};

class TrackView : public GView
{
protected:
    Track *pTrack;

    int digitPress;

    TabView *tabParrent;
    ThreadLocal *localThr;
    ViewPull barsPull;

    GPannel *pan;
    GTrackPannel *trackPan;
    GEffectsPannel *effPan;
    GClipboardPannel *clipPan;

    std::vector<SingleCommand> commandSequence;

public:

    virtual void setUI();
    virtual bool isMovableY() { return true; }

    TrackView(Track *from):pTrack(from),digitPress(-1),localThr(0), pan(0) {

        trackPan = new GTrackPannel(300,480,800);
        effPan = new GEffectsPannel(300,480,800);
        clipPan = new GClipboardPannel(300,480,800);

        trackPan->setPressView(this);
        effPan->setPressView(this);
        clipPan->setPressView(this);

        effPan->preOpen();
        clipPan->preOpen();

        pan = trackPan;
    }

    virtual ~TrackView(){}

    virtual void keyevent(std::string press);
    virtual void onTabCommand(TabCommand command);
    virtual void onTrackCommand(TrackCommand command);

    void onclick(int x1, int y1);
    void ondblclick(int x1, int y1); //{ onclick(x1,y1);}

    virtual void ongesture(int offset, bool horizontal);

    int horizonMove(int offset);

    void setDisplayBar(int barPosition);


    TabView *getPa() { return tabParrent; }
    void setPa(TabView* tv) { tabParrent = tv; }

    void draw(QPainter *painter);

    void prepareThread(ul shiftTheCursor);
    void launchThread();
    void stopThread();
    void connectThreadSignal(MasterView *masterView);
    int threadSeconds() { return localThr->calculateSeconds(); }

    void *getPannel() { return pan; }

    bool gotChanges();

    void reverseCommand(SingleCommand &command);//TODO как и другие элементы комманд убрать в общий класс

protected:

    void switchNoteState(byte changeState);
    void switchEffect(int effIndex);

    void switchBeatEffect(int effIndex);

};

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

    int getBarLen() { return nBeats; }

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

    void drawNote(QPainter *painter, byte noteDur, byte dotted, byte durDet,
                  int x1, int y1);

    void drawMidiNote(QPainter *painter, byte noteDur, byte dotted, byte durDet, int midiNote,
                      int x1, int y1);

    void drawEffects(QPainter *painter, int x1, int y1, int w1, int h1, EffectsPack *eff);


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

#endif // TABVIEWS_H
