#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "athread.h"

#include <QLabel>
#include <QProgressBar>

#include <QMouseEvent>

#include <QDockWidget>
#include <QToolBar>

#include <QScreen>
#include <QApplication>

#include <QStyle>

#include "centerview.h"

#include <QScrollArea>

#include <QAudioOutput>

#include <QTimer>

#include "g0/aclipboard.h"

namespace Ui {
class MainWindow;
}






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

    void pushForceKey(std::string keyevent);

    CenterView* getCenterView() { return center; }
    void setCenterView(CenterView* newCenter) { center = newCenter; }

    bool eventFilter(QObject *object, QEvent *e);

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

   QAction* addToolButton(QToolBar *toolBar, std::string button, std::string confValue);

    void moveEvent(QMoveEvent *ev);
    virtual void setStatusBarMessage(int index, std::string text, int timeOut=0);

    void pleaseRepaint()
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
    void paintEvent(QPaintEvent *event);
    void mousePressEvent( QMouseEvent * event );
    void mouseDoubleClickEvent( QMouseEvent * event );
    void keyPressEvent ( QKeyEvent * event );

    void mouseReleaseEvent(QMouseEvent *mEvent);

    bool event(QEvent *event) Q_DECL_OVERRIDE;
    bool gestureEvent(QGestureEvent *event);

    void resizeEvent(QResizeEvent* event);


    void createUI();
    void recreateUI();

    void createMenuTool();

    void createMainToolbar();
    void createFloDocks();

    QDockWidget* createToolDock(std::string dockname,void *pannel);
    QMenu *createToolMenu(void *pannel);

    void setViewPannel(int val)
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



/*
class NewStyle : public QCommonStyle
{
public:
    virtual int pixelMetric(PixelMetric pm, const QStyleOption* option, const QWidget* widget) const override
    {
        if ((pm == QStyle::PM_ToolBarExtensionExtent) ||
            (pm == QStyle::PM_ToolBarSeparatorExtent) ||
                (pm == QStyle::PM_ToolBarHandleExtent))
        {

            return 100;
        }
        return QCommonStyle::pixelMetric(pm, option, widget);
    }

    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption * option = 0, const QWidget * widget = 0);

};

QIcon NewStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption * option = 0, const QWidget * widget = 0)
{
    std::string iconPlace = ":/icons/^" + std::string(".png");

    QIcon icon(iconPlace.c_str());

    switch (standardIcon)
       {
       case QStyle::SP_ToolBarHorizontalExtensionButton :
           return icon;

       case QStyle::SP_ToolBarVerticalExtensionButton :
           return icon;
       }
       return QCommonStyle::standardIcon(standardIcon, option, widget);
}
*/

#endif // MAINWINDOW_H
