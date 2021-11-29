#ifndef INPUTVIEWS_H
#define INPUTVIEWS_H

#include "tab/tab.h"
#include "gview.h"

#include "g0/rec.h"

#include <QStringList>

class BarView;

class PatternInput : public GView
{
protected:

    std::vector<GCheckButton> checkButtons; //one track
    std::vector<GLabel> lineInstrLabels;

    std::unique_ptr<Bar> bar;
    std::unique_ptr<BarView> barView;

    std::unique_ptr<GLabel> sigNumBut;
    std::unique_ptr<GLabel> sigDenBut;
    std::unique_ptr<GLabel> bpmBut;
    std::unique_ptr<GLabel> bpmValue;
    std::unique_ptr<GLabel> repeatLabel;
    std::unique_ptr<GCheckButton> butRepeat;

    int currentDen;

    //Check buttons
    //bar
    //button to create the bar

public:

    virtual void setUI();

    void keyevent(std::string press);

    PatternInput():currentDen(4)
    {
        sigNumBut = std::make_unique<GLabel>(100,330-75,"16");
        sigDenBut = std::make_unique<GLabel>(150,330-75,"16");
        bpmBut = std::make_unique<GLabel>(20,330-75,"new bpm");
        bpmValue = std::make_unique<GLabel>(20,300-75,"120");
        butRepeat = std::make_unique<GCheckButton>(100,310,70,30);
        repeatLabel = std::make_unique<GLabel>(20,330,"Repeat bar:","",false);
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
            lineInstrLabels.push_back(std::move(lineLab));
            shX += 70;
            for (size_t i = 0; i < 16; ++i) {
                GCheckButton but(shX,shY-30,30,30);
                shX += but.getW() + 10;
                checkButtons.push_back(but);
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
protected:
    std::unique_ptr<GLabel> labA;
    std::unique_ptr<GLabel> stopMetr;
    std::unique_ptr<GLabel> labB;

    std::unique_ptr<GLabel> labClean;
    std::unique_ptr<GLabel> labStat;
    std::unique_ptr<GLabel> labExp;
    std::unique_ptr<GLabel> bpmLabel;

    GRect leftPress;
    GRect rightPress;

    std::unique_ptr<Bar> ryBar;
    std::unique_ptr<BarView> barView;

    struct intPair {
        int first;
        int second;
    };

    std::vector<intPair> presses;
public:

    virtual void setUI();

    void measureTime(); //perfect to do in another thread

    TapRyView():leftPress(0,280,200,200), rightPress(600,280,200,200) {
        labA = std::make_unique<GLabel>(50,100-80,"Launch pseudo-metronome");
        labA->setW(labA->getW() + 20);
        stopMetr = std::make_unique<GLabel>(300,100-80,"stop");
        labB = std::make_unique<GLabel>(50,200-55,"Please click here :)");
        labB->setVisible(false);
        labClean = std::make_unique<GLabel>(500,100-80,"clean");
        labStat = std::make_unique<GLabel>(50,10,"Info:                 ");
        labStat->setBorder(false);
        labStat->setVisible(false);
        labExp = std::make_unique<GLabel>(200,150-80,"stop record");
        labExp->setVisible(false);
        bpmLabel = std::make_unique<GLabel>(400,100-80,"120");
        barView = 0;
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
protected:
    std::unique_ptr<Bar> bar;
    std::unique_ptr<BarView> barView;
    std::unique_ptr<GLabel> createBut;

public:

    virtual void setUI();

    MorzeInput() {
        createBut = std::make_unique<GLabel>(100,100,"create tab from text");
    }

    virtual void keyevent(std::string keypress);
    virtual void draw(QPainter *painter);
    virtual void onclick(int x1, int y1);

    void playBar();
};


class RecordView : public GView
{
protected:

    std::unique_ptr<Bar> bar;
    std::unique_ptr<BarView> barView;


    std::unique_ptr<GLabel> zoom;
    std::unique_ptr<GLabel> bpm;

    std::unique_ptr<GLabel> eLevel1;
    std::unique_ptr<GLabel> eLevel2;
    std::unique_ptr<GLabel> eLevel3;

    int wavePosition;

    int waveLimit;
    void *recorderPtr = nullptr;

    bool recording;
    bool playing;
    //int windowWave[500000]; //for test first
    //int energyLevels[4000];
    //byte energyLevelTypes[4000];
    //byte wtypes[500000];

    TunerInstance tunerItself;
    std::unique_ptr<GLabel> tunerLabel;

    QStringList recFiles;
    std::string currentFile;

    GWave waveItself; //move inside

public:
    RecordView():wavePosition(0),waveLimit(0),recorderPtr(0) {
        recording = 0;
        playing = 0;
        zoom = std::make_unique<GLabel>(20,100-55-20,"10");
        bpm = std::make_unique<GLabel>(60,100-55-20,"120");

        eLevel1 = std::make_unique<GLabel>(100,100-55-20,"500");
        eLevel2 = std::make_unique<GLabel>(150,100-55-20,"850");
        eLevel3 = std::make_unique<GLabel>(200,100-55-20,"500");

        currentFile = "record.graw";
        loadCurrentFile();

        tunerLabel = std::make_unique<GLabel>(20,25+30,"tuner-is-off");
    }

    virtual void setUI();

    void loadCurrentFile();


    virtual void draw(QPainter *painter);
    virtual void onclick(int x1, int y1);

    virtual void keyevent(std::string press);

    virtual void ongesture(int offset, bool horizontal) {
        if (horizontal) {
            int zoomCoef = atoi(zoom->getText().c_str());
            int PREwavePosition = wavePosition + offset*zoomCoef;
            if (PREwavePosition < 0) //SCROLL GOOD BACK!
                wavePosition = 0;
            else
                if (PREwavePosition > waveLimit)
                    wavePosition = waveLimit - 1;
                else
                    wavePosition = PREwavePosition;
        }
    }
};


#endif // INPUTVIEWS_H
