#include "InputViews.hpp"

#include "TabViews.hpp"
#include "tab/tools/TabClipboard.hpp"
#include "midi/MidiFile.hpp"

#include <QInputDialog>

#include "midi/MidiExport.hpp"

#include <QAudioRecorder>
#include <QAudioDeviceInfo>

#include <QMessageBox>
#include <QDir>

#include <fstream>

#include <QDebug>


#include "MainViews.hpp"
#include "MainWindow.hpp"



using namespace gtmy;


//
void changeColor(const std::string& color, QPainter* src);
//

///=========================

void PatternInput::draw(QPainter *painter)
{
    _repeatLabel->draw(painter);

    for (size_t i = 0; i < _checkButtons.size(); ++i)
        _checkButtons[i].draw(painter);



    _sigDenBut->draw(painter);     
    _sigNumBut->draw(painter);
    _bpmBut->draw(painter);


    _bpmValue->draw(painter);

    if (_barView)
        _barView->draw(painter);

    _butRepeat->draw(painter);
}


void PatternInput::createBar()
{
    _bar = std::make_unique<Bar>();
    _bar->flush();

    int num = atoi(_sigNumBut->getText().c_str());
    int den = atoi(_sigDenBut->getText().c_str());

    _bar->setSignDenum(den);
    _bar->setSignNum(num);

    for (size_t i = 0; i < (_checkButtons.size()/4); ++i) {
        auto beat = std::make_unique<Beat>();
        beat->setDuration(_currentDen);
        beat->setDotted(0);
        beat->setDurationDetail(0);
        beat->setPause(false);
        bool oneChecked = false;
        for (size_t iCoef = 0; iCoef < 4; ++iCoef) {
            size_t ind = i + iCoef*(_checkButtons.size()/4);
            if (_checkButtons[ind].isChecked()){
                auto newNote = std::make_unique<Note>();
                std::uint8_t localFret = atoi(_lineInstrLabels[iCoef].getText().c_str());
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

        _bar->push_back(std::move(beat));
    }

    _barView = std::make_unique<BarView>(_bar.get(), 6);
    _barView->setShifts(250,300-55-30);

    AClipboard::current()->setPtr(_bar.get());
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

    AClipboard::current()->setPtr(_bar.get());
    AClipboard::current()->setClipboardType(ClipboardType::BarPointer); //ptr

    if (_butRepeat->isChecked())
    {
        _bar->setRepeat(1);
        _bar->setRepeat(2,10);
    }

    patternTrack->push_back(std::move(_bar));

    auto& tuning = patternTrack->getTuningRef();
    tuning.setStringsAmount(6);

    for (int i = 0; i < 7; ++i)
        tuning.setTune(i, 0);

    patternTab.push_back(std::move(patternTrack));

    int patternBPM = atoi(_bpmValue->getText().c_str());
    patternTab.setBPM(patternBPM);
    patternTab.connectTracks();

    std::string fullOutName = AConfig::getInst().testsLocation + std::string("midiOutput.mid");

    std::ofstream outFile(fullOutName);
    auto outMidi = exportMidi(&patternTab);
    outMidi->writeStream(outFile);
    outFile.close();
    outMidi->printToStream(std::cout);

    //STARTMIDI

    _bar->setParent(0);

    patternTrack->remove(0);
}

void PatternInput::setUI()
{
    for (size_t i = 0; i < _lineInstrLabels.size(); ++i) {
        int shY = 100-55+50*i;
        int shX = 30;
        if (getMaster())
        getMaster()->setComboBox(i,"drums",shX-25,shY-40,70,30,-1);
    }

    if (getMaster()) {
       size_t i = _lineInstrLabels.size();
       getMaster()->SetButton(i,_sigDenBut->getText(),_sigDenBut->getX(),
                              _sigDenBut->getY(),_sigDenBut->getW(),_sigDenBut->getH(),"newDen");
       ++i;
       getMaster()->SetButton(i,_sigNumBut->getText(),_sigNumBut->getX(),
                              _sigNumBut->getY(),_sigNumBut->getW(),_sigNumBut->getH(),"newNum");
       ++i;
       getMaster()->SetButton(i,_bpmBut->getText(),_bpmBut->getX(),
                              _bpmBut->getY(),_bpmBut->getW(),_bpmBut->getH(),"newBpm");
       ++i;
       getMaster()->SetButton(i,"play",_bpmBut->getX()+180,
                              _bpmBut->getY(),30,20,CONF_PARAM("TrackView.playMidi"));
    }
}

void PatternInput::keyevent(std::string press)
{
    if (press==CONF_PARAM("TrackView.playMidi"))
    {
        if (_bar)
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
            _bpmValue->setText(std::to_string(newBpm));
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
            _lineInstrLabels.clear();
            _checkButtons.clear();

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
                _lineInstrLabels.push_back(std::move(lineLab));

                shX += 70;

                for (size_t i = 0; i < newNum; ++i)
                {
                    GCheckButton but(shX,shY-30,30,30);
                    shX += but.getW() + 10;
                    _checkButtons.push_back(but);

                    if (shX > maxShX)
                        maxShX = shX;
                }
            }

            _sigNumBut->setText(std::to_string(newNum));
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
        if (ok) {
            int listValue = -1;
            for (int i = 0; i < items.size(); ++i){
                if (items.at(i)==resp){
                    listValue = i;
                    break;
                }
            }
            if (listValue >=0)
                _currentDen = listValue;
        }

       std::string stdResp = resp.toStdString();
       _sigDenBut->setText(stdResp);
    }
}

void PatternInput::onclick(int x1, int y1)
{
    if (_butRepeat->hit(x1,y1))
    {
        _butRepeat->onclick(x1,y1);
        if (_butRepeat->isChecked()==false)
        {
            //STARTMIDI (stop)
        }
    }

    if (_sigNumBut->hit(x1,y1))
    {
        getMaster()->pushForceKey("newNum");
    }

    if (_sigDenBut->hit(x1,y1))
    {
        //16 is default
        getMaster()->pushForceKey("newDen");
    }

    for (size_t i = 0; i < _checkButtons.size(); ++i)
       if  (_checkButtons[i].hit(x1,y1))
       {
           _checkButtons[i].onclick(x1,y1);
           createBar();
           break;
       }

    for (size_t i = 0; i < _lineInstrLabels.size(); ++i)
       if  (_lineInstrLabels[i].hit(x1,y1))
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

    if (_bpmBut->hit(x1,y1))
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

        _leftPress.setX(0); _leftPress.setY(fullH-200);
        _leftPress.setW(200); _leftPress.setH(200);

        _rightPress.setX(fullW-200); _rightPress.setY(fullH-200);
        _rightPress.setW(200); _rightPress.setH(200);

        getMaster()->setComboBox(0,"drums",_leftPress.getX()+10,_leftPress.getY()+10,150,30,-1);
        getMaster()->setComboBox(1,"drums",_rightPress.getX()+10,_rightPress.getY()+10,150,30,-1);

        getMaster()->SetButton(2,_labClean.get(),"clean tap");
        getMaster()->SetButton(3,_stopMetr.get(),"stop metr");
        getMaster()->SetButton(4,_bpmLabel.get(),"newBpm");
        getMaster()->SetButton(5,_labA.get(),"start metr");

        getMaster()->SetButton(6,"play",600,_labClean->getY(),50,20,"ent");
    }


}

