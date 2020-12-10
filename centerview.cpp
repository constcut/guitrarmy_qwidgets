#include "centerview.h"

#include "g0/astreaming.h"

#include <QTextBlock>
#include <QTextCodec>

//base
#include <QFileDialog>
#include "g0/aexpimp.h"


static AStreaming logger("c-v");


GQCombo::GQCombo(QWidget *pa):QComboBox(pa)
{
    elementNumber = -1;
    pushItem=false;
    keyPress = 0;
    connect(this,SIGNAL(activated(int)),SLOT(elementChosen(int)));
    //connect(this,SIGNAL(currentIndexChanged(int)),SLOT(elementChosen(int)));
}

void GQCombo::elementChosen(int index)
{
    LOG(<<"log from combo "<<elementNumber<<" "<<index);
    stringExtended line;
    line<<"com:"<<elementNumber<<":"<<index;

    if (keyPress)
    {
        if (pushItem==false)
        keyPress->pushForceKey(line.c_str());
        else
        {
            QString itemStr = this->currentText();
            std::string itemStd = itemStr.toStdString();
            keyPress->pushForceKey(itemStd);
        }
    }
}

GQButton::GQButton(QWidget *pa):QPushButton(pa)
{
    buttonNumber=-1;
    keyPress = 0;
    connect(this, SIGNAL (clicked()), SLOT (buttonWasClicked()));
}

void GQButton::buttonWasClicked()
{
    if (buttonNumber!= -1)
    {
        if (pressSyn.empty() == false)
        {
             keyPress->pushForceKey(pressSyn);
        }
        else
            if (keyPress)
            {
                stringExtended genPress;
                genPress<<"b:"<<buttonNumber;
                keyPress->pushForceKey(genPress.c_str());
                //audio_logger<<"log from button"<<genPress.c_str();
            }
    }

}

void CenterView::addComboBox(std::string params, int x1, int y1, int w1, int h1, int forceValue)
{
    GQCombo *newBox = new GQCombo(this);
    newBox->setGeometry(x1,y1,w1,h1);
    newBox->setParams("params not set");

    int VType = getCurrentViewType();

    renewComboParams(newBox,params);


    QString style; /*  "QComboBox{ font-size: 10; }\
            QComboBox{ background-color: green; }\
 QComboBox:focus:pressed{ background-color: darkgreen; }\
 QComboBox:focus{ background-color: white; }\
 QComboBox:hover{ background-color: dark blue; }"; */

    stringExtended styleString;

    styleString << "QComboBox{ font-size: 10; }"
                << "QComboBox{ background-color:" << CONF_PARAM("colors.combo.background") << "; }"
                << "QComboBox:focus:pressed{ background-color:" << CONF_PARAM("colors.combo.pressed")<< "; }"
                << "QComboBox:focus{ background-color:"  << CONF_PARAM("colors.combo.focus")<< "; }"
                << "QComboBox:hover{ background-color:"  << CONF_PARAM("colors.combo.focus")<< "; }";


    style = QString(styleString.c_str());

    newBox->setStyleSheet(style);

    if (forceValue>0)
        newBox->setCurrentIndex(forceValue);

    static QFont comboFont("Arial",10);

    newBox->setFont(comboFont);


    //avoid first call
    newBox->setElementNum(uiWidgets.at(VType).size());
    newBox->setKeyPress(this);

    newBox->show();

    uiWidgets.at(VType).push_back(newBox);


   // patternInstruments.push_back(newBox);
}

void CenterView::ViewWasChanged()
{

    lastCheckedView; //is prev
    //could be used to set invisible old

    //SCROLL UP
    if (fatherScroll)
        fatherScroll->ensureVisible(0,0);

    int VType = getCurrentViewType();



    std::map<int, std::vector<QWidget*> >::iterator itF = uiWidgets.find(lastCheckedView);
    if (itF != uiWidgets.end())
    {
        std::vector<QWidget*> localWidgets = itF->second;
        for (int j = 0; j < localWidgets.size(); ++j)
                localWidgets[j]->setVisible(false);
    }


    if (VType!=1)
    {
        std::map<int, std::vector<QWidget*> >::iterator itForce = uiWidgets.find(1);
        if (itForce != uiWidgets.end())
        {
            std::vector<QWidget*> localWidgets = itForce->second;
            for (int j = 0; j < localWidgets.size(); ++j)
                    localWidgets[j]->setVisible(false);
        }
    } //fix for old tab view

    /*
    for (int i = 0 ; i < 16; ++i)
    {
        std::map<int, std::vector<QWidget*> >::iterator itF = uiWidgets.find(i);
        if (itF != uiWidgets.end())
        {
            std::vector<QWidget*> localWidgets = itF->second;
            for (int j = 0; j < localWidgets.size(); ++j)
            {
                if (i != VType)
                    localWidgets[j]->setVisible(false);
                else
                  localWidgets[j]->setVisible(true);
            }
        }
    }*/

    //if (fatherScroll)
        requestHeight(250);
        requestWidth(650);


    stringExtended tipLine;

    if (VType == 1)
        tipLine << "Double-click on preview or bar number to open track";
    if (VType == 2)
        tipLine << "TrackView opened. You can edit tab here";
    if (VType == 3)
        tipLine << "PatternInput opened. You can set drums here";
    if (VType == 4)
        tipLine << "TapInput opened. You can tap rhythm here";
    if (VType == 5)
        tipLine << "RecordInput opened. Yet you can only record sound here";
    if (VType == 6)
        tipLine << "ConfigView opened. You can set configuration here, to apply - move to any view, to save - use toolMenu";
    if (VType == 8)
        tipLine << "TestsView opened. Some tablatures prepared here for tests";

    int showTime = 2000;
    if (VType == 1)
        showTime = 5000;

    if (CONF_PARAM("showStatusViewHint")=="1")
    if (tipLine.size())
    setStatusBarMessage(0,tipLine.c_str(),showTime);


    //Some undone way
    if (VType==2)
    {
        int panInd=1;
        if (CONF_PARAM("skipTabView")!="1")
            panInd=2;

        if (statusSetter)
        statusSetter->setViewPannel(panInd);
    }

}

