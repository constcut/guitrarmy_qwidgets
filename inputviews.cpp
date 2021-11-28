#include "inputviews.h"

#include "tabviews.h"
#include "g0/aclipboard.h"
#include "g0/midifile.h"

#include <QInputDialog>

#include "g0/wavefile.h"
#include "g0/fft.h"

#include "g0/waveanalys.h"

#include <QAudioRecorder>
#include <QAudioDeviceInfo>

#include <QMessageBox>
#include <QDir>

#include "g0/rec.h"

#include <fstream>


#include <QDebug>

//TODO



//for record - clean later
#ifdef WIN32
#include <windows.h>
#endif


#include "mainviews.h"
//dont like it..
#include "mainwindow.h"

#include "midiengine.h"



//TUNER=====================

TunerInstance* TunerInstance::mainInst = 0;

//
void changeColor(const std::string& color, QPainter* src);
//

TunerInstance *TunerInstance::getInst()
{
    return mainInst;
}

TunerInstance::TunerInstance()
{
    if (mainInst == 0)
        mainInst = this;
}

void TunerInstance::setFreq(double newFreq)
{
    freq = newFreq;

    MasterView *master = (MasterView*)masterRepainter;
    GLabel *lab = (GLabel*)label;

    lab->setText(std::to_string(freq));

    master->pleaseRepaint();
}

double TunerInstance::getFreq()
{
    return freq;
}

void TunerInstance::setViews(void *mast, void *lab)
{
    masterRepainter = mast;
    label = lab;
}

///=========================

void PatternInput::draw(QPainter *painter)
{
    repeatLabel->draw(painter);

    for (size_t i = 0; i < checkButtons.size(); ++i)
        checkButtons[i].draw(painter);



    sigDenBut->draw(painter);     
    sigNumBut->draw(painter);
    bpmBut->draw(painter);


    bpmValue->draw(painter);

    if (barView)
        barView->draw(painter);

    butRepeat->draw(painter);
}


void PatternInput::createBar()
{

    if (bar)
        delete bar;
    if (barView)
        delete barView;

    bar = new Bar;

    bar->flush();

    int num = atoi(sigNumBut->getText().c_str());
    int den = atoi(sigDenBut->getText().c_str());

    bar->setSignDenum(den);
    bar->setSignNum(num);

    for (size_t i = 0; i < (checkButtons.size()/4); ++i)
    {
        Beat *beat=new Beat();

        beat->setDuration(currentDen);
        beat->setDotted(0);
        beat->setDurationDetail(0);
        beat->setPause(false);

        bool oneChecked = false;

        for (size_t iCoef = 0; iCoef < 4; ++iCoef)
        {
            size_t ind = i + iCoef*(checkButtons.size()/4);
            if (checkButtons[ind].isChecked())
            {
                Note *newNote=new Note();

                std::uint8_t localFret = atoi(lineInstrLabels[iCoef].getText().c_str());


                if (getMaster())
                    localFret = getMaster()->getComboBoxValue(iCoef)+27;


                /*
                switch (iCoef)
                {
                    case 0: localFret = 57; break;
                    case 1: localFret = 49; break;
                    case 2: localFret = 38; break;
                    case 3: localFret = 36; break;
                }
                */


                newNote->setFret(localFret); //0
                newNote->setStringNumber(iCoef+1);
                newNote->setState(0);
                //newNote.setVolume();
                beat->push_back(newNote);
                oneChecked = true;
            }
        }

        if (oneChecked == false)
            beat->setPause(true);

        bar->push_back(beat);
    }



    barView = new BarView(bar,6);
    barView->setShifts(250,300-55-30);


    AClipboard::current()->setPtr(bar);
    AClipboard::current()->setType(4); //ptr

}

void PatternInput::playBar() {
    //createBar();

    Tab patternTab;
    Track *patternTrack=new Track();
    patternTrack->setParent(&patternTab);
    patternTrack->setDrums(true);
    patternTrack->setInstrument(0);
    patternTrack->setVolume(15);
    patternTrack->setPan(8);

    AClipboard::current()->setPtr(bar);
    AClipboard::current()->setType(4); //ptr

    if (butRepeat->isChecked())
    {
        bar->setRepeat(1);
        bar->setRepeat(2,10);
    }

    patternTrack->push_back(bar);

    patternTrack->tuning.setStringsAmount(6);

    for (int ii=0; ii < 7; ++ii)
        patternTrack->tuning.setTune(ii,0);

    patternTab.push_back(patternTrack);

    int patternBPM = atoi(bpmValue->getText().c_str());
    patternTab.setBPM(patternBPM);
    patternTab.connectTracks();



    MidiEngine::closeDefaultFile();
    std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");

    std::ofstream outFile(fullOutName);
    MidiFile outMidi;

    outMidi.fromTab(&patternTab);
    outMidi.writeStream(outFile);
    outFile.close();

    outMidi.printToStream(std::cout);

    MidiEngine::openDefaultFile();
    MidiEngine::startDefaultFile();

    bar->setParent(0);

    patternTrack->remove(0);
}

