#ifndef ATHREAD_H
#define ATHREAD_H

#include <QThread>
#include "g0/Threads.hpp"


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


class ThreadWave : public QThread, public WaveMoveThr
{
    Q_OBJECT
signals:
    void updateUI();
    void nowFinished();

public:
    void run() Q_DECL_OVERRIDE {
        threadRun();
    }

    void callUpdate() override {
        emit updateUI();
    }

    void noticeFinished() override
    {
        emit nowFinished();
    }

    void sleepThread(int ms) override {
        msleep(ms);
    }
};

#endif // ATHREAD_H