void CenterView::renewComboParams(int index, std::string params)
{
    int VType = getCurrentViewType();
    int inWidAmount = uiWidgets.at(VType).size();
    if (inWidAmount > index)
    {
        GQCombo *newBox = dynamic_cast<GQCombo*>(uiWidgets.at(VType)[index]);
        renewComboParams(newBox,params);
    }
}

void CenterView::renewComboParams(GQCombo *newBox, std::string params)
{
       if (newBox==0) return;//attention investigate simmiliar

        std::string oldParams = newBox->getParams();
        if (oldParams != params)
        {
            int countBefore = newBox->count(); //amount of already set items

            //here set same as in add proc
            ////this point must be refactored later
            ///not to repeat such long & useles twice code
            size_t separator = params.find(";");
            if (separator != std::string::npos)
            {
                //make list from
                std::string part;
                do
                {
                    part = params.substr(0,separator);
                    QString uniText = QTextCodec::codecForLocale()->toUnicode(part.c_str());
                    newBox->addItem(uniText);

                    params = params.substr(separator+1);
                    separator = params.find(";");
                }
                while (separator != std::string::npos);
                params = "done.custom";//avoid issues
            }

            if (params=="mutesolo")
            {
                  newBox->addItem("none");
                  newBox->addItem("mute");
                  newBox->addItem("solo");
            }

            if (params=="changecombo4")
            {
                newBox->addItem("off");
                newBox->addItem("on");
            }

            if (params=="instruments")
            {
                std::string instruments[]=
                        {
                            "Acoustic Grand Piano",
                            "Bright Acoustic Piano",
                            "Electric Grand Piano",
                            "Honky-tonk Piano",
                            "Rhodes Piano",
                            "Chorused Piano",
                            "Harpsichord",
                            "Clavinet",
                            "Celesta",
                            "Glockenspiel",
                            "Music Box",
                            "Vibraphone",
                            "Marimba",
                            "Xylophone",
                            "Tubular Bells",
                            "Dulcimer",
                            "Hammond Organ",
                            "Percussive Organ",
                            "Rock Organ",
                            "Church Organ",
                            "Reed Organ",
                            "Accodion",
                            "Hrmonica",
                            "Tango Accodion",
                            "Acoustic Guitar (nylon)",
                            "Acoustic Guitar (steel)",
                            "Electric Guitar (jazz)",
                            "Electric Guitar (clean)",
                            "Electric Guitar (muted)",
                            "Overdriven Guitar",
                            "Distortion Guitar",
                            "Guitar Harmonics",
                            "Acoustic Bass",
                            "Electric Bass (finger)",
                            "Electric Bass (pick)",
                            "Fretless Bass",
                            "Slap Bass 1",
                            "Slap Bass 2",
                            "Synth Bass 1",
                            "Synth Bass 2",
                            "Violin",
                            "Viola",
                            "Cello",
                            "Contrabass",
                            "Tremolo Strings",
                            "Pizzicato Strings",
                            "Orchestral Harp",
                            "Timpani",
                            "String Ensemble 1",
                            "String Ensemble 2",
                            "SynthStrings 1",
                            "SynthStrings 2",
                            "Choir Aahs",
                            "Voice Oohs",
                            "Synth Voice",
                            "Orchetra Hit",
                            "Trumpet",
                            "Trombone",
                            "Tuba",
                            "Muted Trumpet",
                            "French Horn",
                            "Brass Section",
                            "Synth Brass 1",
                            "Synth Brass 2",
                            "Soprano Sax",
                            "Alto Sax",
                            "Tenor Sax",
                            "Baritone Sax",
                            "Oboe",
                            "English Horn",
                            "Bassoon",
                            "Clarinet",
                            "Piccolo",
                            "Flute",
                            "Recorder",
                            "Pan Flute",
                            "Bottle Blow",
                            "Shakuhachi",
                            "Wistle",
                            "Ocarina",
                            "Lead 1 (square)",
                            "Lead 2 (sawtooth)",
                            "Lead 3 (caliope lead)",
                            "Lead 4 (chiff lead)",
                            "Lead 5 (charang)",
                            "Lead 6 (voice)",
                            "Lead 7 (hiths)",
                            "Lead 8 (bass + lead)",
                            "Pad 1 (new age)",
                            "Pad 2 (warm)",
                            "Pad 3 (polysynth)",
                            "Pad 4 (choir)",
                            "Pad 5 (bowed)",
                            "Pad 6 (metalic)",
                            "Pad 7 (halo)",
                            "Pad 8 (sweep)",
                            "FX 1 (rain)",
                            "FX 2 (soundrack)",
                            "FX 3 (crystl)",
                            "FX 4 (atmosphere)",
                            "FX 5 (brightness)",
                            "FX 6 (goblins)",
                            "FX 7 (echoes)",
                            "FX 8 (sci-fi)",
                            "Sitar",
                            "Banjo",
                            "Shamisen",
                            "Koto",
                            "Kalimba",
                            "Bigpipe",
                            "Fiddle",
                            "Shanai",
                            "Tinkle Bell",
                            "Agogo",
                            "Steel Drums",
                            "Woodblock",
                            "Taiko Drum",
                            "Melodic Tom",
                            "Synth Drum",
                            "Reverce Cymbal",
                            "Guitar Fret Noise",
                            "Breath Noise",
                            "Seashore",
                            "Bird Tweet",
                            "Telephone ring",
                            "Helicopter",
                            "Applause",
                            "Gunshot"
                        };

                //moved
                for (int i = 0; i < 128; ++i)
                {
                    newBox->addItem(instruments[i].c_str());
                }

            }
            if (params=="volume")
            {
                for (int i = 0; i < 17; ++i)
                {
                    stringExtended sX;
                    sX<<i;
                    newBox->addItem(sX.c_str());
                }
            }


            if (params=="drums")
            {
                std::string percussions[]=
                {
                "27 - High Q",
                "28 - Slap",
                "29 - Scratch Push",
                "30 - Scratch Pull",
                "31 - Sticks",
                "32 - Square Click",
                "33 - Metronome Click",
                "34 - Metronome Bell",
                "35 - Acoustic Bass Drum",
                "36 - Bass Drum 1",
                "37 - Side Stick",
                "38 - Acostic Snare",
                "39 - Hand Clap",
                "40 - Electric Snare",
                "41 - Low Floor Tom",
                "42 - Closed Hi-Hat",
                "43 - High Floor Tom",
                "44 - Pedal Hi-Hat",
                "45 - Low Tom",
                "46 - Open Hi-Hat",
                "47 - Low-Mid Tom",
                "48 - High-Mid Tom",
                "49 - Crash Cymbal 1",
                "50 - High Tom",
                "51 - Ride Cymbal 1",
                "52 - Chinese Cymbal",
                "53- Ride Bell",
                "54 - Tambourine",
                "55 - Splash Cymbal",
                "56 - Cowbell",
                "57 - Crash Cymbal 2",
                "58 - Vibraslap",
                "59 - Ride Cymbal 2",
                "60 - High Bongo",
                "61 - Low Bongo",
                "62 - Mute High Conga",
                "63 - Open High Conga",
                "64 - Low Conga",
                "65 - High Timbale",
                "66 - Low Timbale",
                "67 - High Agogo",
                "68 - Low Agogo",
                "69 - Cabasa",
                "70 - Maracas",
                "71 - Short Whistle",
                "72 - Long Whistle",
                "73 - Short Guiro",
                "74 - Long Guiro",
                "75 - Claves",
                "76 - High Wood Block",
                "77 - Low  Wood Block",
                "78 - Mute Cuica",
                "79 - Open Cuica",
                "80 - Mute Triangle",
                "81 - Open Triangle",
                "82 - Shaker",
                "83 - Jingle Bell",
                "84 - Bell Tree",
                "85 - Castinets",
                "86 - Mute Surdo",
                "87 - Open Surdo"
                };

                for (int i = 0; i < 61; ++i)
                {
                    newBox->addItem(percussions[i].c_str());
                }
            }

            if (params == "changecombo1")
            {
                newBox->addItem("instrument");
                newBox->addItem("bpm");
                newBox->addItem("volume");
                newBox->addItem("pan");
            }

            if (params == "changecombo2")
            {
                newBox->addItem("not set");
                //depends on ch com 1 then
            }

            if (params == "changecombo3")
            {
                newBox->addItem("right now");
                //in 1 beat etc

                for (int i = 1; i < 32; ++i)
                {
                    stringExtended sX;
                    sX << i << " beats later";
                    newBox->addItem(sX.c_str());
                }
            }

            if (params == "bendcombo1")
            {
                newBox->addItem("bend");
                newBox->addItem("bend-release");
                newBox->addItem("bend-release-bend");
                newBox->addItem("prebend");
                newBox->addItem("prebend-release");
            }

            if (params == "bendcombo2")
            {
                 newBox->addItem("3.00");
                 newBox->addItem("2.75");
                 newBox->addItem("2.50");
                 newBox->addItem("2.25");
                 newBox->addItem("2.00");
                 newBox->addItem("1.75");
                 newBox->addItem("1.50");
                 newBox->addItem("1.25");
                 newBox->addItem("1.00");
                 newBox->addItem("0.75");
                 newBox->addItem("0.50");
                 newBox->addItem("0.25");
           }

            if (params == "chordcombo1")
            {
                newBox->addItem("C");
                newBox->addItem("C#");
                newBox->addItem("D");
                newBox->addItem("D#");
                //etc
            }

            if (params == "chordcombo2")
            {
                newBox->addItem("M");
                newBox->addItem("7");
                newBox->addItem("7M");
                newBox->addItem("6");
                //etc
            }

            if ((params == "chordcombo3")||
                (params == "chordcombo4")||
                (params == "chordcombo5")||
                (params == "chordcombo6")||
                (params == "chordcombo7"))
            {
                newBox->addItem("yet-noset");
            }

            if (params == "pan")
            {
                //newBox->addItem("L8");
                newBox->addItem("L8 - 100%");
                newBox->addItem("L7");
                newBox->addItem("L6 ");
                newBox->addItem("L5");
                newBox->addItem("L4");
                newBox->addItem("L3");
                newBox->addItem("L2");
                newBox->addItem("L1");
                newBox->addItem("Center - 0%");
                newBox->addItem("R1");
                newBox->addItem("R2");
                newBox->addItem("R3");
                newBox->addItem("R4");
                newBox->addItem("R5");
                newBox->addItem("R6");
                newBox->addItem("R7");
                newBox->addItem("R8 - 100%");
            }

            if (params=="bpm")
            {
                for (int i = 1; i < 300; ++i)
                {
                    stringExtended sX;
                    sX<<i;
                    newBox->addItem(sX.c_str());
                }
            }

            while (countBefore--)
                newBox->removeItem(0);
        }
}

