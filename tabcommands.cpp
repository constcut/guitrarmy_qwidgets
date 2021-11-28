#include "tabcommands.h"

#include "tabviews.h"
#include "mainviews.h"

//other dep
#include "g0/midifile.h"
#include "g0/aexpimp.h"

#include <QInputDialog>
#include <QMutex>
#include <QFileDialog>
#include <QApplication>
#include <QScreen>


#include <QDebug>
#include <fstream>


//Trackview events:

void reactOnComboTrackViewQt(const std::string& press, Track* pTrack, MasterView* mw) { //TODO keyeventsUI
    std::string rest = press.substr(4);
    size_t separator = rest.find(":");
    std::string combo = rest.substr(0,separator);
    std::string item = rest.substr(separator+1);
    int itemNum = atoi(item.c_str());
    if (combo=="0") //Это выбор другого трека
    {
        char mini[2]={0};
        mini[0] = itemNum + 49;
        mw->pushForceKey("esc");
        mw->pushForceKey(mini);
    }
    if (combo=="1")
        pTrack->setInstrument(itemNum);
    if (combo=="2")
        pTrack->setVolume(itemNum);
    if (combo=="5")
        pTrack->setStatus(itemNum);
    if (combo=="6")
        pTrack->setPan(itemNum);
}




void changeBarSignsQt(Track* pTrack, int&  selectionBarFirst, int& selectionBarLast) {
    bool ok=false;
    int newNum = QInputDialog::getInt(0,"Input",
                         "New Num:", QLineEdit::Normal,
                         1,128,1,&ok);
    if (!ok)
        return;
    ok=false;
    int newDen = QInputDialog::getInt(0,"Input",
                         "New Denum(1,2,4,8,16):", QLineEdit::Normal,
                         1,128,1,&ok);
    if (ok)
        pTrack->changeBarSigns(newNum,newDen);
}




void handleKeyInput(int digit, int& digitPress, Track* pTrack, size_t cursor, size_t cursorBeat, size_t stringCursor, std::vector<SingleCommand>& commandSequence) {

    //group operations
    if (digitPress>=0) {
        if (digitPress<10) {
            int pre = digitPress;
            digitPress*=10;
            digitPress+=digit;
            if (digitPress > pTrack->getGPCOMPInts(3)) { //Destoy all GPCOMP TODO
                digitPress = digit;
                if (digit == pre)
                    return; //no changes
            }
        }
        else
            digitPress = digit;
    }
    else
        digitPress = digit;

    if ( pTrack->at(cursor)->size() > cursorBeat ) {
        std::uint8_t lastFret = pTrack->at(cursor)->at(cursorBeat)->getFret(stringCursor+1);

        SingleCommand command(ReversableCommand::SetFret,lastFret);
        command.setPosition(0,cursor,cursorBeat,stringCursor+1);
        commandSequence.push_back(std::move(command));
        pTrack->at(cursor)->at(cursorBeat)->setFret(digitPress,stringCursor+1);
        Note *inputedNote =  pTrack->at(cursor)->at(cursorBeat)->getNote(stringCursor+1);
        std::uint8_t tune = pTrack->tuning.getTune(stringCursor);
        int chan = 0;
        if (pTrack->isDrums()) {
            chan = 9; //tune to 0 attention refact error
            tune = 0;
        }
        std::uint8_t midiNote = inputedNote->getMidiNote(tune);
        MidiEngine::sendSignalShort(0x90|chan,midiNote,120);
        ///MidiEngine::sendSignalShortDelay(250,0x80|chan,midiNote,120);
        //MidiEngine::sendSignalShortDelay(750,0x90|chan,midiNote+2,120);
    }
}



