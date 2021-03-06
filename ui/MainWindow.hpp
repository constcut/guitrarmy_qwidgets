#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QMouseEvent>
#include <QDockWidget>
#include <QToolBar>
#include <QScreen>
#include <QApplication>
#include <QStyle>
#include <QScrollArea>
#include <QAudioOutput>
#include <QTimer>

#include "ui/CenterView.hpp"
#include "tab/tools/TabClipboard.hpp"


namespace Ui {
class MainWindow;
}


namespace gtmy {


    class GPannel;


    class MainWindow : public QMainWindow, public MasterView
    {
        Q_OBJECT

        int _lastPressX;
        int _lastPressY;

        QDockWidget *_dock, *_dock2, *_dock3, *_dock4;
        QDockWidget *_dock5, *_dock6, *_dock7, *_dock8, *_dock9;

        QMenu *_menu1, *_menu2, *_menu3, *_menu4, *_pannelsMenu;
        QToolBar *_menuToolBar;

        QAction *_hideA1, *_hideA2, *_hideA3, *_hideA4, *_hideA5;
        QMutex _keyMutex;

        GQCombo *_winCombo;

        AudioInfo *_pAudioInfo;
        QAudioInput *_pAudioInput;

        AClipboard _clip1, _clip2, _clip3;
        QScrollArea *_centerScroll;
        CenterView *_center;

        QLabel *_statusLabel;
        QLabel *_statusLabelSecond;
        QLabel *_statusLabelThird;
        QProgressBar *_statusProgress;

        QTimer* _audioPushTimer;

        AudioSpeaker* _audioSpeaker;
        QAudioOutput* _audioOutput;

        void handlePanelAction(int action, int row);

    public:

        explicit MainWindow(QWidget *parent = nullptr);
        virtual ~MainWindow() = default;

        CenterView* getCenterView() { return _center; }
        void setCenterView(CenterView* newCenter) { _center = newCenter; }

        void pleaseRepaint() override {
            update();
        }

        int getCurrentViewType();

        void startAudioInput();
        void stopAudioInput();
        void initAudioInput();
        void initAudioOutput();
        void startAudioOutput(std::string localName);
        void stopAudioOutput();

        void pushForceKey(std::string keyevent) override;

        bool eventFilter(QObject *object, QEvent *e) override;
        void paintEvent(QPaintEvent* event) override;
        void mousePressEvent(QMouseEvent* event ) override;
        void mouseDoubleClickEvent(QMouseEvent* event ) override;
        void keyPressEvent (QKeyEvent* event ) override;
        void mouseReleaseEvent(QMouseEvent* mEvent) override;
        bool event(QEvent* event) Q_DECL_OVERRIDE;
        bool gestureEvent(QGestureEvent* event);
        void resizeEvent(QResizeEvent* event) override;
        void moveEvent(QMoveEvent *ev) override;

        void createUI();
        void recreateUI();
        void createMenuTool();
        void createMainToolbar();
        void createFloDocks();
        QDockWidget* createToolDock(std::string dockname, GPannel *pannel);
        QMenu *createToolMenu(GPannel *pannel);
        QAction* addToolButton(QToolBar *toolBar, std::string button, std::string confValue);
        virtual void setStatusBarMessage(size_t index, std::string text, int timeOut=0) override;

        void setViewPannel(size_t val) override {
            if (_winCombo)
                _winCombo->setCurrentIndex(val);
        }

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

}


#endif // MAINWINDOW_H