void CenterView::setComboBox(int index, std::string params, int x1, int y1, int w1, int h1, int forceValue)
{
    int VType = getCurrentViewType();

    double scaleCoef = AConfig::getInstance()->getScaleCoef();

    int inWidAmount = uiWidgets.at(VType).size();
    if (inWidAmount > index)
    {
        //change values
        GQCombo *newCom = dynamic_cast<GQCombo*>(uiWidgets.at(VType)[index]);

        if (newCom==0) return;//attention investigate simmiliar
        x1*=scaleCoef;
        y1*=scaleCoef;
        h1*=scaleCoef;
        w1*=scaleCoef;

        if ((newCom->geometry().x() != x1) ||
            (newCom->geometry().y() != y1))
        {
            newCom->setGeometry(x1,y1,w1,h1); //
        }

        std::string oldParams = newCom->getParams();

        ///main issue?

        if (oldParams != params)
        {
            newCom->setParams(params);
            //newCom->setVisible(false);

            size_t separator = params.find(";");
            if (separator != std::string::npos)
            {
                int countBefore = newCom->count();
                //while (newCom->count()>0)
                //    newCom->removeItem(0);
                //make list from


                std::string part;
                do
                {
                    part = params.substr(0,separator);
                    QString uniText = QTextCodec::codecForLocale()->toUnicode(part.c_str());
                    newCom->addItem(uniText);

                    params = params.substr(separator+1);
                    separator = params.find(";");
                }
                while (separator != std::string::npos);
                params = "done.custom";//avoid issues

                while (countBefore--)
                    newCom->removeItem(0);
            }

            //if (ownChild)
            //newCom->setVisible(true);
        }


        if (forceValue>=0)
        {
           if (newCom->currentIndex() != forceValue)
           {
               //newCom->hide(); //will be bugged
               newCom->setCurrentIndex(forceValue); //bug could be here
               //newCom->show();
           }
        }

        newCom->show();

    }
    else
    {
        x1*=scaleCoef;
        y1*=scaleCoef;
        h1*=scaleCoef;
        w1*=scaleCoef;
        addComboBox(params,x1,y1,w1,h1,forceValue);
    }
}

