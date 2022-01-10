#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <vector>
#include <memory>

#include "GView.hpp"
#include "ui/GPannel.hpp"
#include "tab/tools/Commands.hpp"
#include "BarView.hpp"

#include "g0/Threads.hpp"


namespace gtmy {

    class TabView;
    using ViewPull = std::vector<BarView>;

    class TrackView : public GView
    {
    protected:
        Track* _pTrack;

        TabView* _tabParrent;
        std::unique_ptr<ThreadLocal> _animationThread; //Подумать над хранением, возможно удастся спрятать?
        std::vector<std::unique_ptr<ThreadLocal>> _finishPool;

        ViewPull _barsPull;

        GPannel* _pan; //TODO просто выпилить и ниже
        std::unique_ptr<GTrackPannel> _trackPan;
        std::unique_ptr<GEffectsPannel> _effPan;
        std::unique_ptr<GClipboardPannel> _clipPan;

    public:

        Track* getTrack() const { return _pTrack; }

        virtual void setUI();
        virtual bool isMovableY() { return true; } //Emm

        TrackView(Track *from):_pTrack(from), _pan(0) {
            _trackPan = std::make_unique<GTrackPannel>(300,480,800);
            _effPan = std::make_unique<GEffectsPannel>(300,480,800);
            _clipPan = std::make_unique<GClipboardPannel>(300,480,800);
            _trackPan->setPressView(this);
            _effPan->setPressView(this);
            _clipPan->setPressView(this);
            _effPan->preOpen();
            _clipPan->preOpen();
            _pan = _trackPan.get();
        }

        virtual ~TrackView();

        virtual void keyevent(std::string press);
        virtual void onTabCommand(TabCommand command);
        virtual void onTrackCommand(TrackCommand command);

        void onclick(int x1, int y1);
        void ondblclick(int x1, int y1);

        virtual void ongesture(int offset, bool horizontal);

        int horizonMove(int offset);

        void setDisplayBar(int barPosition);

        TabView *getPa() const { return _tabParrent; }
        void setPa(TabView* tv) { _tabParrent = tv; }

        void draw(QPainter *painter);

        void prepareThread(size_t shiftTheCursor);
        void launchThread();
        void stopThread();
        void connectThreadSignal(MasterView *masterView);
        int threadSeconds() { return _animationThread->calculateSeconds(); }


        bool gotChanges();

    };

}

#endif // TRACKVIEW_H
