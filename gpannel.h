#ifndef GPANNEL_H
#define GPANNEL_H

#include "gview.h"

class GPannel: public GView
{
//protected:
public:

    GLabel pannelLabel; //edit
    bool openned; //false
    std::vector<GLabel> buttons;
    GView *pressView;

    bool noOnOffButton;

    std::string openedText;
    std::string closedText;

    void setTexts(std::string openedOne, std::string closedOne)
    {
        openedText = openedOne;
        closedText = closedOne;
        if (openned)
            pannelLabel.setText(openedText);
        else
            pannelLabel.setText(closedText);
    }

public:
    GPannel(int y1, int y2, int width, int x1=0)
        : GView(x1,y1,width,y2), pannelLabel(w-40, y2-20,"pan")
    ,openned(false),pressView(0),noOnOffButton(false)
    {
        //assignButtons();
    }



    void setPressView(GView *pressViewSet) {pressView = pressViewSet;}
    bool isOpenned() { return openned; }

    void preOpen() { openned = true; }
    void close() { openned = false; }

    void setNoOpenButton()
    {
        noOnOffButton = true;
        openned = true;
    }

    //add Button
    void aB(std::string capture, std::string syn="")
    {
        int thatX = x;
        int thatY = y;
        if (syn.empty())
            syn=capture;
        GLabel labelButton(thatX,thatY,capture,syn);
        buttons.push_back(labelButton);

    }

    virtual void setButtons(int from=0, int lastLine=0)
    {
        int xCur = x+5;
        int yCur = y+15;

        if (lastLine==0)
            lastLine=40;

        for (size_t i = from; i < buttons.size(); ++i)
        {
            if (buttons[i].isVisible()==false) continue;

             buttons[i].setX(xCur);
             buttons[i].setY(yCur);
             xCur += buttons[i].getW();
             xCur += 20;

             if (xCur >= (w-lastLine)) //panel - button width (was 40)
             {
                 xCur = x+5;
                 yCur += 40;
             }

        }

        int realH = yCur-y;
        realH += 40; //+ status bar

        if (pressView)
            realH += pressView->getMaster()->getStatusBarHeight();

        if (realH != h)
        {
            if (pressView)
            {
                h = realH;
                y = pressView->getMaster()->getHeight() - h;

                int xCur = x+5;
                int yCur = y+15;

                if (lastLine==0)
                    lastLine=40;

                for (size_t i = from; i < buttons.size(); ++i)
                {
                    //simple reset them
                    if (buttons[i].isVisible()==false) continue;

                     buttons[i].setX(xCur);
                     buttons[i].setY(yCur);
                     xCur += buttons[i].getW();
                     xCur += 20;

                     if (xCur >= (w-lastLine)) //panel - button width (was 40)
                     {
                         xCur = x+5;
                         yCur += 40;
                     }
                }
            }
        }

        pannelLabel.setX(w-40);
        pannelLabel.setY(h-20);
    }


    void draw(Painter *painter)
    {

        if (openned)
        {
            painter->fillRect(x,y,w,h,CONF_PARAM("colors.panBG"));
            painter->drawRect(x,y,w,h);

             //buttons
            for (size_t i = 0; i < buttons.size(); ++i)
            {
                if (buttons[i].isVisible())
                 buttons[i].draw(painter);
            }
        }

        //if (noOnOffButton == false)
            //pannelLabel.draw(painter);
    }

    virtual void onclick(int x1, int y1);


    virtual void keyevent(std::string press)
    {
        if ( pressView )
                pressView->keyevent(press);
    }

    virtual void resetButtons(){}

    virtual bool hit(int hX, int hY)
    {
        if (isOpenned())
            return GView::hit(hX,hY);

        return false;
    }
};


class GStickPannel : public GPannel
{
public:
    GStickPannel(int y1, int y2, int w1, int x1=0):GPannel(y1,y2,w1,x1)
    {
         assignButtons();
         setTexts("pannel","pannel"); //useless
    }