int CenterView::getComboBoxValue(int index)
{
    int VType = getCurrentViewType();
     if (uiWidgets.at(VType).size()>index)
     {
          QComboBox *newCom = dynamic_cast<QComboBox*>(uiWidgets.at(VType)[index]);
          int selected = newCom->currentIndex();
          return selected;
     }
     return -1;

}
void CenterView::SetButton(int index,std::string text, int x1, int y1, int w1, int h1, std::string pressSyn)
{
    int VType = getCurrentViewType();
    double scaleCoef = AConfig::getInstance()->getScaleCoef();

    if (uiWidgets.at(VType).size()>index)
    {
        //change values
        QPushButton *newBut = dynamic_cast<QPushButton*>(uiWidgets.at(VType)[index]);
        x1*=scaleCoef;
        y1*=scaleCoef;
        h1*=scaleCoef;
        w1*=scaleCoef;

        if ((newBut->geometry().x() != x1) ||
            (newBut->geometry().y() != y1))
        {
          newBut->setGeometry(x1,y1-h1,w1+10,h1*2);
        }

        QString newText = text.c_str();

        if (newBut->text() != newText)
            newBut->setText(newText);

        newBut->setVisible(true);

    }
    else
    {
        x1*=scaleCoef;
        y1*=scaleCoef;
        h1*=scaleCoef;
        w1*=scaleCoef;
        addButton(text,x1,y1,w1,h1,pressSyn);
    }
}

void CenterView::addButton(std::string text, int x1, int y1, int w1, int h1, std::string pressSyn)
{
    GQButton *newBut = new GQButton(this);
    newBut->setText(text.c_str());
    newBut->setGeometry(x1,y1-h1,w1+10,h1*2);

    if (pressSyn.empty()==false)
    {
        newBut->setPressSyn(pressSyn);
    }


   QString style; /*=  "QPushButton{ font-size: 10; }\
           QPushButton{ background-color: green; }\
QPushButton:focus:pressed{ background-color: darkgreen; }\
QPushButton:focus{ background-color: white; }\
QPushButton:hover{ background-color: darkblue; }"; */


   stringExtended styleString;

   styleString << "QPushButton{ font-size: 10; }"

               << "QPushButton:focus:pressed{ background-color:" << CONF_PARAM("colors.button.pressed")<< "; }"
               << "QPushButton:focus{ background-color:"  << CONF_PARAM("colors.button.focus")<< "; }"
               << "QPushButton:hover{ background-color:"  << CONF_PARAM("colors.button.focus")<< "; }";



   if (text=="play")
        styleString << "QPushButton{ background-color:" << CONF_PARAM("colors.button.bgspecial") << "; }";
   else
       styleString << "QPushButton{ background-color:" << CONF_PARAM("colors.button.background") << "; }";

   style = QString(styleString.c_str());

   newBut->setStyleSheet(style);

   static QFont buttonsFont("Arial",10);

   newBut->setFont(buttonsFont);

   int VType = getCurrentViewType();

   //testButtons.push_back(newBut);
    newBut->setButtonNum(uiWidgets.at(VType).size());
    newBut->setKeyPress(this);

   uiWidgets.at(VType).push_back(newBut);

   newBut->setVisible(true);
}


CenterView::CenterView(QWidget *parent):MasterView(),ownChild(0),QWidget(parent)
{
    welcomeText = 0;
    confEdit = 0;
    statusSetter = 0;
    fatherScroll = 0;
    xOffsetGesture=0;
    yOffsetGesture=0;
    isPressed=false;

    for (int i = 0; i < 16; ++i)
    uiWidgets[i] = std::vector<QWidget*>();
}

void CenterView::connectThread(void *localThr)
{
    ThreadLocal *threadLocal = (ThreadLocal*)localThr;


    this->connect(
        threadLocal,
        SIGNAL(updateUI()),
        SLOT(update()),
        Qt::QueuedConnection);
}

void CenterView::connectMainThread(void *localThr)
{
    ThreadLocal *threadLocal = (ThreadLocal*)localThr;


    this->connect(
        threadLocal,
        SIGNAL(nowFinished()),
        SLOT(threadFinished()),
        Qt::QueuedConnection);
}

int CenterView::getToolBarHeight()
{
    return 0;
}

int CenterView::getStatusBarHeight()
{
    return 0;
}

int CenterView::getWidth()
{
    double scaleCoef = AConfig::getInstance()->getScaleCoef();

    if (ownChild==0)
    {
        scaleCoef -= 0.5;
        if (scaleCoef < 0.0)
            scaleCoef = 0.25;
    }

    int response = this->geometry().width();
    response /= scaleCoef;
    return response;
}

int CenterView::getHeight()
{
    double scaleCoef = AConfig::getInstance()->getScaleCoef();

    if (ownChild==0)
    {
        scaleCoef -= 0.5;
        if (scaleCoef < 0.0)
            scaleCoef = 0.25;
    }

    int response = this->geometry().height();
    response /= scaleCoef;
    return response;
}

