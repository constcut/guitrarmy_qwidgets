//This deprecated file formated with clang-format
#include "GView.hpp"

#include <QDebug>
#include <QFileDialog>

#include "g0/Config.hpp"

bool gViewLog = false;

using namespace gtmy;


GView *MasterView::changeChild(GView *newChild) {
  GView *oldCh = child;

  lastView = oldCh;

  child = newChild;
  if (child) child->setMaster(this);
  if (oldCh)
    oldCh->setMaster(0);
  else
    firstChld = newChild;

  // must repaint

  return oldCh;
}


GLabel::GLabel(int x, int y, std::string text, std::string pressSyn,
               bool showBord)
    : visible(true), showBorder(showBord) {
  int size = text.length();
  // int height = 12;
  this->x = x;
  this->y = y;
  h = 20;
  w = size * 6 + 20;
  ownText = text;
  pressSynonim = pressSyn;

  if (CONF_PARAM("images") == "1") {
    QImage *imgPtr = ImagePreloader::getInstance().getImage(text);
    if (imgPtr) {
      imageLabel = std::make_unique<GImage>(x, y, text);
      setW(imageLabel->getW());
      setH(imageLabel->getH());
    }
  }
}


bool GLabel::hit(int hX, int hY) {
  bool hitten = false;

  if (imageLabel == 0) {
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
  if (checked)
    painter->fillRect(x, y, w, h, Qt::darkGray);  // painter->fillRect(x,y,w,h,CONF_PARAM("colors.curBar"));
  painter->drawRect(x, y, w, h);
}


void GCheckButton::onclick([[maybe_unused]] int x1, [[maybe_unused]] int y1) {
  if (checked)
    checked = false;
  else
    checked = true;
}


void GLabel::draw(QPainter *painter) {
  if (visible == false) return;

  if (imageLabel == 0) {
    painter->drawText(x + 5, y, ownText.c_str());

    if (showBorder)
      painter->drawRect(x, y - 15, w, h);  // 10 half of text height
  } else {
    imageLabel->setX(x);
    imageLabel->setY(y - 10);
    imageLabel->draw(painter);

    if (showBorder)
      painter->drawRect(x, y - 10, imageLabel->getW(), imageLabel->getH());
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
