#include "tabcommands.h"

#include "tabviews.h"
#include "mainviews.h"

//other dep
#include "g0/midifile.h"
#include "g0/gmyfile.h"
#include "g0/aexpimp.h"
#include "g0/gtpfiles.h"

#include <QInputDialog>
#include <QMutex>
#include <QFileDialog>
#include <QApplication>
#include <QScreen>

#include <unordered_map>

#include "midiengine.h"

#include "libtim/miditopcm.h"

#include <QDebug>


#include <fstream>


void TrackView::reverseCommand(SingleCommand &command)
{
    byte type = command.getType();
    byte value = command.getValue();
    byte value2 = command.getValue2();

    //byte trackN = command.getTrackNum();

    int barN = command.getBarNum();
    int beatN = command.getBeatNum();
    byte stringN = command.getStringNum();

    if (type == 1) //eff
    {
        int ind = value;
        bool effect = pTrack->getV(barN)->getV(beatN)->getNote(stringN)->effPack.get(ind);
        effect = !effect;
        pTrack->getV(barN)->getV(beatN)->getNote(stringN)->effPack.set(ind,effect);
    }

    if (type == 2) //beat eff
    {
        int ind = value;
        bool effect = pTrack->getV(barN)->getV(beatN)->effPack.get(ind);
        effect = !effect;
        pTrack->getV(barN)->getV(beatN)->effPack.set(ind,effect);
    }

    if (type == 3) //fret
    {
        if (value != 255)
            pTrack->getV(barN)->getV(beatN)->setFret(value,stringN);
        else
            pTrack->getV(barN)->getV(beatN)->deleteNote(stringN);

    }

    if (type == 4) //duration
    {
        pTrack->getV(barN)->getV(beatN)->setDuration(value);
    }

    if (type == 5) //detail
    {
        pTrack->getV(barN)->getV(beatN)->setDurationDetail(value);
    }

    if (type == 6) //dot
    {
        pTrack->getV(barN)->getV(beatN)->setDotted(value);
    }

    if (type == 7) //pause
    {
        if (command.storedNotes)
        {
            for (size_t i = 0; i < command.storedNotes->size(); ++i)
            {
                Note *note = command.storedNotes->operator [](i);
                pTrack->getV(barN)->getV(beatN)->add(note);
            }

            pTrack->getV(barN)->getV(beatN)->setPause(false);

            command.releaseStoredNotes();
        }
    }

    if (type == 8)
    {
        if (command.storedNotes) //delete note
        {
            Note *note = command.storedNotes->operator [](0);
            pTrack->getV(barN)->getV(beatN)->add(note);

            pTrack->getV(barN)->getV(beatN)->setPause(false);

            command.releaseStoredNotes();
        }
        else //deleted beat
        {
            Beat *beat = new Beat();

            byte dur = value&7;
            byte rhythmDetail = value & 0x78; //4 bits after first 3
            rhythmDetail>>=3;

            byte dotAppear = stringN & 3; //wow

            beat->setPause(true);
            beat->setDotted(dotAppear);
            beat->setDuration(dur);
            beat->setDurationDetail(rhythmDetail);

            pTrack->getV(barN)->insertBefore(beat,beatN);
            pTrack->connectAll(); //oups?
        }
    }

    if (type == 16)
    {
        int len = beatN; //param
        if (!len)
        { //as len = 1 but default
            pTrack->remove(barN);
            cursor = displayIndex;
            tabParrent->setCurrentBar(cursor);
        }
        else
        {
            for (int i = 0; i < len; ++i)
                pTrack->remove(barN);

            cursor = displayIndex;
            tabParrent->setCurrentBar(cursor);
        }
        if (cursor)
            --cursor;
    }

    if (type == 17)
    {
        Note *note = (pTrack->getV(barN)->getV(beatN)->getNote(stringN));
        note->setState(value);
    }

    if (type == 18)
    {
        pTrack->getV(barN)->remove(beatN);
        pTrack->connectAll();
        if (cursorBeat)
            --cursorBeat;
        //may be shift cursorBeat (when activate <> undo)
    }

    if (type == 19)
    {
        pTrack->getV(barN)->setSignDenum(value);
        pTrack->getV(barN)->setSignNum(value2);
    }

    if (type == 24)
    {
        int len = beatN;
        if (!len)
        {
            Bar *addition = (Bar*)(command.outerPtr);
            pTrack->insertBefore(addition,barN);
            pTrack->connectAll();
        }
    }

    if (type == 25)
    {
        Bar *addition = (Bar*)(command.outerPtrEnd);
        Bar *firstBar  = (Bar*)(command.outerPtr);
        //sicky
        for (;addition != firstBar; addition=(Bar*)addition->getPrev())
        {
            if (addition == 0)
                break;

            qDebug() << "Addition addr "<<(addition);

            pTrack->insertBefore(addition,barN);
        }
        pTrack->insertBefore(firstBar,barN);
        pTrack->connectAll();
        cursor=barN;
    }

    if (type == 26)
    {
        Beat *firstBeat = (Beat*)(command.outerPtr);
        Beat *lastBeat  = (Beat*)(command.outerPtrEnd);
        Bar *firstPa = (Bar*)(firstBeat->getParent());
        Bar *lastPa = (Bar*)(lastBeat->getParent());//s

        Beat *curBeat = lastBeat;

        if (firstPa == lastPa)
        {
            if (value && value2)
                pTrack->insertBefore(firstPa,barN);
            else
            {
                while(curBeat != firstBeat)
                {
                    pTrack->getV(barN)->insertBefore(curBeat,beatN);
                    curBeat = (Beat*)curBeat->getPrev();
                    if (curBeat == 0)
                        break;
                }
                if (curBeat==firstBeat)
                    pTrack->getV(barN)->insertBefore(curBeat,beatN);
            }
        }
        else
        {
            if (value==0)
                ++barN; //sift when first not full

            Bar *curBar = (Bar*)lastPa->getPrev();

            if (value2)
            {
                //last pa if full
                pTrack->insertBefore(lastPa,barN);
            }
            else
            {
                //int indexInBar = -1;

                while(curBeat->getParent() == lastPa)
                {
                    pTrack->getV(barN)->insertBefore(curBeat,0);
                    curBeat = (Beat*)curBeat->getPrev();
                    if (curBeat == 0)
                    {
                        //what is this
                        if(curBar)
                        curBeat = curBar->getV(curBar->len()-1); //issuepossible
                        break;
                    }
                }
            }


           //middle part - if there is particular cut - then
           //then we have pointers to alive bars

           //could find their indexes

            Bar *firstInSeq = command.startBar;
            Bar *lastInSeq  = command.endBar;

            if ((lastInSeq != firstPa) || (firstInSeq != lastPa))
            {
                Bar *currentBar = lastInSeq;

                while (currentBar != firstInSeq)
                {
                    pTrack->insertBefore(currentBar,barN);

                    currentBar = (Bar*)currentBar->getPrev();
                    if (currentBar==0)
                        break;
                }

                if (currentBar == firstInSeq)
                    pTrack->insertBefore(currentBar,barN);
            }



           if (value)
           {    //first is full

               pTrack->insertBefore(firstPa,barN);
           }
           else
           {
               --barN;
               //curBeat = curBeat->getPrev(); //slide into first pa property


               curBeat = firstBeat;
               //insert here
               int counter = 0;

               if (curBeat)
               while (curBeat->getParent() == firstPa)
               {
                   pTrack->getV(barN)->
                           insertBefore(curBeat,
                                        pTrack->getV(barN)->len()
                                                    -counter);

                   ++counter;
                   curBeat = (Beat*)curBeat->getNext();
                   if (curBeat==0)
                       break;
               }
           }
        }

       pTrack->connectAll();
       cursor=barN;
       if (displayIndex>cursor)
           displayIndex=cursor;
       //from here till first beat insert at barN

    }
}

