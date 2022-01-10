#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <vector>
#include <memory>

#include "GView.hpp"
#include "BarView.hpp"
#include "tab/tools/Commands.hpp"
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

    public:

        Track* getTrack() const { return _pTrack; }

        virtual void setUI() override;
        virtual bool isMovableY() override { return true; } //Emm

        TrackView(Track *from): _pTrack(from) {}

        virtual ~TrackView();

        virtual void keyevent(std::string press) override;
        virtual void onTabCommand(TabCommand command) override;
        virtual void onTrackCommand(TrackCommand command) override;

        void onclick(int x1, int y1) override;
        void ondblclick(int x1, int y1) override;

        virtual void ongesture(int offset, bool horizontal) override;

        int horizonMove(int offset);

        void setDisplayBar(int barPosition);

        TabView *getPa() const { return _tabParrent; }
        void setPa(TabView* tv) { _tabParrent = tv; }

        void draw(QPainter *painter) override;

        void prepareThread(size_t shiftTheCursor);
        void launchThread();
        void stopThread();
        void connectThreadSignal(MasterView *masterView);
        int threadSeconds() { return _animationThread->calculateSeconds(); }


        bool gotChanges() const;

    };

}

#endif // TRACKVIEW_H