    virtual void assignButtons()
    {
        aB("backview","esc");
        aB("new","newtab");
        aB("open",CONF_PARAM("Main.open"));
        aB("tab","tabview");
        aB("tests","tests");
        aB("tap","tap");
        aB("pattern","pattern");
        aB("config","config");
        aB("record","rec");

        aB("morze");
        aB("info");

        aB("openPannel");

        setButtons();
    }

    void setGropedButtons()
    {
        setGroupedHidden();
        setButtons();
    }

    void setGroupedVisible()
    {
        buttons[5].setVisible(true);
        buttons[6].setVisible(true);
        buttons[8].setVisible(true);
        buttons[9].setVisible(true);
    }

    void setGroupedHidden()
    {
        buttons[5].setVisible(false);
        buttons[6].setVisible(false);
        buttons[8].setVisible(false);
        buttons[9].setVisible(false);
    }

    virtual void resetButtons()
    {
       int hi =  pressView->getMaster()->getHeight();
       int wi = pressView->getMaster()->getWidth();

       if (wi > hi)
       {
            setGroupedVisible();
            setButtons();
       }
       else
       {
           setGropedButtons();
       }
    }


};



class GEffectsPannel : public GPannel
{
public:

    GEffectsPannel(int y1, int y2, int width, int x1=0):GPannel(y1,y2,width,x1)
    {
        assignButtons();
        setTexts("eff","eff");
    }


    virtual void assignButtons()
    {
           aB("vib",CONF_PARAM("effects.vibrato"));
           aB("sli",CONF_PARAM("effects.slide"));
           aB("ham",CONF_PARAM("effects.hammer"));
           aB("lr",CONF_PARAM("effects.letring"));
           aB("pm",CONF_PARAM("effects.palmmute"));
           aB("harm",CONF_PARAM("effects.harmonics"));
           aB("trem",CONF_PARAM("effects.tremolo"));
           aB("trill",CONF_PARAM("effects.trill"));
           aB("stok",CONF_PARAM("effects.stokatto"));
           aB("tapp");
           aB("slap");
           aB("pop");
           aB("fadeIn",CONF_PARAM("effects.fadein"));
           //aB("f Out");
           aB("upm","up m");
           aB("downm","down m");

           aB("acc",CONF_PARAM("effects.accent"));
           aB("hacc","h acc");

            //line

           aB("bend");
           aB("chord");
           aB("txt","text");
           aB("changes");
           aB("fing");
           aB("upstroke");
           aB("downstroke"); // all the strings play

         //  aB("back","eff");


          setButtons();

    }

    virtual void resetButtons()
    {
        int hi =  pressView->getMaster()->getHeight();
        int wi = pressView->getMaster()->getWidth();

        setButtons();
    }


};

class GClipboardPannel : public GPannel
{
public:

    GClipboardPannel(int y1, int y2, int width, int x1=0):
        GPannel(y1,y2,width,x1)
    {
        assignButtons();
        setTexts("clip","clip");
    }


    virtual void assignButtons()
    {
        aB("copy");
        //aB("copyBeat"); //shit
        //aB("copyBars");
        aB("cut");
        aB("paste");

        aB("select >");
        aB("select <");

        aB("clip1");
        aB("clip2");
        aB("clip3");

       // aB("back","clip");

        setButtons();
    }

    virtual void resetButtons()
    {
        int hi =  pressView->getMaster()->getHeight();
        int wi = pressView->getMaster()->getWidth();

        setButtons();
    }

};



class GUserPannel : public GPannel
{
public:

    GUserPannel(int y1, int y2, int width, int x1=0):
        GPannel(y1,y2,width,x1)
    {
        assignButtons();
        setTexts("user","user");
    }


    virtual void assignButtons()
    {


        //aB("undo","undo");

        aB("backview","esc");
        aB("new","newtab");
        aB("open",CONF_PARAM("Main.open"));
        aB("tab","tabview");
        aB("tests","tests");
        aB("tap","tap");
        aB("pattern","pattern");
        aB("config","config");
        aB("record","rec");

        //aB()

        aB("morze");
        aB("info");

        aB("openPannel");


        aB("save","save"); //just to be

        aB("play",CONF_PARAM("TrackView.playMidi"));

        //invert??
        //aB("play",CONF_PARAM("TrackView.playAMusic"));

        aB("zoomIn","zoomIn");
        aB("zoomOut","zoomOut");



        setButtons();
    }

