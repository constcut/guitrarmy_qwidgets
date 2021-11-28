#include "tab.h"

#include "g0/midifile.h"
#include "g0/gmyfile.h"
#include "g0/aexpimp.h"
#include "g0/gtpfiles.h"
#include "midiengine.h"
#include "g0/aclipboard.h"

#include <fstream>
#include <QDebug>

void Track::switchEffect(int effIndex) {

    if (this->at(_cursor)->at(_cursorBeat)->getPause())
        return;

    if (this->at(_cursor)->at(_cursorBeat)->size()==0)
        return;

    auto pa = parent;
    if (pa->playing())
        return;

    int ind = effIndex;

    Note *theNote = this->at(_cursor)->at(_cursorBeat)->getNote(_stringCursor+1);
    if (theNote) {
        bool effect = theNote->effPack.get(ind);
        effect = !effect;
        this->at(_cursor)->at(_cursorBeat)->getNote(_stringCursor+1)->effPack.set(ind,effect);

        SingleCommand command(1,effIndex); //note effect
        command.setPosition(0, _cursor, _cursorBeat, _stringCursor+1);
        commandSequence.push_back(std::move(command));
    }
}

void Track::switchBeatEffect(int effIndex) {

    if (this->at(_cursor)->at(_cursorBeat)->getPause())
        return;
    auto pa = parent;
    if (pa->playing())
        return;
    int ind = effIndex;
    //check for pause
    bool effect = this->at(_cursor)->at(_cursorBeat)->effPack.get(ind);
    effect = !effect;
    this->at(_cursor)->at(_cursorBeat)->effPack.set(ind,effect);

    SingleCommand command(2,effIndex); //beat effect
    command.setPosition(0, _cursor, _cursorBeat);
    commandSequence.push_back(std::move(command));
}

void Track::switchNoteState(std::uint8_t changeState)
{
    size_t& cursor = this->cursor(); //TODO _
    size_t& cursorBeat = this->cursorBeat();
    size_t& stringCursor = this->stringCursor();

    Note *note = (this->at(cursor)->at(cursorBeat)->getNote(stringCursor+1));

    if ((this->at(cursor)->at(cursorBeat)->getPause()) ||
        (this->at(cursor)->at(cursorBeat)->size()==0) ||(note==0)) {

        this->at(cursor)->at(cursorBeat)->setPause(false);
        auto newNote = std::make_unique<Note>();
        newNote->setState(changeState);
        newNote->setFret(0);
        newNote->setStringNumber(stringCursor+1);
        this->at(cursor)->at(cursorBeat)->push_back(std::move(newNote));
        SingleCommand command(3,255);
        command.setPosition(0,cursor,cursorBeat,stringCursor+1);
        commandSequence.push_back(std::move(command));
        return;
    }

    std::uint8_t state = note->getState();
    if (state == changeState)
        note->setState(0);
    else
        note->setState(changeState);

    SingleCommand command(17,state);
    command.setPosition(0,cursor,cursorBeat,stringCursor+1);
    commandSequence.push_back(std::move(command));
}


