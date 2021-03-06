// This deprecated file formated with clang-format

#include "ui/GPannel.hpp"

#include <QDebug>

using namespace gtmy;

void GPannel::onclick(int x1, int y1) {
  if (_openned) {
    for (size_t i = 0; i < _buttons.size(); ++i) {
      if (_buttons[i].hit(x1, y1)) {
        qDebug() << "Hit pannel button " << _buttons[i].getText().c_str();
        keyevent(_buttons[i].getPressSyn());
        break;
      }
    }
  }
}

void GPannel::setTexts(std::string openedOne, std::string closedOne) {
  _openedText = openedOne;
  _closedText = closedOne;
  if (_openned)
    _pannelLabel.setText(_openedText);
  else
    _pannelLabel.setText(_closedText);
}

void GPannel::addButton(std::string capture, std::string syn) {
  int thatX = x;
  int thatY = y;
  if (syn.empty()) syn = capture;
  GLabel labelButton(thatX, thatY, capture, syn);
  _buttons.push_back(std::move(labelButton));
}

void GPannel::setButtons(int from, int lastLine) {
  int xCur = x + 5;
  int yCur = y + 15;

  if (lastLine == 0) lastLine = 40;

  for (size_t i = from; i < _buttons.size(); ++i) {
    if (_buttons[i].isVisible() == false) continue;

    _buttons[i].setX(xCur);
    _buttons[i].setY(yCur);
    xCur += _buttons[i].getW();
    xCur += 20;

    if (xCur >= (w - lastLine))  // panel - button width (was 40)
    {
      xCur = x + 5;
      yCur += 40;
    }
  }

  int realH = yCur - y;
  realH += 40;  //+ status bar

  if (_pressView) realH += _pressView->getMaster()->getStatusBarHeight();

  if (realH != h) {
    if (_pressView) {
      h = realH;
      y = _pressView->getMaster()->getHeight() - h;

      int xCur = x + 5;
      int yCur = y + 15;

      if (lastLine == 0) lastLine = 40;

      for (size_t i = from; i < _buttons.size(); ++i) {
        // simple reset them
        if (_buttons[i].isVisible() == false) continue;

        _buttons[i].setX(xCur);
        _buttons[i].setY(yCur);
        xCur += _buttons[i].getW();
        xCur += 20;

        if (xCur >= (w - lastLine))  // panel - button width (was 40)
        {
          xCur = x + 5;
          yCur += 40;
        }
      }
    }
  }

  _pannelLabel.setX(w - 40);
  _pannelLabel.setY(h - 20);
}

void GPannel::draw(QPainter *painter) {
  if (_openned) {
    painter->fillRect(x, y, w, h, QColor(CONF_PARAM("colors.panBG").c_str()));
    painter->drawRect(x, y, w, h);

    // buttons
    for (size_t i = 0; i < _buttons.size(); ++i) {
      if (_buttons[i].isVisible()) _buttons[i].draw(painter);
    }
  }
}

void GStickPannel::assignButtons() {
  addButton("backview", "esc");
  addButton("new", "newtab");
  addButton("open", CONF_PARAM("Main.open"));
  addButton("tab", "tabview");
  addButton("tests", "tests");
  addButton("tap", "tap");
  addButton("pattern", "pattern");
  addButton("config", "config");
  addButton("record", "rec");

  addButton("morze");
  addButton("info");

  addButton("openPannel");

  setButtons();
}

void GStickPannel::setGropedButtons() {
  setGroupedHidden();
  setButtons();
}

void GStickPannel::setGroupedVisible() {
  _buttons[5].setVisible(true);
  _buttons[6].setVisible(true);
  _buttons[8].setVisible(true);
  _buttons[9].setVisible(true);
}

void GStickPannel::setGroupedHidden() {
  _buttons[5].setVisible(false);
  _buttons[6].setVisible(false);
  _buttons[8].setVisible(false);
  _buttons[9].setVisible(false);
}

void GStickPannel::resetButtons() {
  int hi = _pressView->getMaster()->getHeight();
  int wi = _pressView->getMaster()->getWidth();
  if (wi > hi) {
    setGroupedVisible();
    setButtons();
  } else
    setGropedButtons();
}

void GEffectsPannel::assignButtons() {
  addButton("vib", CONF_PARAM("effects.vibrato"));
  addButton("sli", CONF_PARAM("effects.slide"));
  addButton("ham", CONF_PARAM("effects.hammer"));
  addButton("lr", CONF_PARAM("effects.letring"));
  addButton("pm", CONF_PARAM("effects.palmmute"));
  addButton("harm", CONF_PARAM("effects.harmonics"));
  addButton("trem", CONF_PARAM("effects.tremolo"));
  addButton("trill", CONF_PARAM("effects.trill"));
  addButton("stok", CONF_PARAM("effects.stokatto"));
  addButton("tapp");
  addButton("slap");
  addButton("pop");
  addButton("fadeIn", CONF_PARAM("effects.fadein"));
  // aB("f Out");
  addButton("upm", "up m");
  addButton("downm", "down m");

  addButton("acc", CONF_PARAM("effects.accent"));
  addButton("hacc", "h acc");

  // line

  addButton("bend");
  addButton("chord");
  addButton("txt", "text");
  addButton("changes");
  addButton("fing");
  addButton("upstroke");
  addButton("downstroke");  // all the strings play

  //  aB("back","eff");

  setButtons();
}