void PatternInput::setUI()
{
    for (size_t i = 0; i < lineInstrLabels.size(); ++i)
    {
        //old style
      //  lineInstrLabels[i].draw(painter);


        int shY = 100-55+50*i;
        int shX = 30;

        if (getMaster())
        getMaster()->setComboBox(i,"drums",shX-25,shY-40,70,30,-1);
    }



    //make set, as with buttons?
    //if (getMaster())
    //getMaster()->setComboBox(lines,"drums",shX-25,shY-40,70,30,-1);

    if (getMaster())
    {
       int i = lineInstrLabels.size();
       getMaster()->SetButton(i,sigDenBut->getText(),sigDenBut->getX(),
                              sigDenBut->getY(),sigDenBut->getW(),sigDenBut->getH(),"newDen");
       ++i;
       getMaster()->SetButton(i,sigNumBut->getText(),sigNumBut->getX(),
                              sigNumBut->getY(),sigNumBut->getW(),sigNumBut->getH(),"newNum");
       ++i;
       getMaster()->SetButton(i,bpmBut->getText(),bpmBut->getX(),
                              bpmBut->getY(),bpmBut->getW(),bpmBut->getH(),"newBpm");

       ++i;
       getMaster()->SetButton(i,"play",bpmBut->getX()+180,
                              bpmBut->getY(),30,20,CONF_PARAM("TrackView.playMidi"));


    }
}

void PatternInput::keyevent(std::string press)
{
    if (press==CONF_PARAM("TrackView.playMidi"))
    {
        if (bar)
        {
            playBar();
        }
    }

    if (press=="newBpm")
    {
        bool ok=false;
        int newBpm = QInputDialog::getInt(0,"Input",
                             "New bpm:",20,
                             40,255,1,&ok);

        if (ok)
            bpmValue->setText(std::to_string(newBpm));
    }

    if (press=="newNum")
    {
        int maxShX = 0;

        bool ok=false;
        int newNum = QInputDialog::getInt(0,"Input",
                             "New amount of beats:",20,
                             1,256,1,&ok);
        if (ok)
        {
            lineInstrLabels.clear();
            checkButtons.clear();

            int shX = 30;
            int shY = 100-55;

            for (int lines=0; lines < 4; ++lines)
            {
                shY = 100-55+50*lines;
                shX = 30;

                std::string instrText;
                switch(lines)
                {
                    case 0: instrText = "57"; break;
                    case 1: instrText = "49"; break;
                    case 2: instrText = "38"; break;
                    case 3: instrText = "36"; break;
                }

                GLabel lineLab(shX,shY,instrText);
                lineInstrLabels.push_back(lineLab);

                shX += 70;

                for (int i = 0; i < newNum; ++i)
                {
                    GCheckButton but(shX,shY-30,30,30);
                    shX += but.getW() + 10;
                    checkButtons.push_back(but);

                    if (shX > maxShX)
                        maxShX = shX;
                }
            }

            sigNumBut->setText(std::to_string(newNum));
            getMaster()->requestWidth(maxShX + 10);

        }
    }

    if (press=="newDen")
    {

        bool ok=false;
        QStringList items;
        items.push_back("1");
        items.push_back("2");
        items.push_back("4");
        items.push_back("8");
        items.push_back("16");
        items.push_back("32");
        QString resp = QInputDialog::getItem(0,"Input ",
                                     "denumeration",items,0,false,&ok);

        if (ok)
        {
            int listValue = -1;

            for (int i = 0; i < items.size(); ++i)
            {
                if (items.at(i)==resp)
                {
                    listValue = i;
                  break;
                }
            }

            if (listValue >=0)
                currentDen = listValue;
        }

       std::string stdResp = resp.toStdString();
       sigDenBut->setText(stdResp);
    }
}

void PatternInput::onclick(int x1, int y1)
{
    if (butRepeat->hit(x1,y1))
    {
        butRepeat->onclick(x1,y1);
        if (butRepeat->isChecked()==false)
        {
            MidiEngine::stopDefaultFile();
        }
    }

    if (sigNumBut->hit(x1,y1))
    {
        getMaster()->pushForceKey("newNum");
    }

    if (sigDenBut->hit(x1,y1))
    {
        //16 is default
        getMaster()->pushForceKey("newDen");
    }

    for (size_t i = 0; i < checkButtons.size(); ++i)
       if  (checkButtons[i].hit(x1,y1))
       {
           checkButtons[i].onclick(x1,y1);
           createBar();
           break;
       }

    for (size_t i = 0; i < lineInstrLabels.size(); ++i)
       if  (lineInstrLabels[i].hit(x1,y1))
       {
           /*now comboboxes
           bool ok=false;

           int newInstr = QInputDialog::getInt(0,"Input",
                                "Midi Drum:", 40,
                                35,127,1,&ok);
           if (ok)
           {
               sX<<newInstr;
               lineInstrLabels[i].setText(sX.c_str());
           }
           */
           createBar();
       }

    if (bpmBut->hit(x1,y1))
    {
        getMaster()->pushForceKey("newBpm");
    }

}

