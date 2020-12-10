#ifndef INPUTVIEWS_H
#define INPUTVIEWS_H

#include "g0/tab.h"
#include "gview.h"

#include "g0/rec.h"

#include <QStringList>

class BarView;

class PatternInput : public GView
{
protected:

    std::vector<GCheckButton> checkButtons; //one track
    std::vector<GLabel> lineInstrLabels;

    Bar *bar;
    BarView *barView;

    GLabel *sigNumBut;
    GLabel *sigDenBut;

    GLabel *bpmBut;
    GLabel *bpmValue;

    GLabel *repeatLabel;
    GCheckButton *butRepeat;

    int currentDen;

    //Check buttons
    //bar
    //button to create the bar

public:

    virtual void setUI();

    void keyevent(std::string press);

    PatternInput():bar(0),barView(0),currentDen(4)
    {
        sigNumBut = new GLabel(100,330-75,"16");
        //sigNumBut->setH(50);

        sigDenBut = new GLabel(150,330-75,"16");
        //sigDenBut->setH(50);

        bpmBut = new GLabel(20,330-75,"new bpm");
        bpmValue = new GLabel(20,300-75,"120");

        //bpmBut->setH(50);

        butRepeat = new GCheckButton(100,310,70,30);

        repeatLabel = new GLabel(20,330,"Repeat bar:","",false);

        addButtons();
    }

    void addButtons()
    {
        int shX = 30;
        int shY = 100-55;

        for (int lines=0; lines < 4; ++lines)
        {
            shY = 100-55+50*lines;
            shX = 30;

            std::string instrText;
            switch(lines)
            {
                case 0: instrText = "57"; break;
                case 1: instrText = "49"; break;
                case 2: instrText = "38"; break;
                case 3: instrText = "36"; break;
            }

            GLabel lineLab(shX,shY,instrText);



            lineInstrLabels.push_back(lineLab);

            shX += 70;

            for (int i = 0; i < 16; ++i)
            {
                GCheckButton but(shX,shY-30,30,30);
                shX += but.getW() + 10;
                checkButtons.push_back(but);
            }
        }
    }

    virtual void draw(Painter *painter);
    virtual void onclick(int x1, int y1);

    void createBar();
    void playBar();
};




class TapRyView : public GView
{
protected:
    GLabel *labA;
    GLabel *stopMetr;
    GLabel *labB;

    GLabel *labClean;
    GLabel *labStat;
    GLabel *labExp;

    GLabel *bpmLabel;

    GRect leftPress;
    GRect rightPress;

    Bar *ryBar;

    BarView *barView;

    struct intPair
    {
        int first;
        int second;
    };

    std::vector<intPair> presses;
public:

    virtual void setUI();

    void measureTime(); //perfect to do in another thread

    TapRyView():leftPress(0,280,200,200),
        rightPress(600,280,200,200)
    {


        ryBar = 0;
        labA = new GLabel(50,100-80,"Launch pseudo-metronome");
        //labA->setH(40);
        labA->setW(labA->getW() + 20);

        stopMetr = new GLabel(300,100-80,"stop");
        //stopMetr->setH(40);

        labB = new GLabel(50,200-55,"Please click here :)");
        //labB->setH(40);
        labB->setVisible(false);

        labClean = new GLabel(500,100-80,"clean");
        //labClean->setH(40);

        labStat = new GLabel(50,10,"Info:                 ");
        labStat->setBorder(false);
        labStat->setVisible(false);

        labExp = new GLabel(200,150-80,"stop record");
        //labExp->setH(40);
        labExp->setVisible(false);

        bpmLabel = new GLabel(400,100-80,"120");
       // bpmLabel->setH(40);

        barView = 0;
    }

    void draw(Painter *painter);
    void onclick(int x1, int y1);

    void createBar();
    void copyAndPlayBar();


    void keyevent(std::string press);

    void ondblclick(int x1, int y1)
    {
        //onclick(x1,y1);
    }
   // void keyevent(std::string press)

};

class MorzeInput : public GView
{
protected:
    Bar *bar;
    BarView *barView;
    GLabel *createBut;

public:

    virtual void setUI();

    MorzeInput():bar(0),barView(0)
    {
        createBut = new GLabel(100,100,"create tab from text");
    }

    virtual void keyevent(std::string keypress);
    virtual void draw(Painter *painter);
    virtual void onclick(int x1, int y1);

    void playBar();
};


class RecordView : public GView
{
protected:

    Bar *bar;
    BarView *barView;


    GLabel *zoom;
    GLabel *bpm;

    GLabel *eLevel1;
    GLabel *eLevel2;
    GLabel *eLevel3;

    int wavePosition;

    int waveLimit;
    void *recorderPtr;

    bool recording;
    bool playing;
    //int windowWave[500000]; //for test first
    //int energyLevels[4000];
    //byte energyLevelTypes[4000];
    //byte wtypes[500000];

    TunerInstance tunerItself;
    GLabel *tunerLabel;

    QStringList recFiles;
    std::string currentFile;

    GWave waveItself; //move inside

public:
    RecordView():bar(0),barView(0),wavePosition(0),recorderPtr(0),waveLimit(0)
    {
        //status = new GLabel(60,100-55-20,"Record not started");


        recording = 0;
        playing = 0;

        zoom = new GLabel(20,100-55-20,"10");
        bpm = new GLabel(60,100-55-20,"120");

        eLevel1 = new GLabel(100,100-55-20,"500");
        eLevel2 = new GLabel(150,100-55-20,"850");
        eLevel3 = new GLabel(200,100-55-20,"500");

        currentFile = "record.graw";
        loadCurrentFile();

        tunerLabel = new GLabel(20,25+30,"tuner-is-off");
    }

    virtual void setUI();

    void loadCurrentFile();


    virtual void draw(Painter *painter);
    virtual void onclick(int x1, int y1);

    virtual void keyevent(std::string press);

    virtual void ongesture(int offset, bool horizontal)
    {
        if (horizontal)
        {
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
