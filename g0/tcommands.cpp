#include "tab.h"

#include "g0/midifile.h"
#include "g0/gmyfile.h"
#include "g0/aexpimp.h"
#include "g0/gtpfiles.h"

#include <QInputDialog>
#include <QMutex>
#include <QFileDialog>
#include <QApplication>
#include <QScreen>

#include <fstream>

#include "midiengine.h"
#include "libtim/miditopcm.h"
#include <QDebug>

//TODO rename file when old tab commands would be erased


void setSignTillEnd(Tab* pTab, size_t currentBar) {
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
    {
        //how to make undo? list of index old num old denum?
        for (size_t i = currentBar; i < pTab->getV(0)->len(); ++i)
        {
            pTab->getV(0)->getV(i)->setSignDenum(newDen); //TODO проработать размеры, ументь их делать общими для табы, и делать полиритмию
            pTab->getV(0)->getV(i)->setSignNum(newNum);
        }
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
    std::ofstream file(gfileName);
    GmyFile gmyFile;
    gmyFile.saveToFile(&file, pTab);
    file.close();
}


void muteTrack(Tab* pTab, size_t displayTrack) { //Move into Tab
    byte curStat = pTab->getV(displayTrack)->getStatus();
    if (curStat==1)
        pTab->getV(displayTrack)->setStatus(0);
    else
        pTab->getV(displayTrack)->setStatus(1);
}

void soloTrack(Tab* pTab, size_t displayTrack) { //Move into Tab
    byte curStat = pTab->getV(displayTrack)->getStatus();
    if (curStat==2)
        pTab->getV(displayTrack)->setStatus(0);
    else
        pTab->getV(displayTrack)->setStatus(2);
}


enum MoveDirections {
    none = 0,
    left = 1,
    right = 2,
    up = 3,
    down = 4
};


void moveCursorInTrack(MoveDirections dir, size_t& displayBar, size_t barsCount = 0) {
    switch(dir) { //Move into Tab
        case MoveDirections::right:
            if (displayBar < barsCount)
                ++displayBar;
        break;
        case MoveDirections::left:
            if (displayBar > 0)
                --displayBar;
        break;
        case none:
        case MoveDirections::up:
        case MoveDirections::down:
            qDebug() << "ERROR: Empty move direction!";
    }
}


void MoveCursorOfTrack(MoveDirections dir, size_t& displayTrack, size_t& currentTrack, size_t trackLen=0) {
    switch(dir) { //Move into Tab
        case MoveDirections::up:
        if (displayTrack > 0) {
            --displayTrack;
            currentTrack = displayTrack;
        }
        break;
        case MoveDirections::down:
        if (displayTrack < trackLen){
            ++displayTrack;
            if (currentTrack < displayTrack)
                currentTrack = displayTrack;
        }
        break;
    case none:
    case MoveDirections::right:
    case MoveDirections::left:
        qDebug() << "ERROR: Empty move direction!";
    }
}


void changeDrumsFlag(Track* pTrack) {
    bool drums = pTrack->isDrums();
    drums = !drums;
    pTrack->setDrums(drums);
}


int Tab::changeTrackInstrument() {
    Track* pTrack = getV(currentTrack);

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

    int curInstr = pTrack->getInstrument();

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
        pTrack->setInstrument(backToNumber);

    return backToNumber;
}


int Tab::changeTrackPanoram() {
    Track* pTrack = getV(currentTrack);
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
    int curPan = pTrack->getPan();
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
        pTrack->setPan(backToNumber);
    }
    return backToNumber;
}


void changeTrackVolume(Track* pTrack)   {
    bool ok=false;
    int newVol = QInputDialog::getInt(0,"Input",
                         "Vol Instrument:", QLineEdit::Normal,
                         0,16,1, &ok);
    if (ok)
       pTrack->setVolume(newVol);
}


void changeTrackName(Track* pTrack) {
    bool ok=false;
    //refact inputs to gview
    QString newName = QInputDialog::getText(0,"Input",
                         "New Instrument name:", QLineEdit::Normal,"untitled",&ok);

    std::string stdName = newName.toStdString();
    if (ok)
       pTrack->setName(stdName);
}