    virtual void resetButtons()
    {
        int hi =  pressView->getMaster()->getHeight();
        int wi = pressView->getMaster()->getWidth();

        setButtons();
    }

};



class GTrackPannel : public GPannel
{


 public:

    GTrackPannel(int y1, int y2, int width, int x1=0):GPannel(y1,y2,width,x1)
    {

        assignButtons();
        setTexts("view","edit");
    }

    virtual void draw(Painter *painter)
    {
        GPannel::draw(painter);

        /*
        if (effPan->isOpenned())
            effPan->draw(panter);

        if (clipPan-)
        */
    }

    virtual void assignButtons()
    {




        ///8

           aB("0");
           aB("1");
           aB("2");
           aB("3");
           aB("4");
           aB("5");
           aB("6");

           aB("prevBeat",CONF_PARAM("TrackView.prevBeat"));
           aB("upString",CONF_PARAM("TrackView.stringUp"));
           aB("nextBeat",   CONF_PARAM("TrackView.nextBeat"));




           aB("play",CONF_PARAM("TrackView.playAMusic"));
           aB("play",CONF_PARAM("TrackView.playMidi"));
           //15

           aB("qp","q"); //quantity plus
           aB("qm","w"); //minus

           aB("p");
           aB("del",CONF_PARAM("TrackView.deleteNote"));

           aB("ins");

           aB(".","dot"); //link

           aB("-3-"); //trumplets

           aB("7");
           aB("8");
           aB("9");

           aB("undo");
           aB("newBar");

           aB("prevPage");
           aB("nextPage");

           aB("prevBar",CONF_PARAM("TrackView.prevBar"));
           aB("downString",CONF_PARAM("TrackView.stringDown"));
           aB("nextBar",CONF_PARAM("TrackView.nextBar"));




           //aB("add","i");

           //always in main menu now
           //aB("2main",CONF_PARAM("TrackView.toMainMenu"));

           aB("leeg");
           aB("x","dead");



         //  aB("eff");
         //  aB("clip");

           aB("save","quicksave");
           aB("open","quickopen");

           aB("prevTrack");
           aB("nextTrack");


          resetButtons();
    }

    void setCrossButtons()
    {
        int yCur = y+15;

        //pages
        buttons[6].setX(w-150);
        buttons[6].setY(yCur);
        buttons[7].setX(w-50); //-50
        buttons[7].setY(yCur);

        yCur += 40;
        //STRING MOVEW - CENTER
        buttons[2].setX(w-100);
        buttons[2].setY(yCur);
        buttons[3].setX(w-100);
        buttons[3].setY(yCur+40);

        //arrow
        buttons[0].setX(w-150);
        buttons[0].setY(yCur);
        buttons[1].setX(w-50);
        buttons[1].setY(yCur);

        yCur += 40;
        buttons[4].setX(w-150);
        buttons[4].setY(yCur);
        buttons[5].setX(w-50);
        buttons[5].setY(yCur);
    }

    virtual void resetButtons()
    {
        int hi =0; // pressView->getMaster()->getHeight();
        int wi =0; // pressView->getMaster()->getWidth();

        if (pressView)
        {
            hi = pressView->getMaster()->getHeight();
            wi = pressView->getMaster()->getWidth();
        }

        if (hi > wi)
        {
            //horizontal orientation
            y = hi-200;

        }
        else
        {
            y = 300;

            if ((hi-180) > y)
                y = hi-180;
        }



        setButtons(8,190); //200

        setCrossButtons();
    }
};


class GTabPannel : public GPannel
{
public:

    GTabPannel(int y1, int y2, int width, int x1=0):GPannel(y1,y2,width,x1)
    {
        assignButtons();
        setTexts("pannel","pannel");
    }


