#ifndef COMMONVIEWS_H
#define COMMONVIEWS_H

#include "gview.h"
#include "g0/tests.h"
#include "inputviews.h"

#include <memory>


class TabView;
class TestsView;
class ConfigView;
class TapRyView;
class PatternInput;
class GStickPannel;

class InfoView;
class MorzeInput;
class RecordView;

class WelcomeView;

class BendInput;
class ChordInput;
class ChangesInput;


class MainView : public GView
{
protected:

    std::unique_ptr<TabView> tabsView;
    std::unique_ptr<TestsView> testsView;
    std::unique_ptr<ConfigView> configView;
    std::unique_ptr<TapRyView> tapRyView;
    std::unique_ptr<PatternInput> patternInp;

    std::unique_ptr<InfoView> infView;
    std::unique_ptr<MorzeInput> morzeInp;
    std::unique_ptr<RecordView> recordView;

    std::unique_ptr<BendInput> bendInp;
    std::unique_ptr<ChangesInput> chanInp;
    std::unique_ptr<ChordInput> chordInp;

    std::unique_ptr<WelcomeView> welcome;

    std::unique_ptr<GStickPannel> pan;

    GView *currentView;
    std::vector<GView*> lastViews;



    void changeViewToLast()
    {
        if (lastViews.size()==0) return;

        GView *lastOne = lastViews[lastViews.size()-1];
        lastViews.pop_back();
        currentView = lastOne;
        if (dependent==false)
            currentView->setUI();
    }

    bool dependent;

 public:

    void setDependent() {dependent=true;}

    void changeCurrentView(GView *newView)
    {
        lastViews.push_back(currentView);
        currentView = newView;

        currentView->setMaster(getMaster());
        if (dependent==false)
            newView->setUI();
    }

    GView *getCurrenView() { return currentView; }

    MainView();
    void draw(QPainter *painter);
    void onclick(int x1, int y1);
    void ondblclick(int x1, int y1);

    void ongesture(int offset, bool horizontal)
    {
        if (currentView)
            currentView->ongesture(offset,horizontal);
    }

    virtual void keyevent(std::string press);
    virtual void onTabCommand(TabCommand command);
    virtual void onTrackCommand(TrackCommand command);


    bool isPlaying();

    //void setCurrentView(GView *view) { currentView = view; }

    virtual void setMaster(MasterView *mast);
};

class WelcomeView : public GView
{
protected:
    std::unique_ptr<GLabel> top;
    std::unique_ptr<GLabel> mid;
    std::unique_ptr<GLabel> bot;

public:
    WelcomeView()
    {
        top = std::make_unique<GLabel>(50,100,"Guitarmy - tablature compose tool","",false);
        mid = std::make_unique<GLabel>(50,150,"This is early beta version - thank you for download","",false);
        bot = std::make_unique<GLabel>(50,200,";)","",false);
    }
    void draw(QPainter *painter)
    {
        top->draw(painter);
        mid->draw(painter);
        bot->draw(painter);
    }
    void onclick([[maybe_unused]]int x1, [[maybe_unused]]int y1)
    {}
};




class ConfigView : public GView
{
protected:

    std::unique_ptr<GLabel> labA, labB;
    std::unique_ptr<GLabel> labC, labD;

    std::unique_ptr<GLabel> labScalePlus, labScaleMinus;
public:
    ConfigView();
    //~ConfigView(){}
    void draw(QPainter *painter);
    void onclick(int x1, int y1);
    virtual void keyevent(std::string press);

};