void CenterView::setStatusBarMessage(int index, std::string text, int timeOut=0)
{
   if (statusSetter)
   {
       statusSetter->setStatusBarMessage(index,text,timeOut);
   }
}

/*
void checkBase()
{

    if (CONF_PARAM("onloadBaseCheck")=="1")
    {

        return; //release version
        QString testsLoc = getTestsLocation() + QString("gb");//QString("gtp")
        //
        QDir    curDir(testsLoc);

        QStringList filters;
        QString filter = "*";  filters.push_back(filter); //all files yet, then refact good
        QStringList files = curDir.entryList(filters,QDir::Files | QDir::NoSymLinks);
        //QDir::Dirs  - search also for sub directories
        QStringList outputList;

        std::map<std::string,std::string> mdCheckList



        for (int i = 166156; i < files.size(); ++i)
        {
            QString fileName = testsLoc +QString("/")+ files[i];


            QFile baseFile(fileName);
            baseFile.setFileName(fileName);
            if (baseFile.open(QIODevice::ReadOnly) == true)
            {
                QByteArray wholeFile = baseFile.readAll();
                baseFile.close();

                int fileSize = wholeFile.size();

                QCryptographicHash mdfiver(QCryptographicHash::Md5);

                mdfiver.reset();
                mdfiver.addData(wholeFile);
                QString mdsum = mdfiver.result().toHex();

                ///+testsLoc
                stringExtended sX;

                int fileType = sayType(wholeFile);


                std::string md5Std = mdsum.toStdString();



                if (mdCheckList.find(md5Std)!=mdCheckList.end())
                {
                    logger << "ATTENTION MD5 is same !!!";
                }
                else
                {
                    std::string fnStd = files[i].toStdString();
                    mdCheckList.insert(std::pair<std::string,std::string>
                                       (md5Std,fnStd));
                }


                sX<<(i+1)<<" "<<fileName.toStdString()<<" "<<fileSize<<" "<<mdsum.toStdString()<<" "<<fileType<<"\n";
                //missing file type gtp gmy ptb etc

                logger<<sX.c_str();
                logger<<"+";
                QString newLine = sX.c_str();
                outputList.push_back(newLine);

                if ((fileType>=3) && (fileType <=5))
                {

                    GTabLoader tabLoader;

                    std::string fnStd = fileName.toStdString();

                    if (tabLoader.open(fnStd) == true)
                    {
                        Tab *tab = tabLoader.getTab();
                        logger << "Tab bpm "<<tab->getBPM();
                        delete tab;
                    }


                    //gtps
                    //slow and stupid but noe exceed code rigt now
                }
            }
            else
            {
                logger <<"ERROR oppening file "<<fileName.toStdString().c_str();;
            }

        }
    }

} */