void GClipboardPannel::assignButtons() {
  addButton("copy");
  // aB("copyBeat"); //shit
  // aB("copyBars");
  addButton("cut");
  addButton("paste");
  addButton("select >");
  addButton("select <");
  addButton("clip1");
  addButton("clip2");
  addButton("clip3");

  setButtons();
}

void GUserPannel::assignButtons() {
  addButton("backview", "esc");
  addButton("new", "newtab");
  addButton("open", CONF_PARAM("Main.open"));
  addButton("tab", "tabview");
  addButton("tests", "tests");
  addButton("tap", "tap");
  addButton("pattern", "pattern");
  addButton("config", "config");
  addButton("record", "rec");

  addButton("morze");
  addButton("info");
  addButton("openPannel");
  addButton("save", "save");  // just to be
  addButton("play", CONF_PARAM("TrackView.playMidi"));

  addButton("zoomIn", "zoomIn");
  addButton("zoomOut", "zoomOut");
  setButtons();
}

void GTrackPannel::assignButtons() {
  addButton("0");
  addButton("1");
  addButton("2");
  addButton("3");
  addButton("4");
  addButton("5");
  addButton("6");

  addButton("prevBeat", CONF_PARAM("TrackView.prevBeat"));
  addButton("upString", CONF_PARAM("TrackView.stringUp"));
  addButton("nextBeat", CONF_PARAM("TrackView.nextBeat"));

  addButton("play", CONF_PARAM("TrackView.playAMusic"));
  addButton("play", CONF_PARAM("TrackView.playMidi"));
  // 15

  addButton("qp", "q");  // quantity plus
  addButton("qm", "w");  // minus

  addButton("p");
  addButton("del", CONF_PARAM("TrackView.deleteNote"));

  addButton("ins");

  addButton(".", "dot");  // link

  addButton("-3-");  // trumplets

  addButton("7");
  addButton("8");
  addButton("9");

  addButton("undo");
  addButton("newBar");

  addButton("prevPage");
  addButton("nextPage");

  addButton("prevBar", CONF_PARAM("TrackView.prevBar"));
  addButton("downString", CONF_PARAM("TrackView.stringDown"));
  addButton("nextBar", CONF_PARAM("TrackView.nextBar"));

  // aB("add","i");

  // always in main menu now
  // aB("2main",CONF_PARAM("TrackView.toMainMenu"));

  addButton("leeg");
  addButton("x", "dead");

  //  aB("eff");
  //  aB("clip");

  addButton("save", "quicksave");
  addButton("open", "quickopen");

  addButton("prevTrack");
  addButton("nextTrack");

  resetButtons();
}

void GTrackPannel::setCrossButtons() {
  int yCur = y + 15;

  // pages
  _buttons[6].setX(w - 150);
  _buttons[6].setY(yCur);
  _buttons[7].setX(w - 50);  //-50
  _buttons[7].setY(yCur);

  yCur += 40;
  // STRING MOVEW - CENTER
  _buttons[2].setX(w - 100);
  _buttons[2].setY(yCur);
  _buttons[3].setX(w - 100);
  _buttons[3].setY(yCur + 40);

  // arrow
  _buttons[0].setX(w - 150);
  _buttons[0].setY(yCur);
  _buttons[1].setX(w - 50);
  _buttons[1].setY(yCur);

  yCur += 40;
  _buttons[4].setX(w - 150);
  _buttons[4].setY(yCur);
  _buttons[5].setX(w - 50);
  _buttons[5].setY(yCur);
}

void GTrackPannel::resetButtons() {
  int hi = 0;  // pressView->getMaster()->getHeight();
  int wi = 0;  // pressView->getMaster()->getWidth();

  if (_pressView) {
    hi = _pressView->getMaster()->getHeight();
    wi = _pressView->getMaster()->getWidth();
  }

  if (hi > wi) {
    // horizontal orientation
    y = hi - 200;

  } else {
    y = 300;

    if ((hi - 180) > y) y = hi - 180;
  }

  setButtons(8, 190);  // 200

  setCrossButtons();
}

void GTabPannel::assignButtons() {
  addButton("goToN");

  addButton("opentrack");

  addButton("newTrack");
  addButton("deleteTrack");
  addButton("drums");

  addButton("marker");

  addButton("44", "signs");
  addButton("prev", "<<<");
  addButton("^", "^^^");
  addButton("next", ">>>");

  addButton("repBegin", "|:");
  addButton("repEnd", ":|");
  addButton("alt");
  addButton("bpm");
  addButton("instr");
  addButton("volume");
  addButton("pan");
  addButton("name");
  addButton("tune");

  addButton("mute");

  addButton("solo");

  addButton("V", "vvv");
  addButton("save", "quicksave");
  addButton("open", "quickopen");

  setButtons();
}

