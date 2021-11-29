#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <vector>

#include "gview.h"
#include "gpannel.h"
//#include "g0/aclipboard.h"
#include "tab/tabcommands.h"
#include "barview.h"


class TabView;
using ViewPull = std::vector<BarView>;

class TrackView : public GView
{
protected:
    Track *pTrack;

    TabView *tabParrent;
    ThreadLocal *localThr;
    ViewPull barsPull;

    GPannel *pan;
    GTrackPannel *trackPan;
    GEffectsPannel *effPan;
    GClipboardPannel *clipPan;


public:

    Track* getTrack() { return pTrack; }

    virtual void setUI();
    virtual bool isMovableY() { return true; }

    TrackView(Track *from):pTrack(from),localThr(0), pan(0) {

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

    void prepareThread(size_t shiftTheCursor);
    void launchThread();
    void stopThread();
    void connectThreadSignal(MasterView *masterView);
    int threadSeconds() { return localThr->calculateSeconds(); }

    void *getPannel() { return pan; }

    bool gotChanges();

};


#endif // TRACKVIEW_H