void CenterView::pushForceKey(std::string keyevent)
{
    if (keyevent.empty()==false)
    {
        LOG( << "Push force '"<<keyevent<<"' size="<<(int)keyevent.size());
        //for (int i = 0; i < keyevent.size(); ++i)
        //     audio_logger << ".[" << i << "] = " << (int)(keyevent[i]);
    }

    if (keyevent=="ctrl+b")
    {
        QString baseFileName = getTestsLocation() + QString("base.txt");

        QFile baseFile;
        baseFile.setFileName(baseFileName);
        baseFile.open(QIODevice::ReadOnly);

        int lineCount = 0;

        if (lineCount)
        {
            int preLineCount = lineCount;

            while (preLineCount > 0)
            {
                --preLineCount;
                QString oneMoreLine = baseFile.readLine();
            }
        }

        int goodTest = 0;
        int badTests = 0;


        while (!baseFile.atEnd())
        {
            QString oneMoreLine = baseFile.readLine();

            std::string fileName = oneMoreLine.toStdString();
            fileName = fileName.substr(fileName.find(":")+2);

            std::string type = fileName.substr(fileName.find(" : ")+3);
            type = type.substr(0,type.size()-1);
            fileName = fileName.substr(0,fileName.find(" : "));

            LOG( << "'"<<type.c_str()<<"' and '"<<fileName.c_str()<<"' = "<<lineCount);

            ++lineCount;
            GTabLoader tabLoader;

            if ((type.substr(0,2)=="TP") || (type.substr(0,2)=="PT"))
            {
                try
                {
                    tabLoader.open(fileName);
                    Tab *tab = tabLoader.getTab();
                    LOG( <<"OK!");
                    delete tab;
                    ++goodTest;
                }
                catch(...)
                {
                    logger <<"Failed!";
                    ++badTests;
                }
            }
        }

        LOG( << "Good "<<goodTest<<"; bad "<<badTests);


        baseFile.close();
    }

    if (keyevent=="alt+b")
    {
        //opening base
        /*
        QFileDialog *fd = new QFileDialog;


        fd->setStyleSheet("QScrollBar:horizontal {\
                          border: 2px solid grey;\
                          background: #32CC99;\
                          height: 15px;\
                          margin: 0px 20px 0 20px;\
                      }\
                      QLineEdit { height: 20px; \
                      }");

        fd->setViewMode(QFileDialog::List);
        fd->setFileMode(QFileDialog::Directory);
        fd->setOption(QFileDialog::ShowDirsOnly);

        QString directoryName;
        if (fd->exec())
        directoryName = fd->selectedFiles().at(0);

        audio_logger << s.toStdString().c_str() << " -chosen folder";

        delete fd;
        */

        QString testsLoc = getTestsLocation() + QString("gb");//QString("gtp")

        QString directoryName = testsLoc; //"D:/base/gbase1/gtp4";

        QDir    searchDir(directoryName);

        QStringList filters;
        QString filter = "*";  filters.push_back(filter); //all files yet, then refact good
        QStringList files = searchDir.entryList(filters,QDir::Files | QDir::NoSymLinks);

        QString baseFileName = getTestsLocation() + QString("base.txt");

        QFile baseFile;
        baseFile.setFileName(baseFileName);
        baseFile.open(QIODevice::WriteOnly);


        logger << "Starting base file ";
        int fullCount = 0;

        for (int i = 0; i < files.size(); ++ i)
        {
            QString currentFile = directoryName +QString("/") + files[i];
            QFile fileDesc;

            fileDesc.setFileName(currentFile);
            fileDesc.open(QIODevice::ReadOnly);
            QByteArray fileHead = fileDesc.read(40);
            fileDesc.close();

            stringExtended sX; sX<<fullCount<<" : ";
            QString currentLine = QString(sX.c_str()) + currentFile + QString(" : ");

            if (i%100 == 0)
            {
                LOG( <<"For "<<i<<" there was "<<fullCount);
            }

            if (fileHead[1] == 'F' &&
            fileHead[2] == 'I' &&
            fileHead[3] == 'C' &&
            fileHead[4] == 'H')
            {
                //gtp 2-3-4-5
                byte version = fileHead[21]-48;
                QString tail = "TP";

                if (version==3) tail += "3";
                if (version==4) tail += "4";
                if (version==5) tail += "5";


                currentLine += tail;
                currentLine += QString("\n");
                ++fullCount;
                baseFile.write(currentLine.toStdString().c_str());
            }

            if (fileHead[0]=='p'&&
                    fileHead[1]=='t'&&
                    fileHead[2]=='a'&&
                    fileHead[3]=='b')
            {
               QString tail = "PT";

               currentLine += tail;
               currentLine += QString("\n");

               ++fullCount;
               baseFile.write(currentLine.toStdString().c_str());
            }

        }

        baseFile.close();
        LOG( << "Ended base file ");
    }

    if (keyevent=="openPannel")
    {
        if (statusSetter)
            statusSetter->pushForceKey(keyevent);
        return;
    }

    if ((keyevent=="edit / clipboard")||
            (keyevent=="menu")||
            (keyevent=="effects")||
            (keyevent=="track / bar")||
            (keyevent=="num / move") ||
            (keyevent=="all"))
    {
        statusSetter->pushForceKey(keyevent);
        return;
    }

    if ((keyevent=="start_audioi") ||
         (keyevent=="stop_audioi") ||
            (keyevent=="stop_record_output"))
    {
        statusSetter->pushForceKey(keyevent);
        return;
    }

    if (keyevent.substr(0,19)=="start_record_output")
    {
        statusSetter->pushForceKey(keyevent);
        return;
    }

    if (keyevent=="playMidi")
    {
        keyevent = CONF_PARAM("TrackView.playMidi");
    }
    if (keyevent=="playAMusic")
    {
        keyevent = CONF_PARAM("TrackView.playAMusic");
    }



    if (welcomeText)
    {
        delete welcomeText;
        welcomeText = 0;
    //if (welcomeText->isHidden()==false)
      //  welcomeText->hide();
    }

    if (confEdit)
    {
        AConfig::getInstance()->cleanValues();
        QTextDocument *confDocument = confEdit->document();

        for (int i = 0; i < confDocument->lineCount(); ++i)
        {
           QString oneMoreLine = confDocument->findBlockByLineNumber(i).text();
           AConfig::getInstance()->addLine(oneMoreLine.toStdString());
        }

        AConfig::getInstance()->checkConfig();

        delete confEdit;
        confEdit = 0;
    }

    checkView();
    if (getFirstChild())
        getFirstChild()->keyevent(keyevent);

    checkView();

    update();
}

bool CenterView::isPlaying()
{
    MainView *mv = (MainView*)getFirstChild();

    return mv->isPlaying();
}

void CenterView::addToPlaylist(std::vector<std::string> playElement)
{
        playlist.push_back(playElement);
}

bool CenterView::isPlaylistHere()
{
     return playlist.empty()==false;
}

void CenterView::goOnPlaylist()
{
    std::vector<std::string> playElement = playlist.front();
    playlist.pop_front();

    for (size_t i = 0; i < playElement.size(); ++i)
        pushForceKey(playElement[i]);
}

void CenterView::cleanPlayList()
{
    playlist.clear();
}

void CenterView::pleaseRepaint()
{
    update();
}

int CenterView::getCurrentViewType()
{
    GView *gview = getChild();
    MainView *mv = dynamic_cast<MainView*>(gview);

    if (mv)
    {
        gview = mv->getCurrenView();

        if (dynamic_cast<TabView*>(gview) != 0)
            return 1;

        if (dynamic_cast<TrackView*>(gview) != 0)
            return 2;

        if (dynamic_cast<PatternInput*>(gview) != 0)
            return 3;

        if (dynamic_cast<TapRyView*>(gview) != 0)
            return 4;

        if (dynamic_cast<RecordView*>(gview) != 0)
            return 5;

        if (dynamic_cast<ConfigView*>(gview) != 0)
            return 6;

        if (dynamic_cast<InfoView*>(gview) != 0)
            return 7;

        if (dynamic_cast<TestsView*>(gview) != 0)
            return 8;

        if (dynamic_cast<MorzeInput*>(gview) != 0)
            return 9;

        if (dynamic_cast<BendInput*>(gview) != 0)
            return 10;
        if (dynamic_cast<ChangesInput*>(gview) != 0)
            return 11;
        if (dynamic_cast<ChordInput*>(gview) != 0)
            return 12;

        //4 more then need extra vectors for ui

    }
    return 0;
}

void CenterView::startAudioInput()
{
    logger<<"Audio input started";
    audioInfo->start();
    audioInput->start(audioInfo);

}

void CenterView::stopAudioInput()
{
    logger<<"Audio input stopped";
    audioInfo->stop();
    audioInput->stop();
}

void CenterView::initAudioInput() //refact - hide audio f()
{
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(format)) {
        logger << "Default format not supported - trying to use nearest";
        format = info.nearestFormat(format);
    }

    audioInfo  = new AudioInfo(format, this);
    //connect(audioInfo, SIGNAL(update()), SLOT(refreshDisplay()));
    audioInput = new QAudioInput(QAudioDeviceInfo::defaultInputDevice(), format, this);
    //m_volumeSlider->setValue(m_audioInput->volume() * 100);

}

