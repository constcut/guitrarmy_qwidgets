#ifndef GPANNEL_H
#define GPANNEL_H
// This deprecated file formated with clang-format

#include "GView.hpp"

namespace gtmy {

    class GPannel : public GView {
     public:
      GLabel pannelLabel;  // edit
      bool openned;        // false
      std::vector<GLabel> buttons;
      GView *pressView;

      bool noOnOffButton;

      std::string openedText;
      std::string closedText;

      void setTexts(std::string openedOne, std::string closedOne);

     public:
      GPannel(int y1, int y2, int width, int x1 = 0)
          : GView(x1, y1, width, y2),
            pannelLabel(w - 40, y2 - 20, "pan"),
            openned(false),
            pressView(0),
            noOnOffButton(false) {}

      void setPressView(GView *pressViewSet) { pressView = pressViewSet; }
      bool isOpenned() const { return openned; }

      void preOpen() { openned = true; }
      void close() { openned = false; }

      void setNoOpenButton() {
        noOnOffButton = true;
        openned = true;
      }

      void addButton(std::string capture, std::string syn = "");
      virtual void setButtons(int from = 0, int lastLine = 0);

      void draw(QPainter *painter);

      virtual void onclick(int x1, int y1);
      virtual void keyevent(std::string press) {
        if (pressView) pressView->keyevent(press);
      }

      virtual void resetButtons() {}
      virtual bool hit(int hX, int hY) {
        if (isOpenned()) return GView::hit(hX, hY);

        return false;
      }
    };

    class GStickPannel : public GPannel {
     public:
      GStickPannel(int y1, int y2, int w1, int x1 = 0) : GPannel(y1, y2, w1, x1) {
        assignButtons();
        setTexts("pannel", "pannel");  // useless
      }

      virtual void assignButtons();
      void setGropedButtons();
      void setGroupedVisible();
      void setGroupedHidden();
      virtual void resetButtons();
    };

    class GEffectsPannel : public GPannel {
     public:
      GEffectsPannel(int y1, int y2, int width, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("eff", "eff");
      }

      virtual void assignButtons();
      virtual void resetButtons() { setButtons(); }
    };

    class GClipboardPannel : public GPannel {
     public:
      GClipboardPannel(int y1, int y2, int width, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("clip", "clip");
      }
      virtual void assignButtons();
      virtual void resetButtons() { setButtons(); }
    };

    class GUserPannel : public GPannel {
     public:
      GUserPannel(int y1, int y2, int width, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("user", "user");
      }

      virtual void assignButtons();
      virtual void resetButtons() { setButtons(); }
    };

    class GTrackPannel : public GPannel {
     public:
      GTrackPannel(int y1, int y2, int width, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("view", "edit");
      }

      virtual void draw(QPainter *painter) { GPannel::draw(painter); }

      virtual void assignButtons();
      void setCrossButtons();
      virtual void resetButtons();
    };

    class GTabPannel : public GPannel {
     public:
      GTabPannel(int y1, int y2, int width, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("pannel", "pannel");
      }

      virtual void assignButtons();
      virtual void resetButtons() {
        setButtons();  // start from 10
      }
    };

    /// ONE LINE PANNELS
    /// --
    class GNumPannel : public GPannel {
     public:
      GNumPannel(int y1 = 1, int y2 = 2, int width = 3, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("pannel", "pannel");
      }
      virtual void resetButtons() { setButtons(); }

      virtual void assignButtons();
    };

    class GMovePannel : public GPannel {
     public:
      GMovePannel(int y1 = 1, int y2 = 2, int width = 3, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("pannel", "pannel");
      }
      virtual void resetButtons() { setButtons(); }

      virtual void assignButtons();
    };

    class GEditPannel : public GPannel {
     public:
      GEditPannel(int y1 = 1, int y2 = 2, int width = 3, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("pannel", "pannel");
      }

      virtual void resetButtons() { setButtons(); }

      virtual void assignButtons();
    };

    class GNotePannel : public GPannel {
     public:
      GNotePannel(int y1 = 1, int y2 = 2, int width = 3, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("pannel", "pannel");
      }
      virtual void resetButtons() { setButtons(); }
      virtual void assignButtons();
    };

    class GBeatPannel : public GPannel {
     public:
      GBeatPannel(int y1 = 1, int y2 = 2, int width = 3, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("pannel", "pannel");
      }
      virtual void resetButtons() { setButtons(); }
      virtual void assignButtons();
    };

    class GTrackNewPannel : public GPannel {
     public:
      GTrackNewPannel(int y1 = 1, int y2 = 2, int width = 3, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("pannel", "pannel");
      }
      virtual void resetButtons() { setButtons(); }

      virtual void assignButtons();
    };

    class GBarPannel : public GPannel {
     public:
      GBarPannel(int y1 = 1, int y2 = 2, int width = 3, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("pannel", "pannel");
      }
      virtual void resetButtons() { setButtons(); }
      virtual void assignButtons();
    };

    class GMenuNewPannel : public GPannel {
     public:
      GMenuNewPannel(int y1 = 1, int y2 = 2, int width = 3, int x1 = 0)
          : GPannel(y1, y2, width, x1) {
        assignButtons();
        setTexts("pannel", "pannel");
      }
      virtual void resetButtons() { setButtons(); }
      virtual void assignButtons() {
        // yet just use user then make it best way here
        setButtons();
      }
    };

}  // namespace gtmy

#endif  // GPANNEL_H
