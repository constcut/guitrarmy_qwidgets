#ifndef CENTERVIEW_H
#define CENTERVIEW_H

#include <unordered_map>

#include "GView.hpp"
#include "TabViews.hpp"
#include "Components.hpp"
#include "tab/tools/Commands.hpp"
#include "audio/AudioSpeaker.hpp"


#include <QWidget>
#include <QMutex>
#include <QByteArray>

#include <QAudioInput>
#include <QGesture>

#include <QTextBrowser>
#include <QTextEdit>
#include <QScrollArea>


namespace gtmy {


    class CenterView : public QWidget, public MasterView
    {
           Q_OBJECT

    private:

        QMutex _keyMutex;
        std::list<std::vector<std::string>> _playlist;

        std::unique_ptr<AudioInfo> _audioInfo;
        std::unique_ptr<QAudioInput> _audioInput;

        int _lastPressX;
        int _lastPressY;

        std::unique_ptr<QTextBrowser> _welcomeText;
        std::unique_ptr<QTextEdit> _confEdit;

        MasterView* _statusSetter;
        QScrollArea* _fatherScroll;

        std::map<int, std::vector<QWidget*>> _uiWidgets;

        std::unordered_map<std::string, TabCommand> _keyToTabCommandsMap;
        std::unordered_map<std::string, TrackCommand> _keyToTrackCommandsMap;

        int _lastCheckedView;

        int _xOffsetGesture;
        int _yOffsetGesture;
        bool _isPressed;

    public:

        void flushPressed() {
            _isPressed = false;
        }
        void setFatherScroll(QScrollArea *fScroll) {
            _fatherScroll = fScroll;
        }

        void checkView() {
            int newView = getCurrentViewType();
            if (newView != _lastCheckedView)
                ViewWasChanged();
            _lastCheckedView = newView;
        }

        void ViewWasChanged();

        void SetButton(size_t index, std::string text, int x1, int y1, int w1, int h1, std::string pressSyn);
        void addButton(std::string text, int x1, int y1, int w1, int h1, std::string pressSyn);
        void addComboBox(std::string params, int x1, int y1, int w1, int h1, int forceValue);
        void setComboBox(size_t index, std::string params, int x1, int y1, int w1, int h1, int forceValue);

        void renewComboParams(size_t index, std::string params);
        void renewComboParams(GQCombo *newBox, std::string params);

        int getComboBoxValue(size_t index);

        virtual void requestHeight(int newH) {
            this->setMinimumHeight(newH);
        }

        virtual void requestWidth(int newW) {
            this->setMinimumWidth(newW);
        }

        CenterView *ownChild;
        CenterView(QWidget *parent=0);

        void setStatusSetter(MasterView *statSett) {_statusSetter = statSett;}

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

        virtual void setStatusBarMessage(size_t index, std::string text, int timeOut=0);

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

}

#endif // CENTERVIEW_H