void TapRyView::draw(QPainter *painter)
{
    int fullW = getMaster()->getWidth();
    int fullH = getMaster()->getHeight();


    _leftPress.setX(0); _leftPress.setY(fullH-200);
    _leftPress.setW(200); _leftPress.setH(200);
    _rightPress.setX(fullW-200); _rightPress.setY(fullH-200);
    _rightPress.setW(200); _rightPress.setH(200);


    _rightPress.draw(painter);
    _leftPress.draw(painter);



    _labA->draw(painter);
    _labB->draw(painter);
    _labStat->draw(painter);
    _labExp->draw(painter);

    _labClean->draw(painter);
    _stopMetr->draw(painter);

    //bpmLabel->setY(bpmLabel->getY() + 50);
    //bpmLabel->setBorder(false);

    _bpmLabel->draw(painter);


    if (_barView)
        _barView->draw(painter);
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

    if (_presses.size() < 2)
        return; //avoid crah

    int thatBPM = atoi(_bpmLabel->getText().c_str());


    if (_ryBar)
        _ryBar = nullptr;


    _ryBar = std::make_unique<Bar>();
    _ryBar->flush();

    _ryBar->setSignDenum(4); _ryBar->setSignNum(8);

    std::uint8_t lastDur = 0;

    int lastPressInstr = 0;

    for (size_t i = 1; i < _presses.size(); ++i)
    {
        int step = _presses[i].first-_presses[i-1].first;
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



        if (_presses[i].second == 1)
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

        _ryBar->push_back(std::move(ryBeat));
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

        _ryBar->push_back(std::move(ryBeat));
    }


    while (_ryBar->getCompleteStatus()==2)
        _ryBar->setSignNum(_ryBar->getSignNum()+1);



    qDebug() << "tap ry done";

    //getMaster()->setStatusBarMessage(0,sX.c_str(),3000);
    //labStat->setText(sX.c_str());


    if (_barView == nullptr) {
        _barView = std::make_unique<BarView>(_ryBar.get(),6);
        _barView->setShifts(210,80);
    }
        else _barView->setBar(_ryBar.get());

    AClipboard::current()->setPtr(_ryBar.get());
    AClipboard::current()->setClipboardType(ClipboardType::BarPointer); //ptr

}