void TapRyView::measureTime()
{
    static bool firstDraw = true;
    if (firstDraw)
    {
        //CUT TO Tap
        qDebug() << "Tap first draw";
        clock_t beforMes = getTime(); //over here? /(CLOCKS_PER_SEC/1000)
        QThread::msleep(100); //away from usleep 1000*
        clock_t afterMes = getTime();
        int waited = (afterMes-beforMes);
        qDebug() <<"Waited calculation: "<<waited;
        if (waited >= 100)
        {
            //ok
            double theScale = 100.0/waited;
            //AConfig::getInstance()->setTimeCoef(1);
            AConfig::getInstance()->setTimeCoef(theScale);

            qDebug() <<"itsays time coef to "<<theScale;
        }
        else
        {
            //scale
           double theScale = 100.0/waited;
           //int scale = theScale;
           AConfig::getInstance()->setTimeCoef(theScale);

           qDebug() <<"itsays time coef to "<<theScale;
        }
        //CUT TO Tap
        firstDraw = false;
    }
}

void TapRyView::setUI()
{


    if (getMaster())
    {

        int fullW = getMaster()->getWidth();
        int fullH = getMaster()->getHeight();

        leftPress.setX(0); leftPress.setY(fullH-200);
        leftPress.setW(200); leftPress.setH(200);

        rightPress.setX(fullW-200); rightPress.setY(fullH-200);
        rightPress.setW(200); rightPress.setH(200);

        getMaster()->setComboBox(0,"drums",leftPress.getX()+10,leftPress.getY()+10,150,30,-1);
        getMaster()->setComboBox(1,"drums",rightPress.getX()+10,rightPress.getY()+10,150,30,-1);

        getMaster()->SetButton(2,labClean,"clean tap");
        getMaster()->SetButton(3,stopMetr,"stop metr");
        getMaster()->SetButton(4,bpmLabel,"newBpm");
        getMaster()->SetButton(5,labA,"start metr");

        getMaster()->SetButton(6,"play",600,labClean->getY(),50,20,"ent");
    }


}

void TapRyView::draw(QPainter *painter)
{
    static std::uint8_t firstDraw = 1;

    int fullW = getMaster()->getWidth();
    int fullH = getMaster()->getHeight();


    leftPress.setX(0); leftPress.setY(fullH-200);
    leftPress.setW(200); leftPress.setH(200);
    rightPress.setX(fullW-200); rightPress.setY(fullH-200);
    rightPress.setW(200); rightPress.setH(200);


    rightPress.draw(painter);
    leftPress.draw(painter);



    labA->draw(painter);
    labB->draw(painter);
    labStat->draw(painter);
    labExp->draw(painter);

    labClean->draw(painter);
    stopMetr->draw(painter);

    //bpmLabel->setY(bpmLabel->getY() + 50);
    //bpmLabel->setBorder(false);

    bpmLabel->draw(painter);


    if (barView)
        barView->draw(painter);
}


//THIS function had to be optimized - refact
void findClosestRhythm(short absValue, std::uint8_t &durGet, std::uint8_t &detGet, std::uint8_t &dotGet, int thatBPM)
{
   short pabsValue = absValue;

   short pabsNet[7] = {4000, 2000, 1000, 500, 250, 125, 62};

   short wideNet[13] = {500, 545, 571, 600, 615, 666, 727, 750, 800, 857, 888, 923,  1000};
   //                   l    ld11 h7   ld5  h13  h3   h11  ld    h5   hd7  h9   hd13    h

   // lo = 1; hi=0; then dot(easiear to write); then 4b trum;
   // 1 0 0;  1 1 11;  0 0 7;  1 1 5;  0 0 13;  0 0 3; 0 0 11; 0 0 5; 0 1 7;  0 0 9; 0 1 13; 0 0 0;

   std::uint8_t widePrepare[13][3] = { {1,0,0}, {1,1,11}, {0,0,7}, {1,1,5}, {0,0,13}, {0,0,3}, {0,0,11}, {1,1,0},
                               {0,0,5}, {0,1,7}, {0,0,9}, {0,1,13}, {0,0,0}};

   //if (pabsValue > 4000)
   //attention + check all the values for whole note with dot

   if (thatBPM!=120)
   {
       short base = (240000/thatBPM)*2;;

       for (int i = 0; i < 7; ++i)
       {
           pabsNet[i] = base;
           base /= 2;
       }
   }

   short hiInd = 5; //high
   short loInd = 6; //low

  // log << "Find closest "<<pabsValue;

   for (short i = 0; i < 7; ++i)
   {
       if (pabsNet[i] == pabsValue)
       {
           durGet = i;
           detGet=dotGet=0;
           return;
       }

       if (pabsNet[i] < pabsValue)
       {
           loInd = i;
           hiInd = i-1;
           break;
       }
   }

   durGet = 0;

   short diffLo = pabsValue - pabsNet[loInd];
   short diffHi = pabsNet[hiInd]-pabsValue;

   if (diffHi > diffLo)
       durGet = loInd;
   else
       durGet = hiInd;

   dotGet = 0;
   detGet = 0;

   return;

  // log << "Lo "<<pabsNet[loInd]<<"; Hi "<<pabsNet[hiInd];

   //Lo 3 Hi 2
   short coefPabsValue = pabsValue;

   short reLo = loInd;

   if (reLo > 3)
       while (reLo > 3)
       {
           coefPabsValue *= 2;
           --reLo;
       }

   if (reLo < 3)
        while (reLo < 3)
        {
            coefPabsValue /= 2;
            ++reLo;
        }

  // log <<"coefPabsValue= "<<coefPabsValue<<"; coefLoInd= "<<reLo;

   short closestInd  = 0;
   short closestDiff = 10000;

   //there could be special optimization for the search
   for (short i = 0; i < 13; ++i)
   {
        if (wideNet[i] == coefPabsValue)
        {
            durGet = widePrepare[i][0] + hiInd;
            dotGet = widePrepare[i][1];
            detGet = widePrepare[i][2];
            return;
        }
        else
        {
            short diff = wideNet[i] > coefPabsValue ? wideNet[i] - coefPabsValue : coefPabsValue - wideNet[i];
            if (diff < closestDiff)
            {
                closestDiff = diff;
                closestInd = i;
            }
        }
   }

   //log <<"Closest difference= "<<closestDiff;

   durGet = widePrepare[closestInd][0] + hiInd;
   dotGet = widePrepare[closestInd][1];
   detGet = widePrepare[closestInd][2];
}


