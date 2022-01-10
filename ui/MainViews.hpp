#ifndef COMMONVIEWS_H
#define COMMONVIEWS_H

#include "GView.hpp"
#include "GPannel.hpp"
#include "g0/Tests.hpp"
#include "InputViews.hpp"

#include <memory>


namespace gtmy {


    class TabView;
    class TestsView;
    class ConfigView;
    class TapRyView;
    class PatternInput;
    class GStickPannel;

    class InfoView;
    class MorzeInput;

    class WelcomeView;

    class BendInput;
    class ChordInput;
    class ChangesInput;


    class MainView : public GView
    {
    private:

        std::unique_ptr<TabView> _tabsView;
        std::unique_ptr<TestsView> _testsView;
        std::unique_ptr<ConfigView> _configView;
        std::unique_ptr<TapRyView> _tapRyView;
        std::unique_ptr<PatternInput> _patternInp;

        std::unique_ptr<InfoView> _infView;
        std::unique_ptr<MorzeInput> _morzeInp;

        std::unique_ptr<BendInput> _bendInp;
        std::unique_ptr<ChangesInput> _chanInp;
        std::unique_ptr<ChordInput> _chordInp;

        std::unique_ptr<WelcomeView> _welcome;

        std::unique_ptr<GStickPannel> _pan;

        GView* _currentView;
        std::vector<GView*> _lastViews;

        void changeViewToLast();

        bool _dependent;

     public:

        void setDependent() {_dependent=true;}

        void changeCurrentView(GView* newView);

        GView *getCurrenView() { return _currentView; }

        MainView();
        void draw(QPainter *painter);
        void onclick(int x1, int y1);
        void ondblclick(int x1, int y1);

        void ongesture(int offset, bool horizontal) {
            if (_currentView)
                _currentView->ongesture(offset,horizontal);
        }

        virtual void keyevent(std::string press);
        virtual void onTabCommand(TabCommand command);
        virtual void onTrackCommand(TrackCommand command);

        bool isPlaying();

        virtual void setMaster(MasterView *mast);
    };


    class WelcomeView : public GView
    {
    private:
        std::unique_ptr<GLabel> _top;
        std::unique_ptr<GLabel> _mid;
        std::unique_ptr<GLabel> _bot;

    public:
        WelcomeView();
        void draw(QPainter *painter);
        void onclick([[maybe_unused]]int x1, [[maybe_unused]]int y1)
        {}
    };




    class ConfigView : public GView
    {
    private:

        std::unique_ptr<GLabel> _labA, _labB;
        std::unique_ptr<GLabel> _labC, _labD;

        std::unique_ptr<GLabel> _labScalePlus, _labScaleMinus;
    public:
        ConfigView();
        //~ConfigView(){}
        void draw(QPainter *painter);
        void onclick(int x1, int y1);
        virtual void keyevent(std::string press);

    };



    class InfoView : public GView
    {
    private:
        std::unique_ptr<GLabel> _labelA;
        std::unique_ptr<GLabel> _labelA2;
        std::unique_ptr<GLabel> _icons;
        std::unique_ptr<GLabel> _labelB;
        std::unique_ptr<GLabel> _labelC;
        std::unique_ptr<GLabel> _labelD;
        std::unique_ptr<GLabel> _labelE;
        std::unique_ptr<GLabel> _help;
        std::unique_ptr<GLabel> _logShow;
        std::unique_ptr<GLabel> _sendCrash;

    public:
        InfoView();

        void onclick(int x1, int y1);
        void draw(QPainter *painter);
    };


    class TestsView : public GView
    {
    private:
        GLabel _upper, _bottom, _g3, _g5;
        std::unique_ptr<GLabel> _playlistButton;
        std::unique_ptr<GLabel> _playlist2Button;
        std::unique_ptr<GLabel> _stopPlaylist;

        std::vector<GLabel> _buttons;

        MainView *_mainView;
        TabView *_tabsView;

    public:
        TestsView(MainView *mainV, TabView *tabV);

        virtual void setUI();

        void setAllButtons();

        void draw(QPainter *painter);
        void onclick(int x1, int y1);
        void ondblclick(int x1, int y1);

        void fastTestAll();
        void openTestNumber(int num);

        virtual void keyevent(std::string press);
    };

    /// N ew inputs
    ///


    class ChordInput : public GView
    {
    private:
        std::unique_ptr<GLabel> _top;
    public:
        void setUI();

        ChordInput() {
            _top = std::make_unique<GLabel>(100,110,"Chord input","",false);
        }
        virtual ~ChordInput() {}

        void draw(QPainter *painter) {
            _top->draw(painter);
        }
        void keyevent([[maybe_unused]]std::string press){}
    };


    class ChangesInput : public GView
    {
    private:
        std::unique_ptr<GLabel> _top;

    public:

        static Beat *ptrToBeat;
        static void setPtrBeat(Beat *beatPtr);

        void setUI();

        ChangesInput() {
            _top = std::make_unique<GLabel>(10,10,"Changes input","",false);
        }
        virtual ~ChangesInput(){}

        void turnOffChange(std::string combo);
        void turnOnChange(std::string combo);

        void changeMainValue(int combo, int newValue);
        void changeSubValue(int combo, int newValue);

        void draw(QPainter *painter);
        void keyevent(std::string press);
    };


    class BendPoints;
    class Note;

    class BendInput : public GView
    {
    private:
        std::unique_ptr<GLabel> _top;

        std::unique_ptr<GLabel> _okButton;
        std::unique_ptr<GLabel> _delButton;
    public:

        static BendPoints *ptrToBend;
        static Note *ptrToNote;

        static void setPtrBend(BendPoints* ptr);
        static void setPtrNote(Note* ptr);

        void fillBend(int type, int height);

        void setUI();

        BendInput();

        virtual ~BendInput() {}
        void draw(QPainter *painter);
        void keyevent(std::string press);

        void onclick(int x1, int y1);
    };


}

#endif // COMMONVIEWS_H