void TapRyView::copyAndPlayBar()
{
    //createBar();

    if (_ryBar==0)
        return;

    int thatBPM = atoi(_bpmLabel->getText().c_str());

    //createBar();
    //copy paste
    Tab patternTab;
    auto patternTrack= std::make_unique<Track>();
    patternTrack->setParent(&patternTab);
    patternTrack->setDrums(true);
    patternTrack->setInstrument(0);
    patternTrack->setVolume(15);
    patternTrack->setPan(8);

    patternTrack->push_back(std::move(_ryBar));

    auto& tuning = patternTrack->getTuningRef();
    tuning.setStringsAmount(6);
    for (int i = 0; i < 7; ++i)
        tuning.setTune(i, 0);

    patternTab.push_back(std::move(patternTrack));
    patternTab.setBPM(thatBPM);
    patternTab.connectTracks();

    std::string fullOutName = AConfig::getInst().testsLocation + std::string("midiOutput.mid");

    std::ofstream outFile(fullOutName);
    auto outMidi = exportMidi(&patternTab);
    outMidi->writeStream(outFile);


    //STARTMIDI

    _ryBar->setParent(0);
    patternTrack->remove(0);

}

void TapRyView::keyevent(std::string press)
{
    if (press==CONF_PARAM("TrackView.playMidi"))
    {
        copyAndPlayBar();
    }

    if (press == "clean tap")
    {
        _presses.clear();
        _ryBar = nullptr;
        _barView = nullptr;
    }

    if (press == "stop metr")
    {
        //STARTMIDI++--
    }

    if (press == "newBpm")
    {
        bool ok=false;

        int newBpm = QInputDialog::getInt(0,"Input",
                             "New bpm:",20,
                             40,255,1,&ok);
        if (ok)
        {
            _bpmLabel->setText(std::to_string(newBpm));
            getMaster()->SetButton(4,_bpmLabel.get(),"newBpm");
        }
    }

    if (press == "start metr")
    {
        //START PSEUDO METRONOME
        _ryBar = nullptr;
        _barView = nullptr;

        _ryBar = std::make_unique<Bar>();
        _ryBar->flush();

        _ryBar->setSignDenum(4); _ryBar->setSignNum(4);

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

            _ryBar->push_back(std::move(ryBeat));
        }

        Tab patternTab;
        auto patternTrack= std::make_unique<Track>();
        patternTrack->setParent(&patternTab);
        patternTrack->setDrums(true);
        patternTrack->setInstrument(0);
        patternTrack->setVolume(15);
        //patternTrack.setPan();

        _ryBar->setRepeat(1);
        _ryBar->setRepeat(2,100);

        patternTrack->push_back(std::move(_ryBar));

        auto& tuning = patternTrack->getTuningRef();
        tuning.setStringsAmount(6);
        for (int i = 0; i < 7; ++i)
            tuning.setTune(i, 0);

        patternTab.push_back(std::move(patternTrack));

        int thatBPM = atoi(_bpmLabel->getText().c_str());
        patternTab.setBPM(thatBPM);
        patternTab.connectTracks();

        std::string fullOutName = AConfig::getInst().testsLocation + std::string("midiOutput.mid");

        std::ofstream outFile(fullOutName);
        auto outMidi = exportMidi(&patternTab);
        outMidi->writeStream(outFile);
        outFile.close();

        //STARTMIDI

        patternTrack->remove(0);

        //END OF PSEUDO METRONOME
    }
}

