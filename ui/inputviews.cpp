#include "inputviews.h"

#include "tabviews.h"
#include "tab/TabClipboard.hpp"
#include "midi/MidiFile.hpp"

#include <QInputDialog>

#include "midi/MidiExport.hpp"

#include <QAudioRecorder>
#include <QAudioDeviceInfo>

#include <QMessageBox>
#include <QDir>

#include <fstream>


#include <QDebug>



#ifdef WIN32
#include <windows.h>
#endif


#include "mainviews.h"
#include "mainwindow.h"

#include "midi/MidiEngine.hpp"



//
void changeColor(const std::string& color, QPainter* src);
//

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
    bar = std::make_unique<Bar>();
    bar->flush();

    int num = atoi(sigNumBut->getText().c_str());
    int den = atoi(sigDenBut->getText().c_str());

    bar->setSignDenum(den);
    bar->setSignNum(num);

    for (size_t i = 0; i < (checkButtons.size()/4); ++i) {
        auto beat = std::make_unique<Beat>();
        beat->setDuration(currentDen);
        beat->setDotted(0);
        beat->setDurationDetail(0);
        beat->setPause(false);
        bool oneChecked = false;
        for (size_t iCoef = 0; iCoef < 4; ++iCoef) {
            size_t ind = i + iCoef*(checkButtons.size()/4);
            if (checkButtons[ind].isChecked()){
                auto newNote = std::make_unique<Note>();
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
                beat->push_back(std::move(newNote));
                oneChecked = true;
            }
        }

        if (oneChecked == false)
            beat->setPause(true);

        bar->push_back(std::move(beat));
    }

    barView = std::make_unique<BarView>(bar.get(), 6);
    barView->setShifts(250,300-55-30);

    AClipboard::current()->setPtr(bar.get());
    AClipboard::current()->setClipboardType(ClipboardType::BarPointer); //ptr

}

void PatternInput::playBar() {
    //createBar();

    Tab patternTab;
    auto patternTrack = std::make_unique<Track>();
    patternTrack->setParent(&patternTab);
    patternTrack->setDrums(true);
    patternTrack->setInstrument(0);
    patternTrack->setVolume(15);
    patternTrack->setPan(8);

    AClipboard::current()->setPtr(bar.get());
    AClipboard::current()->setClipboardType(ClipboardType::BarPointer); //ptr

    if (butRepeat->isChecked())
    {
        bar->setRepeat(1);
        bar->setRepeat(2,10);
    }

    patternTrack->push_back(std::move(bar));

    patternTrack->tuning.setStringsAmount(6);

    for (int ii=0; ii < 7; ++ii)
        patternTrack->tuning.setTune(ii,0);

    patternTab.push_back(std::move(patternTrack));

    int patternBPM = atoi(bpmValue->getText().c_str());
    patternTab.setBPM(patternBPM);
    patternTab.connectTracks();



    MidiEngine::closeDefaultFile();
    std::string fullOutName = AConfig::getInst().testsLocation + std::string("midiOutput.mid");

    std::ofstream outFile(fullOutName);
    auto outMidi = exportMidi(&patternTab);
    outMidi->writeStream(outFile);
    outFile.close();
    outMidi->printToStream(std::cout);

    MidiEngine::openDefaultFile();
    MidiEngine::startDefaultFile();

    bar->setParent(0);

    patternTrack->remove(0);
}