void TapRyView::createBar()
{
    //create tab values push them into stat

    if (presses.size() < 2)
        return; //avoid crah

    int thatBPM = atoi(bpmLabel->getText().c_str());


    if (ryBar) delete ryBar;


    ryBar = new Bar();
    ryBar->flush();

    ryBar->setSignDenum(4); ryBar->setSignNum(8);

    std::uint8_t lastDur = 0;

    int lastPressInstr = 0;

    for (size_t i = 1; i < presses.size(); ++i)
    {
        int step = presses[i].first-presses[i-1].first;
        std::uint8_t dur=0,det=0,dot=0;
        findClosestRhythm(step,dur,det,dot,thatBPM);

        --dur;
        lastDur = dur;

         //<<":"<<det<<":"<<dot<<"];";

        //find closest
        //on 120 bpm 2000, 1000, 500 etc from whole

        qDebug() << "Step "<<i <<" = "<<step<<"; dur - "<<dur;

        if (step < 0)
        {
            qDebug() << "hmm";
        }

        Beat *ryBeat=new Beat();
        ryBeat->setDuration(dur);
        ryBeat->setDotted(dot);
        ryBeat->setDurationDetail(det);
        ryBeat->setPause(0);

        Note *rNote= new Note();
        rNote->setState(0);



        if (presses[i].second == 1)
        {
            if (getMaster())
            {
                int drumInstr = getMaster()->getComboBoxValue(0);
                if (drumInstr == -1)
                    drumInstr = 36;
                else
                    drumInstr+=27;

                lastPressInstr = drumInstr;
                rNote->setFret(drumInstr );
            }
        }
        else
        {
            if (getMaster())
            {
                int drumInstr = getMaster()->getComboBoxValue(1);
                if (drumInstr == -1)
                    drumInstr = 38;
                else
                    drumInstr+=27;
                lastPressInstr = drumInstr;
                rNote->setFret(drumInstr );
            }
        }

        rNote->setStringNumber(5);
        ryBeat->push_back(rNote);

        ryBar->push_back(ryBeat);
    }

    //last note thats missing
    if (lastDur < 8)
    {
        Beat *ryBeat= new Beat();
        ryBeat->setDuration(lastDur);
        ryBeat->setDotted(0);
        ryBeat->setDurationDetail(0);
        ryBeat->setPause(0);

        Note *rNote=new Note();
        rNote->setState(0);
        rNote->setFret(lastPressInstr);
        rNote->setStringNumber(5);
        ryBeat->push_back(rNote);

        ryBar->push_back(ryBeat);
    }


    while (ryBar->getCompleteStatus()==2)
        ryBar->setSignNum(ryBar->getSignNum()+1);



    qDebug() << "tap ry done";

    //getMaster()->setStatusBarMessage(0,sX.c_str(),3000);
    //labStat->setText(sX.c_str());

    //ryBar to the bar view
    ///if (barView) delete barView;
    if (barView==0)
    {
        barView = new BarView(ryBar,6);
        barView->setShifts(210,80);
    }
        else barView->setBar(ryBar);

    AClipboard::current()->setPtr(ryBar);
    AClipboard::current()->setType(4); //ptr

}

void TapRyView::copyAndPlayBar()
{
    //createBar();

    if (ryBar==0)
        return;

    int thatBPM = atoi(bpmLabel->getText().c_str());

    //createBar();

    Bar *bar = ryBar;


    //copy paste
    Tab patternTab;
    Track *patternTrack=new Track();
    patternTrack->setParent(&patternTab);
    patternTrack->setDrums(true);
    patternTrack->setInstrument(0);
    patternTrack->setVolume(15);
    patternTrack->setPan(8);

    //bar->setRepeat(1);
    //bar->setRepeat(2,2);

    patternTrack->push_back(bar);

    patternTrack->tuning.setStringsAmount(6);

    for (int ii=0; ii < 7; ++ii)
        patternTrack->tuning.setTune(ii,0);

    patternTab.push_back(patternTrack);
    patternTab.setBPM(thatBPM);
    patternTab.connectTracks();


    MidiEngine::closeDefaultFile();
    std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");

    std::ofstream outFile(fullOutName);
    MidiFile outMidi;

    outMidi.fromTab(&patternTab);
    outMidi.writeStream(outFile);

    //outMidi.printToStream(std::cout);

    MidiEngine::openDefaultFile();
    MidiEngine::startDefaultFile();

    ryBar->setParent(0);

    patternTrack->remove(0);
    //getMaster()->pleaseRepaint();
}

