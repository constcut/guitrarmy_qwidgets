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


void Tab::setSignsTillEnd(int num, int denom) {
    for (size_t i = currentBar; i < this->getV(0)->len(); ++i){
        this->getV(0)->getV(i)->setSignDenum(denom); //TODO проработать размеры, ументь их делать общими для табы, и делать полиритмию
        this->getV(0)->getV(i)->setSignNum(num);
    }
}


void Tab::muteTrack() { //Move into Tab
    byte curStat = this->getV(displayTrack)->getStatus();
    if (curStat==1)
        this->getV(displayTrack)->setStatus(0);
    else
        this->getV(displayTrack)->setStatus(1);
}


void Tab::soloTrack() { //Move into Tab
    byte curStat = this->getV(displayTrack)->getStatus();
    if (curStat==2)
        this->getV(displayTrack)->setStatus(0);
    else
        this->getV(displayTrack)->setStatus(2);
}


void Tab::moveCursorInTrackRight() {
    if (displayBar < getV(0)->len() - 1)
        ++displayBar;
}

void Tab::moveCursorInTrackLeft() {
    if (displayBar > 0)
        --displayBar;
}



void Tab::moveCursorOfTrackUp() {
    if (displayTrack > 0) {
        --displayTrack;
        currentTrack = displayTrack;
    }
}

void Tab::moveCursorOfTrackDown() {
    if (displayTrack < len()){
        ++displayTrack;
        if (currentTrack < displayTrack)
            currentTrack = displayTrack;
    }
}


bool Tab::changeDrumsFlag() {
    Track* pTrack = this->getV(displayTrack);
    bool drums = pTrack->isDrums();
    drums = !drums;
    pTrack->setDrums(drums);
    return drums;
}



void Tab::changeTrackVolume(int newVol) {
     getV(currentTrack)->setVolume(newVol);
}


void Tab::changeTrackName(std::string newName) {
    getV(currentTrack)->setName(newName);
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



void Tab::deleteTrack() {
    this->remove(displayTrack);
    if (displayTrack)
        --displayTrack;
}


void Tab::midiPause() {
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



void Tab::setMarker(std::string text) {
    Bar* fromFirstTrack = getV(0)->getV(currentBar);
    fromFirstTrack->setGPCOMPMarker(text,0);
}


void Tab::openReprise() {
    Bar *firstTrackBar = this->getV(0)->getV(currentBar);
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



void Tab::closeReprise(size_t count) { //TODO argument repeat times
    Bar *firstTrackBar = this->getV(0)->getV(currentBar);
    byte repeat = firstTrackBar->getRepeat();
    byte repeatOpens = repeat & 1;
    byte repeatCloses = repeat & 2;
    if (repeatCloses) {
        firstTrackBar->setRepeat(0); //flush
        firstTrackBar->setRepeat(repeatOpens);
    }
    else {
        if (count)
            firstTrackBar->setRepeat(2, count);
    }
}


void Tab::gotoBar(size_t pos) {
    currentBar = pos;
    displayBar = pos;
}

void Tab::saveAs(std::string filename) {
    std::ofstream file(filename);
    GmyFile gmyFile;
    gmyFile.saveToFile(&file, this);
    file.close();
}



void Tab::onTabCommand(TabCommand command) {
    if (command == TabCommand::Mute)
        muteTrack();
    else if (command == TabCommand::Solo)
        soloTrack();
    else if (command == TabCommand::MoveRight)
        moveCursorInTrackRight();
    else if (command == TabCommand::MoveLeft)
        moveCursorInTrackLeft();
    else if (command == TabCommand::MoveUp)
        moveCursorOfTrackUp();
    else if (command == TabCommand::MoveDown)
        moveCursorOfTrackDown();
    else if (command == TabCommand::Drums)
        changeDrumsFlag();
    else if (command == TabCommand::PauseMidi)
        midiPause();
    else if (command == TabCommand::OpenReprise)
        openReprise();
}