class InfoView : public GView
{
protected:
    std::unique_ptr<GLabel> labelA;
    std::unique_ptr<GLabel> labelA2;
    std::unique_ptr<GLabel> icons;
    std::unique_ptr<GLabel> labelB;
    std::unique_ptr<GLabel> labelC;
    std::unique_ptr<GLabel> labelD;
    std::unique_ptr<GLabel> labelE;
    std::unique_ptr<GLabel> help;
    std::unique_ptr<GLabel> logShow;
    std::unique_ptr<GLabel> sendCrash;



public:
    InfoView()
    {
        int verticalShift = -60; //-10; //40

        labelA = std::make_unique<GLabel>(10,100+verticalShift,"Here is Guitarmy early beta test. This software is provided 'as-is', without any express or implied ");
        labelA2 = std::make_unique<GLabel>(10,125+verticalShift,
                            "warranty. In no event will the authors be held liable for any damages arising from the use of this software.");

        icons  = std::make_unique<GLabel>(10,150+verticalShift,
                            "Some of icons used for app taken from icons8.com");

        labelB = std::make_unique<GLabel>(10, 175+verticalShift, "This software is developed using Qt library under LGPL licence. Press here for information");
        labelC = std::make_unique<GLabel>(10, 200+verticalShift, "Link to the sources of Qt library used for development(5.4.2): guitarmy.in/lgpl/src.zip");

        labelD = std::make_unique<GLabel>(10, 225+verticalShift, "Link to the licence text: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html");
        labelE = std::make_unique<GLabel>(10, 250+verticalShift, "Address to contact author about LGPL topics: support@guitarmy.in");

        help = std::make_unique<GLabel>(10,300+verticalShift,"Show help");

        logShow = std::make_unique<GLabel>(10,330+verticalShift,"Check crashes");

        sendCrash = std::make_unique<GLabel>(10,360+verticalShift,"Send crashed");

        labelA->setBorder(false);
        labelA2->setBorder(false);
        icons->setBorder(false);

        labelB->setBorder(false);
        labelC->setBorder(false);
        labelD->setBorder(false);
        labelE->setBorder(false);
    }

    void onclick(int x1, int y1);

    void draw(QPainter *painter)
    {
        labelA->draw(painter);
        labelA2->draw(painter);

        icons->draw(painter);

        labelB->draw(painter);
        labelC->draw(painter);

        labelD->draw(painter);
        labelE->draw(painter);

        help->draw(painter);
        logShow->draw(painter);
        sendCrash->draw(painter);
    }
};

class TestsView : public GView
{
protected:
    GLabel upper, bottom, g3, g5;
    std::unique_ptr<GLabel> playlistButton;
    std::unique_ptr<GLabel> playlist2Button;
    std::unique_ptr<GLabel> stopPlaylist;

    std::vector<GLabel> buttons;

    MainView *mainView;
    TabView *tabsView;

public:
    TestsView(MainView *mainV, TabView *tabV):upper(30,0+30,"Start all tests!(dbl click)"),
    bottom(250,0+30,"Information about full tests: not started"), g3(600,0+30,"G3____"), g5(700, 30, "G5_____")
    ,mainView(mainV),tabsView(tabV)
    {
        playlistButton = std::make_unique<GLabel>(400,0+30,"playlist");
        playlist2Button = std::make_unique<GLabel>(400,115-85+30,"playlist2");
        stopPlaylist = std::make_unique<GLabel>(450,0+30,"stop playlist");
        setAllButtons();
    }

    virtual void setUI()
    {

        for (size_t i = 0; i < buttons.size(); ++i)  {
           if (mainView->getMaster())
                mainView->getMaster()->SetButton(i,buttons[i].getText(),
                                   buttons[i].getX(),buttons[i].getY(),
                                   buttons[i].getW(),buttons[i].getH(),"");

           }

        if (mainView->getMaster())
        {
            size_t i = buttons.size();

            mainView->getMaster()->SetButton(i,playlistButton->getText(),
                                   playlistButton->getX(),playlistButton->getY(),
                                   playlistButton->getW(),playlistButton->getH(),playlistButton->getText());
            ++i;

            mainView->getMaster()->SetButton(i,stopPlaylist->getText(),
                                   stopPlaylist->getX(),stopPlaylist->getY(),
                                   stopPlaylist->getW(),stopPlaylist->getH(),stopPlaylist->getText());
            ++i;

            mainView->getMaster()->SetButton(i,playlist2Button->getText(),
                                   playlist2Button->getX(),playlist2Button->getY(),
                                   playlist2Button->getW(),playlist2Button->getH(),playlist2Button->getText());

        }
    }