void TapRyView::keyevent(std::string press)
{
    if (press==CONF_PARAM("TrackView.playMidi"))
    {
        copyAndPlayBar();
    }

    if (press == "clean tap")
    {
        //info << "Cleaned "<<(int)presses.size()<<" presses;";
        presses.clear();
        if (ryBar) delete ryBar;
            ryBar = 0;
        if (barView) delete barView;
            barView = 0;
        //labStat->setText(info.c_str());
        //getMaster()->pleaseRepaint();
    }

    if (press == "stop metr")
    {
        MidiEngine::stopDefaultFile();
        MidiEngine::closeDefaultFile();
    }

    if (press == "newBpm")
    {
        bool ok=false;

        int newBpm = QInputDialog::getInt(0,"Input",
                             "New bpm:",20,
                             40,255,1,&ok);
        if (ok)
        {
            bpmLabel->setText(std::to_string(newBpm));
            getMaster()->SetButton(4,bpmLabel,"newBpm");
        }
    }

    if (press == "start metr")
    {
        //START PSEUDO METRONOME

        if (ryBar) delete ryBar;
        if (barView) delete barView;
        barView=0;

        ryBar = new Bar();
        ryBar->flush();

        ryBar->setSignDenum(4); ryBar->setSignNum(4);

        for (int i =0; i < 4; ++i)
        {
            Beat *ryBeat= new Beat();
            ryBeat->setDuration(2);
            ryBeat->setDotted(0);
            ryBeat->setDurationDetail(0);
            ryBeat->setPause(0);

            Note *rNote=new Note();
            rNote->setState(0);

            if (i==0)
                rNote->setFret(38);
            else
                rNote->setFret(36);

            rNote->setStringNumber(5);
            ryBeat->push_back(rNote);

            ryBar->push_back(ryBeat);
        }

        Tab patternTab;
        Track *patternTrack=new Track();
        patternTrack->setParent(&patternTab);
        patternTrack->setDrums(true);
        patternTrack->setInstrument(0);
        patternTrack->setVolume(15);
        //patternTrack.setPan();

        ryBar->setRepeat(1);
        ryBar->setRepeat(2,100);

        patternTrack->push_back(ryBar);

        patternTrack->tuning.setStringsAmount(6);

        for (int ii=0; ii < 7; ++ii)
            patternTrack->tuning.setTune(ii,0);

        patternTab.push_back(patternTrack);

        int thatBPM = atoi(bpmLabel->getText().c_str());
        patternTab.setBPM(thatBPM);
        patternTab.connectTracks();


        MidiEngine::closeDefaultFile();
        std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");


        std::ofstream outFile(fullOutName);
        MidiFile outMidi;

        outMidi.fromTab(&patternTab);
        outMidi.writeStream(outFile);
        outFile.close();

        //outMidi.printToStream(std::cout);

        MidiEngine::openDefaultFile();
        MidiEngine::startDefaultFile();

        patternTrack->remove(0);

        //END OF PSEUDO METRONOME
    }
}

void TapRyView::onclick(int x1, int y1)
{
    if (stopMetr->hit(x1,y1))
    {
       getMaster()->pushForceKey("stop metr");
    }

    if (bpmLabel->hit(x1,y1))
    {
       getMaster()->pushForceKey("newBpm");
    }

    if (leftPress.hit(x1,y1)||rightPress.hit(x1,y1))
    {

        if (leftPress.hit(x1,y1))
            leftPress.pressIt();
            else
                rightPress.pressIt();

        //most important

        if (presses.size() == 0)
        {
            getTime();
        }



        clock_t timez = getTime();
        //sX<<"Click time "<<timez;
        //labStat->setText(sX.c_str());

        int timeNow= timez; //*AConfig::getInstance()->getTimeCoef();
        intPair newPair;
        newPair.first = timeNow;
        newPair.second = leftPress.hit(x1,y1) ? 1: 2;

        if (timeNow < 0)
        {
            qDebug() <<"Y";
        }
        presses.push_back(newPair);

        //add value to vector or poly
        std::string info=
        "Presses " + std::to_string( presses.size() ) + "; last- " + std::to_string(timeNow);

        qDebug() << info.c_str();

        labStat->setText(info.c_str());

        createBar();

        int midiNote = 0;

        if (leftPress.hit(x1,y1))
        {
            int drumInstr = getMaster()->getComboBoxValue(0);
            if (drumInstr == -1)
                drumInstr = 36;
            else
                drumInstr+=27;
            midiNote = drumInstr;
        }
        else
        {
            int drumInstr = getMaster()->getComboBoxValue(1);
            if (drumInstr == -1)
                drumInstr = 38;
            else
                drumInstr+=27;
            midiNote = drumInstr;
        }


        MidiEngine::sendSignalShort(0x90|9,midiNote,120);//refact velocy

        //getMaster()->pleaseRepaint();
    }

    if (labClean->hit(x1,y1))
    {
        getMaster()->pushForceKey("clean tap");
    }

    if (labA->hit(x1,y1))
    {
        getMaster()->pushForceKey("start metr");
    }

    if (labExp->hit(x1,y1))
    {
        //copyAndPlayBar();
    }

}

//

//RECORD VIEW