void Track::reverseCommand(SingleCommand command) //TODO get rid of this->cursor() etc
{
    std::uint8_t type = command.getType();
    std::uint8_t value = command.getValue();
    std::uint8_t value2 = command.getValue2();

    int barN = command.getBarNum();
    int beatN = command.getBeatNum();
    std::uint8_t stringN = command.getStringNum();

    if (type == 1) //eff
    {
        int ind = value;
        bool effect = this->at(barN)->at(beatN)->getNote(stringN)->effPack.get(ind);
        effect = !effect;
        this->at(barN)->at(beatN)->getNote(stringN)->effPack.set(ind,effect);
    }

    if (type == 2) //beat eff
    {
        int ind = value;
        bool effect = this->at(barN)->at(beatN)->effPack.get(ind);
        effect = !effect;
        this->at(barN)->at(beatN)->effPack.set(ind,effect);
    }

    if (type == 3) //fret
    {
        if (value != 255)
            this->at(barN)->at(beatN)->setFret(value,stringN);
        else
            this->at(barN)->at(beatN)->deleteNote(stringN);

    }

    if (type == 4) //duration
    {
        this->at(barN)->at(beatN)->setDuration(value);
    }

    if (type == 5) //detail
    {
        this->at(barN)->at(beatN)->setDurationDetail(value);
    }

    if (type == 6) //dot
    {
        this->at(barN)->at(beatN)->setDotted(value);
    }

    if (type == 7) //pause
    {
        if (command.storedNotes)
        {
            for (size_t i = 0; i < command.storedNotes->size(); ++i)
            {
                auto note = std::move(command.storedNotes->operator [](i));
                this->at(barN)->at(beatN)->push_back(std::move(note));
            }

            this->at(barN)->at(beatN)->setPause(false);
        }
    }

    if (type == 8)
    {
        if (command.storedNotes) //delete note
        {
            auto note = std::move(command.storedNotes->operator [](0));
            this->at(barN)->at(beatN)->push_back(std::move(note));

            this->at(barN)->at(beatN)->setPause(false);
        }
        else //deleted beat
        {
            auto beat = std::make_unique<Beat>();

            std::uint8_t dur = value&7;
            std::uint8_t rhythmDetail = value & 0x78; //4 bits after first 3
            rhythmDetail>>=3;

            std::uint8_t dotAppear = stringN & 3; //wow

            beat->setPause(true);
            beat->setDotted(dotAppear);
            beat->setDuration(dur);
            beat->setDurationDetail(rhythmDetail);

            this->at(barN)->insertBefore(std::move(beat),beatN);
            this->connectAll(); //oups?
        }
    }

    if (type == 16)
    {
        int len = beatN; //param
        if (!len)
        { //as len = 1 but default
            this->remove(barN);
            this->cursor() = this->displayIndex();
            //tabParrent->setCurrentBar(this->cursor());
            //TODO must update UI
        }
        else
        {
            for (int i = 0; i < len; ++i)
                this->remove(barN);

            this->cursor() = this->displayIndex();
            //tabParrent->setCurrentBar(this->cursor());
            //TODO must update UI
        }
        if (_cursor)
            --_cursor;
    }

    if (type == 17)
    {
        Note *note = (this->at(barN)->at(beatN)->getNote(stringN));
        note->setState(value);
    }

    if (type == 18)
    {
        this->at(barN)->remove(beatN);
        this->connectAll();
        if (this->cursorBeat())
            --this->cursorBeat();
        //may be shift cursorBeat (when activate <> undo)
    }

    if (type == 19)
    {
        this->at(barN)->setSignDenum(value);
        this->at(barN)->setSignNum(value2);
    }

    if (type == 24)
    {
        int len = beatN;
        if (!len)
        {
            Bar *addition = (Bar*)(command.outerPtr);
            this->insertBefore(addition,barN);
            this->connectAll();
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

            this->insertBefore(addition,barN);
        }
        this->insertBefore(firstBar,barN);
        this->connectAll();
        this->cursor()=barN;
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
                this->insertBefore(firstPa,barN);
            else
            {
                while(curBeat != firstBeat)
                {
                    this->at(barN)->insertBefore(curBeat,beatN);
                    curBeat = (Beat*)curBeat->getPrev();
                    if (curBeat == 0)
                        break;
                }
                if (curBeat==firstBeat)
                    this->at(barN)->insertBefore(curBeat,beatN);
            }
        }
        else
        {
            if (value==0)
                ++barN; //sift when first not full

            Bar *curBar = (Bar*)lastPa->getPrev();

            if (value2){
                this->insertBefore(lastPa,barN);
            }
            else{
                while(curBeat->getParent() == lastPa){
                    this->at(barN)->insertBefore(curBeat,0);
                    curBeat = (Beat*)curBeat->getPrev();
                    if (curBeat == 0)
                    {
                        //what is this
                        if(curBar)
                        curBeat = curBar->at(curBar->size()-1); //issuepossible
                        break;
                    }
                }
            }


            Bar *firstInSeq = command.startBar;
            Bar *lastInSeq  = command.endBar;

            if ((lastInSeq != firstPa) || (firstInSeq != lastPa)) {
                Bar *currentBar = lastInSeq;

                while (currentBar != firstInSeq)
                {
                    this->insertBefore(currentBar,barN);

                    currentBar = (Bar*)currentBar->getPrev();
                    if (currentBar==0)
                        break;
                }

                if (currentBar == firstInSeq)
                    this->insertBefore(currentBar,barN);
            }



           if (value) {    //first is full
               this->insertBefore(firstPa,barN);
           }
           else {
               --barN;
               //curBeat = curBeat->getPrev(); //slide into first pa property


               curBeat = firstBeat;
               //insert here
               int counter = 0;

               if (curBeat)
               while (curBeat->getParent() == firstPa)
               {
                   this->at(barN)->
                           insertBefore(curBeat,
                                        this->at(barN)->size()
                                                    -counter);

                   ++counter;
                   curBeat = (Beat*)curBeat->getNext();
                   if (curBeat==0)
                       break;
               }
           }
        }

       this->connectAll();
       this->cursor()=barN;
       if (this->displayIndex() > this->cursor())
           this->displayIndex() = this->cursor();
       //from here till first beat insert at barN

    }
}