void GNumPannel::assignButtons() {
  addButton("0");
  addButton("1");
  addButton("2");
  addButton("3");
  addButton("4");
  addButton("5");
  addButton("6");
  addButton("7");
  addButton("8");
  addButton("9");
  addButton("prevBeat", CONF_PARAM("TrackView.prevBeat"));
  addButton("upString", CONF_PARAM("TrackView.stringUp"));
  addButton("nextBeat", CONF_PARAM("TrackView.nextBeat"));

  addButton("prevBar", CONF_PARAM("TrackView.prevBar"));
  addButton("downString", CONF_PARAM("TrackView.stringDown"));
  addButton("nextBar", CONF_PARAM("TrackView.nextBar"));

  addButton("prevPage");
  addButton("nextPage");

  addButton("prevTrack");
  addButton("nextTrack");

  setButtons();
}

void GMovePannel::assignButtons() {
  // cross buttons + oth
  addButton("prevBeat", CONF_PARAM("TrackView.prevBeat"));
  addButton("upString", CONF_PARAM("TrackView.stringUp"));
  addButton("nextBeat", CONF_PARAM("TrackView.nextBeat"));

  addButton("prevBar", CONF_PARAM("TrackView.prevBar"));
  addButton("downString", CONF_PARAM("TrackView.stringDown"));
  addButton("nextBar", CONF_PARAM("TrackView.nextBar"));

  addButton("prevPage");
  addButton("nextPage");

  addButton("prevTrack");
  addButton("nextTrack");

  setButtons();
}

void GEditPannel::assignButtons() {
  addButton("leeg");
  addButton("x", "dead");

  addButton("undo");
  addButton("newBar");
  addButton("qp", "q");  // quantity plus
  addButton("qm", "w");  // minus

  addButton("p");
  addButton("del", CONF_PARAM("TrackView.deleteNote"));

  addButton("ins");

  addButton(".", "dot");  // link

  addButton("-3-");  // trumplets

  addButton("copy");
  addButton("copyBeat");
  addButton("copyBars");
  addButton("cut");
  addButton("paste");

  addButton("select >");
  addButton("select <");

  addButton("clip1");
  addButton("clip2");
  addButton("clip3");

  setButtons();
}

void GNotePannel::assignButtons() {
  addButton("vib", CONF_PARAM("effects.vibrato"));
  addButton("sli", CONF_PARAM("effects.slide"));
  addButton("ham", CONF_PARAM("effects.hammer"));
  addButton("lr", CONF_PARAM("effects.letring"));
  addButton("pm", CONF_PARAM("effects.palmmute"));
  addButton("harm", CONF_PARAM("effects.harmonics"));
  addButton("trem", CONF_PARAM("effects.tremolo"));
  addButton("trill", CONF_PARAM("effects.trill"));

  addButton("acc", CONF_PARAM("effects.accent"));
  addButton("hacc", "h acc");

  addButton("bend");

  addButton("chord");
  addButton("txt", "text");
  addButton("changes");

  addButton("upstroke");
  addButton("downstroke");
  ;
  addButton("fadeIn", CONF_PARAM("effects.fadein"));
  // aB("f Out");

  addButton("stok", CONF_PARAM("effects.stokatto"));
  addButton("tapp");
  addButton("slap");
  addButton("pop");
  addButton("fing");
  addButton("upm", "up m");
  addButton("downm", "down m");

  setButtons();
}

void GBeatPannel::assignButtons() {
  addButton("chord");
  addButton("txt", "text");
  addButton("changes");

  addButton("upstroke");
  addButton("downstroke");
  addButton("fadeIn", CONF_PARAM("effects.fadein"));
  // aB("f Out");

  addButton("stok", CONF_PARAM("effects.stokatto"));
  addButton("tapp");
  addButton("slap");
  addButton("pop");
  addButton("fing");
  addButton("upm", "up m");
  addButton("downm", "down m");
  setButtons();
}

void GTrackNewPannel::assignButtons() {
  addButton("bpm");
  addButton("instr");
  addButton("volume");
  addButton("pan");
  addButton("name");
  addButton("tune");

  addButton("mute");
  addButton("solo");

  addButton("repBegin", "|:");
  addButton("repEnd", ":|");
  addButton("alt");
  addButton("marker");

  addButton("44", "signs");
  addButton("goToN");

  // aB("play",CONF_PARAM("TrackView.playAMusic"));
  addButton("play", CONF_PARAM("TrackView.playMidi"));

  addButton("newTrack");
  addButton("drums");
  addButton("deleteTrack");

  addButton("play", CONF_PARAM("TrackView.playMidi"));

  addButton("save", "quicksave");
  addButton("open", "quickopen");

  setButtons();
}

void GBarPannel::assignButtons() {
  addButton("repBegin", "|:");
  addButton("repEnd", ":|");
  addButton("alt");
  addButton("marker");

  addButton("44", "signs");
  addButton("goToN");

  addButton("play", CONF_PARAM("TrackView.playAMusic"));
  addButton("play", CONF_PARAM("TrackView.playMidi"));

  addButton("save", "quicksave");
  addButton("open", "quickopen");

  setButtons();
}