void TrackView::switchEffect(int effIndex)
{
    if (pTrack->getV(cursor)->getV(cursorBeat)->getPause())
        return;

    if (pTrack->getV(cursor)->getV(cursorBeat)->len()==0)
        return;

    if (tabParrent->getPlaying())
        return;

    int ind = effIndex;
    //check for pause

    Note *theNote = pTrack->getV(cursor)->getV(cursorBeat)->getNote(stringCursor+1);
    if (theNote)
    {
        bool effect = theNote->effPack.get(ind);
        effect = !effect;
        pTrack->getV(cursor)->getV(cursorBeat)->getNote(stringCursor+1)->effPack.set(ind,effect);

        SingleCommand command(1,effIndex); //note effect
        command.setPosition(0,cursor,cursorBeat,stringCursor+1);
        commandSequence.push_back(command);
    }
}

void TrackView::switchBeatEffect(int effIndex)
{
    if (pTrack->getV(cursor)->getV(cursorBeat)->getPause())
        return;

    if (tabParrent->getPlaying())
        return;

    int ind = effIndex;
    //check for pause
    bool effect = pTrack->getV(cursor)->getV(cursorBeat)->effPack.get(ind);
    effect = !effect;
    pTrack->getV(cursor)->getV(cursorBeat)->effPack.set(ind,effect);

    SingleCommand command(2,effIndex); //beat effect
    command.setPosition(0,cursor,cursorBeat);
    commandSequence.push_back(command);
}

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
    {
        pTrack->setInstrument(itemNum);
    }
    if (combo=="2")
    {
        pTrack->setVolume(itemNum);
    }
    if (combo=="5")
    {
        pTrack->setStatus(itemNum);
    }
    if (combo=="6")
    {
        pTrack->setPan(itemNum);
    }
}

void gotoTrackStart(size_t& cursorBeat, size_t& cursor, size_t& displayIndex) {
    cursor = 0;
    cursorBeat = 0;
    displayIndex = 0;
}


void changeBarSigns(Track* pTrack, int&  selectionBarFirst, int& selectionBarLast) {
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
        if ((selectionBarFirst != -1) && (selectionBarLast != -1))
           for (int i = selectionBarFirst; i <= selectionBarLast; ++i) {
               pTrack->getV(i)->setSignNum(newNum);
               pTrack->getV(i)->setSignDenum(newDen);
           }
}

void moveSelectionLeft(Track *pTrack, int& selectionBeatFirst, int& selectionBarFirst) {
    if (selectionBeatFirst)
        --selectionBeatFirst;
    else
    {
        if (selectionBarFirst)
        {
            --selectionBarFirst;
            selectionBeatFirst = pTrack->getV(selectionBarFirst)->len()-1;
        }
    }
}

void moveSelectionRight(Track *pTrack, int& selectionBeatLast, int& selectionBarLast) {
    if (selectionBarLast >= 0)
    {
        if (selectionBeatLast < (pTrack->getV(selectionBarLast)->len()-1)) //TODO лучше способ хранить зоны выделенности (флаг вкюченности и size_t)
            ++selectionBeatLast;
        else
            if (selectionBarLast < (pTrack->len()-1))
            {
                ++selectionBarLast;
                selectionBeatLast = 0;
            }
    }
}

void insertBar(Track *pTrack, size_t cursor, size_t cursorBeat,  std::vector<SingleCommand>& commandSequence) {
    Beat *beat=new Beat();
    beat->setPause(true);
    beat->setDuration(4);
    beat->setDotted(0);
    beat->setDurationDetail(0);

    pTrack->getV(cursor)->insertBefore(beat,cursorBeat);
    pTrack->connectAll(); //autochain cries

    SingleCommand command(18);
    command.setPosition(0,cursor,cursorBeat);
    commandSequence.push_back(command);
    return;
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



    if ( pTrack->getV(cursor)->len() > cursorBeat ) {
        byte lastFret = pTrack->getV(cursor)->getV(cursorBeat)->getFret(stringCursor+1);

        SingleCommand command(3,lastFret);
        command.setPosition(0,cursor,cursorBeat,stringCursor+1);
        commandSequence.push_back(command);
        pTrack->getV(cursor)->getV(cursorBeat)->setFret(digitPress,stringCursor+1);
        Note *inputedNote =  pTrack->getV(cursor)->getV(cursorBeat)->getNote(stringCursor+1);
        byte tune = pTrack->tuning.getTune(stringCursor);
        int chan = 0;
        if (pTrack->isDrums())
        {
            chan = 9; //tune to 0 attention refact error
            tune = 0;
        }
        byte midiNote = inputedNote->getMidiNote(tune);
        MidiEngine::sendSignalShort(0x90|chan,midiNote,120);
       //attention something gone wrong there - recheck on local
        ///MidiEngine::sendSignalShortDelay(250,0x80|chan,midiNote,120);
        //MidiEngine::sendSignalShortDelay(750,0x90|chan,midiNote+2,120);
    }
}



