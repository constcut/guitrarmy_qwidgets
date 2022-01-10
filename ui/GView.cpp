//This deprecated file formated with clang-format
#include "GView.hpp"

#include <QDebug>
#include <QFileDialog>

#include "g0/Config.hpp"

bool gViewLog = false;

using namespace gtmy;


GView *MasterView::changeChild(GView *newChild) {
  GView *oldCh = _child;

  _lastView = oldCh;

  _child = newChild;
  if (_child) _child->setMaster(this);
  if (oldCh)
    oldCh->setMaster(0);
  else
    _firstChld = newChild;

  // must repaint

  return oldCh;
}


GLabel::GLabel(int x, int y, std::string text, std::string pressSyn,
               bool showBord)
    : _visible(true), _showBorder(showBord) {
  int size = text.length();
  // int height = 12;
  this->x = x;
  this->y = y;
  h = 20;
  w = size * 6 + 20;
  _ownText = text;
  _pressSynonim = pressSyn;

  if (CONF_PARAM("images") == "1") {
    QImage *imgPtr = ImagePreloader::getInstance().getImage(text);
    if (imgPtr) {
      _imageLabel = std::make_unique<GImage>(x, y, text);
      setW(_imageLabel->getW());
      setH(_imageLabel->getH());
    }
  }
}


bool GLabel::hit(int hX, int hY) {
  bool hitten = false;

  if (_imageLabel == 0) {
    y = y - h / 2;
    hitten = GView::hit(hX, hY);
    y = y + h / 2;
  } else {
    hitten = GView::hit(hX, hY);
  }

  if (hitten) {
    qDebug() << "GLabel hitten " << this->getText().c_str();
  }

  return hitten;
}


void GCheckButton::draw(QPainter *painter) {
  if (_checked)
    painter->fillRect(x, y, w, h, Qt::darkGray);  // painter->fillRect(x,y,w,h,CONF_PARAM("colors.curBar"));
  painter->drawRect(x, y, w, h);
}


void GCheckButton::onclick([[maybe_unused]] int x1, [[maybe_unused]] int y1) {
  if (_checked)
    _checked = false;
  else
    _checked = true;
}


void GLabel::draw(QPainter *painter) {
  if (_visible == false) return;

  if (_imageLabel == 0) {
    painter->drawText(x + 5, y, _ownText.c_str());

    if (_showBorder)
      painter->drawRect(x, y - 15, w, h);  // 10 half of text height
  } else {
    _imageLabel->setX(x);
    _imageLabel->setY(y - 10);
    _imageLabel->draw(painter);

    if (_showBorder)
      painter->drawRect(x, y - 10, _imageLabel->getW(), _imageLabel->getH());
  }
}


bool GView::hit(int hX, int hY) {
  if ((hX >= x) && (hY >= y)) {
    int xDiff = hX - x;
    int yDiff = hY - y;
    if ((xDiff <= w) && (yDiff <= h))
        return true;
  }
  return false;
}