int Tab::changeTrackBpm() {
    Tab* pTab = this;
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


Track* Tab::createNewTrack() { //Move into Tab
    Tab* pTab = this;
    Track *track=new Track();
    std::string iName("NewInstrument");
    track->setName(iName);
    track->setInstrument(25);
    track->setVolume(15);
    track->setDrums(false);
    track->setPan(8); //center now
    track->tuning.setStringsAmount(6);

    track->tuning.setTune(0,64);
    track->tuning.setTune(1,59);
    track->tuning.setTune(2,55);
    track->tuning.setTune(3,50);
    track->tuning.setTune(4,45);
    track->tuning.setTune(5,40);

    for (ul barI=0; barI < pTab->getV(0)->len(); ++barI) {
        Bar *bar=new Bar();
        bar->flush();
        bar->setSignDenum(4); bar->setSignNum(4);
        bar->setRepeat(0);
        Beat *beat=new Beat();
        beat->setPause(true);
        beat->setDotted(0);
        beat->setDuration(3);
        beat->setDurationDetail(0);
        bar->add(beat);
        track->add(bar);
    }
    pTab->add(track);
    pTab->connectTracks();
    return track;
}


void deleteTrack(Tab* pTab, size_t& displayTrack) {
    bool ok=false;
    int inp = QInputDialog::getInt(0,"Delete track","Delete track",0,0,1,1,&ok);

    if ((ok) && (inp))
    {
        pTab->remove(displayTrack);

        if (displayTrack)
            --displayTrack;
    }
}

void midiPause(bool& isPlaying) {
    if (isPlaying == false) {
        MidiEngine::closeDefaultFile();
        MidiEngine::openDefaultFile();
        MidiEngine::startDefaultFile();
        isPlaying = true;
    }
    else {
        MidiEngine::stopDefaultFile();
        isPlaying = false;
    }
}

void setMarker(Bar* fromFirstTrack) {
    bool ok=false;
    QString markerText= QInputDialog::getText(0,"Input",
                         "Marker:", QLineEdit::Normal,"untitled",&ok);
    if (ok) {
        std::string stdMarkerText = markerText.toStdString();
        fromFirstTrack->setGPCOMPMarker(stdMarkerText,0);
    }
}


void openReprise(Bar *firstTrackBar) {
    byte repeat = firstTrackBar->getRepeat();
    byte repeatOpens = repeat & 1;
    byte repeatCloses = repeat & 2;
    if (repeatOpens){
        firstTrackBar->setRepeat(0); //flush
        firstTrackBar->setRepeat(repeatCloses);
    }
    else
        firstTrackBar->setRepeat(1);
}

void closeReprise(Bar *firstTrackBar) {
    byte repeat = firstTrackBar->getRepeat();
    byte repeatOpens = repeat & 1;
    byte repeatCloses = repeat & 2;
    if (repeatCloses) {
        firstTrackBar->setRepeat(0); //flush
        firstTrackBar->setRepeat(repeatOpens);
    }
    else {
        bool ok=false;
        int newTimes = QInputDialog::getInt(0,"Input", "Repeat times:",
                            QLineEdit::Normal,2,99,1,&ok);
        if ((ok)&&(newTimes))
            firstTrackBar->setRepeat(2,newTimes);
    }
}

void goToBar(size_t trackLen, size_t& currentBar, size_t& displayBar) {
    bool ok=false; //TODO позже разделить Qt запросы и установку параметров
    int newTimes = QInputDialog::getInt(0,"Input",
                         "Bar to jump:", QLineEdit::Normal, 1, trackLen, 1, &ok);
    if (ok) {
        --newTimes;
        currentBar = newTimes;
        displayBar = newTimes;
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


//TODO think a while on how better use those functions
//WE need sepparete engine with Qt

void Tab::onTabCommand(TabCommand command) {
    if (command == TabCommand::SetSignTillEnd)  //TODO хэндлеры для более простого вызова
        setSignTillEnd(this, currentBar);
    else if (command == TabCommand::SaveAs)
        saveAs(this);
    else if (command == TabCommand::Mute)
        muteTrack(this, displayTrack);
    else if (command == TabCommand::Solo)
        soloTrack(this, displayTrack);
    else if (command == TabCommand::MoveRight)
        moveCursorInTrack(MoveDirections::right, displayBar, this->getV(0)->len() - 1); //TODO перероверить
    else if (command == TabCommand::MoveLeft)
        moveCursorInTrack(MoveDirections::left, displayBar);
    else if (command == TabCommand::MoveUp)
        MoveCursorOfTrack(MoveDirections::up, displayTrack, currentTrack);
    else if (command == TabCommand::MoveDown)
        MoveCursorOfTrack(MoveDirections::down, displayTrack, currentTrack, this->getV(0)->len());
    else if (command == TabCommand::Drums)
        changeDrumsFlag(this->getV(displayTrack));
    else if (command == TabCommand::Volume)
        changeTrackVolume(this->getV(displayTrack));
    else if (command == TabCommand::Name)
        changeTrackName(this->getV(displayTrack));
    else if (command == TabCommand::DeleteTrack)
        deleteTrack(this, displayTrack);
    else if (command == TabCommand::PauseMidi)
        midiPause(isPlaying);
    else if (command == TabCommand::AddMarker)
        setMarker(this->getV(0)->getV(currentBar));
    else if (command == TabCommand::OpenReprise)
        openReprise(this->getV(0)->getV(currentBar));
    else if (command == TabCommand::CloseReprise)
        closeReprise(this->getV(0)->getV(currentBar));
    else if (command == TabCommand::GotoBar)
        goToBar(this->getV(0)->len(), currentBar, displayBar);
    //if (press == "alt");//TODO
    else if (command == TabCommand::Tune)
        setTune(this->getV(currentTrack));
}