void Track::gotoTrackStart() {
    _cursor = 0;
    _cursorBeat = 0;
    _displayIndex = 0;
}


void Track::moveSelectionLeft() {
    if (_selectionBeatFirst)
        --_selectionBeatFirst;
    else
    {
        if (_selectionBarFirst)
        {
            --_selectionBarFirst;
            _selectionBeatFirst = at(_selectionBarFirst)->size()-1;
        }
    }
}


void Track::moveSelectionRight() {
    if (_selectionBarLast >= 0)
    {
        if (_selectionBeatLast < (at(_selectionBarLast)->size()-1)) //TODO лучше способ хранить зоны выделенности (флаг вкюченности и size_t)
            ++_selectionBeatLast;
        else
            if (_selectionBarLast < (size()-1))
            {
                ++_selectionBarLast;
                _selectionBeatLast = 0;
            }
    }
}


void Track::insertBar() {
    Beat *beat=new Beat();
    beat->setPause(true);
    beat->setDuration(4);
    beat->setDotted(0);
    beat->setDurationDetail(0);

    at(_cursor)->insertBefore(beat, _cursorBeat);
    connectAll(); //autochain cries

    SingleCommand command(18);
    command.setPosition(0, _cursor, _cursorBeat);
    commandSequence.push_back(std::move(command));
}


void Track::moveToNextBar() {
    if ((_cursor+1) != size()){
        ++_cursor;
        _cursorBeat = 0;

        if (_cursor > (_lastSeen-1))
            _displayIndex = _cursor;

        if (at(_cursor)->at(_cursorBeat)->getPause() == false)
            _stringCursor = at(_cursor)->at(_cursorBeat)->at(0)->getStringNumber()-1;
    }

    _digitPress=-1; // flush input after movement
}


void Track::moveToPrevBar() {
    if (_cursor > 0) {
        --_cursor;
        _cursorBeat = 0;

        if (_cursor < _displayIndex)
            _displayIndex = _cursor;

        if (at(_cursor)->at(_cursorBeat)->getPause() == false)
            _stringCursor = at(_cursor)->at(_cursorBeat)->at(0)->getStringNumber()-1;
    }

    _digitPress=-1; // flush input after movement
}


void Track::moveToPrevPage() {
    if (_displayIndex > 7) {
        _displayIndex -= 7; //not real paging
        _cursor = _displayIndex;
        _cursorBeat = 0;
        _digitPress = -1;
        //TODO установить в родителя через PA (вначале проверив куда идёт вызов функции ниже)
        Tab* pTab = parent;
        pTab->getCurrentBar() = _cursor; //QT dependency отвязать TODO
    }
}


void Track::moveToNextPage() {
    if ((_lastSeen+1) <= size()) {
        _displayIndex = _lastSeen+1;
        _cursor = _displayIndex;
        _cursorBeat = 0;
        _digitPress = -1;
        //tabView->setCurrentBar(cursor); //TODO как выше
    }
}


void Track::moveToNextTrack() {
    Tab* pTab = parent;
    pTab->moveCursorOfTrackDown();
    _digitPress = -1;
}


void Track::moveToPrevTrack() {
    Tab* pTab = parent;
    pTab->moveCursorOfTrackUp();
    _digitPress = -1;
}


void Track::moveToStringUp() {
    if ((_stringCursor+1) < tuning.getStringsAmount())
        ++_stringCursor;
    _digitPress=-1; // flush input after movement
    return;
}


void Track::moveToStringDown() {
    if (_stringCursor > 0)
        --_stringCursor;
    _digitPress=-1; // flush input after movement
}


