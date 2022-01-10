#ifndef TABVIEWS_H
#define TABVIEWS_H

#include <vector>
#include <memory>

#include "GView.hpp"
#include "tab/tools/TabClipboard.hpp"
#include "tab/tools/Commands.hpp"

#include "TrackView.hpp"


namespace gtmy {


    //TODO функции в отдельный хэдер
    void changeColor(const std::string& color, QPainter* src);
    void drawEllipse(QColor c, QPainter *painter, int x, int y, int w, int h);
    void drawEllipse(QPainter *painter, int x, int y, int w, int h);
    int translateDefaulColor(const std::string& confParam);



    class TabView : public GView
    {
    protected:
        std::unique_ptr<Tab> pTab;

        std::unique_ptr<GLabel> statusLabel;
        std::unique_ptr<GLabel> bpmLabel;

        std::unique_ptr<ThreadLocal> localThr; //Подумать, можно ли перенести?
        std::vector<std::unique_ptr<ThreadLocal>> finishPool;

    public:

        virtual void setUI() override;

        void setCurrentBar(int curBar) {
            pTab->getCurrentBar() = curBar; }

        virtual bool isMovableX() override { return true; }
        virtual bool isMovableY() override { return true; }

        TabView();
        virtual ~TabView();

        std::vector<std::unique_ptr<TrackView>> tracksView;

        int getCurTrack() { return pTab->getCurrentTrack(); }
        int getLastOpenedTrack() { return pTab->getLastOpenedTrack(); }

        void setTab(std::unique_ptr<Tab> point2Tab);// {pTab = point2Tab;}
        void refreshTabStats();
        std::unique_ptr<Tab>& getTab() { return pTab; }

         void draw(QPainter *painter) override;

         virtual void keyevent(std::string press) override;
         virtual void onTabCommand(TabCommand command) override;
         virtual void onTrackCommand(TrackCommand command) override;

         void setPlaying(bool playValue) { pTab->setPlaying(playValue); }
         bool getPlaying();

         void addSingleTrack(Track *track);

         void onclick(int x1, int y1) override;
         void ondblclick(int x1, int y1) override;

         virtual void ongesture(int offset, bool horizontal) override;

         void prepareAllThreads(size_t shiftTheCursor);
         void launchAllThreads();
         void stopAllThreads();

         void connectAllThreadsSignal(MasterView *masterView);

         bool gotChanges() const;

         //void ondblclick(int x1, int y1);
    };


}


#endif // TABVIEWS_H
