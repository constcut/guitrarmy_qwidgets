#ifndef THREADS_HPP
#define THREADS_HPP

#include <vector>
#include <stdint.h>

#include <QThread>


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


class Tab;
class Track;
class Bar;


class PlayAnimationThr: public AThread
{
protected:
    size_t *incrementA;
    size_t *incrementB;

    size_t limit;
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
   void setLimit(size_t max) { limit = max; }
   void setInc(size_t *ptrA, size_t *ptrB) { incrementA = ptrA; incrementB = ptrB; }
   //void setMasterView(MasterView *mvPtr);// {mv=mvPtr;}

   void setupValues(Tab* tab_ptr, Track* track_ptr, size_t shiftTheCursor);

   void addNumDenum(uint8_t nu, uint8_t de, size_t nextIndex);

   void addBeatTimes(Bar* bar);

   void threadRun();

   int calculateSeconds();

protected:
   virtual void sleepThread(int ms)=0;
};


class ThreadLocal : public QThread, public PlayAnimationThr
{
    Q_OBJECT
signals:
    void updateUI();
    void nowFinished();

public:
    void run() Q_DECL_OVERRIDE {
        threadRun();
    }

    void noticeFinished() override
    {
        emit nowFinished();
    }

    void callUpdate() override {
        emit updateUI();
    }

    void sleepThread(int ms) override {
        msleep(ms);
    }
};

#endif