void Track::moveToPrevBeat() {
    //scrol if out of bar
    if (_cursorBeat==0) {
        if (_cursor) {
            --_cursor;
            if (_cursor < _displayIndex)
                _displayIndex = _cursor;
            _cursorBeat = at(_cursor)->size()-1;
        }
    }
    else
        --_cursorBeat;

    if (at(_cursor)->at(_cursorBeat)->getPause() == false)
        _stringCursor = at(_cursor)->at(_cursorBeat)->at(0)->getStringNumber()-1;

    _digitPress=-1; // flush input after movement
}


void Track::moveToNextBeat() {
    ++_cursorBeat;
    if (_cursorBeat >= at(_cursor)->size()) {
        if (1) //pan->isOpenned())
        {
            static int lastDur = 4; //TODO?
            if (_cursorBeat) {
                Bar *b = at(_cursor);
                Beat *beat = b->at(b->size()-1);
                lastDur = beat->getDuration();
                //THERE IS A GOOOD CHANCE TO RECOUNT AGAIN
                /// lastDur from prev position
            }
            if (at(_cursor)->getCompleteStatus()==1)
            {
                Bar *bar = at(_cursor);
                Beat *beat=new Beat();
                beat->setPause(true);
                beat->setDuration(lastDur);
                beat->setDotted(0);
                beat->setDurationDetail(0);
                bar->push_back(beat);

                SingleCommand command(18);
                command.setPosition(0,_cursor,_cursorBeat);
                commandSequence.push_back(std::move(command));

                ///ADD COMMAND              - TASK!!!!!!!!!!!!
            }
            else //in edit mode - else add new bar
            //scrol if out of bar
            {
                if ((_cursor+1) == size())
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
                    newBar->push_back(beat);
                    push_back(newBar);

                    SingleCommand command(16);
                    command.setPosition(0, _cursor+1,0);
                    commandSequence.push_back(std::move(command));

                    ++_lastSeen;
                    _cursorBeat = 0;
                    ++_cursor;
                }
                else
                {
                    if ((_cursor+1) != size()) {
                        ++_cursor;
                       if (_cursor > (_lastSeen-1))
                            _displayIndex = _cursor;
                        _cursorBeat = 0;
                     }
                        else
                        --_cursorBeat;
                }
            }
        }
        else //TODO view mode maybe remove?
        {
            if ((_cursor+1) != size())
              {
                ++_cursor;
                if (_cursor > (_lastSeen-1))
                    _displayIndex = _cursor;
                _cursorBeat = 0;
              }
              else
                --_cursorBeat;
        }
    }

    if (at(_cursor)->at(_cursorBeat)->getPause() == false)
        _stringCursor = at(_cursor)->at(_cursorBeat)->at(0)->getStringNumber()-1;
        //need acces
    _digitPress=-1; // flush input after movement
}


void Track::setTrackPause() {
    SingleCommand command(7);
    command.setPosition(0, _cursor, _cursorBeat);
    command.requestStoredNotes();
    for (size_t i = 0; i < at(_cursor)->at(_cursorBeat)->size(); ++i) {
        auto note = std::move(at(_cursor)->at(_cursorBeat)->at(i));
        command.storedNotes->push_back(std::move(note));
    }
    commandSequence.push_back(std::move(command));
    at(_cursor)->at(_cursorBeat)->setPause(true);
    at(_cursor)->at(_cursorBeat)->clear();
    _digitPress = -1;
}


void Track::deleteBar() {
    SingleCommand command(24);
    command.setPosition(0, _cursor,0);
    command.outerPtr = at(_cursor).get();
    commandSequence.push_back(std::move(command));

    //attention question for memoryleaks
    remove(_cursor);
    connectAll(); //should go to auto state later
    if (_cursor > 0)
       --_cursor;
}


void Track::deleteSelectedBars() {
    if (_selectionBarFirst != -1)  {
        if (_selectionBarFirst > 0)
            --_cursor; //attention
        SingleCommand command(25);
        command.setPosition(0, _selectionBarFirst,0);
        command.outerPtr = at(_selectionBarFirst).get();
        command.outerPtrEnd = at(_selectionBarLast).get();
        commandSequence.push_back(std::move(command));
        for (int i = _selectionBarLast; i >= _selectionBarFirst; --i)
            remove(i);
        connectAll();
    }
    _selectionBarFirst=_selectionBarLast=_selectionBeatFirst=_selectionBeatLast=-1;
}


