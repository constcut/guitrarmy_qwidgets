#ifndef CENTERVIEW_H
#define CENTERVIEW_H

#include "gview.h"

#include "mainviews.h"
#include "tabviews.h"

#include <unordered_map>
#include "tab/tabcommands.h"
#include "audio/audiospeaker.h"


#include <QWidget>
#include <QMutex>
#include <QByteArray>

#include <QAudioInput>
#include <QGesture>

#include <QTextBrowser>
#include <QTextEdit>

#include <QPushButton>
#include <QComboBox>


#include <QScrollArea>


class GQButton : public QPushButton //TODO в отдельные файлы
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

    std::unique_ptr<AudioInfo> audioInfo;  //Выделить фрагменты аудиодвижка в один класс, использовать его здесь
    std::unique_ptr<QAudioInput> audioInput;

    int lastPressX,lastPressY;

    std::unique_ptr<QTextBrowser> welcomeText;
    std::unique_ptr<QTextEdit> confEdit;

    MasterView *statusSetter;
    QScrollArea *fatherScroll;


    std::map < int , std::vector<QWidget*> > uiWidgets;

    //TODO возможно перенести на этап загрузки конфигурации, а лучше вообще избавиться от строк
    std::unordered_map<std::string, TabCommand> tabCommands;
    std::unordered_map<std::string, TrackCommand> trackCommands;


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

    void draw(QPainter *painter);
    void onclick(int x1, int y1);
    void ondblclick(int x1, int y1);
    void ongesture(int offset, bool horizontal) ;

    void showHelp();
    void showConf();
    void fitTextBrowser();

    void connectThread(std::unique_ptr<ThreadLocal>& localThrlocalThr);
    void connectMainThread(std::unique_ptr<ThreadLocal>& localThrlocalThr);

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