    virtual void assignButtons()
    {
           aB("goToN");

           aB("opentrack");

           aB("newTrack");
           aB("deleteTrack");
           aB("drums");

           aB("marker");

           aB("44","signs");
           aB("prev","<<<");
           aB("^","^^^");
           aB("next",">>>");


           aB("repBegin","|:");
           aB("repEnd",":|");
           aB("alt");
           aB("bpm");
           aB("instr");
           aB("volume");
           aB("pan");
           aB("name");
           aB("tune");


           aB("mute");

           aB("solo");

           aB("V","vvv");
           aB("save","quicksave");
           aB("open","quickopen");

          setButtons();
    }

    virtual void resetButtons()
    {
        setButtons(); //start from 10

        //setCrossButtons();
    }
};


///ONE LINE PANNELS
/// --
class GNumPannel : public GPannel
{
public:
    GNumPannel(int y1=1, int y2=2, int width=3, int x1=0):GPannel(y1,y2,width,x1)
    { assignButtons();setTexts("pannel","pannel");}
    virtual void resetButtons()
    { setButtons();}

    virtual void assignButtons()
    {
        aB("0");aB("1");aB("2");aB("3");aB("4");
        aB("5");aB("6");aB("7");aB("8");aB("9");
        aB("prevBeat",CONF_PARAM("TrackView.prevBeat"));
        aB("upString",CONF_PARAM("TrackView.stringUp"));
        aB("nextBeat",   CONF_PARAM("TrackView.nextBeat"));

        aB("prevBar",CONF_PARAM("TrackView.prevBar"));
        aB("downString",CONF_PARAM("TrackView.stringDown"));
        aB("nextBar",CONF_PARAM("TrackView.nextBar"));

        aB("prevPage");
        aB("nextPage");

        aB("prevTrack");
        aB("nextTrack");

        setButtons();
    }
};

class GMovePannel : public GPannel
{
public:
    GMovePannel(int y1=1, int y2=2, int width=3, int x1=0):GPannel(y1,y2,width,x1)
    { assignButtons();setTexts("pannel","pannel");}
    virtual void resetButtons()
    { setButtons();}

    virtual void assignButtons()
    {
       //cross buttons + oth
        aB("prevBeat",CONF_PARAM("TrackView.prevBeat"));
        aB("upString",CONF_PARAM("TrackView.stringUp"));
        aB("nextBeat",   CONF_PARAM("TrackView.nextBeat"));

        aB("prevBar",CONF_PARAM("TrackView.prevBar"));
        aB("downString",CONF_PARAM("TrackView.stringDown"));
        aB("nextBar",CONF_PARAM("TrackView.nextBar"));

        aB("prevPage");
        aB("nextPage");

        aB("prevTrack");
        aB("nextTrack");


        setButtons();
    }
};

class GEditPannel : public GPannel
{
public:
    GEditPannel(int y1=1, int y2=2, int width=3, int x1=0):GPannel(y1,y2,width,x1)
    { assignButtons();setTexts("pannel","pannel");}
    virtual void resetButtons()
    { setButtons();}

    virtual void assignButtons()
    {
        aB("leeg");
        aB("x","dead");

        aB("undo");
        aB("newBar");
        aB("qp","q"); //quantity plus
        aB("qm","w"); //minus

        aB("p");
        aB("del",CONF_PARAM("TrackView.deleteNote"));

        aB("ins");

        aB(".","dot"); //link

        aB("-3-"); //trumplets

        aB("copy");
        aB("copyBeat");
        aB("copyBars");
        aB("cut");
        aB("paste");

        aB("select >");
        aB("select <");

        aB("clip1");
        aB("clip2");
        aB("clip3");


        setButtons();
    }
};

class GNotePannel : public GPannel
{
public:
    GNotePannel(int y1=1, int y2=2, int width=3, int x1=0):GPannel(y1,y2,width,x1)
    { assignButtons();setTexts("pannel","pannel");}
    virtual void resetButtons()
    { setButtons();}