void Track::deleteSelectedBeats() {
    if (_selectionBarFirst != -1) {

        connectAll();
        Beat *firstBeat = nullptr, *lastBeat = nullptr;
        SingleCommand command(26);
        command.setPosition(0,_selectionBarFirst,_selectionBeatFirst);
        command.outerPtr = firstBeat = at(_selectionBarFirst)->at(_selectionBeatFirst).get();
        command.outerPtrEnd = lastBeat = at(_selectionBarLast)->at(_selectionBeatLast).get();
        at(_selectionBarFirst)->at(_selectionBeatFirst)->setParent(at(_selectionBarFirst).get());
        at(_selectionBarLast)->at(_selectionBeatLast)->setParent(at(_selectionBarLast).get());

        bool wholeFirst = false;
        bool wholeLast = false;

        if (_selectionBeatFirst==0)
            wholeFirst = true;
        if (_selectionBeatLast == at(_selectionBarLast)->size()-1)
            wholeLast = true;

        command.setValue(wholeFirst);
        command.setValue2(wholeLast);

        if (_selectionBarFirst == _selectionBarLast)
        {
            //remove from single bar
            if (wholeFirst && wholeLast)
                remove(_selectionBarFirst);
            else
                for (int bI = _selectionBeatLast; bI >= _selectionBeatFirst; --bI)
                    at(_selectionBarFirst)->remove(bI);

        }
        else
        { //first and last remove depending on condition
            if (wholeLast)
            {
                remove(_selectionBarLast);
            }
            else
            {
                for (int bI = _selectionBeatLast; bI >= 0; --bI)
                    at(_selectionBarLast)->remove(bI);
            }

            ///GET range of bars
            Bar *lastBarInMiddle = at(_selectionBarLast-1);
            Bar *firstBarInMiddle = at(_selectionBarFirst+1);

            command.startBar = firstBarInMiddle;
            command.endBar = lastBarInMiddle;

            for (int bI = _selectionBarLast-1; bI > _selectionBarFirst; --bI)
                remove(bI);

            if (wholeFirst) {
                remove(_selectionBarFirst);
            }
            else {
                for (int bI = at(_selectionBarFirst)->size()-1; bI >= _selectionBeatFirst; --bI)
                    at(_selectionBarFirst)->remove(bI);
            }
        }

        commandSequence.push_back(std::move(command));
        connectAll();
    }
    _selectionBarFirst=_selectionBarLast=_selectionBeatFirst=_selectionBeatLast=-1;
}


void Track::deleteNote() {
    if (at(_cursor)->at(_cursorBeat)->size())
    {
        SingleCommand command(8);
        command.setPosition(0,_cursor,_cursorBeat);
        command.requestStoredNotes();
        Note *note = at(_cursor)->at(_cursorBeat)->getNoteInstance(_stringCursor+1);
        command.storedNotes->push_back(note);
        if (note->getFret()!=255) {
            //delete one note
            at(_cursor)->at(_cursorBeat)->deleteNote(_stringCursor+1);//shift from 0 to 1
            commandSequence.push_back(std::move(command));
        }
        else
            delete note;
    }
    else
    {
        if (at(_cursor)->size() > 1) {
            std::uint8_t packedValue = 0;
            std::uint8_t dur = at(_cursor)->at(_cursorBeat)->getDuration();
            std::uint8_t det =  at(_cursor)->at(_cursorBeat)->getDurationDetail();
            std::uint8_t dot =  at(_cursor)->at(_cursorBeat)->getDotted();
            packedValue = dur;
            packedValue |= det<<3;
            Beat *beat = at(_cursor)->at(_cursorBeat);
            at(_cursor)->remove(_cursorBeat);
            connectAll(); //oups?
            delete beat;//cleanup

            SingleCommand command(8,packedValue);
            command.setPosition(0,_cursor,_cursorBeat,dot); //wow wow know it
            commandSequence.push_back(std::move(command));

            if (_cursorBeat)
                --_cursorBeat;
        }
    }

    _digitPress = -1;
}