    void setAllButtons()
    {
        int wi = 800;
        int hi = 480;  
        if (mainView->getMaster()) {
            wi = mainView->getMaster()->getWidth();
            hi = mainView->getMaster()->getHeight();
        }
        buttons.clear();

        int xSh = 30;
        int ySh = 120-85+30;

        for (size_t i = 0; i < 11; ++i) {
            GLabel button(xSh,ySh,"1." + std::to_string(i+1));
            button.setW(40);
            buttons.push_back(std::move(button));

            xSh += 20 + button.getW();
            if (xSh >= (wi-50)) {
                xSh = 30;
                ySh += button.getH()*2;
            }
        }

        for (size_t i = 0; i < 38; ++i)
        {
            GLabel button(xSh,ySh,"2." + std::to_string(i+1));
            button.setW(40);
            buttons.push_back(std::move(button));

            xSh += 20 + button.getW();
           if (xSh >= (wi-50))
            {
                xSh = 30;
                ySh += button.getH()*2;
            }
        }

        for (size_t i = 0; i < 70; ++i) {
            GLabel button(xSh,ySh,"3." + std::to_string(i+1));
            button.setW(40);
            buttons.push_back(std::move(button));
            xSh += 20 + button.getW();
            if (xSh >= (wi-50)) {
                xSh = 30;
                ySh += button.getH()*2;
            }
        }

        //move after first buttons add
         ySh += 50;
         xSh = 30;
         playlistButton->setX(xSh);
         playlistButton->setY(ySh);
         xSh += playlistButton->getW() + 10;
         stopPlaylist->setX(xSh);;
         stopPlaylist->setY(ySh);
         xSh += stopPlaylist->getW() + 10;
         playlist2Button->setX(xSh);
         playlist2Button->setY(ySh);


         if (mainView->getMaster())
         {
             double scaleCoef = AConfig::getInst().getScaleCoef();
             ySh += 50;
             ySh *= scaleCoef; // YET FOR VIEWS MUST DONE REQUEST
             //REFACT
             mainView->getMaster()->requestHeight(ySh);
         }

    }

    void draw(QPainter *painter)
    {
        setAllButtons();

        upper.draw(painter);
        bottom.draw(painter);

        g3.draw(painter);
        g5.draw(painter);

        playlistButton->draw(painter);
        playlist2Button->draw(painter);
        stopPlaylist->draw(painter);

        for (size_t i = 0; i < buttons.size(); ++i)
            buttons[i].draw(painter);
    }
    void onclick(int x1, int y1);

    void ondblclick(int x1, int y1)
    {
        if (bottom.hit(x1,y1))
        {
            //fastTestAll();
            keyevent("1");
            getMaster()->pleaseRepaint();
            keyevent("2");
            getMaster()->pleaseRepaint();
            keyevent("3");
            getMaster()->pleaseRepaint();
            keyevent("4");
            getMaster()->pleaseRepaint();

        }
        if (upper.hit(x1,y1)) {
            keyevent("playlist");
            getMaster()->pleaseRepaint();
        }
        if (g3.hit(x1,y1)) {
            clock_t now = getTime();
            bottom.setText("starting 1 pack");
            greatCheckScenarioCase(1,1,12,3);
            greatCheckScenarioCase(2,1,35,3);
            greatCheckScenarioCase(3,1,70,3);
            clock_t after = getTime();
            clock_t diff = after-now;
            std::string sX = "TIme spent for g3 ; - " + std::to_string(diff) + " ms";;
            bottom.setText(sX.c_str());
        }
        if (g5.hit(x1,y1)) {
            clock_t now = getTime();
            bottom.setText("starting 1 pack");
            greatCheckScenarioCase(1,1,12,5);
            greatCheckScenarioCase(2,1,35,5);
            greatCheckScenarioCase(3,1,70,5);
            greatCheckScenarioCase(4,1,109,5);
            clock_t after = getTime();
            clock_t diff = after-now;
            std::string sX = "TIme spent for g3 ; - " + std::to_string(diff) + " ms";;
            bottom.setText(sX.c_str());
        }
    }

