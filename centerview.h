#ifndef CENTERVIEW_H
#define CENTERVIEW_H

#include "gview.h"

#include "mainviews.h"
#include "tabviews.h"

#include <QWidget>
#include <QMutex>

#include <QByteArray>
#include <QIODevice>
#include <QAudioFormat>
#include <QAudioInput>

#include <QGesture>

#include <QTextBrowser>
#include <QTextEdit>

#include <QPushButton>
#include <QComboBox>

#include "apainter.h"

#include <QScrollArea>

class AudioInfo : public QIODevice
{
    Q_OBJECT

protected:

public: //temp operations
    QByteArray collector;

public:
    AudioInfo(const QAudioFormat &format, QObject *parent);
    ~AudioInfo();

    void start();
    void stop();

    qreal level() const { return m_level; }

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    const QAudioFormat m_format;
    quint32 m_maxAmplitude;
    qreal m_level; // 0.0 <= m_level <= 1.0

signals:
    void update();
};


class AudioSpeaker : public QIODevice
{
    Q_OBJECT

public:
    AudioSpeaker(const QAudioFormat &format, QObject *parent=0);
    ~AudioSpeaker();

    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;

    void setAudioBufer(QByteArray &aStream);

private:
    void generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate);

private:
    qint64 m_pos;
    QByteArray m_buffer;
};


class GQButton : public QPushButton
{
    Q_OBJECT
protected:
        int buttonNumber;
        std::string pressSyn;
        MasterView *keyPress;
public:
    GQButton(QWidget *pa=0);

    void setButtonNum(int newBN)
    {    buttonNumber = newBN; }

    void setKeyPress(MasterView *mast)
    { keyPress = mast;}

    void setPressSyn(std::string newSyn)
    { pressSyn = newSyn; }

public slots:

    void buttonWasClicked();

};

class GQCombo : public QComboBox
{
    Q_OBJECT
protected:
    int elementNumber;
    MasterView *keyPress;

    std::string params;
    bool pushItem;
public:

    bool getPushItem() {return pushItem;}//option
    void setPushItem(bool pushNew){pushItem=pushNew;}

    void setElementNum(int newBN)
    {    elementNumber = newBN; }

    void setKeyPress(MasterView *mast)
    { keyPress = mast;}

    void setParams(std::string newParams)
    {params = newParams;}

    std::string getParams(){return params;}

    GQCombo(QWidget *pa=0);

public slots:
    void elementChosen(int index);
};

class CenterView : public QWidget, public MasterView
{
       Q_OBJECT

protected:
    QMutex keyMute;

    std::list<std::vector<std::string> > playlist;

    AudioInfo *audioInfo;
    QAudioInput *audioInput;

    int lastPressX,lastPressY;

    QTextBrowser *welcomeText;
    QTextEdit *confEdit;

    MasterView *statusSetter;
    QScrollArea *fatherScroll;

    std::map < int , std::vector<QWidget*> > uiWidgets;

    //std::vector<QPushButton*> testButtons;
    //std::vector<QComboBox*> patternInstruments;

    int lastCheckedView;

    int xOffsetGesture;
    int yOffsetGesture;
    bool isPressed;

public:
    void flushPressed()
    {  isPressed = false;  }
    void setFatherScroll(QScrollArea *fScroll)
    {   fatherScroll = fScroll;}


    void checkView()
    {
        int newView = getCurrentViewType();

        if (newView != lastCheckedView)
            ViewWasChanged();

        lastCheckedView = newView;
    }

    void ViewWasChanged();

    void SetButton(int index,std::string text, int x1, int y1, int w1, int h1, std::string pressSyn);
    void addButton(std::string text, int x1, int y1, int w1, int h1, std::string pressSyn);
    void addComboBox(std::string params, int x1, int y1, int w1, int h1, int forceValue);
    void setComboBox(int index, std::string params, int x1, int y1, int w1, int h1, int forceValue);

    void renewComboParams(int index, std::string params);
    void renewComboParams(GQCombo *newBox, std::string params);



    int getComboBoxValue(int index);

    virtual void requestHeight(int newH)
    { this->setMinimumHeight(newH);}

    virtual void requestWidth(int newW)
    { this->setMinimumWidth(newW);}

    CenterView *ownChild;
    CenterView(QWidget *parent=0);

    void setStatusSetter(MasterView *statSett) {statusSetter = statSett;}

    void draw(Painter *painter);
    void onclick(int x1, int y1);
    void ondblclick(int x1, int y1);
    void ongesture(int offset, bool horizontal) ;

    void showHelp();
    void showConf();
    void fitTextBrowser();

    void connectThread(void *localThr);
    void connectMainThread(void *localThr);

    virtual int getToolBarHeight();
    virtual int getStatusBarHeight();

    virtual int getWidth();
    virtual int getHeight();

    virtual void setStatusBarMessage(int index, std::string text, int timeOut=0);

    virtual void pushForceKey(std::string keyevent);
    virtual bool isPlaying();

    virtual void addToPlaylist(std::vector<std::string> playElement);
    virtual bool isPlaylistHere();
    virtual void goOnPlaylist();    //REFACT MOVE TO ORIGIN
    virtual void cleanPlayList();

    void pleaseRepaint();
    int getCurrentViewType();

    void startAudioInput();
    void stopAudioInput();

    void initAudioInput();
    //==============
    //==============

    //handling events
    void paintEvent(QPaintEvent *event);
    void mousePressEvent( QMouseEvent * event );
    void mouseDoubleClickEvent( QMouseEvent * event );

    void mouseMoveEvent( QMouseEvent *event );
    void keyPressEvent ( QKeyEvent * event );
    void mouseReleaseEvent(QMouseEvent *mEvent);
    bool gestureEvent(QGestureEvent *event);
    void resizeEvent(QResizeEvent* event);

public slots:

    void threadFinished();

};

#endif // CENTERVIEW_H