void playTrack(TabView* tabParrent, ThreadLocal* localThr, size_t& cursorBeat, size_t cursor, Track* pTrack, MasterView* mw) { //TODO объединить - воспроизведение должно быть из одного источника запускаться

    if (tabParrent->getPlaying()==true) {
        if (localThr)
           if (localThr->getStatus()) {
                //animation stopped
                tabParrent->setPlaying(false);
                //cursor = displayIndex; //auto repeat from page
                cursorBeat = 0;
            }
    }

    if (tabParrent->getPlaying() == false) {
        //to start not from begin always
        size_t shiftTheCursor = 0;
        if (cursor != 0){
            Bar *barPtr = pTrack->at(cursor).get();

            for (size_t i = 0; i < pTrack->timeLoop.size();++i){
                 if (pTrack->timeLoop.at(i) == barPtr){
                     shiftTheCursor = i;
                     break;
                 }
            }
        }

        clock_t beforeT = getTime();
        pTrack->connectAll();
        clock_t afterT = getTime();
        int diffT = afterT - beforeT;
        qDebug() <<"Repair chains "<<diffT;
        Tab *tab = tabParrent->getTab();
        tab->connectTracks();
        MidiFile generatedMidi;
        generatedMidi.fromTab(tabParrent->getTab(),shiftTheCursor);

        /*
        if ((CONF_PARAM("mergeMidiTracks")=="1") || (press=="playMerge")){
            MidiTrack *newTrack = MidiEngine::uniteFileToTrack(&generatedMidi);
            generatedMidi.clear();
            generatedMidi.add(newTrack);
        }*/

        clock_t after2T = getTime();
        diffT = after2T - afterT;
        diffT /= (CLOCKS_PER_SEC/1000);
        qDebug() <<"Generate midi "<<diffT;

        MidiEngine::closeDefaultFile();
        std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");

        std::ofstream outFile2(fullOutName);

        if (!outFile2.is_open())
            qDebug() << "Failed to open out file :(";
        else
            qDebug() <<"File opened " << fullOutName.c_str();

        generatedMidi.writeStream(outFile2);
        outFile2.close();

        if (CONF_PARAM("midi.config").empty() == false){
            qDebug() << "Midi config " << CONF_PARAM("midi.config").c_str();
            //MidiToPcm generator(CONF_PARAM("midi.config"));
            std::string outputSound = getTestsLocation() + std::string("waveOutput.wav");
            //generator.convert(fullOutName,outputSound); //TODO sf mit
        }
        tabParrent->prepareAllThreads(shiftTheCursor);
        tabParrent->connectAllThreadsSignal(mw);
        std::string midiConfig = CONF_PARAM("midi.config");

        if (CONF_PARAM("midi.config").empty() == false) {
            ///NEED TO SEND start_record_output waveOutput.wav
            mw->pushForceKey("start_record_output waveOutput.wav");
        }
        else {
            MidiEngine::openDefaultFile();
            MidiEngine::startDefaultFile();
        }
        tabParrent->launchAllThreads();
        tabParrent->setPlaying(true);
    }
    else {
        if (CONF_PARAM("midi.config").empty() == false)
            mw->pushForceKey("stop_record_output");
        else
            MidiEngine::stopDefaultFile();

        tabParrent->stopAllThreads();
        tabParrent->setPlaying(false);
    }
}



void saveAsFromTrack(TabView* tabParent) {
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

    std::string dir="";
#ifdef __ANDROID_API__
    dir="/sdcard/";
    fd->setDirectory("/sdcard/");
    QScreen *screen = QApplication::screens().at(0);
    fd->setGeometry(0,0,screen->geometry().width(),screen->geometry().height());
#endif

    QString saveFileName = fd->getSaveFileName(0,"Save tab as",dir.c_str(),"Guitarmy files (*.gmy)");
    delete fd;
    GmyFile gmyFile;
    std::string  gfileName = saveFileName.toStdString();
    std::ofstream file(gfileName);
    gmyFile.saveToFile(&file,tabParent->getTab());
    return;
}



void setBendOnNote(Note* currentNote, MasterView* mw) {

   if (currentNote->effPack.get(17)) {
       Package *bendPack = currentNote->effPack.getPack(17);
       if (bendPack) //attention possible errors escaped
       {
        BendPoints *bend = (BendPoints*) bendPack->getPointer();
        BendInput::setPtrNote(currentNote);
        BendInput::setPtrBend(bend);
       }
       else {
           BendInput::setPtrBend(0);
           BendInput::setPtrNote(currentNote);
       }
   }
   else {
        BendInput::setPtrBend(0);
        BendInput::setPtrNote(currentNote);
   }
    if (mw)
        mw->pushForceKey("bend_view");
}

void setTextOnBeat(Track *track) {
    auto& beat = track->at(track->cursor())->at(track->cursorBeat());
    std::string beatText;
    beat->getGPCOMPText(beatText);

    bool ok=false;
    QString newText = QInputDialog::getText(0,"Input",
                     "Input text:", QLineEdit::Normal,beatText.c_str(),&ok);
    if (ok) {
        beatText = newText.toStdString();
        track->setTextOnBeat(beatText);
    }
}