void TapRyView::onclick(int x1, int y1)
{
    if (_stopMetr->hit(x1,y1))
    {
       getMaster()->pushForceKey("stop metr");
    }

    if (_bpmLabel->hit(x1,y1))
    {
       getMaster()->pushForceKey("newBpm");
    }

    if (_leftPress.hit(x1,y1)||_rightPress.hit(x1,y1))
    {

        if (_leftPress.hit(x1,y1))
            _leftPress.pressIt();
            else
                _rightPress.pressIt();

        //most important

        if (_presses.size() == 0)
        {
            getTime();
        }



        clock_t timez = getTime();
        //sX<<"Click time "<<timez;
        //labStat->setText(sX.c_str());

        int timeNow= timez; //*AConfig::getInstance().getTimeCoef();
        intPair newPair;
        newPair.first = timeNow;
        newPair.second = _leftPress.hit(x1,y1) ? 1: 2;

        if (timeNow < 0)
        {
            qDebug() <<"Y";
        }
        _presses.push_back(newPair);

        //add value to vector or poly
        std::string info=
        "Presses " + std::to_string( _presses.size() ) + "; last- " + std::to_string(timeNow);

        qDebug() << info.c_str();

        _labStat->setText(info.c_str());

        createBar();

        int midiNote = 0;

        if (_leftPress.hit(x1,y1))
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

        //STARTMIDI
        //MidiEngine::sendSignalShort(0x90|9,midiNote,120);//refact velocy

        //getMaster()->pleaseRepaint();
    }

    if (_labClean->hit(x1,y1))
    {
        getMaster()->pushForceKey("clean tap");
    }

    if (_labA->hit(x1,y1))
    {
        getMaster()->pushForceKey("start metr");
    }

    if (_labExp->hit(x1,y1))
    {
        //copyAndPlayBar();
    }

}

//



void MorzeInput::draw(QPainter *painter)
{
    _createBut->draw(painter);

    if (_barView)
        _barView->draw(painter);
}

//REFACT NOTES - GROUP input metods into GInputView

void MorzeInput::playBar()
{
    if (_bar)
    {
        Tab morzeTab;
        auto morzeTrack= std::make_unique<Track>();
        morzeTrack->setParent(&morzeTab);
        morzeTrack->setDrums(true);
        morzeTrack->setInstrument(0);
        morzeTrack->setVolume(15);
        morzeTrack->setPan(8);

        morzeTrack->push_back(std::move(_bar));

        auto& tuning = morzeTrack->getTuningRef();
        tuning.setStringsAmount(6);
        for (int i=0; i < 7; ++i)
            tuning.setTune(i, 0);

        morzeTab.push_back(std::move(morzeTrack));
        morzeTab.setBPM(120);
        morzeTab.connectTracks();

        std::string fullOutName = AConfig::getInst().testsLocation + std::string("midiOutput.mid");

        std::ofstream outFile(fullOutName);
        auto outMidi = exportMidi(&morzeTab);

        outMidi->writeStream(outFile);
        outFile.close();
        outMidi->printToStream(std::cout);

        //STARTMIDI

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
    if (_createBut->hit(x1,y1))
    {
        //INPUT TEXT

        bool ok=false;
        QString inputedText = QInputDialog::getText(0,"Input","Text:",QLineEdit::Normal,"simpletest",&ok);

        if (ok)
        {
            _bar = std::make_unique<Bar>();

            _bar->flush();
            _bar->setSignDenum(4); _bar->setSignNum(7);

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

                _bar->push_back(std::move(ryBeat)); //����� �
                _bar->push_back(std::move(ryBeat2));

                qDebug() << "Added "<<dur<<" "<<dur2<<" beat";
            }


            _barView = std::make_unique<BarView>(_bar.get(),6);
            _barView->setShifts(100,200-55);

            AClipboard::current()->setPtr(_bar.get());
            AClipboard::current()->setClipboardType(ClipboardType::BarPointer); //ptr

            _bar->setRepeat(1);
            _bar->setRepeat(2, 2);

            playBar();
        }
    }
}