void PatternInput::setUI()
{
    for (size_t i = 0; i < lineInstrLabels.size(); ++i) {
        int shY = 100-55+50*i;
        int shX = 30;
        if (getMaster())
        getMaster()->setComboBox(i,"drums",shX-25,shY-40,70,30,-1);
    }

    if (getMaster()) {
       size_t i = lineInstrLabels.size();
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
        size_t newNum = QInputDialog::getInt(0,"Input",
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
                lineInstrLabels.push_back(std::move(lineLab));

                shX += 70;

                for (size_t i = 0; i < newNum; ++i)
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

            for (size_t i = 0; i < items.size(); ++i)
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
            //AConfig::getInstance().setTimeCoef(1);
            AConfig::getInst().setTimeCoef(theScale);

            qDebug() <<"itsays time coef to "<<theScale;
        }
        else
        {
            //scale
           double theScale = 100.0/waited;
           //int scale = theScale;
           AConfig::getInst().setTimeCoef(theScale);

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

        getMaster()->SetButton(2,labClean.get(),"clean tap");
        getMaster()->SetButton(3,stopMetr.get(),"stop metr");
        getMaster()->SetButton(4,bpmLabel.get(),"newBpm");
        getMaster()->SetButton(5,labA.get(),"start metr");

        getMaster()->SetButton(6,"play",600,labClean->getY(),50,20,"ent");
    }


}

void TapRyView::draw(QPainter *painter)
{
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

       for (size_t i = 0; i < 7; ++i)
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


    if (ryBar)
        ryBar = nullptr;


    ryBar = std::make_unique<Bar>();
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

        auto ryBeat= std::make_unique<Beat>();
        ryBeat->setDuration(dur);
        ryBeat->setDotted(dot);
        ryBeat->setDurationDetail(det);
        ryBeat->setPause(0);

        auto rNote= std::make_unique<Note>();
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
        ryBeat->push_back(std::move(rNote));

        ryBar->push_back(std::move(ryBeat));
    }

    //last note thats missing
    if (lastDur < 8)
    {
        auto ryBeat= std::make_unique<Beat>();
        ryBeat->setDuration(lastDur);
        ryBeat->setDotted(0);
        ryBeat->setDurationDetail(0);
        ryBeat->setPause(0);

        auto rNote = std::make_unique<Note>();
        rNote->setState(0);
        rNote->setFret(lastPressInstr);
        rNote->setStringNumber(5);
        ryBeat->push_back(std::move(rNote));

        ryBar->push_back(std::move(ryBeat));
    }


    while (ryBar->getCompleteStatus()==2)
        ryBar->setSignNum(ryBar->getSignNum()+1);



    qDebug() << "tap ry done";

    //getMaster()->setStatusBarMessage(0,sX.c_str(),3000);
    //labStat->setText(sX.c_str());


    if (barView == nullptr) {
        barView = std::make_unique<BarView>(ryBar.get(),6);
        barView->setShifts(210,80);
    }
        else barView->setBar(ryBar.get());

    AClipboard::current()->setPtr(ryBar.get());
    AClipboard::current()->setClipboardType(ClipboardType::BarPointer); //ptr

}