void Track::incDuration() {
    std::uint8_t beatDur = at(_cursor)->at(_cursorBeat)->getDuration();

    SingleCommand command(4,beatDur);
    command.setPosition(0,_cursor, _cursorBeat);
    commandSequence.push_back(std::move(command));

    if (beatDur)
     --beatDur;
    //block not go out
    at(_cursor)->at(_cursorBeat)->setDuration(beatDur);
}


void Track::decDuration() {
    std::uint8_t beatDur = at(_cursor)->at(_cursorBeat)->getDuration();
    SingleCommand command(4,beatDur);
    command.setPosition(0, _cursor, _cursorBeat);
    commandSequence.push_back(std::move(command));
    if (beatDur < 6)
        ++beatDur;
    at(_cursor)->at(_cursorBeat)->setDuration(beatDur);
}


void Track::saveFromTrack() {
    GmyFile gmyFile;
    std::string gfilename =  std::string(getTestsLocation())  + "first.gmy";
    std::cerr << "Test loc " << getTestsLocation() << std::endl;
    std::ofstream file(gfilename.c_str());
    Tab* pTab =  parent; //TODO инкапсулировать в обычный класс, а не шаблон
    gmyFile.saveToFile(&file, pTab);
    //TODO just get parent tab
    file.close();
    return;
}


void Track::newBar() {
    auto addition = std::make_unique<Bar>();
    Bar *bOrigin = at(_cursor).get();
    addition->flush();
    addition->setSignDenum(bOrigin->getSignDenum());
    addition->setSignNum(bOrigin->getSignNum());

    auto addBeat = std::make_unique<Beat>();
    addBeat->setDuration(3);
    addBeat->setDotted(0);
    addBeat->setDurationDetail(0);
    addBeat->setPause(true);
    addition->push_back(std::move(addBeat));

    SingleCommand command(16);
    command.setPosition(0,_cursor,0);
    commandSequence.push_back(std::move(command));
    insertBefore(std::move(addition), _cursor);
    connectAll();
    _cursorBeat = 0;//poits to new
    return;
}


void Track::setDotOnBeat() {
    Beat* beat = at(_cursor)->at(_cursorBeat).get();
    std::uint8_t dotted = beat->getDotted();
    SingleCommand command(6,dotted);
    command.setPosition(0,_cursor, _cursorBeat);
    commandSequence.push_back(std::move(command));
    if (dotted & 1)
        beat->setDotted(0);
    else
        beat->setDotted(1);
}


void Track::setTriolOnBeat() {
    Beat* beat = at(_cursor)->at(_cursorBeat).get();
    std::uint8_t curDetail = beat->getDurationDetail();
    SingleCommand command(5,curDetail);
    command.setPosition(0,_cursor,_cursorBeat);
    commandSequence.push_back(std::move(command));
    if (curDetail == 3)
        beat->setDurationDetail(0);
    else
        beat->setDurationDetail(3);
}


void Track::setTextOnBeat(std::string newText) {
    Beat* beat = at(_cursor)->at(_cursorBeat).get();
    beat->setGPCOMPText(newText);
}


void Track::clipboardCopyBar() {
    Bar* bar = at(_cursor).get();
    if (_selectionBarFirst == -1) {
        Bar *cloner = new Bar;
        cloner->flush();
        cloner->clone(bar);

        AClipboard::current()->setPtr(cloner);
        AClipboard::current()->setType(4);
    }
    else {
        Tab* pTab = parent;
        int trackInd = pTab->getLastOpenedTrack();
        AClipboard::current()->setBeginIndexes(trackInd, _selectionBarFirst, _selectionBeatFirst);
        AClipboard::current()->setType(1); //copy single beat
        AClipboard::current()->setEndIndexes(trackInd, _selectionBarLast, _selectionBeatLast);
    }
    _selectionBarFirst=_selectionBarLast=_selectionBeatFirst=_selectionBeatLast=-1;
} //refact name


void Track::clipboarCopyBeat() {
    Tab* pTab = parent;
    int trackInd = pTab->getLastOpenedTrack();

    if (_selectionBarFirst == -1)
    {
        AClipboard::current()->setBeginIndexes(trackInd,_cursor,_cursorBeat);
        AClipboard::current()->setType(1); //copy single beat
        AClipboard::current()->setEndIndexes(trackInd,_cursor,_cursorBeat);
    }
    else
    {
        AClipboard::current()->setBeginIndexes(trackInd,_selectionBarFirst, _selectionBeatFirst);
        AClipboard::current()->setType(1); //copy single beat
        AClipboard::current()->setEndIndexes(trackInd, _selectionBarLast, _selectionBeatLast);
    }
    _selectionBarFirst=_selectionBarLast=_selectionBeatFirst=_selectionBeatLast=-1;
    return;
}