void RecordView::setUI()
{
    QString testsLoc = getTestsLocation();//! MOVE TO records folder
    QDir    curDir(testsLoc);

    QStringList filters;
    QString filter = "*.graw";  filters.push_back(filter); //all files yet, then refact good
    QStringList files = curDir.entryList(filters,QDir::Files | QDir::NoSymLinks);

    recFiles = files;

    std::string recordsNames="";

    for (int i = 0; i < files.size(); ++i)
    {
      recordsNames += files[i].toStdString();
      recordsNames += std::string(";");
    }

    getMaster()->setComboBox(0,recordsNames,250,5,500,30,-1); //scetches

    //set up only once
    tunerItself.setViews(getMaster(),tunerLabel);
}

void RecordView::loadCurrentFile()
{
    int zoomValue = atoi(zoom->getText().c_str());
    int bpmValue = atoi(bpm->getText().c_str());
    int eLev1Value = atoi(eLevel1->getText().c_str());
    int eLev2Value = atoi(eLevel2->getText().c_str());
    int eLev3Value = atoi(eLevel3->getText().c_str());

    wavePosition = 0;

    std::vector<int> parameters;
    parameters.push_back(zoomValue);
    parameters.push_back(bpmValue);
    parameters.push_back(eLev1Value);
    parameters.push_back(eLev2Value);
    parameters.push_back(eLev3Value);
    parameters.push_back(wavePosition);

    std::string wavePath = std::string(getTestsLocation()) + currentFile;

    waveItself.loadFile(wavePath,&parameters);

    bar = waveItself.generateBar();

    if (bar)
    {
        //need to delete old one
        ///if (barView) delete barView;
        barView = new BarView(bar,6);
        barView->setShifts(20,50);
    }
}

void RecordView::draw(QPainter *painter)
{

    zoom->draw(painter);
    bpm->draw(painter);
    eLevel1->draw(painter);
    eLevel2->draw(painter);
    eLevel3->draw(painter);

    tunerLabel->draw(painter);

    int bpmValue = atoi(bpm->getText().c_str());

    if (barView)
        barView->draw(painter);

    short *rawData = (short*)waveItself.origin.data();
    waveLimit = waveItself.origin.size()/2;

    if (rawData==0)
        return;


    int zoomCoef = atoi(zoom->getText().c_str());
    int localPosition = 0;
    int scaleShiftUp = 0;
    int liftDown = 0;
    changeColor("white", painter);
    //draw main wave scaled, shifted
    for (int i = wavePosition; i < waveLimit; )
    {
        int maxShift = 0;
        for (int coefIndex = 0; coefIndex < zoomCoef; ++coefIndex)
        {
            if ((i%1000) == 0)
            {
                painter->drawText(localPosition,200-55-20-scaleShiftUp*10+300,
                    std::to_string(i).c_str());
                ++scaleShiftUp; if (scaleShiftUp > 3)
                    scaleShiftUp = 0;
            }
            if (i >= waveLimit) break;

            int curShift = rawData[i];
            if (abs(curShift) > maxShift)
                maxShift = abs(curShift);

            if (zoomCoef == 1)
                maxShift = curShift; //normal look

            ++i;
            if (i >= waveLimit) break;
        }
        //colored no more
        int chosenShift = maxShift/(512); // max peak 65535 - 256*256
        //painter->drawLine(localPosition,200+chosenShift-60,localPosition,200+chosenShift-55);
        ++localPosition;
        if (localPosition > 2000) break; //escape outside screen
    }

    double floatBPM = bpmValue;
    int bpmDependentWindow = 8000.0/(floatBPM/60.0)/32.0;

    //draw energy levels
    int toShift = wavePosition/bpmDependentWindow;

    for (int i = toShift+1; i < waveItself.energyLevels.size(); ++i)
    {
        int lastELevel = waveItself.energyLevels[i-1];
        int currentLevel = waveItself.energyLevels[i];
        changeColor("green", painter);

        int pseudoI = i - toShift;
        painter->drawLine(bpmDependentWindow*pseudoI/zoomCoef,200+lastELevel/20,bpmDependentWindow*(pseudoI+1)/zoomCoef, 200 + currentLevel/20);

        switch (waveItself.energyTypes[i-1])
        {
            case 0:  changeColor("gray", painter); break;
            case 1:  changeColor("red", painter); break;
            case 2: changeColor("yellow", painter); break;
            case 3: changeColor("green", painter); break;
            case 4: changeColor("blue",painter); break;
        }


        int vShift = (i%5)*20;
        painter->drawText(bpmDependentWindow*pseudoI/zoomCoef,400+vShift,
            std::to_string(lastELevel).c_str());


        int defRad = 3;
        if (waveItself.energyTypes[i-1]==3) defRad=5;
        painter->drawEllipse( (bpmDependentWindow)*(pseudoI)/zoomCoef,200+lastELevel/20,defRad,defRad);
    }

    //draw notes

    changeColor("white", painter);
    for (int i = 0; i < waveItself.notes.size(); ++i)
    {
        int notePeak = waveItself.notes[i].notePeak - toShift;
        int noteEnd = waveItself.notes[i].noteEnd - toShift;
        int noteBegin = waveItself.notes[i].noteBegin - toShift;
        double freq = waveItself.notes[i].freq;

        std::string freqStr = std::to_string(freq);
        freqStr = freqStr.substr(0,5);

        int miniVShift = (i%3)*30;

        painter->drawLine(bpmDependentWindow*(notePeak+1)/zoomCoef,230+miniVShift,bpmDependentWindow*(noteEnd+1)/zoomCoef,230+miniVShift);
        painter->drawText(bpmDependentWindow*(notePeak+2)/zoomCoef,250+miniVShift,freqStr.c_str());
    }



}