void setChangesOnBeat(Beat* beat, MasterView* mw) {
    ChangesInput::setPtrBeat(beat);
    if (mw)
        mw->pushForceKey("change_view");
}

void setBarSign(Track* pTrack) {
    bool ok=false;
    int newNum = QInputDialog::getInt(0,"Input","New Num:",
                        QLineEdit::Normal,1,128,1,&ok);

    bool thereWasChange = false;
    if (ok)
        thereWasChange = true;
    ok=false;
    int newDen = QInputDialog::getInt(0,"Input","New Denum(1,2,4,8,16):",
                 QLineEdit::Normal,1,128,1,&ok);
    if (ok)
        thereWasChange = true;

    if (thereWasChange) { //also could set to all here if turned on such flag
        pTrack->setBarSign(newNum, newDen);
    }
}




void TabView::onTrackCommand(TrackCommand command) {
    qDebug() << "ERROR: Track Command falling into TabView";
}


void TrackView::onTrackCommand(TrackCommand command) {

    size_t& cursor = pTrack->cursor(); //TODO get rid slowly
    size_t& cursorBeat = pTrack->cursorBeat();
    size_t& stringCursor = pTrack->stringCursor();
    int& selectionBarFirst = pTrack->selectBarFirst();
    int& selectionBarLast = pTrack->selectBarLast();

    if (command == TrackCommand::PlayFromStart) {
        pTrack->gotoTrackStart();
        onTabCommand(TabCommand::PlayMidi);
    }
    else if (command == TrackCommand::SetSignForSelected)
      changeBarSignsQt(pTrack, selectionBarFirst, selectionBarLast);
    else if (command == TrackCommand::PlayTrackMidi) //TODO единый вызов запуска (играется не 1 трек) //|| (press=="playMerge")
        playTrack(tabParrent, localThr, cursorBeat, cursor, pTrack, getMaster());
    else if (command == TrackCommand::SaveAsFromTrack)
        saveAsFromTrack(tabParrent);
    else if (command == TrackCommand::Bend)
        setBendOnNote(pTrack->at(cursor)->at(cursorBeat)->getNote(stringCursor+1), getMaster());
    else if (command == TrackCommand::Chord) {
        if (getMaster()) getMaster()->pushForceKey("chord_view"); }
    else if (command == TrackCommand::Text)
        setTextOnBeat(pTrack);
    else if (command == TrackCommand::Changes)
        setChangesOnBeat(pTrack->at(cursor)->at(cursorBeat).get(), getMaster());
    else if (command == TrackCommand::SetBarSign)
        setBarSign(pTrack);
    else
        pTrack->onTrackCommand(command);

}



void TrackView::keyevent(std::string press) //TODO масштабные макротесты, чтобы покрывать все сценарии
{
    size_t& cursor = pTrack->cursor();
    size_t& cursorBeat = pTrack->cursorBeat();
    size_t& stringCursor = pTrack->stringCursor();

    if (press.substr(0,4)=="com:")
        reactOnComboTrackViewQt(press, pTrack, tabParrent->getMaster());
    else if (isdigit(press[0]))
        handleKeyInput(press[0]-48, pTrack->digitPress(), pTrack, cursor, cursorBeat, stringCursor, pTrack->commandSequence);
    else {
        qDebug() << "Key event falls into TabView from TrackView " << press.c_str();
        tabParrent->keyevent(press); //TODO проверить
    }//TODO перепроверить что все команды работают без повторного запуска через TabView
}

//Tab commands functions, TODO cover under some engine inside of TAB
//And make handlers for all the functions (used them without arguments)


void TrackView::onTabCommand(TabCommand command) {
    tabParrent->onTabCommand(command);
}



