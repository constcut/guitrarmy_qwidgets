#ifndef REC_H
#define REC_H

#include <QByteArray>

#include "g0/types.h"

#include "g0/tab.h"


//Tuner

class TunerInstance
{
    //singleton
protected:
    void *masterRepainter;
    void *label;
//#include <QMutex> //for tuner only

    double freq;

public:
    TunerInstance();

    static TunerInstance *getInst();
    static TunerInstance *mainInst;

    void setFreq(double newFreq);
    double getFreq();

    void setViews(void *mast, void *lab);

};


//Recognizer

class GWave;

struct RecognizedNote
{
    int noteBegin;
    int notePeak;
    int noteEnd;

    double freq;

    int summEnergy; /// fff ff f mp p pp pp
};

class ScaledWave
{
protected:
    std::vector<short> waveData;
    double coef;

public:
    int makeFrom(GWave *wav, double scaleCoef);

    //tick scale another way - when
    //each time we recieve only one
    //sample 16bit length
};


class BaseRhy
{
public:
    virtual void findNotesPositions(std::vector<int> *energyLevels, std::vector<int> *energyTypes, std::vector<RecognizedNote> *notes, std::vector<int> *params);

};

class BaseMel
{
public:
    //used to set up freq on founded rhy notes
    virtual void setupFreq(short *source, int bpmWindow, std::vector<RecognizedNote> *notes);
};

class GWave
{
protected:

    std::vector<int> scaleValue;
    std::vector<ScaledWave> scaledLines;

    BaseRhy rAn;
    BaseMel mAn;

public:
    QByteArray origin;//hide later
    std::vector<RecognizedNote> notes;
    std::vector<int> energyLevels;
    std::vector<int> energyTypes;

    Bar *innerBar;


    byte runRythmicAnalyse();
    byte runMelodicAnalyse();

    bool loadFile(std::string fileName, std::vector<int> *params);

    Track *generateTrack();
    Bar *generateBar();//short *source, int &cursor);

};




//Recorder-mixer






#endif // REC_H