    virtual void assignButtons()
    {
        aB("vib",CONF_PARAM("effects.vibrato"));
        aB("sli",CONF_PARAM("effects.slide"));
        aB("ham",CONF_PARAM("effects.hammer"));
        aB("lr",CONF_PARAM("effects.letring"));
        aB("pm",CONF_PARAM("effects.palmmute"));
        aB("harm",CONF_PARAM("effects.harmonics"));
        aB("trem",CONF_PARAM("effects.tremolo"));
        aB("trill",CONF_PARAM("effects.trill"));


        aB("acc",CONF_PARAM("effects.accent"));
        aB("hacc","h acc");

        aB("bend");

        aB("chord");
        aB("txt","text");
        aB("changes");

        aB("upstroke");
        aB("downstroke");
        ;
        aB("fadeIn",CONF_PARAM("effects.fadein"));
        //aB("f Out");

        aB("stok",CONF_PARAM("effects.stokatto"));
        aB("tapp");
        aB("slap");
        aB("pop");
        aB("fing");
        aB("upm","up m");
        aB("downm","down m");

        setButtons();
    }
};

class GBeatPannel : public GPannel
{
public:
    GBeatPannel(int y1=1, int y2=2, int width=3, int x1=0):GPannel(y1,y2,width,x1)
    { assignButtons();setTexts("pannel","pannel");}
    virtual void resetButtons()
    { setButtons();}

    virtual void assignButtons()
    {

        aB("chord");
        aB("txt","text");
        aB("changes");

        aB("upstroke");
        aB("downstroke");
        ;
        aB("fadeIn",CONF_PARAM("effects.fadein"));
        //aB("f Out");

        aB("stok",CONF_PARAM("effects.stokatto"));
        aB("tapp");
        aB("slap");
        aB("pop");
        aB("fing");
        aB("upm","up m");
        aB("downm","down m");
        setButtons();
    }
};

class GTrackNewPannel : public GPannel
{
public:
    GTrackNewPannel(int y1=1, int y2=2, int width=3, int x1=0):GPannel(y1,y2,width,x1)
    { assignButtons();setTexts("pannel","pannel");}
    virtual void resetButtons()
    { setButtons();}

    virtual void assignButtons()
    {

        //aB("track","opentrack");


        aB("bpm");
        aB("instr");
        aB("volume");
        aB("pan");
        aB("name");
        aB("tune");

        aB("mute");
        aB("solo");

        aB("repBegin","|:");
        aB("repEnd",":|");
        aB("alt");
        aB("marker");

        aB("44","signs");
        aB("goToN");

        //aB("play",CONF_PARAM("TrackView.playAMusic"));
        aB("play",CONF_PARAM("TrackView.playMidi"));


        aB("newTrack");
        aB("drums");
        aB("deleteTrack");

        aB("play",CONF_PARAM("TrackView.playMidi"));


        aB("save","quicksave");
        aB("open","quickopen");

        setButtons();
    }
};

class GBarPannel : public GPannel
{
public:
    GBarPannel(int y1=1, int y2=2, int width=3, int x1=0):GPannel(y1,y2,width,x1)
    { assignButtons();setTexts("pannel","pannel");}
    virtual void resetButtons()
    { setButtons();}

    virtual void assignButtons()
    {
        aB("repBegin","|:");
        aB("repEnd",":|");
        aB("alt");
        aB("marker");

        aB("44","signs");
        aB("goToN");

        aB("play",CONF_PARAM("TrackView.playAMusic"));
        aB("play",CONF_PARAM("TrackView.playMidi"));

        aB("save","quicksave");
        aB("open","quickopen");

        setButtons();
    }
};

class GMenuNewPannel : public GPannel
{
public:
    GMenuNewPannel(int y1=1, int y2=2, int width=3, int x1=0):GPannel(y1,y2,width,x1)
    { assignButtons();setTexts("pannel","pannel");}
    virtual void resetButtons()
    { setButtons();}

    virtual void assignButtons()
    {
        //yet just use user then make it best way here
        setButtons();
    }
};


#endif // GPANNEL_H
