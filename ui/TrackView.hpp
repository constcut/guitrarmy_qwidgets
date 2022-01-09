#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <vector>
#include <memory>

#include "GView.hpp"
#include "ui/GPannel.hpp"
#include "tab/Commands.hpp"
#include "BarView.hpp"

#include "g0/Threads.hpp"


namespace gtmy {

    class TabView;
    using ViewPull = std::vector<BarView>;

    class TrackView : public GView
    {
    protected:
        Track *pTrack;

        TabView *tabParrent;
        std::unique_ptr<ThreadLocal> localThr; //Подумать над хранением, возможно удастся спрятать?
        std::vector<std::unique_ptr<ThreadLocal>> finishPool;

        ViewPull barsPull;

        GPannel* pan;
        std::unique_ptr<GTrackPannel> trackPan;
        std::unique_ptr<GEffectsPannel> effPan;
        std::unique_ptr<GClipboardPannel> clipPan;


    public:

        Track* getTrack() { return pTrack; }

        virtual void setUI();
        virtual bool isMovableY() { return true; }

        TrackView(Track *from):pTrack(from), pan(0) {
            trackPan = std::make_unique<GTrackPannel>(300,480,800);
            effPan = std::make_unique<GEffectsPannel>(300,480,800);
            clipPan = std::make_unique<GClipboardPannel>(300,480,800);
            trackPan->setPressView(this);
            effPan->setPressView(this);
            clipPan->setPressView(this);
            effPan->preOpen();
            clipPan->preOpen();
            pan = trackPan.get();
        }

        virtual ~TrackView();

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


        bool gotChanges();

    };

}

#endif // TRACKVIEW_H