void playPressedQt(Tab* pTab, ThreadLocal* localThr, size_t currentBar, TabView *tabView) {
    //pre action for repeat
    if (tabView->getPlaying()==true)
        if (localThr)
            if (localThr->getStatus())
                tabView->setPlaying(false);

    if (tabView->getPlaying() == false) {
        size_t shiftTheCursor = 0;
        if (currentBar != 0) {
            Bar *barPtr = pTab->at(0)->at(currentBar).get();
            for (size_t i = 0; i < pTab->at(0)->timeLoop.size();++i)
                 if (pTab->at(0)->timeLoop.at(i) == barPtr) {
                     shiftTheCursor = i;
                     break;
                 }
        }
        //Разделить все этапы с интерфейсом TODO
        pTab->connectTracks();
        MidiFile generatedMidi;
        generatedMidi.fromTab(pTab,shiftTheCursor);
        MidiEngine::closeDefaultFile();
        std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");
        std::ofstream outFile2;
        if (!outFile2.is_open())
            qDebug() << "Failed to open out file :(";
        else
            qDebug() <<"File opened "<<fullOutName.c_str();

        generatedMidi.writeStream(outFile2);
        outFile2.close();

        tabView->prepareAllThreads(shiftTheCursor);
        tabView->connectAllThreadsSignal(tabView->getMaster());
        MidiEngine::openDefaultFile();
        MidiEngine::startDefaultFile();
        tabView->launchAllThreads();
        tabView->setPlaying(true);
    }
    else
    {
        MidiEngine::stopDefaultFile();
        tabView->stopAllThreads();
        tabView->setPlaying(false);
    }
}

void generateMidiQt(Tab* pTab, GLabel* statusLabel) {
    MidiFile generatedMidi;
    generatedMidi.fromTab(pTab);

    MidiEngine::closeDefaultFile();
    std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");
    std::ofstream outFile2(fullOutName);

    if (! outFile2.is_open()){
        qDebug() << "Failed to open out file :(";
        statusLabel->setText("failed to open generated");
    }
    size_t outFileSize2 = generatedMidi.writeStream(outFile2);
    qDebug() << "File wroten. " << outFileSize2 << " bytes. ";
    outFile2.close();
    generatedMidi.printToStream(std::cout);
    statusLabel->setText("generation done. p for play");
}


void openTrackQt(size_t tracksLen, int& lastOpenedTrack, TabView* tabView, size_t digit) {
    if (digit && digit <= tracksLen) {
        TrackView *trackView = tabView->tracksView[digit-1]; //А обновление интерфейса в модуль Qt TODO выше
        lastOpenedTrack = digit-1;
        MainView *mainView = (MainView*)tabView->getMaster()->getFirstChild();
        mainView->changeCurrentView(trackView);
    }
}


void TabView::keyevent(std::string press) {
    if (isdigit(*(press.c_str()))) //The only one left here
        openTrackQt(pTab->size(),pTab->getLastOpenedTrack(), this, press.c_str()[0]-48);
}

//Tab commands area

void deleteTrack(Tab* pTab) { //TODOM
    bool ok=false;
    int inp = QInputDialog::getInt(0,"Delete track","Delete track",0,0,1,1,&ok);
    if ((ok) && (inp))
        pTab->deleteTrack();
}

void changeTrackName(Tab* pTab) { //TODOM
    bool ok=false;
    //refact inputs to gview
    QString newName = QInputDialog::getText(0,"Input",
                         "New Instrument name:", QLineEdit::Normal,"untitled",&ok);
    std::string stdName = newName.toStdString();
    if (ok)
       pTab->changeTrackName(stdName);
}

void changeTrackVolume(Tab* pTab)  { //TODOM
    bool ok=false;
    int newVol = QInputDialog::getInt(0,"Input",
                         "Vol Instrument:", QLineEdit::Normal,
                         0,16,1, &ok);
    if (ok)
        pTab->changeTrackVolume(newVol);

}

void setSignTillEnd(Tab* pTab) { //TODOM
    bool ok=false;
    int newNum = QInputDialog::getInt(0,"Input",
                         "New Num:", QLineEdit::Normal,
                         1,128,1,&ok);
    if (!ok)
        return;
    ok=  false;
    int newDen = QInputDialog::getInt(0,"Input",
                         "New Denum(1,2,4,8,16):", QLineEdit::Normal,
                         1,128,1,&ok);
    if (ok)
        pTab->setSignsTillEnd(newDen, newNum);

}

void setMarker(Tab* pTab) {
    bool ok=false;
    QString markerText= QInputDialog::getText(0,"Input",
                         "Marker:", QLineEdit::Normal,"untitled",&ok);
    if (ok) {
        std::string stdMarkerText = markerText.toStdString();
        pTab->setMarker(stdMarkerText);
    }
}

void goToBar(Tab* pTab) {
    size_t trackLen = pTab->at(0)->size();
    bool ok=false; //TODO позже разделить Qt запросы и установку параметров
    int newTimes = QInputDialog::getInt(0,"Input",
                         "Bar to jump:", QLineEdit::Normal, 1, trackLen, 1, &ok);
    if (ok) {
        --newTimes;
        pTab->gotoBar(newTimes);
    }
}