void CenterView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);


    QFont defaultFont("Times");//arial-9
    defaultFont.setPixelSize(14);
    //defaultFont.setWeight();
    painter.setFont(defaultFont);

    Painter abstractedPainer(&painter);

    if (getChild())
        abstractedPainer.fillRect(0,0,width(),height(),CONF_PARAM("colors.background"));

    abstractedPainer.drawImage(0,0,"bg");
    abstractedPainer.changeColor(CONF_PARAM("colors.default"));

    double scaleCoef = AConfig::getInstance()->getScaleCoef();

    int VType = getCurrentViewType();

    if ((VType==1))
    {
       //scaleCoef += 0.5;
    }

    if (ownChild==0)
    {
        scaleCoef /= 2;
        if (scaleCoef < 0.0)
            scaleCoef = 0.25;
    }

    qreal scaleX = scaleCoef;
    qreal scaleY = scaleCoef;

    painter.scale(scaleX,scaleY);
    //painter.drawRect(0,0,geometry().width(),geometry().height());


    if (getChild())
    {
    GView *gview = getChild();
    MainView *mv = dynamic_cast<MainView*>(gview);

        if (mv)
        {
            gview = mv->getCurrenView();

            if (gview->isMovableX()==false)
                xOffsetGesture=0;
            if (gview->isMovableY()==false)
                yOffsetGesture=0;

            painter.translate(xOffsetGesture,yOffsetGesture);
        }
    }

    if (ownChild==0)
        abstractedPainer.drawText(10,10,"DOUBLE CLICK TO OPEN TRACK (ON PREVIEW OR BAR NUMBER)");


    if (getChild())
    getChild()->draw(&abstractedPainer);
    else
        abstractedPainer.fillRect(0,0,getWidth(),getHeight(),"red");


    if (ownChild) ownChild->setVisible(false);

    int curVType = getCurrentViewType();
    if (curVType==1) //opens on tab view
    {

        if (ownChild)
        {
        ownChild->setVisible(true);


        MainView *mw = getFirstChild();
        TabView *tabV = mw->getCurrenView();

        if (tabV->tracksView.size())
        {
            TrackView *trackV = tabV->tracksView[tabV->getCurTrack()];
            trackV->setMaster(mw->getMaster());

            MainView *mw2 =  ownChild->getFirstChild();
            mw2->changeCurrentView(trackV);
        }


        //find track view
        }

        //no preview
    }
    else
    {
        if (curVType==6)
        {
            showConf();
        }
        else
        {
            if (confEdit)
            {
                confEdit->setVisible(true);
                AConfig::getInstance()->cleanValues();
                QTextDocument *confDocument = confEdit->document();

                for (int i = 0; i < confDocument->lineCount(); ++i)
                {
                   QString oneMoreLine = confDocument->findBlockByLineNumber(i).text();
                   AConfig::getInstance()->addLine(oneMoreLine.toStdString());
                }

                AConfig::getInstance()->checkConfig();

                delete confEdit;
                confEdit = 0;
            }
        }

        if (ownChild)
        ownChild->setVisible(false);
    }


}

void CenterView::draw(Painter *painter)
{
    if (getChild())
    getChild()->draw(painter);

    int yL = getHeight();
    int xL = getWidth();

    painter->drawRect(0,0,xL,yL);
    painter->drawRect(10,10,xL-10,yL-10);
}

void CenterView::mousePressEvent( QMouseEvent * event )
{
    int xPress = event->localPos().x();
    int yPress = event->localPos().y();


    if(event->buttons()&Qt::LeftButton)
    {
        double scaleCoef = AConfig::getInstance()->getScaleCoef();

        lastPressX = xPress/scaleCoef;
        lastPressY = yPress/scaleCoef;

        isPressed = true;

    }
}

void CenterView::mouseDoubleClickEvent( QMouseEvent * event )
{
    int xPress = event->localPos().x(); //event->globalX();
    int yPress = event->localPos().y(); //event->globalY();

    checkView();
    ondblclick(xPress,yPress);
    checkView();
     }

void CenterView::keyPressEvent ( QKeyEvent * event )
{

    int value = event->key();
    char key = (char)value;
    char c_style[2];

    c_style[0] = tolower(key);
    c_style[1] = 0;

    std::string singleKey = "";
    singleKey = c_style;

    //no escape no more
    if (value == Qt::Key_Right)
         singleKey =">>>";
    else if (value == Qt::Key_Left)
         singleKey ="<<<";
    else if (value == Qt::Key_Up)
         singleKey ="^^^";
    else if (value == Qt::Key_Down)
         singleKey ="vvv";
    else if (value == Qt::Key_Space)
         singleKey ="spc";
    else if (value == Qt::Key_Return)
         singleKey ="ent";
    else if (value == Qt::Key_Delete)
         singleKey ="del";
    else if (value == Qt::Key_Escape)
         singleKey ="esc";
    else if (value == Qt::Key_Back)
        singleKey = "esc"; //key back !!
    else if (value == Qt::Key_Insert)
        singleKey = "ins";


    if (event->modifiers() & Qt::ControlModifier)
        singleKey = std::string("ctrl+") + singleKey;

    if (event->modifiers() & Qt::AltModifier)
        singleKey = std::string("alt+") + singleKey;

    if (event->modifiers() & Qt::ShiftModifier)
        singleKey = std::string("shift+") + singleKey;

checkView();
     if (getChild())
     getChild()->keyevent(singleKey);
checkView();

    event->accept();

    this->repaint();
}