void moveToNextBar(size_t& cursor, Track* pTrack, size_t& cursorBeat, size_t& displayIndex, size_t& stringCursor, int& digitPress, size_t& lastSeen) {
    if ((cursor+1) != pTrack->len())
    {
        ++cursor;
        cursorBeat = 0;

        if (cursor > (lastSeen-1))
            displayIndex = cursor;

        if (pTrack->getV(cursor)->getV(cursorBeat)->getPause() == false)
            stringCursor = pTrack->getV(cursor)->getV(cursorBeat)->getV(0)->getStringNumber()-1;

    }

    digitPress=-1; // flush input after movement
}


void moveToPrevBar(size_t& cursor, Track* pTrack, size_t& cursorBeat, size_t& displayIndex, size_t& stringCursor, int& digitPress) {
    if (cursor>0)
    {
        --cursor;
        cursorBeat = 0;

        if (cursor < displayIndex)
            displayIndex = cursor;

        if (pTrack->getV(cursor)->getV(cursorBeat)->getPause() == false)
            stringCursor = pTrack->getV(cursor)->getV(cursorBeat)->getV(0)->getStringNumber()-1;

    }

    digitPress=-1; // flush input after movement
}


void moveToPrevPage(size_t& cursor, TabView* tabView, size_t& cursorBeat, size_t& displayIndex, int& digitPress) {
    if (displayIndex > 7) {
        displayIndex -= 7; //not real paging
        cursor = displayIndex;
        cursorBeat = 0;
        digitPress = -1;
        tabView->setCurrentBar(cursor); //QT dependency отвязать TODO
    }
}


void moveToNextPage(size_t& cursor, TabView* tabView, Track *pTrack, size_t& cursorBeat, size_t& displayIndex, int& digitPress, size_t& lastSeen) {
    if ((lastSeen+1) <= pTrack->len()) {
        displayIndex = lastSeen+1;
        cursor = displayIndex;
        cursorBeat = 0;
        digitPress = -1;
        tabView->setCurrentBar(cursor);
    }
}


void moveToNextTrack(TabView* tabParrent, int& digitPress) {
    int trackInd = tabParrent->getLastOpenedTrack();
    ++trackInd;
    char pressImit[2]={0};
    pressImit[0]=trackInd+49;
    std::string pressIm = pressImit;
    //refact for open function
    tabParrent->keyevent(pressIm);
    digitPress = -1;
    return;
}


void moveToPrevTrack(TabView* tabParrent, int& digitPress) {
    int trackInd = tabParrent->getLastOpenedTrack();
    --trackInd;
    char pressImit[2]={0};
    pressImit[0]=trackInd+49;
    std::string pressIm = pressImit;
    //refact for open function
    tabParrent->keyevent(pressIm);
    digitPress = -1;
}

void moveToStringUp(Track* pTrack, size_t& stringCursor, int& digitPress) {
    if ((stringCursor+1) < pTrack->tuning.getStringsAmount())
        ++stringCursor;
    digitPress=-1; // flush input after movement
    return;

}

void moveToStringDown(Track* pTrack, size_t& stringCursor, int& digitPress) {
    if (stringCursor > 0)
        --stringCursor;
    digitPress=-1; // flush input after movement
}


void moveToPrevBeat(size_t& cursorBeat, size_t& cursor, size_t& displayIndex, size_t& stringCursor, int& digitPress, Track* pTrack) {
    //scrol if out of bar
    if (cursorBeat==0)
    {
        if (cursor)
        {
            --cursor;
            if (cursor < displayIndex)
                displayIndex = cursor;
               cursorBeat = pTrack->getV(cursor)->len()-1;
        }
    }
    else
        --cursorBeat;

    if (pTrack->getV(cursor)->getV(cursorBeat)->getPause() == false)
        stringCursor = pTrack->getV(cursor)->getV(cursorBeat)->getV(0)->getStringNumber()-1;

    digitPress=-1; // flush input after movement
}

void moveToNextBeat(size_t& cursorBeat, size_t& cursor, size_t& displayIndex, size_t& stringCursor, int& digitPress, size_t& lastSeen, Track* pTrack,
                     std::vector<SingleCommand>& commandSequence) {
    ++cursorBeat;
    if ((cursorBeat) >= pTrack->getV(cursor)->len())
    {
        if (1) //pan->isOpenned())
        {
            static int lastDur = 4; //TODO?
            if (cursorBeat) {
                Bar *b = pTrack->getV(cursor);
                Beat *beat = b->getV(b->len()-1);
                lastDur = beat->getDuration();
                //THERE IS A GOOOD CHANCE TO RECOUNT AGAIN
                /// lastDur from prev position
            }
            if (pTrack->getV(cursor)->getCompleteStatus()==1)
            {
                Bar *bar = pTrack->getV(cursor);
                Beat *beat=new Beat();
                beat->setPause(true);
                beat->setDuration(lastDur);
                beat->setDotted(0);
                beat->setDurationDetail(0);
                bar->add(beat);

                SingleCommand command(18);
                command.setPosition(0,cursor,cursorBeat);
                commandSequence.push_back(command);

                ///ADD COMMAND              - TASK!!!!!!!!!!!!
            }
            else //in edit mode - else add new bar
            //scrol if out of bar
            {
                if ((cursor+1) == pTrack->len())
                {
                    Bar *newBar = new Bar();
                    newBar->flush();
                    newBar->setSignDenum(4);
                    newBar->setSignNum(4);
                    newBar->setRepeat(0);

                    Beat *beat=new Beat();
                    beat->setPause(true);
                    beat->setDuration(lastDur);
                    beat->setDotted(0);
                    beat->setDurationDetail(0);
                    newBar->add(beat);
                    pTrack->add(newBar);

                    SingleCommand command(16);
                    command.setPosition(0,cursor+1,0);
                    commandSequence.push_back(command);

                    ++lastSeen;
                    cursorBeat = 0;
                    ++cursor;
                }
                else
                {
                    if ((cursor+1) != pTrack->len()) {
                        ++cursor;
                       if (cursor > (lastSeen-1))
                            displayIndex = cursor;
                        cursorBeat = 0;
                     }
                        else
                        --cursorBeat;
                }
            }
        }
        else //TODO view mode maybe remove?
        {
            if ((cursor+1) != pTrack->len())
              {
                ++cursor;
                if (cursor > (lastSeen-1))
                    displayIndex = cursor;
                cursorBeat = 0;
              }
                else
                --cursorBeat;
        }
    }

    if (pTrack->getV(cursor)->getV(cursorBeat)->getPause() == false)
        stringCursor = pTrack->getV(cursor)->getV(cursorBeat)->getV(0)->getStringNumber()-1;
        //need acces
    digitPress=-1; // flush input after movement
    return;
}