void setTune(Track* pTrack) {

    bool ok=false;
    QStringList items;
    char iBuf[10];

    double fTable []=
    {16.3515978313,
     17.3239144361,
     18.3540479948,
     19.4454364826,
     20.6017223071,
     21.8267644646,
     23.1246514195,
     24.4997147489,
     25.9565435987,
     27.5000000000,
     29.1352350949,
     30.8677063285
    };

    for (int i = 0; i < 8; ++i) {
        //octaves
        iBuf[0] = i +49;
        iBuf[1] = 0;
        std::string octave = iBuf;
        std::string note;
        int midiNote = 0;
        for (int j = 0; j < 12; ++j) {
            //Notes
            switch (j) {
                case 0: note = "C";  break;
                case 1: note = "C#"; break;
                case 2: note = "D"; break;
                case 3: note = "D#"; break;
                case 4: note = "E"; break;
                case 5: note = "F"; break;
                case 6: note = "F#"; break;
                case 7: note = "G"; break;
                case 8: note = "G#"; break;
                case 9: note = "A"; break;
                case 10: note = "A#"; break;
                case 11: note = "B"; break;
                default: note="noteX";
            }

            int coefOctave = 1;
            for (int z=0; z <i; ++z)
                coefOctave*=2;
            double theFreq = fTable[j]*coefOctave;
            midiNote = 12+j + 12*i;
            std::string fullLine = note + octave +" - " +
                std::to_string(midiNote) +" - " + std::to_string(theFreq);
            items.push_back(fullLine.c_str());
        }
    }

    //items.push_back("another thesr");
    //items.push_back("once_more");

    for (int i = 0; i < pTrack->tuning.getStringsAmount(); ++i) {
        int preValue = pTrack->tuning.getTune(i)-12;
        QString resp = QInputDialog::getItem(0,"Input tune",
                                        ("String #" + std::to_string(i+1)).c_str(),items,preValue,false,&ok);
        int respIndex = -1;
        for (int j = 0; j < items.size(); ++j)
            if (items.at(j)==resp) {
                respIndex = j;
                break;
            }

        if (ok)
            if (respIndex>=0)
                pTrack->tuning.setTune(i,respIndex+12);
    }
}


void saveAs(Tab* pTab) { //Move into Tab (но на этапе уже получения имени файла)

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

    std::string dir="";
#ifdef __ANDROID_API__
    dir="/sdcard/";
    fd->setDirectory("/sdcard/");
    QScreen *screen = QApplication::screens().at(0);
    fd->setGeometry(0,0,screen->geometry().width(),screen->geometry().height());
#endif

    QString saveFileName = fd->getSaveFileName(0,"Save tab as",dir.c_str(),"Guitarmy files (*.gmy)");
    delete fd; //TODO изучить возможность переиспользования

    std::string  gfileName = saveFileName.toStdString();
    pTab->saveAs(gfileName);
}


void closeReprise(Tab* pTab) { //TODO argument repeat times
    auto& firstTrackBar = pTab->at(0)->at(pTab->getCurrentBar());
    std::uint8_t repeat = firstTrackBar->getRepeat();
    std::uint8_t repeatCloses = repeat & 2;
    if (repeatCloses) {
        pTab->closeReprise(0);
    }
    else {
        bool ok=false;
        int newTimes = QInputDialog::getInt(0,"Input", "Repeat times:",
                            QLineEdit::Normal,2,99,1,&ok);
        if ((ok)&&(newTimes))
            pTab->closeReprise(newTimes);
    }
}

int changeTrackBpm(Tab* pTab) {
    bool ok=false;
    int newBpm = QInputDialog::getInt(0,"Input",
                         "New Bpm:", QLineEdit::Normal,
                         1,999,1,&ok);
    if (ok) {
        pTab->setBPM(newBpm);
        return newBpm;
    }
    else
        return pTab->getBPM();
}