    void fastTestAll();

    void openTestNumber(int num);

    //void ondblclick(int x1, int y1){}
    virtual void keyevent(std::string press)
    {
        if (press=="playlist")
        {
            //Start the playlist
            //Try from 1?
            for (size_t i = 1; i < buttons.size(); ++i) //48- start of 3?
            {
                std::string onText = "on:" + std::to_string(i);
                std::vector<std::string> playlistElement;

                playlistElement.push_back("tests");
                playlistElement.push_back(onText.c_str());
                playlistElement.push_back("spc");

                getMaster()->addToPlaylist(playlistElement);
            }
            qDebug() << "Playlist configured of " << buttons.size() << " entries ";

            getMaster()->goOnPlaylist();
        }

        if (press=="playlist2")
        {
            for (size_t i = 51; i < buttons.size(); ++i)
            {
                std::string onText = "on:" + std::to_string(i);
                std::vector<std::string> playlistElement;

                playlistElement.push_back("tests");
                playlistElement.push_back(onText.c_str());
                playlistElement.push_back("ent");

                getMaster()->addToPlaylist(playlistElement);
            }

            getMaster()->goOnPlaylist();
        }

        if (press=="stop playlist")
        {
            getMaster()->cleanPlayList();
        }

        size_t separatorOn = press.find(":");

        if (separatorOn != std::string::npos)
        {
            std::string theNumber = press.substr(separatorOn+1);
            int buttonNumber = atoi(theNumber.c_str());
            openTestNumber(buttonNumber);
        }

        if (press == "1")
        {
            clock_t now = getTime();
            bottom.setText("starting 1 pack");
            greatCheckScenarioCase(1,1,12,4);
            clock_t after = getTime();
            clock_t diff = after-now;
            std::string sX = "TIme spent for pack 1; - " + std::to_string(diff) + " ms";;
            bottom.setText(sX.c_str());
        }
        if (press == "2")
        {
            clock_t now = getTime();

            bottom.setText("starting 2 pack");
            greatCheckScenarioCase(2,1,35,4);
            clock_t after = getTime();
            clock_t diff = after-now;
            std::string sX = "TIme spent for pack 2; - " + std::to_string(diff) + " ms";
            bottom.setText(sX.c_str());

        }
        if (press == "3")
        {
            clock_t now = getTime();

            bottom.setText("starting 3 pack");
            greatCheckScenarioCase(3,1,70,4);
            bottom.setText("pack 3 finished");
            clock_t after = getTime();
            clock_t diff = after-now;

            std::string sX = "TIme spent for pack 3; - " + std::to_string(diff) + " ms";

            bottom.setText(sX.c_str());
        }
        if (press == "4")
        {
            clock_t now = getTime();
            bottom.setText("starting 4 pack");
            greatCheckScenarioCase(4,1,109,4);
            clock_t after = getTime();
            clock_t diff = after-now;
            std::string sX = "TIme spent for pack 4; - " + std::to_string(diff) + " ms";
            bottom.setText(sX.c_str());
        }
        if (press=="m")
        {
           // getMaster()->resetToFirstChild();
        }

    }
};

/// N ew inputs
///