void TapRyView::copyAndPlayBar()
{
    //createBar();

    if (ryBar==0)
        return;

    int thatBPM = atoi(bpmLabel->getText().c_str());

    //createBar();



    //copy paste
    Tab patternTab;
    auto patternTrack= std::make_unique<Track>();
    patternTrack->setParent(&patternTab);
    patternTrack->setDrums(true);
    patternTrack->setInstrument(0);
    patternTrack->setVolume(15);
    patternTrack->setPan(8);

    patternTrack->push_back(std::move(ryBar));
    patternTrack->tuning.setStringsAmount(6);

    for (int ii=0; ii < 7; ++ii)
        patternTrack->tuning.setTune(ii,0);

    patternTab.push_back(std::move(patternTrack));
    patternTab.setBPM(thatBPM);
    patternTab.connectTracks();


    MidiEngine::closeDefaultFile();
    std::string fullOutName = AConfig::getInst().testsLocation + std::string("midiOutput.mid");

    std::ofstream outFile(fullOutName);
    auto outMidi = exportMidi(&patternTab);

    outMidi->writeStream(outFile);

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
        presses.clear();
        ryBar = nullptr;
        barView = nullptr;
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
            getMaster()->SetButton(4,bpmLabel.get(),"newBpm");
        }
    }

    if (press == "start metr")
    {
        //START PSEUDO METRONOME
        ryBar = nullptr;
        barView = nullptr;

        ryBar = std::make_unique<Bar>();
        ryBar->flush();

        ryBar->setSignDenum(4); ryBar->setSignNum(4);

        for (size_t i =0; i < 4; ++i)
        {
            auto ryBeat= std::make_unique<Beat>();
            ryBeat->setDuration(2);
            ryBeat->setDotted(0);
            ryBeat->setDurationDetail(0);
            ryBeat->setPause(0);

            auto rNote = std::make_unique<Note>();
            rNote->setState(0);

            if (i==0)
                rNote->setFret(38);
            else
                rNote->setFret(36);

            rNote->setStringNumber(5);
            ryBeat->push_back(std::move(rNote));

            ryBar->push_back(std::move(ryBeat));
        }

        Tab patternTab;
        auto patternTrack= std::make_unique<Track>();
        patternTrack->setParent(&patternTab);
        patternTrack->setDrums(true);
        patternTrack->setInstrument(0);
        patternTrack->setVolume(15);
        //patternTrack.setPan();

        ryBar->setRepeat(1);
        ryBar->setRepeat(2,100);

        patternTrack->push_back(std::move(ryBar));

        patternTrack->tuning.setStringsAmount(6);

        for (int ii=0; ii < 7; ++ii)
            patternTrack->tuning.setTune(ii,0);

        patternTab.push_back(std::move(patternTrack));

        int thatBPM = atoi(bpmLabel->getText().c_str());
        patternTab.setBPM(thatBPM);
        patternTab.connectTracks();


        MidiEngine::closeDefaultFile();
        std::string fullOutName = AConfig::getInst().testsLocation + std::string("midiOutput.mid");


        std::ofstream outFile(fullOutName);
        auto outMidi = exportMidi(&patternTab);
        outMidi->writeStream(outFile);
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

        int timeNow= timez; //*AConfig::getInstance().getTimeCoef();
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
        auto morzeTrack= std::make_unique<Track>();
        morzeTrack->setParent(&morzeTab);
        morzeTrack->setDrums(true);
        morzeTrack->setInstrument(0);
        morzeTrack->setVolume(15);
        morzeTrack->setPan(8);

        morzeTrack->push_back(std::move(bar));

        morzeTrack->tuning.setStringsAmount(6);

        for (int ii=0; ii < 7; ++ii)
            morzeTrack->tuning.setTune(ii,0);

        morzeTab.push_back(std::move(morzeTrack));
        morzeTab.setBPM(120);
        morzeTab.connectTracks();


        MidiEngine::closeDefaultFile();
        std::string fullOutName = AConfig::getInst().testsLocation + std::string("midiOutput.mid");

        std::ofstream outFile(fullOutName);
        auto outMidi = exportMidi(&morzeTab);

        outMidi->writeStream(outFile);
        outFile.close();
        outMidi->printToStream(std::cout);

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
            bar = std::make_unique<Bar>();

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

                auto ryBeat = std::make_unique<Beat>();
                ryBeat->setDuration(dur);
                ryBeat->setDotted(dot);
                ryBeat->setDurationDetail(det);
                ryBeat->setPause(0);

                auto ryBeat2 = std::make_unique<Beat>();
                ryBeat2->setDuration(dur2);
                ryBeat2->setDotted(dot);
                ryBeat2->setDurationDetail(det);
                ryBeat2->setPause(0);

                auto rNote = std::make_unique<Note>();
                rNote->setState(0);
                rNote->setFret(36);
                rNote->setStringNumber(5);

                auto rNote2 = std::make_unique<Note>();
                rNote2->setState(0);
                rNote2->setFret(38);
                rNote2->setStringNumber(5);

                ryBeat->push_back(std::move(rNote));
                ryBeat2->push_back(std::move(rNote2));

                bar->push_back(std::move(ryBeat)); //����� �
                bar->push_back(std::move(ryBeat2));

                qDebug() << "Added "<<dur<<" "<<dur2<<" beat";
            }


            barView = std::make_unique<BarView>(bar.get(),6);
            barView->setShifts(100,200-55);

            AClipboard::current()->setPtr(bar.get());
            AClipboard::current()->setClipboardType(ClipboardType::BarPointer); //ptr

            bar->setRepeat(1);
            bar->setRepeat(2, 2);

            playBar();
        }
    }
}



