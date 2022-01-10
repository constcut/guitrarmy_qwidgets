#ifndef GVIEW_H
#define GVIEW_H
//This deprecated file formated with clang-format

#include <time.h>

#include <QDebug>
#include <QPainter>

#include "g0/Config.hpp"
#include "tab/Tab.hpp"
#include "tab/tools/Commands.hpp"
#include "ui/ImagePreloader.hpp"


namespace gtmy {


class MasterView;


    class GView {
     protected:
      int x;
      int y;
      int w;
      int h;

      MasterView *_master;

     public:
      GView() : x(0), y(0), w(0), h(0), _master(0) {}
      GView(int xV, int yV, int wV, int hV)
          : x(xV), y(yV), w(wV), h(hV), _master(0) {}
      virtual ~GView() {}

      virtual void setMaster(MasterView *mast) { _master = mast; }
      MasterView *getMaster() { return _master; }

      virtual bool isMovableX() { return false; }
      virtual bool isMovableY() { return false; }

      virtual void setUI() {}

      int getX() const { return x; }
      int getY() const { return y; }
      int getW() const { return w; }
      int getH() const { return h; }

      void setW(int newW) { w = newW; }
      void setH(int newH) { h = newH; }
      void setX(int newX) { x = newX; }
      void setY(int newY) { y = newY; }

      virtual bool hit(int hX, int hY);
      virtual void draw([[maybe_unused]] QPainter *painter) {}  // virt
      virtual void onclick([[maybe_unused]] int x1, [[maybe_unused]] int y1) {}
      virtual void ondblclick([[maybe_unused]] int x1, [[maybe_unused]] int y1) {}                                                             // virt
      virtual void keyevent([[maybe_unused]] std::string press) {}
      virtual void ongesture([[maybe_unused]] int offset,
                             [[maybe_unused]] bool horizontal) {}
      virtual void onTabCommand([[maybe_unused]] TabCommand command) {}

      virtual void onTrackCommand([[maybe_unused]] TrackCommand command) {
        qDebug() << "EMPY on TrackCommand";
      }

    };


    class GImage : public GView {
     private:
      std::string _imageName;

     public:
      GImage(int x, int y, std::string imgName)
          : GView(x, y, 36, 36), _imageName(imgName) {}

      void draw(QPainter *painter) {
        QImage *img = (QImage *)ImagePreloader::getInstance().getImage(_imageName);
        if (img)
            painter->drawImage(x, y, *img);
      }
    };


    class GRect : public GView {
      int _colorPress;

     public:
      GRect(int x, int y, int w, int h) : GView(x, y, w, h), _colorPress(0) {}

      void pressIt() {
        ++_colorPress;
        if (_colorPress > 10) _colorPress = 0;
      }

      void draw(QPainter *painter) {
        painter->drawRect(x, y, w, h);
        painter->fillRect(x + 10, y + 10, w - 20, h - 20, _colorPress);
      }
    };


    class GLabel : public GView {
     private:
      std::string _ownText;
      std::string _pressSynonim;
      std::unique_ptr<GImage> _imageLabel;
      bool _visible;
      bool _showBorder;

     public:
      bool isVisible() const { return _visible; }
      void setVisible(bool value) { _visible = value; }

      void setBorder(bool nowShowBorder) { _showBorder = nowShowBorder; }

      void setText(std::string newText) { _ownText = newText; }
      std::string getText() const { return _ownText; }

      GLabel(int x, int y, std::string text, std::string pressSyn = "",
             bool showBord = true);

      std::string getPressSyn() const { return _pressSynonim; }

      void draw(QPainter *painter);

      virtual bool hit(int hX, int hY);
    };



    class ThreadLocal;

    class MasterView {
     private:

      GView* _child;
      GView* _firstChld;
      GView* _lastView;

     public:
      MasterView() : _child(0) {}
      MasterView(GView *newChild) : _child(newChild), _firstChld(newChild) {
        _child->setMaster(this);
        _lastView = _child;
      }

      GView *getChild() { return _child; }
      GView *changeChild(GView *newChild);
      GView *resetToFirstChild() { return changeChild(_firstChld); }
      GView *resetToLastView() { return changeChild(_lastView); }

      GView *getFirstChild() { return _firstChld; }

      virtual void connectThread([
          [maybe_unused]] std::unique_ptr<ThreadLocal> &thr) {}
      virtual void connectMainThread([
          [maybe_unused]] std::unique_ptr<ThreadLocal> &thr) {}

      virtual void pleaseRepaint() {}

      virtual int getWidth() { return 0; }
      virtual int getHeight() { return 0; }

      virtual void requestHeight([[maybe_unused]] int newH) {}
      virtual void requestWidth([[maybe_unused]] int newW) {}

      virtual int getToolBarHeight() { return 0; }
      virtual int getStatusBarHeight() { return 0; }

      virtual void SetButton([[maybe_unused]] int index,
                             [[maybe_unused]] std::string text,
                             [[maybe_unused]] int x1, [[maybe_unused]] int y1,
                             [[maybe_unused]] int w1, [[maybe_unused]] int h1,
                             [[maybe_unused]] std::string pressSyn) {}

      virtual void SetButton(int index, GLabel *w, std::string pressSyn) {
        SetButton(index, w->getText(), w->getX(), w->getY(), w->getW(), w->getH(),
                  pressSyn);
      }

      virtual void setComboBox([[maybe_unused]] int index,
                               [[maybe_unused]] std::string,
                               [[maybe_unused]] int x1, [[maybe_unused]] int y1,
                               [[maybe_unused]] int w1, [[maybe_unused]] int h1,
                               [[maybe_unused]] int forceValue) {}

      virtual void renewComboParams([[maybe_unused]] int index,
                                    [[maybe_unused]] std::string params) {}

      virtual void setViewPannel([[maybe_unused]] int val) {}
      virtual int getComboBoxValue([[maybe_unused]] int index) { return -1; }
      virtual void showHelp() {}
      virtual void setStatusBarMessage([[maybe_unused]] int index,
                                       [[maybe_unused]] std::string text,
                                       [[maybe_unused]] int timeOut = 0) {}
      virtual void pushForceKey([[maybe_unused]] std::string keyevent) {}
      virtual bool isPlaying() { return false; }
      virtual void addToPlaylist(std::vector<std::string> playElement) {
        Q_UNUSED(playElement);
      }
      virtual bool isPlaylistHere() { return false; }
      virtual void goOnPlaylist() {}
      virtual void cleanPlayList() {}
    };



    class GCheckButton : public GView {
     private:
      bool _checked;

     public:
      GCheckButton(int x1, int y1, int w1, int h1)
          : GView(x1, y1, w1, h1), _checked(false) {}

      bool isChecked() const { return _checked; }
      void draw(QPainter *painter);

      virtual void onclick([[maybe_unused]] int x1, [[maybe_unused]] int y1);
      virtual void ondblclick(int x1, int y1) { onclick(x1, y1); }
    };

}  // namespace gtmy

#endif  // GVIEW_H