void RecordView::keyevent(std::string press)
{
    if (press==CONF_PARAM("TrackView.playAMusic") || press == CONF_PARAM("TrackView.playMidi"))
    {
        std::string pushParam;

        if (playing == false)
            pushParam = "start_record_output " + currentFile;
        else
           pushParam = "stop_record_output";

        playing = !playing;

        //start playing keep a bool
        getMaster()->pushForceKey(pushParam);
    }

    if (press == "record") //used for future button of recording\stopping recording
    {
        std::string pushParam;

        if (recording == false)
            pushParam = "start_audioi";
        else
           pushParam = "stop_audioi";

        recording = !recording;

        getMaster()->pushForceKey(pushParam);
    }

    if (press.substr(0,4)=="com:")
    {
        std::string rest = press.substr(4);
        size_t separator = rest.find(":");
        std::string combo = rest.substr(0,separator);
        std::string item = rest.substr(separator+1);

        int itemNum = atoi(item.c_str());

        if (combo=="0")
        {
           currentFile = recFiles[itemNum].toStdString();
           loadCurrentFile();
        }
    }
}

void RecordView::onclick(int x1, int y1)
{

    //if (this->status->hit(x1,y1))
    { /*
        return;

        ThreadWave *waveThr = new ThreadWave();
        waveThr->setInc(&wavePosition,0);
        waveThr->setLimit((waveLimit)-1);
       //
        //getMaster()->connectThread(waveThr);

        MainWindow *mw = getMaster();
        mw->connect(
                    waveThr,
                    SIGNAL(updateUI()),
                    SLOT(update()),
                    Qt::QueuedConnection);


        waveThr->start();
        //waveThr->
        */
    }

    if (zoom->hit(x1,y1))
    {
        bool ok=false;
        int newZoom = QInputDialog::getInt(0,"Input",
                             "New zoom:",1,
                             1,1000,1,&ok);
        if (ok)
            zoom->setText(std::to_string(newZoom));
    }

    if (bpm->hit(x1,y1))
    {
        bool ok=false;
        int newBpm = QInputDialog::getInt(0,"Input",
                             "New bpm:",120,
                             1,1000,1,&ok);
        if (ok)
            bpm->setText(std::to_string(newBpm));

    }

    if (eLevel1->hit(x1,y1))
    {
        bool ok=false;
        int eLvl = QInputDialog::getInt(0,"Input",
                             "New e-level1:",500,
                             1,2500,1,&ok);
        if (ok)
            eLevel1->setText(std::to_string(eLvl));
    }


    if (eLevel3->hit(x1,y1))
    {
        bool ok=false;
        int eLvl = QInputDialog::getInt(0,"Input",
                             "New e-level3:",500,
                             1,3000,1,&ok);
        if (ok)
            eLevel3->setText(std::to_string(eLvl));
    }


    if (eLevel2->hit(x1,y1))
    {
        bool ok=false;
        int eLvl = QInputDialog::getInt(0,"Input",
                             "New e-level2:",500,
                             1,5000,1,&ok);
        if (ok)
            eLevel2->setText(std::to_string(eLvl));
    }

    //ATTENTION - other function buttons blocked
    return;

    //OLD windows only code
    //if (startBut->hit(x1,y1))
    {
        #ifdef WIN32
            mciSendStringA("open new type waveaudio alias recaudio",0,0,0);
            mciSendStringA("set recaudio bitspersample 16 samplespersec 11025 channels 1",0,0,0);

            mciSendStringA("record recaudio", 0, 0, 0);

            //status->setText("Record started");

        #else

            if (recorderPtr==0)
            {
                QAudioRecorder *recorder = new QAudioRecorder();

                QAudioEncoderSettings audioSettings;
                audioSettings.setCodec("audio/pcm");

                //audioSettings.setBitRate();
                //audioSettings.setQuality(QMultimedia::LowQuality); //checkable
                audioSettings.setSampleRate(11025);
                audioSettings.setChannelCount(1);
                //audioSettings.setEncodingMode(QMultimedia::ConstantQualityEncoding);

                QString container = "audio/x-wav";
                recorder->setEncodingSettings(audioSettings,QVideoEncoderSettings(), container);

                std::string wavePath = std::string(getTestsLocation()) + std::string("rec.wav");
                recorder->setOutputLocation(QUrl::fromLocalFile(wavePath.c_str()));

                recorderPtr = recorder;
            }

            if (recorderPtr)
            {
                QAudioRecorder *recorder  = (QAudioRecorder*)recorderPtr;
                recorder->record();

                //status->setText("Record started");


                QAudioFormat m_format;
                m_format.setSampleRate(8000);
                m_format.setChannelCount(1);
                m_format.setSampleSize(16);
                m_format.setSampleType(QAudioFormat::SignedInt);
                m_format.setByteOrder(QAudioFormat::LittleEndian);
                m_format.setCodec("audio/pcm");

                QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
                if (!info.isFormatSupported(m_format)) {

                    QMessageBox dialog("BAD","Default format not supported - trying to use nearest",
                                       QMessageBox::Information,
                                       QMessageBox::Yes,
                                       QMessageBox::No,
                                       QMessageBox::Cancel |QMessageBox::Escape);
                    //dialog.set
                    if (dialog.exec() == QMessageBox::Yes)
                        return; //escape

                    //QMessageBox:: << "Default format not supported - trying to use nearest";


                    m_format = info.nearestFormat(m_format);
                }
                else
                {

                    QMessageBox dialog("GOOD","Supported work with audioinput",
                                       QMessageBox::Information,
                                       QMessageBox::Yes,
                                       QMessageBox::No,
                                       QMessageBox::Cancel |QMessageBox::Escape);
                    //dialog.set
                    if (dialog.exec() == QMessageBox::Yes)
                        return; //escape
                }
            }

            #endif
    }

    //if (stopBut->hit(x1,y1))
    {

        #ifdef WIN32
            mciSendStringA("stop rec", 0, 0, 0);
            std::string command = "save recaudio \"" + getInvertedLocation() + "rec.wav\"";
            mciSendStringA(command.c_str(), 0, 0, 0);
            mciSendStringA("close rec", 0, 0, 0);

            //status->setText("Record finished");

        #else
            if (recorderPtr)
            {
                QAudioRecorder *recorder  = (QAudioRecorder*)recorderPtr;
                recorder->stop();

                //status->setText("Record finished");
            }
        #endif
    }
}



