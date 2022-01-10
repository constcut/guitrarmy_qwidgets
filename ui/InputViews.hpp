#ifndef INPUTVIEWS_H
#define INPUTVIEWS_H

#include "tab/Tab.hpp"
#include "ui/BarView.hpp"

#include <QStringList>
#include <QAudioRecorder>


namespace gtmy {


    class PatternInput : public GView
    {
    private:

        std::vector<GCheckButton> _checkButtons; //one track
        std::vector<GLabel> _lineInstrLabels;

        std::unique_ptr<Bar> _bar;
        std::unique_ptr<BarView> _barView;

        std::unique_ptr<GLabel> _sigNumBut;
        std::unique_ptr<GLabel> _sigDenBut;
        std::unique_ptr<GLabel> _bpmBut;
        std::unique_ptr<GLabel> _bpmValue;
        std::unique_ptr<GLabel> _repeatLabel;
        std::unique_ptr<GCheckButton> _butRepeat;

        int _currentDen;

    public:

        virtual void setUI();

        void keyevent(std::string press);

        PatternInput():_currentDen(4)
        {
            _sigNumBut = std::make_unique<GLabel>(100,330-75,"16");
            _sigDenBut = std::make_unique<GLabel>(150,330-75,"16");
            _bpmBut = std::make_unique<GLabel>(20,330-75,"new bpm");
            _bpmValue = std::make_unique<GLabel>(20,300-75,"120");
            _butRepeat = std::make_unique<GCheckButton>(100,310,70,30);
            _repeatLabel = std::make_unique<GLabel>(20,330,"Repeat bar:","",false);
            addButtons();
        }

        void addButtons() {
            int shX = 30;
            int shY = 100-55;
            for (int lines=0; lines < 4; ++lines) {
                shY = 100-55+50*lines;
                shX = 30;
                std::string instrText;
                switch(lines) {
                    case 0: instrText = "57"; break;
                    case 1: instrText = "49"; break;
                    case 2: instrText = "38"; break;
                    case 3: instrText = "36"; break;
                }
                GLabel lineLab(shX,shY,instrText);
                _lineInstrLabels.push_back(std::move(lineLab));
                shX += 70;
                for (size_t i = 0; i < 16; ++i) {
                    GCheckButton but(shX,shY-30,30,30);
                    shX += but.getW() + 10;
                    _checkButtons.push_back(but);
                }
            }
        }

        virtual void draw(QPainter *painter);
        virtual void onclick(int x1, int y1);

        void createBar();
        void playBar();
    };




    class TapRyView : public GView
    {
    private:
        std::unique_ptr<GLabel> _labA;
        std::unique_ptr<GLabel> _stopMetr;
        std::unique_ptr<GLabel> _labB;

        std::unique_ptr<GLabel> _labClean;
        std::unique_ptr<GLabel> _labStat;
        std::unique_ptr<GLabel> _labExp;
        std::unique_ptr<GLabel> _bpmLabel;

        GRect _leftPress;
        GRect _rightPress;

        std::unique_ptr<Bar> _ryBar;
        std::unique_ptr<BarView> _barView;

        using intPair = std::pair<int, int>;
        std::vector<intPair> _presses;

    public:

        virtual void setUI();

        void measureTime(); //perfect to do in another thread

        TapRyView():_leftPress(0,280,200,200), _rightPress(600,280,200,200) {
            _labA = std::make_unique<GLabel>(50,100-80,"Launch pseudo-metronome");
            _labA->setW(_labA->getW() + 20);
            _stopMetr = std::make_unique<GLabel>(300,100-80,"stop");
            _labB = std::make_unique<GLabel>(50,200-55,"Please click here :)");
            _labB->setVisible(false);
            _labClean = std::make_unique<GLabel>(500,100-80,"clean");
            _labStat = std::make_unique<GLabel>(50,10,"Info:                 ");
            _labStat->setBorder(false);
            _labStat->setVisible(false);
            _labExp = std::make_unique<GLabel>(200,150-80,"stop record");
            _labExp->setVisible(false);
            _bpmLabel = std::make_unique<GLabel>(400,100-80,"120");
            _barView = 0;
        }

        void draw(QPainter *painter);
        void onclick(int x1, int y1);

        void createBar();
        void copyAndPlayBar();

        void keyevent(std::string press);

        void ondblclick([[maybe_unused]]int x1, [[maybe_unused]]int y1)
        {}
    };


    class MorzeInput : public GView
    {
    private:

        std::unique_ptr<Bar> _bar;
        std::unique_ptr<BarView> _barView;
        std::unique_ptr<GLabel> _createBut;

    public:

        virtual void setUI();

        MorzeInput() {
            _createBut = std::make_unique<GLabel>(100,100,"create tab from text");
        }

        virtual void keyevent(std::string keypress);
        virtual void draw(QPainter *painter);
        virtual void onclick(int x1, int y1);

        void playBar();
    };

}


#endif // INPUTVIEWS_H