void setTrackPause(size_t cursor, size_t cursorBeat, int& digitPress, Track* pTrack, std::vector<SingleCommand>& commandSequence) {
    SingleCommand command(7);
    command.setPosition(0,cursor,cursorBeat);
    command.requestStoredNotes();
    for (ul i = 0; i < pTrack->getV(cursor)->getV(cursorBeat)->len(); ++i) {
        Note *note = pTrack->getV(cursor)->getV(cursorBeat)->getV(i);
        command.storedNotes->push_back(note);
    }
    commandSequence.push_back(command);
    pTrack->getV(cursor)->getV(cursorBeat)->setPause(true);
    pTrack->getV(cursor)->getV(cursorBeat)->clear();
    digitPress = -1;
}


void deleteBar(size_t& cursor, Track* pTrack, std::vector<SingleCommand>& commandSequence) {
    SingleCommand command(24);
    command.setPosition(0,cursor,0);
    command.outerPtr = pTrack->getV(cursor);
    commandSequence.push_back(command);

    //attention question for memoryleaks
    pTrack->remove(cursor);
    pTrack->connectAll(); //should go to auto state later

    if (cursor > 0)
       --cursor;
    return;
}


void deleteSelectedBars(size_t& cursor, Track* pTrack, std::vector<SingleCommand>& commandSequence,
                        int& selectionBarFirst, int& selectionBarLast, int& selectionBeatFirst, int& selectionBeatLast) {
    //yet no commands
    //so - no undo
    if (selectionBarFirst != -1)
    {
        if (selectionBarFirst > 0)
            --cursor; //attention

        SingleCommand command(25);
        command.setPosition(0,selectionBarFirst,0);
        command.outerPtr = pTrack->getV(selectionBarFirst);
        command.outerPtrEnd = pTrack->getV(selectionBarLast);
        commandSequence.push_back(command);

        for (int i = selectionBarLast; i >= selectionBarFirst; --i)
            pTrack->remove(i);

        pTrack->connectAll();
    }
    selectionBarFirst=selectionBarLast=selectionBeatFirst=selectionBeatLast=-1;
}


void deleteSelectedBeats(size_t& cursor, Track* pTrack, std::vector<SingleCommand>& commandSequence,
                         int& selectionBarFirst, int& selectionBarLast, int& selectionBeatFirst, int& selectionBeatLast) {
    if (selectionBarFirst != -1) {

        pTrack->connectAll();
        Beat *firstBeat = nullptr, *lastBeat = nullptr;
        SingleCommand command(26);
        command.setPosition(0,selectionBarFirst,selectionBeatFirst);
        command.outerPtr = firstBeat = pTrack->getV(selectionBarFirst)->getV(selectionBeatFirst);
        command.outerPtrEnd = lastBeat = pTrack->getV(selectionBarLast)->getV(selectionBeatLast);
        pTrack->getV(selectionBarFirst)->getV(selectionBeatFirst)->setParent(pTrack->getV(selectionBarFirst));
        pTrack->getV(selectionBarLast)->getV(selectionBeatLast)->setParent(pTrack->getV(selectionBarLast));

        //chick chick pointers - line between should stay - and here the way
        //to do undo

        bool wholeFirst = false;
        bool wholeLast = false;

        if (selectionBeatFirst==0)
            wholeFirst = true;

        if (selectionBeatLast == pTrack->getV(selectionBarLast)->len()-1)
            wholeLast = true;

        command.setValue(wholeFirst);
        command.setValue2(wholeLast);

        if (selectionBarFirst == selectionBarLast)
        {
            //remove from single bar
            if (wholeFirst && wholeLast)
                pTrack->remove(selectionBarFirst);
            else
                for (int bI = selectionBeatLast; bI >= selectionBeatFirst; --bI)
                    pTrack->getV(selectionBarFirst)->remove(bI);

        }
        else
        { //first and last remove depending on condition
            if (wholeLast)
            {
                pTrack->remove(selectionBarLast);
            }
            else
            {
                for (int bI = selectionBeatLast; bI >= 0; --bI)
                    pTrack->getV(selectionBarLast)->remove(bI);
            }

            ///GET range of bars
            Bar *lastBarInMiddle = pTrack->getV(selectionBarLast-1);
            Bar *firstBarInMiddle = pTrack->getV(selectionBarFirst+1);

            command.startBar = firstBarInMiddle;
            command.endBar = lastBarInMiddle;

            for (int bI = selectionBarLast-1; bI > selectionBarFirst; --bI)
                pTrack->remove(bI);

            if (wholeFirst)
            {
                pTrack->remove(selectionBarFirst);
            }
            else
            {
                for (int bI = pTrack->getV(selectionBarFirst)->len()-1; bI >= selectionBeatFirst; --bI)
                    pTrack->getV(selectionBarFirst)->remove(bI);
            }
        }

        commandSequence.push_back(command);
        pTrack->connectAll();
    }
    selectionBarFirst=selectionBarLast=selectionBeatFirst=selectionBeatLast=-1;
}


void deleteNote(Track* pTrack, size_t cursor, size_t& cursorBeat, size_t stringCursor, int& digitPress, std::vector<SingleCommand>& commandSequence) {
    if (pTrack->getV(cursor)->getV(cursorBeat)->len())
    {
        SingleCommand command(8);
        command.setPosition(0,cursor,cursorBeat);
        command.requestStoredNotes();
        Note *note = pTrack->getV(cursor)->getV(cursorBeat)->getNoteInstance(stringCursor+1);
        command.storedNotes->push_back(note);

        if (note->getFret()!=255) {
            //delete one note
            pTrack->getV(cursor)->getV(cursorBeat)->deleteNote(stringCursor+1);//shift from 0 to 1
            commandSequence.push_back(command);
        }
        else
            delete note;
    }
    else
    {
        if (pTrack->getV(cursor)->len() > 1) {
            byte packedValue = 0;
            byte dur = pTrack->getV(cursor)->getV(cursorBeat)->getDuration();
            byte det =  pTrack->getV(cursor)->getV(cursorBeat)->getDurationDetail();
            byte dot =  pTrack->getV(cursor)->getV(cursorBeat)->getDotted();
            packedValue = dur;
            packedValue |= det<<3;
            Beat *beat = pTrack->getV(cursor)->getV(cursorBeat);
            pTrack->getV(cursor)->remove(cursorBeat);
            pTrack->connectAll(); //oups?
            delete beat;//cleanup

            SingleCommand command(8,packedValue);
            command.setPosition(0,cursor,cursorBeat,dot); //wow wow know it
            commandSequence.push_back(command);

            if (cursorBeat)
                --cursorBeat;
        }
    }

    digitPress = -1;
    return;
}