void Track::clipboardCopyBars() {
    Tab* pTab = parent;
    int trackInd = pTab->getLastOpenedTrack();
    //copyIndex = cursor;
    if (_selectionBarFirst == -1) {
        AClipboard::current()->setBeginIndexes(trackInd, _cursor);
        AClipboard::current()->setType(0); //copy single bar
    }
    else {
        AClipboard::current()->setBeginIndexes(trackInd, _selectionBarFirst);
        AClipboard::current()->setType(2); //copy single bar
        AClipboard::current()->setEndIndexes(trackInd, _selectionBarLast);
    }
    _selectionBarFirst=_selectionBarLast=_selectionBeatFirst=_selectionBeatLast=-1;
    return;
}


void Track::clipboardCutBar() {
    Bar* bar = at(_cursor).get();
    if (_selectionBarFirst == -1) {
        //int trackInd=tabParrent->getLastOpenedTrack();
        Bar *cloner = new Bar;
        cloner->flush();
        cloner->clone(bar);
        AClipboard::current()->setPtr(cloner);
        AClipboard::current()->setType(4);
        deleteBar();
    }
} //Вызов команды функцией


void Track::clipboardPaste() {
    if (AClipboard::current()->getType() >= 0){
        if (AClipboard::current()->getType()==4) {
            auto addition = std::make_unique<Bar>();
            Bar *bOrigin=(Bar*)AClipboard::current()->getPtr();
            addition->clone(bOrigin);
            insertBefore(std::move(addition), _cursor);
            connectAll();
            //AClipboard::current()->setType(-1); //refact attention
            SingleCommand command(16);
            command.setPosition(0, _cursor,0);
            commandSequence.push_back(std::move(command));
            return;
        }

        //TODO tab
        Tab* tab = parent;
        Track *track = tab->at(AClipboard::current()->getTrackIndex()).get();

        if (AClipboard::current()->getType()==0) {
            Bar *origin = track->at(AClipboard::current()->getBarIndex()).get(); //pTrack->getV(copyIndex);
            auto addition = std::make_unique<Bar>();
            addition->clone(origin);

            track->insertBefore(std::move(addition), _cursor);
            track->connectAll();
            AClipboard::current()->setType(-1); //refact attention

            SingleCommand command(16);
            command.setPosition(0, _cursor,0);
            commandSequence.push_back(std::move(command));

            return;
        }

        if (AClipboard::current()->getType()==1) {
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

            if (AClipboard::current()->getSecondBarI()==AClipboard::current()->getBarIndex()) {
                Bar *origin = track->at(AClipboard::current()->getBarIndex()).get();
                auto addition = std::make_unique<Bar>();
                addition->setSignDenum(origin->getSignDenum());
                addition->setSignNum(origin->getSignNum());

                for (int beats = AClipboard::current()->getBeatIndex();
                     beats  <= AClipboard::current()->getSecondBeatI(); ++beats) {
                    auto additionBeat = std::make_unique<Beat>();
                    Beat *beatOrigin = origin->at(beats).get();
                    additionBeat->clone(beatOrigin);
                    addition->push_back(std::move(additionBeat));
                }

                track->insertBefore(std::move(addition), _cursor);

                SingleCommand command(16);
                command.setPosition(0, _cursor,0);
                commandSequence.push_back(std::move(command));

            }
            else
            for (int bars=AClipboard::current()->getSecondBarI(); bars >= AClipboard::current()->getBarIndex(); --bars)
            {
                Bar *origin = track->at(bars).get();
                auto addition = std::make_unique<Bar>();
                addition->setSignDenum(origin->getSignDenum());
                addition->setSignNum(origin->getSignNum());

                if (bars==AClipboard::current()->getSecondBarI())
                {
                    //last
                    for (int beats = 0; beats <= AClipboard::current()->getSecondBeatI(); ++beats)
                    {
                        auto additionBeat = std::make_unique<Beat>();
                        Beat *beatOrigin = origin->at(beats).get();
                        additionBeat->clone(beatOrigin);
                        addition->push_back(std::move(additionBeat));
                    }
                }
                else if (bars == AClipboard::current()->getBarIndex())
                {
                    //first
                    for (size_t beats = AClipboard::current()->getBeatIndex();
                         beats < origin->size(); ++beats)
                    {
                         auto additionBeat = std::make_unique<Beat>();
                        Beat *beatOrigin = origin->at(beats).get();
                        additionBeat->clone(beatOrigin);
                        addition->push_back(std::move(additionBeat));
                    }
                }
                else
                {
                    //midle
                    addition->clone(origin);
                }

                //wrong?
                track->insertBefore(std::move(addition), _cursor);
            }

            int barsRange = AClipboard::current()->getSecondBarI() - AClipboard::current()->getBarIndex();
            SingleCommand command(16);
            command.setPosition(0, _cursor,barsRange);
            commandSequence.push_back(std::move(command));
            //tricke mech
            //will be able isert many times
            AClipboard::current()->setType(-1); //refact attention
            return;
        }
        if (AClipboard::current()->getType()==2)
        {
            for (int bars=AClipboard::current()->getSecondBarI(); bars >= AClipboard::current()->getBarIndex(); --bars)
            {
                Bar *origin = track->at(bars).get();
                auto addition = std::make_unique<Bar>();
                addition->clone(origin);

                track->insertBefore(std::move(addition), _cursor);
            }

            int barsRange = AClipboard::current()->getSecondBarI() - AClipboard::current()->getBarIndex();
            SingleCommand command(16);
            command.setPosition(0, _cursor,barsRange+1);
            commandSequence.push_back(std::move(command));

            track->connectAll();
            AClipboard::current()->setType(-1); //refact attention
            return;
        }
    }
    AClipboard::current()->setType(-1); //refact attention
}


