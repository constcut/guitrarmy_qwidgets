#include "tab.h"

#include "g0/midifile.h"
#include "g0/gmyfile.h"
#include "g0/aexpimp.h"
#include "g0/gtpfiles.h"
#include "midiengine.h"

#include <fstream>
#include <QDebug>

//TODO rename file when old tab commands would be erased
//TODO prepare undo operations
//They have to be for all possible commands

void Tab::setSignsTillEnd(int num, int denom) {
    for (size_t i = currentBar; i < this->at(0)->size(); ++i){
        this->at(0)->at(i)->setSignDenum(denom); //TODO проработать размеры, ументь их делать общими для табы, и делать полиритмию
        this->at(0)->at(i)->setSignNum(num);
    }
}

void Tab::muteTrack() { //Move into Tab
    byte curStat = this->at(displayTrack)->getStatus();
    if (curStat==1)
        this->at(displayTrack)->setStatus(0);
    else
        this->at(displayTrack)->setStatus(1);
}


void Tab::soloTrack() { //Move into Tab
    byte curStat = this->at(displayTrack)->getStatus();
    if (curStat==2)
        this->at(displayTrack)->setStatus(0);
    else
        this->at(displayTrack)->setStatus(2);
}

void Tab::moveCursorInTrackRight() {
    if (displayBar < at(0)->size() - 1)
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
    if (displayTrack < size()){
        ++displayTrack;
        if (currentTrack < displayTrack)
            currentTrack = displayTrack;
    }
}


void Tab::changeDrumsFlag() {
    Track* pTrack = this->at(displayTrack);
    bool drums = pTrack->isDrums();
    drums = !drums;
    pTrack->setDrums(drums);
}



void Tab::changeTrackVolume(int newVol) {
     at(currentTrack)->setVolume(newVol);
}


void Tab::changeTrackName(std::string newName) {
    at(currentTrack)->setName(newName);
}


void Tab::changeTrackInstrument(int val) {
    at(currentTrack)->setInstrument(val);
}

void Tab::changeTrackPanoram(int val) {
    at(currentTrack)->setPan(val);
}

Track* Tab::createNewTrack() { //Move into Tab
    Tab* pTab = this;
    Track *track=new Track();
    track->setParent(pTab);
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

    for (size_t barI=0; barI < pTab->at(0)->size(); ++barI) {
        Bar *bar=new Bar();
        bar->flush();
        bar->setSignDenum(4); bar->setSignNum(4);
        bar->setRepeat(0);
        Beat *beat=new Beat();
        beat->setPause(true);
        beat->setDotted(0);
        beat->setDuration(3);
        beat->setDurationDetail(0);
        bar->push_back(beat);
        track->push_back(bar);
    }
    pTab->push_back(track);
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
    Bar* fromFirstTrack = at(0)->at(currentBar);
    fromFirstTrack->setGPCOMPMarker(text,0);
}


void Tab::openReprise() {
    Bar *firstTrackBar = this->at(0)->at(currentBar);
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
    Bar *firstTrackBar = this->at(0)->at(currentBar);
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
    if (handlers.count(command))
        (this->*handlers.at(command))();
}

//TODO возможно дополнить установку инструмента, панорамы и громкости, чтобы не нужно было обращаться к текущему треку, а использовать "вшитые курсоры"