void CenterView::mouseMoveEvent( QMouseEvent *event )
{
    int xPress = event->pos().x();
    int yPress = event->pos().y();

    int xDiff = lastPressX - xPress;
    int yDiff = lastPressY - yPress;
    bool isMoved = ((abs(xDiff)+abs(yDiff))>20);

    if (isPressed)
    if (isMoved)
    {
        int gesturesMlt = CONF_PARAM("gestures")=="classic"?-1:1;
    checkView();

        if (abs(xDiff*5) < abs(yDiff))
        {


            //vertical definitly
            yDiff*=gesturesMlt;
            yOffsetGesture=yDiff;
            update();

        }
        else if (abs(xDiff) > abs(yDiff*5))
        {

            //horizontal
            xDiff*=gesturesMlt;
            xOffsetGesture = xDiff;
            update();
        }
    }

}

void CenterView::mouseReleaseEvent(QMouseEvent *mEvent)
{
    double scaleCoef = AConfig::getInstance()->getScaleCoef();

    int xPress = mEvent->pos().x()  / scaleCoef;
    int yPress = mEvent->pos().y() / scaleCoef;

    int xDiff = lastPressX - xPress;
    int yDiff = lastPressY - yPress;

    bool isMoved = ((abs(xDiff)+abs(yDiff))>20);

    int gesturesMlt = CONF_PARAM("gestures")=="classic"?-1:1;
checkView();

    xOffsetGesture=0;
    yOffsetGesture=0;
    isPressed = false;
    if (ownChild)
        ownChild->flushPressed();

    if ((abs(xDiff*5) < abs(yDiff)) && (isMoved))
    {
        //vertical definitly
        yDiff*=gesturesMlt;
        if (getChild())
         getChild()->ongesture(yDiff,false);


    }
    else if ((abs(xDiff) > abs(yDiff*5)) && (isMoved))
    {
        //horizontal
        xDiff*=gesturesMlt;
        if ((abs(xDiff)+abs(yDiff))>20)
        if (getChild())
       getChild()->ongesture(xDiff,true);

    }
    else
    {
        //some other
        //press
        if (getChild())
        {
            if (ownChild==0)
            {
                xPress *= 2;
                yPress *= 2;
            }
         getChild()->onclick(xPress,yPress);
        }

    }
checkView();
    update();
}

bool CenterView::gestureEvent(QGestureEvent *event)
{
    stringExtended  sX;



    //if (QGesture *swipe = event->gesture(Qt::SwipeGesture))
    //if (QGesture *tap = event->gesture(Qt::TapGesture))
    //if (QGesture *tapNhold = event->gesture(Qt::TapAndHoldGesture))
    //if (QGesture *pan = event->gesture(Qt::PanGesture))


    return true;
}

void CenterView::resizeEvent(QResizeEvent* event)
{
    if (ownChild)
    {
        int height = event->size().height();
        int width = event->size().width();
        ownChild->setGeometry(0,35*height/100,width,65*height/100);
    }

    if (welcomeText!=0)
    {
        welcomeText->setGeometry(0,0,event->size().width(),event->size().height());
    }

    if (confEdit!=0)
        fitTextBrowser();

    GView *gview = getFirstChild();
    MainView *mv = dynamic_cast<MainView*>(gview);

    if (mv)
    {
        gview = mv->getCurrenView();

        if (ownChild)
        if (gview)
        gview->setUI();
        //possible workaround
    }

}

void CenterView::onclick(int x1, int y1)
{
    checkView();


    if (getChild())
    {
        if (ownChild==0)
        {
            x1 *= 2;
            y1 *= 2;
        }

        getChild()->onclick(x1,y1);
    }

    checkView();
}

void CenterView::ondblclick(int x1, int y1)
{
    if (ownChild==0)
    {
        x1 *= 2;
        y1 *= 2;
    }

    checkView();
    if (getChild())
        getChild()->ondblclick(x1,y1);
    checkView();
    if (ownChild==0)
    {

        int curVType = getCurrentViewType();
        if (curVType==2) //opens on tab view
            {//is a child

            MainView *mw = getFirstChild();
            TrackView *trackView = mw->getCurrenView();

                char track[2]={0};
                char currentTrack = trackView->getPa()->getCurTrack();
                //if (tabV->tracksView.size())
                {
                track[0]=49+currentTrack;
                statusSetter->pushForceKey(track);
                }

               // trackView->ondblclick(0,0);
            }

    }
    checkView();
}

void CenterView::ongesture(int offset, bool horizontal)
{
    checkView();
    if (getChild())
        getChild()->ongesture(offset,horizontal);
    checkView();
}

void CenterView::showHelp()
{
    if (welcomeText==0)
    {
        welcomeText = new QTextBrowser(this);

        QFile helpFile;
        helpFile.setFileName(":/info/help.txt");
        if (helpFile.open(QIODevice::ReadOnly|QIODevice::Text) == false)
            std::cout << "failed to open help file";

        QString wholeFile;
        while (helpFile.atEnd()==false)
        {
            wholeFile += helpFile.readLine();
        }

        QStringList pathsBrowse;
        pathsBrowse << ":/info/"; //getTestsLocation();

        welcomeText->setText(wholeFile);
        welcomeText->setSearchPaths(pathsBrowse);

        welcomeText->setGeometry(0,60,geometry().width(),geometry().height()-60);
        welcomeText->show();
    }
}

void CenterView::showConf()
{
    if (confEdit==0)
    {
        confEdit = new QTextEdit(this);
        confEdit->show();

        stringExtended sX;

        int i = 0;
        QString qs;

        for (std::map<std::string,std::string>::iterator it = AConfig::getInstance()->values.begin();
             it!= AConfig::getInstance()->values.end(); ++it)
        {
            sX << (*it).first.c_str() <<" = "<<(*it).second.c_str()<<"\n";
            qs += sX.c_str();
            //++i;
            sX.clear();
        }

        confEdit->insertPlainText(qs);

        fitTextBrowser();
    }
}

void CenterView::fitTextBrowser()
{
    int toolShift = getToolBarHeight();

    if (welcomeText)
    welcomeText->setGeometry(0,toolShift,geometry().width(),geometry().height()-toolShift);

    if(confEdit)
    confEdit->setGeometry(0,toolShift,geometry().width(),geometry().height()-toolShift);
}


void CenterView::threadFinished()
{
    if (isPlaylistHere())
    {
        goOnPlaylist();
    }
}