void Track::undoOnTrack() {
    if (commandSequence.size()) {
        SingleCommand lastCommand = std::move(commandSequence[commandSequence.size()-1]);
        commandSequence.pop_back();
        reverseCommand(std::move(lastCommand));
    }
}



void Track::onTrackCommand(TrackCommand command) {
    if (handlers.count(command))
        (this->*handlers.at(command))();
}


void Track::changeBarSigns(int newNum, int newDen) {
    if ((_selectionBarFirst != -1) && (_selectionBarLast != -1))
       for (int i = _selectionBarFirst; i <= _selectionBarLast; ++i) {
           at(i)->setSignNum(newNum);
           at(i)->setSignDenum(newDen);
           //TODO undo option?
       }
}


void Track::setBarSign(int newNum, int newDen) {
    Bar* bar = at(_cursor).get();
    std::uint8_t oldDen = bar->getSignDenum();
    std::uint8_t oldNum = bar->getSignNum();
    bar->setSignNum(newNum);
    bar->setSignDenum(newDen);
    if ((bar->getSignDenum() != oldDen) ||
        (bar->getSignNum() != oldNum)) {
        SingleCommand command(19);
        command.setPosition(0,_cursor,0);
        command.setValue(oldDen);
        command.setValue2(oldNum);
        commandSequence.push_back(std::move(command));
    }
}

void Track::LeegNote() {
    switchNoteState(2); //TODO enum
    _digitPress = -1;
}
void Track::DeadNote() {
    switchNoteState(3);
    _digitPress = -1;
}
void Track::Vibratto() {
    switchEffect(1);
}
void Track::Slide() {
    switchEffect(4);
}
void Track::Hammer() {
    switchEffect(10);
}
void Track::LetRing() {
    switchEffect(18);
}
void Track::PalmMute() {
    switchEffect(2);
}
void Track::Harmonics() {
    switchEffect(14);
}
void Track::TremoloPicking() {
    switchEffect(24); //tremlo picking
}
void Track::Trill() {
    switchEffect(24);
}
void Track::Stokatto() {
    switchEffect(23);
}
void Track::FadeIn() {
    switchBeatEffect(20);
} //Todo fade out
void Track::Accent() {
    switchEffect(27);
}
void Track::HeavyAccent() {
    switchEffect(27); //TODO real & new
}
void Track::UpStroke() {
    switchBeatEffect(25);
}
void Track::DownStroke() {
    switchBeatEffect(26);
}