void MorzeInput::draw(QPainter *painter)
{
    createBut->draw(painter);

    if (barView)
        barView->draw(painter);
}

//REFACT NOTES - GROUP input metods into GInputView

void MorzeInput::playBar()
{
    if (bar)
    {
        Tab morzeTab;
        Track *morzeTrack=new Track();
        morzeTrack->setParent(&morzeTab);
        morzeTrack->setDrums(true);
        morzeTrack->setInstrument(0);
        morzeTrack->setVolume(15);
        morzeTrack->setPan(8);

        morzeTrack->push_back(bar);

        morzeTrack->tuning.setStringsAmount(6);

        for (int ii=0; ii < 7; ++ii)
            morzeTrack->tuning.setTune(ii,0);

        morzeTab.push_back(morzeTrack);
        morzeTab.setBPM(120);
        morzeTab.connectTracks();


        MidiEngine::closeDefaultFile();
        std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");

        std::ofstream outFile(fullOutName);
        MidiFile outMidi;

        outMidi.fromTab(&morzeTab);
        outMidi.writeStream(outFile);
        outFile.close();

        outMidi.printToStream(std::cout);

        MidiEngine::openDefaultFile();
        MidiEngine::startDefaultFile();

        morzeTrack->remove(0);
    }
}

void MorzeInput::keyevent(std::string keypress)
{
    if ( (keypress == CONF_PARAM("TrackView.playAMusic"))
        || (keypress == CONF_PARAM("TrackView.playMidi")) )
    {
        playBar();
    }
}

void MorzeInput::setUI()
{
    if (getMaster())
    {
        getMaster()->SetButton(0,"play",20,20,50,20,CONF_PARAM("TrackView.playMidi"));
    }
}

void MorzeInput::onclick(int x1, int y1)
{
    if (createBut->hit(x1,y1))
    {
        //INPUT TEXT

        bool ok=false;
        QString inputedText = QInputDialog::getText(0,"Input","Text:",QLineEdit::Normal,"simpletest",&ok);

        if (ok)
        {
            if (bar)
                delete bar;
            if (barView)
                delete barView;


            bar = new Bar;

            bar->flush();
            bar->setSignDenum(4); bar->setSignNum(7);

            //MORZE operations

            std::string stdMorze = inputedText.toStdString();

            for (size_t i=0; i <stdMorze.size(); ++i)
            {
                std::uint8_t dur=0,det=0,dot=0;
                std::uint8_t dur2=0;
                char currentChar = stdMorze[i];

                if (currentChar%2)
                    dur = 3;
                else
                    dur = 4;

                if (currentChar%3)
                    dur2 = 3;
                else
                    dur2 = 4;

                Beat *ryBeat=new Beat();
                ryBeat->setDuration(dur);
                ryBeat->setDotted(dot);
                ryBeat->setDurationDetail(det);
                ryBeat->setPause(0);

                Beat *ryBeat2=new Beat();
                ryBeat2->setDuration(dur2);
                ryBeat2->setDotted(dot);
                ryBeat2->setDurationDetail(det);
                ryBeat2->setPause(0);

                Note *rNote=new Note();
                rNote->setState(0);
                rNote->setFret(36);
                rNote->setStringNumber(5);

                Note *rNote2=new Note();
                rNote2->setState(0);
                rNote2->setFret(38);
                rNote2->setStringNumber(5);

                ryBeat->push_back(rNote);
                ryBeat2->push_back(rNote2);

                bar->push_back(ryBeat); //����� �
                bar->push_back(ryBeat2);

                qDebug() << "Added "<<dur<<" "<<dur2<<" beat";
            }


            barView = new BarView(bar,6);
            barView->setShifts(100,200-55);



            AClipboard::current()->setPtr(bar);
            AClipboard::current()->setType(4); //ptr

            bar->setRepeat(1);
            bar->setRepeat(2,2);

            playBar();

            //does it cleans?
        }

    }
}