int changeTrackPanoram(Tab* pTab) {
    auto& pTrack = pTab->at(pTab->getCurrentTrack());
    bool ok=false;
    QStringList items;
    items.push_back("L 8 - 100%");
    items.push_back("L 7");
    items.push_back("L 6");
    items.push_back("L 5");
    items.push_back("L 4");
    items.push_back("L 3");
    items.push_back("L 2");
    items.push_back( "L 1");
    items.push_back("C 0 - 0%");
    items.push_back("R 1");
    items.push_back("R 2");
    items.push_back("R 3");
    items.push_back("R 4");
    items.push_back("R 5");
    items.push_back("R 6");
    items.push_back("R 7 ");
    items.push_back("R 8 - 100%");
    /*
    int newPan = QInputDialog::getInt(0,"Input",
                        "Instrument Panoram:", QLineEdit::Normal,
                        0,16,1,&ok);*/
    int curPan = pTrack->getPan(); //TODO pTab->getTrackPan
    QString result = QInputDialog::getItem(0,"Input",
                                "New Panoram:",items, curPan,false,&ok);

    int backToNumber = -1;
    if (ok) {
       for (int i = 0 ; i < 128; ++i)
           if (result == items[i]) {
               backToNumber = i;
               break;
           }
      if (backToNumber >= 0)
        pTab->changeTrackPanoram(backToNumber);
    }
    return backToNumber;
}


int changeTrackInstrument(Tab* pTab) {
    auto& pTrack = pTab->at(pTab->getCurrentTrack());
    //TODO отделить запрос от ядра
    std::string instruments[]= { //Move to sepparated file TODO
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
    "Gunshot"};

     bool ok=false;
    QStringList items;
    for (int i = 0 ; i < 128; ++i) {
        auto s = (std::to_string(i) + " - " + instruments[i]);
        items.push_back(s.c_str());
    }

    int curInstr = pTrack->getInstrument(); //TODO get current instr?

    QString result = QInputDialog::getItem(0,"Input",
                                 "New Instrument:",items, curInstr,false,&ok);
    int backToNumber = -1;
    if (ok)
        for (int i = 0 ; i < 128; ++i)
            if (result == items[i]) {
                backToNumber = i;
                break;
            }

    if (backToNumber>=0)
        pTab->changeTrackInstrument(backToNumber);

    return backToNumber;
}



void TabView::onTabCommand(TabCommand command) {
    //TODO undo для команд таблатуры так же
    if (command == TabCommand::SaveAs)
        saveAs(pTab);
    else if (command == TabCommand::SetSignTillEnd)  //TODO хэндлеры для более простого вызова
        setSignTillEnd(pTab);
    else if (command == TabCommand::Volume)
        ::changeTrackVolume(pTab);
    else if (command == TabCommand::Name)
        ::changeTrackName(pTab);
    else if (command == TabCommand::DeleteTrack)
        ::deleteTrack(pTab);
    else if (command == TabCommand::AddMarker)
        setMarker(pTab);
    else if (command == TabCommand::Instument)
        getMaster()->setComboBox(1,"instruments",240,5,200,30, changeTrackInstrument(pTab)); //Only UI feature
    else if (command == TabCommand::Panoram)
        getMaster()->setComboBox(6,"pan",570,5,50,30, changeTrackPanoram(pTab)); //Как и выше сбивает UI при отмене ввода
    else if (command == TabCommand::BPM) {
        auto newBpm = changeTrackBpm(pTab);
        bpmLabel->setText("bpm=" + std::to_string(newBpm)); //Сейчас обновляет каждый раз, даже при отмене - стоит продумать это при разделении Qt ввода и ядра библиотеки TODO
        getMaster()->setStatusBarMessage(2,"BPM= " + std::to_string(newBpm));
    }         
    else if (command == TabCommand::OpenTrack)
        openTrackQt(pTab->size(),pTab->getLastOpenedTrack(), this, pTab->getDisplayTrack() + 1); //TODO эту часть внутрь движка - разделяя с QT);
    else if (command == TabCommand::NewTrack) {
       pTab->createNewTrack(); this->setTab(pTab); } //Второе нужно для обновления
    else if (command == TabCommand::PlayMidi) //Если нам понадобится playMerge оно осталось только в git истории
        playPressedQt(pTab, localThr, pTab->getCurrentBar(), this);
    else if (command == TabCommand::GenerateMidi)
        generateMidiQt(pTab, statusLabel);
    else if (command == TabCommand::GotoBar)
        goToBar(pTab);
    //if (press == "alt");//TODO
    else if (command == TabCommand::Tune)
        setTune(pTab->at(pTab->getCurrentTrack()).get());
    else if (command == TabCommand::CloseReprise)
        closeReprise(pTab);
    else
        pTab->onTabCommand(command);
}