void incDuration(Track* pTrack, size_t cursor, size_t cursorBeat, std::vector<SingleCommand>& commandSequence) {
    byte beatDur = pTrack->getV(cursor)->getV(cursorBeat)->getDuration();

    SingleCommand command(4,beatDur);
    command.setPosition(0,cursor,cursorBeat);
    commandSequence.push_back(command);

    if (beatDur)
     --beatDur;
    //block not go out
    pTrack->getV(cursor)->getV(cursorBeat)->setDuration(beatDur);
    return;
}


void decDuration(Track* pTrack, size_t cursor, size_t cursorBeat, std::vector<SingleCommand>& commandSequence) {
    byte beatDur = pTrack->getV(cursor)->getV(cursorBeat)->getDuration();

    SingleCommand command(4,beatDur);
    command.setPosition(0,cursor,cursorBeat);
    commandSequence.push_back(command);

    if (beatDur < 6)
    ++beatDur;
    //block not go out
    pTrack->getV(cursor)->getV(cursorBeat)->setDuration(beatDur);
    return;
}


void playTrack(TabView* tabParrent, ThreadLocal* localThr, size_t& cursorBeat, size_t cursor, Track* pTrack, MasterView* mw) { //TODO объединить - воспроизведение должно быть из одного источника запускаться

    if (tabParrent->getPlaying()==true)
    {
        if (localThr)
           if (localThr->getStatus())
            {
                //animation stopped
                tabParrent->setPlaying(false);
                //cursor = displayIndex; //auto repeat from page
                cursorBeat = 0;
            }
    }

    if (tabParrent->getPlaying() == false)
    {
        //to start not from begin always
        ul shiftTheCursor = 0;
        if (cursor != 0){
            Bar *barPtr = pTrack->getV(cursor);

            for (ul i = 0; i < pTrack->timeLoop.len();++i){
                 if (pTrack->timeLoop.getV(i) == barPtr){
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
            MidiToPcm generator(CONF_PARAM("midi.config"));
            std::string outputSound = getTestsLocation() + std::string("waveOutput.wav");
            generator.convert(fullOutName,outputSound);
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


void saveFromTrack(TabView* tabParent) { //TODO единая точка сохранения, так же как и воспроизведения
    GmyFile gmyFile;
    std::string gfilename =  std::string(getTestsLocation())  + "first.gmy";
    std::cerr << "Test loc " << getTestsLocation() << std::endl;
    std::ofstream file(gfilename.c_str());
    gmyFile.saveToFile(&file, tabParent->getTab());
    file.close();
    return;
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


void newBar(Track* pTrack, size_t cursor, size_t& cursorBeat, std::vector<SingleCommand>& commandSequence) {
    Bar *addition = new Bar();
    Bar *bOrigin = pTrack->getV(cursor);
    addition->flush();
    addition->setSignDenum(bOrigin->getSignDenum());
    addition->setSignNum(bOrigin->getSignNum());

    Beat *addBeat=new Beat();
    addBeat->setDuration(3);
    addBeat->setDotted(0);
    addBeat->setDurationDetail(0);
    addBeat->setPause(true);
    addition->add(addBeat);

    SingleCommand command(16);
    command.setPosition(0,cursor,0);
    commandSequence.push_back(command);
    pTrack->insertBefore(addition,cursor);
    pTrack->connectAll();
    cursorBeat = 0;//poits to new
    return;
}


void setDotOnBeat(Beat* beat, size_t cursor, size_t cursorBeat, std::vector<SingleCommand>& commandSequence) {
    byte dotted = beat->getDotted();
    SingleCommand command(6,dotted);
    command.setPosition(0,cursor,cursorBeat);
    commandSequence.push_back(command);

    if (dotted & 1)
        beat->setDotted(0);
    else
        beat->setDotted(1);

    return;
}


void setTriolOnBeat(Beat* beat, size_t cursor, size_t cursorBeat, std::vector<SingleCommand>& commandSequence) { //TODO разные варианты
    byte curDetail = beat->getDurationDetail();
    SingleCommand command(5,curDetail);
    command.setPosition(0,cursor,cursorBeat);
    commandSequence.push_back(command);
    if (curDetail == 3)
        beat->setDurationDetail(0);
    else
        beat->setDurationDetail(3);
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

void setTextOnBeat(Beat *beat) {
    std::string beatText;
    beat->getGPCOMPText(beatText);

    bool ok=false;
    QString newText = QInputDialog::getText(0,"Input",
                     "Input text:", QLineEdit::Normal,beatText.c_str(),&ok);
    if (ok)
    {
        beatText = newText.toStdString();
        beat->setGPCOMPText(beatText);
    }

    return;
}


void setChangesOnBeat(Beat* beat, MasterView* mw) {
    ChangesInput::setPtrBeat(beat);
    if (mw)
        mw->pushForceKey("change_view");
}

void setBarSign(Bar* bar, size_t cursor, std::vector<SingleCommand>& commandSequence) {
    bool ok=false;

    byte oldDen = bar->getSignDenum();
    byte oldNum = bar->getSignNum();

    int newNum = QInputDialog::getInt(0,"Input",
                         "New Num:", QLineEdit::Normal,
                         1,128,1,&ok);

    bool thereWasChange = false;

    if (ok)
    {
        bar->setSignNum(newNum);
        thereWasChange = true;
    }

    ok=false;

    //GET ITEM
    int newDen = QInputDialog::getInt(0,"Input",
                         "New Denum(1,2,4,8,16):", QLineEdit::Normal,
                         1,128,1,&ok);
    if (ok) {
        bar->setSignDenum(newDen);
        thereWasChange = true;
    }

    if (thereWasChange) { //also could set to all here if turned on such flag

        if ((bar->getSignDenum() != oldDen) ||
            (bar->getSignNum() != oldNum))
        {
            SingleCommand command(19);
            command.setPosition(0,cursor,0);
            command.setValue(oldDen);
            command.setValue2(oldNum);
            commandSequence.push_back(command);
        }
    }
}


void clipboardCutBar(int& selectionBarFirst, Bar* bar, TrackView* tv) {
    if (selectionBarFirst == -1) {
        //int trackInd=tabParrent->getLastOpenedTrack();
        Bar *cloner = new Bar;
        cloner->flush();
        cloner->clone(bar);
        AClipboard::current()->setPtr(cloner);
        AClipboard::current()->setType(4);
        tv->keyevent("delete bar"); //TODO command
    }
}


void clipboarCopyBar(Bar* bar, int& selectionBarFirst, int& selectionBarLast, int& selectionBeatFirst,
                   int& selectionBeatLast, TabView *tw) {

    if (selectionBarFirst == -1) {
        //int trackInd=tabParrent->getLastOpenedTrack();
        /*
        AClipboard::current()->setBeginIndexes(trackInd,cursor);
        AClipboard::current()->setType(0); //copy single bar
        */
        Bar *cloner = new Bar;
        cloner->flush();
        cloner->clone(bar);

        AClipboard::current()->setPtr(cloner);
        AClipboard::current()->setType(4);
    }
    else {
        int trackInd = tw->getLastOpenedTrack();
        AClipboard::current()->setBeginIndexes(trackInd,selectionBarFirst,selectionBeatFirst);
        AClipboard::current()->setType(1); //copy single beat
        AClipboard::current()->setEndIndexes(trackInd,selectionBarLast,selectionBeatLast);
    }
    selectionBarFirst=selectionBarLast=selectionBeatFirst=selectionBeatLast=-1;
    //AClipboard::current()->setType(4);
    //AClipboard::current()->setPtr();
    return;
}


void clipboarCopyBeat(int& selectionBarFirst, int& selectionBarLast, int& selectionBeatFirst,
                      int& selectionBeatLast, TabView *tw, size_t cursor, size_t cursorBeat) {
    int trackInd=tw->getLastOpenedTrack();

    if (selectionBarFirst == -1)
    {
        AClipboard::current()->setBeginIndexes(trackInd,cursor,cursorBeat);
        AClipboard::current()->setType(1); //copy single beat
        AClipboard::current()->setEndIndexes(trackInd,cursor,cursorBeat);
    }
    else
    {
        AClipboard::current()->setBeginIndexes(trackInd,selectionBarFirst,selectionBeatFirst);
        AClipboard::current()->setType(1); //copy single beat
        AClipboard::current()->setEndIndexes(trackInd,selectionBarLast,selectionBeatLast);
    }
    selectionBarFirst=selectionBarLast=selectionBeatFirst=selectionBeatLast=-1;
    return;
}


void clipboardCopyBars(int& selectionBarFirst, int& selectionBarLast, int& selectionBeatFirst,
                       int& selectionBeatLast, TabView *tw, size_t cursor) {
    int trackInd=tw->getLastOpenedTrack();
    //copyIndex = cursor;
    if (selectionBarFirst == -1)
    {
        AClipboard::current()->setBeginIndexes(trackInd,cursor);
        AClipboard::current()->setType(0); //copy single bar
    }
    else
    {
        AClipboard::current()->setBeginIndexes(trackInd,selectionBarFirst);
        AClipboard::current()->setType(2); //copy single bar
        AClipboard::current()->setEndIndexes(trackInd,selectionBarLast);
    }
    selectionBarFirst=selectionBarLast=selectionBeatFirst=selectionBeatLast=-1;
    return;
}


void clipboardPaste(Tab *tab, Track* pTrack, const size_t& cursor, std::vector<SingleCommand>& commandSequence) { //insure cursor
    if (AClipboard::current()->getType() >= 0)
    {

        if (AClipboard::current()->getType()==4)
        {
            Bar *addition=new Bar();
            Bar *bOrigin=(Bar*)AClipboard::current()->getPtr();

            addition->clone(bOrigin);

            pTrack->insertBefore(addition,cursor);
            pTrack->connectAll();
            //AClipboard::current()->setType(-1); //refact attention

            SingleCommand command(16);
            command.setPosition(0,cursor,0);
            commandSequence.push_back(command);

            return;
        }


        Track *track = tab->getV(AClipboard::current()->getTrackIndex());

        if (AClipboard::current()->getType()==0)
        {
            Bar *origin = track->getV(AClipboard::current()->getBarIndex()); //pTrack->getV(copyIndex);
            Bar *addition=new Bar();
            addition->clone(origin);

            track->insertBefore(addition,cursor);
            track->connectAll();
            AClipboard::current()->setType(-1); //refact attention

            SingleCommand command(16);
            command.setPosition(0,cursor,0);
            commandSequence.push_back(command);

            return;
        }

        if (AClipboard::current()->getType()==1)
        {
            ///THIS MUST BE CYCLED
            ///
            /*
            Bar *origin = track->getV(AClipboard::current()->getBarIndex()); //pTrack->getV(copyIndex);

            Beat *addition=new Beat();
            Beat *beatOrigin = origin->getV(AClipboard::current()->getBeatIndex());
            addition->clone(beatOrigin);
            Bar *bar = track->getV(AClipboard::current()->getBarIndex());
            bar->insertBefore(addition,cursorBeat);

            */

            if (AClipboard::current()->getSecondBarI()==AClipboard::current()->getBarIndex())
            {


                Bar *origin = track->getV(AClipboard::current()->getBarIndex());
                Bar *addition = new Bar();
                addition->setSignDenum(origin->getSignDenum());
                addition->setSignNum(origin->getSignNum());

                for (int beats = AClipboard::current()->getBeatIndex();
                     beats  <= AClipboard::current()->getSecondBeatI(); ++beats)
                {
                    Beat *additionBeat=new Beat();
                    Beat *beatOrigin = origin->getV(beats);
                    additionBeat->clone(beatOrigin);
                    addition->add(additionBeat);
                }

                track->insertBefore(addition,cursor);

                SingleCommand command(16);
                command.setPosition(0,cursor,0);
                commandSequence.push_back(command);

            }
            else
            for (int bars=AClipboard::current()->getSecondBarI(); bars >= AClipboard::current()->getBarIndex(); --bars)
            {
                Bar *origin = track->getV(bars);
                Bar *addition = new Bar();
                addition->setSignDenum(origin->getSignDenum());
                addition->setSignNum(origin->getSignNum());

                if (bars==AClipboard::current()->getSecondBarI())
                {
                    //last
                    for (int beats = 0; beats <= AClipboard::current()->getSecondBeatI(); ++beats)
                    {
                        Beat *additionBeat=new Beat();
                        Beat *beatOrigin = origin->getV(beats);
                        additionBeat->clone(beatOrigin);
                        addition->add(additionBeat);
                    }
                }
                else if (bars == AClipboard::current()->getBarIndex())
                {
                    //first
                    for (size_t beats = AClipboard::current()->getBeatIndex();
                         beats < origin->len(); ++beats)
                    {
                        Beat *additionBeat=new Beat();
                        Beat *beatOrigin = origin->getV(beats);
                        additionBeat->clone(beatOrigin);
                        addition->add(additionBeat);
                    }
                }
                else
                {
                    //midle
                    addition->clone(origin);
                }

                //wrong?
                track->insertBefore(addition,cursor);
            }

            int barsRange = AClipboard::current()->getSecondBarI() - AClipboard::current()->getBarIndex();
            SingleCommand command(16);
            command.setPosition(0,cursor,barsRange);
            commandSequence.push_back(command);
            //tricke mech
            //will be able isert many times
            AClipboard::current()->setType(-1); //refact attention
            return;
        }
        if (AClipboard::current()->getType()==2)
        {
            for (int bars=AClipboard::current()->getSecondBarI(); bars >= AClipboard::current()->getBarIndex(); --bars)
            {
                Bar *origin = track->getV(bars);
                Bar *addition = new Bar();
                addition->clone(origin);

                track->insertBefore(addition,cursor);
            }

            int barsRange = AClipboard::current()->getSecondBarI() - AClipboard::current()->getBarIndex();
            SingleCommand command(16);
            command.setPosition(0,cursor,barsRange+1);
            commandSequence.push_back(command);

            track->connectAll();
            AClipboard::current()->setType(-1); //refact attention
            return;
        }

        //REFACT
        //--clpboard auto clenup


    }
    AClipboard::current()->setType(-1); //refact attention
    return;
}


void undoOnTrack(TrackView* tw, std::vector<SingleCommand>& commandSequence) {
    if (commandSequence.size()) {
        SingleCommand lastCommand = commandSequence[commandSequence.size()-1];
        commandSequence.pop_back();
        tw->reverseCommand(lastCommand);
    }
    return;
}


void TabView::onTrackCommand(TrackCommand command) {
    qDebug() << "ERROR: Track Command falling into TabView";
}


void TrackView::onTrackCommand(TrackCommand command) {

    if (command == TrackCommand::PlayFromStart) {
        gotoTrackStart(cursorBeat, cursor, displayIndex);
        onTabCommand(TabCommand::PlayMidi);
    }
    else if (command == TrackCommand::GotoStart)
        gotoTrackStart(cursorBeat, cursor, displayIndex);
    else if (command == TrackCommand::SetSignForSelected)
      changeBarSigns(pTrack, selectionBarFirst, selectionBarLast);
    else if (command == TrackCommand::SelectionExpandLeft)
        moveSelectionLeft(pTrack, selectionBeatFirst, selectionBarFirst);
    else if (command == TrackCommand::SelectionExpandRight)
        moveSelectionRight(pTrack, selectionBeatLast, selectionBarLast);
    else if (command == TrackCommand::InsertBar)
        insertBar(pTrack, cursor, cursorBeat, commandSequence);
    else if (command == TrackCommand::NextBar) // => //bar walk
        moveToNextBar(cursor, pTrack, cursorBeat, displayIndex, stringCursor, digitPress, lastSeen);
    else if (command == TrackCommand::PrevBar) // <= //bar walk
        moveToPrevBar(cursor, pTrack, cursorBeat, displayIndex, stringCursor, digitPress);
    else if (command == TrackCommand::PrevPage)
        moveToPrevPage(cursor, tabParrent, cursorBeat, displayIndex, digitPress);
    else if (command == TrackCommand::NextPage)
        moveToNextPage(cursor, tabParrent, pTrack, cursorBeat, displayIndex, digitPress, lastSeen);
    else if (command == TrackCommand::NextTrack)
        moveToNextTrack(tabParrent, digitPress);
    else if (command == TrackCommand::PrevTrack)
        moveToPrevTrack(tabParrent, digitPress);
    else if (command == TrackCommand::StringDown)
        moveToStringUp(pTrack, stringCursor, digitPress);
    else if (command == TrackCommand::StringUp)
        moveToStringDown(pTrack, stringCursor, digitPress);
    else if (command == TrackCommand::PrevBeat)
        moveToPrevBeat(cursorBeat, cursor, displayIndex, stringCursor, digitPress, pTrack);
    else if (command == TrackCommand::NextBeat)
        moveToNextBeat(cursorBeat, cursor, displayIndex, stringCursor, digitPress,  lastSeen, pTrack, commandSequence);
    else if (command == TrackCommand::SetPause)
        setTrackPause(cursor, cursorBeat, digitPress, pTrack, commandSequence);
    else if (command == TrackCommand::DeleteBar)
        deleteBar(cursor, pTrack, commandSequence);
    else if (command == TrackCommand::DeleteSelectedBars)
        deleteSelectedBars(cursor, pTrack, commandSequence, selectionBarFirst, selectionBarLast, selectionBeatFirst, selectionBeatLast);
    else if (command == TrackCommand::DeleteSelectedBeats)
        deleteSelectedBeats(cursor, pTrack, commandSequence, selectionBarFirst, selectionBarLast, selectionBeatFirst, selectionBeatLast);
    else if (command == TrackCommand::DeleteNote)
        deleteNote(pTrack, cursor, cursorBeat, stringCursor, digitPress, commandSequence);
    else if (command == TrackCommand::IncDuration)
        incDuration(pTrack, cursor, cursorBeat, commandSequence);
    else if (command == TrackCommand::DecDuration)
        decDuration(pTrack, cursor, cursorBeat, commandSequence);
    else if (command == TrackCommand::PlayTrackMidi) //TODO единый вызов запуска (играется не 1 трек) //|| (press=="playMerge")
        playTrack(tabParrent, localThr, cursorBeat, cursor, pTrack, getMaster());
    else if (command == TrackCommand::SaveFile)
        saveFromTrack(tabParrent);
    else if (command == TrackCommand::SaveAsFromTrack)
        saveAsFromTrack(tabParrent);
    else if (command == TrackCommand::NewBar)
        newBar(pTrack, cursor, cursorBeat, commandSequence);
    else if (command == TrackCommand::SetDot)
        setDotOnBeat(pTrack->getV(cursor)->getV(cursorBeat), cursor, cursorBeat, commandSequence);
    else if (command == TrackCommand::SetTriole)
        setTriolOnBeat(pTrack->getV(cursor)->getV(cursorBeat), cursor, cursorBeat, commandSequence);
    else if (command == TrackCommand::Leeg) {
        switchNoteState(2); digitPress = -1;
    }
    else if (command == TrackCommand::Dead) {
        switchNoteState(3); digitPress = -1; //TODO review old files, maybe there where sometimes no return in the if statement
    }
    else if (command == TrackCommand::Vibrato)
        switchEffect(1); //TODO move under common core engine (edit, clipboard, navigation)
    else if (command == TrackCommand::Slide)
        switchEffect(4); //TODO cover on new abstraction level tabs-core
    else if (command == TrackCommand::Hammer)
        switchEffect(10);
    else if (command == TrackCommand::LetRing)
        switchEffect(18);
    else if (command == TrackCommand::PalmMute)
        switchEffect(2);
    else if (command == TrackCommand::Harmonics)
        switchEffect(14);
    else if (command == TrackCommand::TremoloPickings)
        switchEffect(24); //tremlo picking
    else if (command == TrackCommand::Trill)
        switchEffect(24);
    else if (command == TrackCommand::Stokatto)
        switchEffect(23);
    else if (command == TrackCommand::FadeIn) //TODO fade out
        switchBeatEffect(20);
    else if (command == TrackCommand::Accent)
        switchEffect(27);
    else if (command == TrackCommand::HeaveAccent)
        switchEffect(27); ///should be another TODO
    else if (command == TrackCommand::Bend)
        setBendOnNote(pTrack->getV(cursor)->getV(cursorBeat)->getNote(stringCursor+1), getMaster());
    else if (command == TrackCommand::Chord) {
        if (getMaster()) getMaster()->pushForceKey("chord_view"); }
    else if (command == TrackCommand::Text)
        setTextOnBeat(pTrack->getV(cursor)->getV(cursorBeat));
    else if (command == TrackCommand::Changes)
        setChangesOnBeat(pTrack->getV(cursor)->getV(cursorBeat), getMaster());
    else if (command == TrackCommand::UpStroke)
        switchBeatEffect(25);
    else if (command == TrackCommand::DownStroke)
        switchBeatEffect(26);
    else if (command == TrackCommand::SetBarSign)
        setBarSign(pTrack->getV(cursor), cursor, commandSequence);
    else if (command == TrackCommand::Cut) //oups - yet works only without selection for 1 bar
        clipboardCutBar(selectionBarFirst, pTrack->getV(cursor), this);
    else if (command == TrackCommand::Copy) //1 bar
        clipboarCopyBar(pTrack->getV(cursor), selectionBarFirst, selectionBarLast, selectionBeatFirst, selectionBeatLast, tabParrent);
    else if (command == TrackCommand::CopyBeat)
        clipboarCopyBeat(selectionBarFirst, selectionBarLast, selectionBeatFirst, selectionBeatLast, tabParrent, cursor, cursorBeat);
    else if (command == TrackCommand::CopyBars)
        clipboardCopyBars(selectionBarFirst, selectionBarLast, selectionBeatFirst, selectionBeatLast, tabParrent, cursor);
    else if (command == TrackCommand::Past )
        clipboardPaste(tabParrent->getTab(), pTrack, cursor, commandSequence);
    else if (command == TrackCommand::Undo)
        undoOnTrack(this, commandSequence);
}



void TrackView::keyevent(std::string press) //TODO масштабные макротесты, чтобы покрывать все сценарии
{
    if (press.substr(0,4)=="com:")
        reactOnComboTrackViewQt(press, pTrack, tabParrent->getMaster());
    else if (isdigit(press[0]))
        handleKeyInput(press[0]-48, digitPress, pTrack, cursor, cursorBeat, stringCursor, commandSequence);
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
            Bar *barPtr = pTab->getV(0)->getV(currentBar);
            for (ul i = 0; i < pTab->getV(0)->timeLoop.len();++i)
                 if (pTab->getV(0)->timeLoop.getV(i) == barPtr) {
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
    ul outFileSize2 = generatedMidi.writeStream(outFile2);
    qDebug() << "File wroten. " << outFileSize2 << " bytes. ";
    outFile2.close();
    generatedMidi.printToStream(std::cout);
    statusLabel->setText("generation done. p for play");
}


void openTrackQt(size_t tracksLen, int& lastOpenedTrack, TabView* tabView, ul digit) {
    if (digit && digit <= tracksLen) {
        TrackView *trackView = tabView->tracksView[digit-1]; //А обновление интерфейса в модуль Qt TODO выше
        lastOpenedTrack = digit-1;
        MainView *mainView = (MainView*)tabView->getMaster()->getFirstChild();
        mainView->changeCurrentView(trackView);
    }
}


void TabView::keyevent(std::string press) {
    if (isdigit(*(press.c_str()))) //The only one left here
        openTrackQt(pTab->len(),lastOpenedTrack, this, press.c_str()[0]-48);
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
    size_t trackLen = pTab->getV(0)->len();
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
    Bar *firstTrackBar = pTab->getV(0)->getV(pTab->getCurrentBar());
    byte repeat = firstTrackBar->getRepeat();
    byte repeatCloses = repeat & 2;
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
    Track* pTrack = pTab->getV(pTab->getCurrentTrack());
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


int changeTrackInstrument(Tab* pTab) {
    Track* pTrack = pTab->getV(pTab->getCurrentTrack());
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
        openTrackQt(pTab->len(),lastOpenedTrack, this, pTab->getDisplayTrack() + 1); //TODO эту часть внутрь движка - разделяя с QT);
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
        setTune(pTab->getV(pTab->getCurrentTrack()));
    else if (command == TabCommand::CloseReprise)
        closeReprise(pTab);
    else
        pTab->onTabCommand(command);
}
