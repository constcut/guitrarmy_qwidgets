#ifndef ATHREAD_H
#define ATHREAD_H

#include <QThread>

#include "g0/types.h"



class AThread
{
public:
    AThread();

    virtual void threadRun() = 0;

    //virtual void startThread()=0;
    virtual void callUpdate() = 0;
    virtual void noticeFinished() = 0;

    //int thrType;
};


class PlayAnimationThr: public AThread
{
protected:
    ul *incrementA;
    ul *incrementB;

    ul limit;
    //MasterView *mv;

    int bpm;

    std::vector<int> waitTimes;
    std::vector<int> waitIndexes;

    std::vector< std::vector<int> > beatTimes;

    int status;

    bool pleaseStop;

public:

    void requestStop() { pleaseStop = true; }

   PlayAnimationThr():incrementA(0),incrementB(0),limit(0),bpm(1),status(0),pleaseStop(false) {}

   int getStatus() { return status; }

   void setBPM(int newBPM) { bpm = newBPM; waitTimes.clear(); }
   void setLimit(ul max) { limit = max; }
   void setInc(ul *ptrA, ul *ptrB) { incrementA = ptrA; incrementB = ptrB; }
   //void setMasterView(MasterView *mvPtr);// {mv=mvPtr;}

   void setupValues(void *tab_ptr, void *track_ptr, ul shiftTheCursor);

   void addNumDenum(byte nu, byte de, ul nextIndex);

   void addBeatTimes(void *bar);

   void threadRun();

   int calculateSeconds();

protected:
   virtual void sleepThread(int ms)=0;
};

class WaveMoveThr: public AThread
{
protected:
    int *incrementA;
    ul *incrementB;

    int limit;
    //MasterView *mv;

    int status;
    bool pleaseStop;

public:

   void requestStop() { pleaseStop = true; }

   WaveMoveThr():incrementA(0),incrementB(0),limit(0),status(0),pleaseStop(false) {}

   int getStatus() { return status; }

   void setLimit(int max) { limit = max; }
   void setInc(int *ptrA, ul *ptrB) { incrementA = ptrA; incrementB = ptrB; }
   //void setMasterView(MasterView *mvPtr);// {mv=mvPtr;}



   void threadRun();

protected:
   virtual void sleepThread(int ms)=0;
};



class ThreadLocal : public QThread ,public PlayAnimationThr
{
    Q_OBJECT
signals:
    void updateUI();
    void nowFinished();

public:
    void run() Q_DECL_OVERRIDE {
        threadRun();
    }

    void noticeFinished()
    {
        emit nowFinished();
    }

    void callUpdate() {
        emit updateUI();
    }

    void sleepThread(int ms) {
        msleep(ms);
    }
};

class ThreadWave : public QThread ,public WaveMoveThr
{
    Q_OBJECT
signals:
    void updateUI();
    void nowFinished();

public:
    void run() Q_DECL_OVERRIDE {
        threadRun();
    }

    void callUpdate() {
        emit updateUI();
    }

    void noticeFinished()
    {
        emit nowFinished();
    }

    void sleepThread(int ms) {
        msleep(ms);
    }
};

#endif // ATHREAD_H