class ChordInput : public GView
{
protected:
    std::unique_ptr<GLabel> top;
public:
    void setUI()
    {
        if (getMaster())
        {
            getMaster()->setComboBox(0,"chordcombo1",50,20,100,40,-1); //note
            getMaster()->setComboBox(1,"chordcombo2",170,20,100,40,-1); //postfix

            getMaster()->setComboBox(2,"chordcombo3",340,20,100,40,-1); // 9 11 13 - not everywhere awailible
            getMaster()->setComboBox(3,"chordcombo4",510,20,100,40,-1); // _ +5 -5 - not everywhere

            //then only on 9 11 13

            getMaster()->setComboBox(4,"chordcombo5",340,70,100,40,-1); // _ +9 -9
            getMaster()->setComboBox(5,"chordcombo6",510,70,100,40,-1); // _ +11 -11
            getMaster()->setComboBox(6,"chordcombo7",660,70,100,40,-1); // _ + -

            getMaster()->SetButton(7,"back",  600, 20, 70,  30, "esc"); //refact

        }
    }

    ChordInput() {
        top = std::make_unique<GLabel>(100,110,"Chord input","",false);
    }
    virtual ~ChordInput() {}

    void draw(QPainter *painter) {
        top->draw(painter);
    }
    void keyevent([[maybe_unused]]std::string press){}
};

class ChangesInput : public GView
{
protected:
    std::unique_ptr<GLabel> top;

public:

    static Beat *ptrToBeat;
    static void setPtrBeat(Beat *beatPtr);

    void setUI()
    {
        if (getMaster())
        {
            //getMaster()->setComboBox(0,"changecombo1",50,20,150,40,-1); //change type
            getMaster()->setComboBox(0,"instruments",220,20,150,40,-1);
            getMaster()->setComboBox(1,"changecombo3",390,20,150,40,-1);
            getMaster()->setComboBox(2,"changecombo4",550,20,50,40,-1);

            getMaster()->setComboBox(3,"bpm",220,70,150,40,-1);
            getMaster()->setComboBox(4,"changecombo3",390,70,150,40,-1);
            getMaster()->setComboBox(5,"changecombo4",550,70,50,40,-1);

            getMaster()->setComboBox(6,"volume",220,120,150,40,-1);
            getMaster()->setComboBox(7,"changecombo3",390,120,150,40,-1);
            getMaster()->setComboBox(8,"changecombo4",550,120,50,40,-1);

            getMaster()->setComboBox(9,"pan",220,170,150,40,-1);
            getMaster()->setComboBox(10,"changecombo3",390,170,150,40,-1);
            getMaster()->setComboBox(11,"changecombo4",550,170,50,40,-1);

            getMaster()->SetButton(12,"back", 600, 20, 70,  30, "esc"); //refact

        }
    }

    ChangesInput() {
        top = std::make_unique<GLabel>(10,10,"Changes input","",false);
    }
    virtual ~ChangesInput(){}

    void turnOffChange(std::string combo);
    void turnOnChange(std::string combo);

    void changeMainValue(int combo, int newValue);
    void changeSubValue(int combo, int newValue);

    void draw(QPainter *painter);
    void keyevent(std::string press);
};


class BendPoints;
class Note;

class BendInput : public GView
{
protected:
    std::unique_ptr<GLabel> top;

    std::unique_ptr<GLabel> okButton;
    std::unique_ptr<GLabel> delButton;
public:

    static BendPoints *ptrToBend;
    static Note *ptrToNote;

    static void setPtrBend(BendPoints* ptr);
    static void setPtrNote(Note* ptr);

    void fillBend(int type, int height);

    void setUI()
    {
        if (getMaster())
        {
            getMaster()->setComboBox(0,"bendcombo1",50,20,150,40,-1);
            getMaster()->setComboBox(1,"bendcombo2",350,20,150,40,-1);
        }
    }

    BendInput()
    {
        top = std::make_unique<GLabel>(10,10,"Bend input","",false);
        okButton = std::make_unique<GLabel>(530,35,"OK");
        delButton = std::make_unique<GLabel>(630,35,"Delete");
    }
    virtual ~BendInput() {}
    void draw(QPainter *painter);
    void keyevent(std::string press);

    void onclick(int x1, int y1);
};




#endif // COMMONVIEWS_H
