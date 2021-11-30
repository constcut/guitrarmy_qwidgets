#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui/athread.h"

#include <QLabel>
#include <QProgressBar>

#include <QMouseEvent>

#include <QDockWidget>
#include <QToolBar>

#include <QScreen>
#include <QApplication>

#include <QStyle>

#include "ui/centerview.h"

#include <QScrollArea>

#include <QAudioOutput>

#include <QTimer>

#include "tab/tabclipboard.h"

namespace Ui {
class MainWindow;
}



class GPannel;


class MainWindow : public QMainWindow, public MasterView
{
    Q_OBJECT

int lastPressX,lastPressY;
QDockWidget *dock,*dock2,*dock3,*dock4;
QDockWidget *dock5,*dock6,*dock7,*dock8,*dock9;
QMenu *menu1,*menu2,*menu3,*menu4,*pannelsMenu;
QToolBar *menuToolBar;

QAction *hideA1,*hideA2,*hideA3,*hideA4,*hideA5;
QMutex keyMute;

GQCombo *win;

//some audio

AudioInfo *audioInfo;
QAudioInput *audioInput;

AClipboard clip1,clip2,clip3;

QScrollArea *centerScroll; //refact scroll

CenterView *center;

public:

    void pushForceKey(std::string keyevent) override;

    CenterView* getCenterView() { return center; }
    void setCenterView(CenterView* newCenter) { center = newCenter; }

    bool eventFilter(QObject *object, QEvent *e) override;

    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

   QAction* addToolButton(QToolBar *toolBar, std::string button, std::string confValue);

    void moveEvent(QMoveEvent *ev) override;
    virtual void setStatusBarMessage(int index, std::string text, int timeOut=0) override;

    void pleaseRepaint() override
    {
        update();
    }

    int getCurrentViewType();

    void startAudioInput();
    void stopAudioInput();

    void initAudioInput();

    void initAudioOutput();

    void startAudioOutput(std::string localName);
    void stopAudioOutput();


    QTimer *audioPushTimer;

    AudioSpeaker *audioSpeaker;
    QAudioOutput *audioOutput;

    //==============
    //==============
    
    //handling events
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent( QMouseEvent * event ) override;
    void mouseDoubleClickEvent( QMouseEvent * event ) override;
    void keyPressEvent ( QKeyEvent * event ) override;

    void mouseReleaseEvent(QMouseEvent *mEvent) override;

    bool event(QEvent *event) Q_DECL_OVERRIDE;
    bool gestureEvent(QGestureEvent *event);

    void resizeEvent(QResizeEvent* event) override;


    void createUI();
    void recreateUI();

    void createMenuTool();

    void createMainToolbar();
    void createFloDocks();

    QDockWidget* createToolDock(std::string dockname, GPannel *pannel);
    QMenu *createToolMenu(GPannel *pannel);

    void setViewPannel(int val) override
    {
        if (win)
            win->setCurrentIndex(val);
    }


protected:
    void handlePanelAction(int action, int row);

    QLabel *statusLabel;
    QLabel *statusLabelSecond;
    QLabel *statusLabelThird;
    QProgressBar *statusProgress;

    //MainView view;

private slots:

    void on_action800x480_triggered();
    void on_action1024x600_triggered();
    void on_action1280x720_triggered();
    void on_actionExit_triggered();

public slots:

    void actionNow(QAction *action);

    void threadFinished();

    void audioTimerSlot();


private:
    Ui::MainWindow *ui;
};



#endif // MAINWINDOW_H
