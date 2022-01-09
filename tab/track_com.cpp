#include "tab.h"

#include "g0/Config.hpp"
#include "midi/midifile.h"
#include "tab/gmyfile.h"
#include "tab/tabloader.h"
#include "tab/gtpfiles.h"
#include "midi/midiengine.h"
#include "tab/tabclipboard.h"

#include <fstream>
#include <QDebug>

void Track::switchEffect(Effect effect) {

    if (this->at(_cursor)->at(_cursorBeat)->getPause())
        return;

    if (this->at(_cursor)->at(_cursorBeat)->size()==0)
        return;

    auto pa = parent;
    if (pa->playing())
        return;

    int ind = (int)effect;

    Note *theNote = this->at(_cursor)->at(_cursorBeat)->getNote(_stringCursor+1);
    if (theNote) {
        bool effectFlag = theNote->effPack.getEffectAt(effect);
        effectFlag = !effectFlag;
        this->at(_cursor)->at(_cursorBeat)->getNote(_stringCursor+1)->effPack.setEffectAt(effect, effectFlag);

        ReversableCommand command(ReversableType::SwitchEffectNote, ind); //note effect
        command.setPosition(0, _cursor, _cursorBeat, _stringCursor+1);
        commandSequence.push_back(std::move(command));
    }
}

void Track::switchBeatEffect(Effect beatEffect) { //TODO insure it fits

    int effIndex = static_cast<int>(beatEffect);
    if (this->at(_cursor)->at(_cursorBeat)->getPause())
        return;
    auto pa = parent;
    if (pa->playing())
        return;
    //check for pause
    bool effect = this->at(_cursor)->at(_cursorBeat)->effPack.getEffectAt(beatEffect);
    effect = !effect;
    this->at(_cursor)->at(_cursorBeat)->effPack.setEffectAt(beatEffect,effect);

    ReversableCommand command(ReversableType::SwitchEffectBeat,effIndex); //beat effect
    command.setPosition(0, _cursor, _cursorBeat);
    commandSequence.push_back(std::move(command));
}

void Track::switchNoteState(NoteStates noteState)
{
    int changeState = static_cast<int>(noteState);
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
        ReversableCommand command(ReversableType::SetFret, 255);
        command.setPosition(0,cursor,cursorBeat,stringCursor+1);
        commandSequence.push_back(std::move(command));
        return;
    }

    std::uint8_t state = note->getState();
    if (state == changeState)
        note->setState(0);
    else
        note->setState(changeState);

    ReversableCommand command(ReversableType::ChangeNoteState,state);
    command.setPosition(0,cursor,cursorBeat,stringCursor+1);
    commandSequence.push_back(std::move(command));
}


void Track::reverseCommand(ReversableCommand command) //TODO get rid of this->cursor() etc
{
    ReversableType type = command.getType();
    std::uint8_t value = command.getValue();
    std::uint8_t value2 = command.getValue2();

    int barN = command.getBarNum();
    int beatN = command.getBeatNum();
    std::uint8_t stringN = command.getStringNum();

    if (type == ReversableType::SwitchEffectNote) //eff
    {
        auto ind = static_cast<Effect>(value);
        bool effect = this->at(barN)->at(beatN)->getNote(stringN)->effPack.getEffectAt(ind);
        effect = !effect;
        this->at(barN)->at(beatN)->getNote(stringN)->effPack.setEffectAt(ind,effect);
    }

    if (type == ReversableType::SwitchEffectBeat) //beat eff
    {
        auto ind = static_cast<Effect>(value);
        bool effect = this->at(barN)->at(beatN)->effPack.getEffectAt(ind);
        effect = !effect;
        this->at(barN)->at(beatN)->effPack.setEffectAt(ind,effect);
    }

    if (type == ReversableType::SetFret) //fret
    {
        if (value != 255)
            this->at(barN)->at(beatN)->setFret(value,stringN);
        else
            this->at(barN)->at(beatN)->deleteNote(stringN);

    }

    if (type == ReversableType::SetDuration) //duration
    {
        this->at(barN)->at(beatN)->setDuration(value);
    }

    if (type == ReversableType::SetDurationDetail) //detail
    {
        this->at(barN)->at(beatN)->setDurationDetail(value);
    }

    if (type == ReversableType::SetDot) //dot
    {
        this->at(barN)->at(beatN)->setDotted(value);
    }

    if (type == ReversableType::SetPause) //pause
    {
        if (command.storedNotes.empty() == false)
        {
            for (size_t i = 0; i < command.storedNotes.size(); ++i)
            {
                auto note = std::move(command.storedNotes.at(i));
                this->at(barN)->at(beatN)->push_back(std::move(note));
            }

            this->at(barN)->at(beatN)->setPause(false);
        }
    }

    if (type == ReversableType::DeleteNote)
    {
        if (command.storedNotes.empty() == false) //delete note
        {
            auto note = std::move(command.storedNotes.at(0));
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

    if (type == ReversableType::Pasty)
    {
        int len = beatN; //param
        if (!len)
        { //as len = 1 but default
            this->remove(barN);
            this->cursor() = this->displayIndex();
            //tabParrent->setCurrentBar(this->cursor());
            //TODO must update UI ???
        }
        else
        {
            for (int i = 0; i < len; ++i)
                this->remove(barN);

            this->cursor() = this->displayIndex();
            //tabParrent->setCurrentBar(this->cursor());
            //TODO must update UI ???
        }
        if (_cursor)
            --_cursor;
    }

    if (type == ReversableType::ChangeNoteState)
    {
        Note *note = (this->at(barN)->at(beatN)->getNote(stringN));
        note->setState(value);
    }

    if (type == ReversableType::InsertNewPause)
    {
        this->at(barN)->remove(beatN);
        this->connectAll();
        if (this->cursorBeat())
            --this->cursorBeat();
        //may be shift cursorBeat (when activate <> undo)
    }

    if (type == ReversableType::SetSign)
    {
        this->at(barN)->setSignDenum(value);
        this->at(barN)->setSignNum(value2);
    }

    if (type == ReversableType::InsertBeat) //TODO тут точно ошибка
    {
        int len = beatN;
        if (!len)
        {
            //Это какая-то другая комманда, вероятно удаление такта
            //auto addition = std::move(command.outerBar);
            //this->insertBefore(std::move(addition), barN);
            //this->connectAll();
        }
    }


    if (type == ReversableType::DeleteBar)
    {
        for (auto it = command.storedBars.end() - 1; it != command.storedBars.begin(); --it)
            insertBefore(std::move(*it), barN);
        this->insertBefore(std::move(command.storedBars.front()),barN);

        this->connectAll();
        this->cursor()=barN;
    }

    if (type == ReversableType::DeleteRangeOfBeats)
    {
        //auto& firstBeat = command.storedBeats->front();
        //auto& lastBeat  = command.storedBeats->back();
        //TODO к сожалению этот участок придётся переписать, позже сверимся со старой версией с декабря
        //Кажется при хранении битов надо так же хранить такты которые были удалены
    }

    //TODO 25-26 effect events + all clipboard operations
}


void Track::gotoTrackStart() {
    parent->addMacro(TrackCommand::GotoStart);
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
    //TODO найти старый код, чтобы удостовериться что всё будет работать правильно
}


void Track::insertNewPause() {
    //parent->addMacro(TrackCommand::InsertNewPause); //TODO
    auto beat = std::make_unique<Beat>();
    beat->setPause(true);
    beat->setDuration(4);
    beat->setDotted(0);
    beat->setDurationDetail(0);

    at(_cursor)->insertBefore(std::move(beat), _cursorBeat);
    connectAll(); //autochain cries

    ReversableCommand command(ReversableType::InsertNewPause);
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
                auto& bar = at(_cursor);
                auto& beat = bar->at(bar->size()-1);
                lastDur = beat->getDuration();
                //THERE IS A GOOOD CHANCE TO RECOUNT AGAIN
                /// lastDur from prev position
            }
            if (at(_cursor)->getCompleteStatus()==1)
            {
                auto& bar = at(_cursor);
                auto beat = std::make_unique<Beat>();
                beat->setPause(true);
                beat->setDuration(lastDur);
                beat->setDotted(0);
                beat->setDurationDetail(0);
                bar->push_back(std::move(beat));

                ReversableCommand command(ReversableType::InsertNewPause);
                command.setPosition(0,_cursor,_cursorBeat);
                commandSequence.push_back(std::move(command));

                ///ADD COMMAND              - TASK!!!!!!!!!!!!
            }
            else //in edit mode - else add new bar
            //scrol if out of bar
            {
                if ((_cursor+1) == size())
                {
                    auto newBar = std::make_unique<Bar>();
                    newBar->flush();
                    newBar->setSignDenum(4);
                    newBar->setSignNum(4);
                    newBar->setRepeat(0);

                    auto beat = std::make_unique<Beat>();
                    beat->setPause(true);
                    beat->setDuration(lastDur);
                    beat->setDotted(0);
                    beat->setDurationDetail(0);
                    newBar->push_back(std::move(beat));
                    push_back(std::move(newBar));

                    ReversableCommand command(ReversableType::InsertNewBar);
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
    ReversableCommand command(ReversableType::SetPause);
    command.setPosition(0, _cursor, _cursorBeat);
    for (size_t i = 0; i < at(_cursor)->at(_cursorBeat)->size(); ++i) {
        auto note = std::move(at(_cursor)->at(_cursorBeat)->at(i));
        command.storedNotes.push_back(std::move(note));
    }
    commandSequence.push_back(std::move(command));
    at(_cursor)->at(_cursorBeat)->setPause(true);
    at(_cursor)->at(_cursorBeat)->clear();
    _digitPress = -1;
}


void Track::deleteBar() {
    ReversableCommand command(ReversableType::DeleteBar);
    command.setPosition(0, _cursor,0);

    command.storedBars.push_back(std::move(at(_cursor)));
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
        ReversableCommand command(ReversableType::DeleteRangeOfBars);
        command.setPosition(0, _selectionBarFirst,0);

        for (int i = _selectionBarFirst; i <= _selectionBarLast; ++i)
            command.storedBars.push_back(std::move(at(i)));
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
        ReversableCommand command(ReversableType::DeleteRangeOfBeats);
        command.setPosition(0,_selectionBarFirst,_selectionBeatFirst);

        for (int i = _selectionBeatFirst; i <= _selectionBeatLast; ++i) {
            command.storedBeats.push_back(std::move(at(_selectionBarFirst)->at(_selectionBeatFirst)));
        }
        at(_selectionBarFirst)->at(_selectionBeatFirst)->setParent(at(_selectionBarFirst).get());
        at(_selectionBarLast)->at(_selectionBeatLast)->setParent(at(_selectionBarLast).get());

        bool wholeFirst = false;
        bool wholeLast = false;

        if (_selectionBeatFirst == 0)
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
            for (int i = _selectionBarFirst+1; i <= _selectionBarLast-1; ++i) {
                command.storedBars.push_back(std::move(at(_selectionBarFirst+1)));
            }

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
        ReversableCommand command(ReversableType::DeleteNote);
        command.setPosition(0,_cursor,_cursorBeat);
        auto note = std::move(at(_cursor)->at(_cursorBeat)->at(_stringCursor+1));
        command.storedNotes.push_back(std::move(note));
        if (note->getFret()!=255) {
            //delete one note
            at(_cursor)->at(_cursorBeat)->deleteNote(_stringCursor+1);//shift from 0 to 1
            commandSequence.push_back(std::move(command));
        }
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
            auto& beat = at(_cursor)->at(_cursorBeat);
            at(_cursor)->remove(_cursorBeat);
            connectAll(); //oups?
            beat = nullptr  ;//cleanup

            ReversableCommand command(ReversableType::DeleteNote,packedValue);
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

    ReversableCommand command(ReversableType::SetDuration, beatDur);
    command.setPosition(0,_cursor, _cursorBeat);
    commandSequence.push_back(std::move(command));

    if (beatDur)
     --beatDur;
    //block not go out
    at(_cursor)->at(_cursorBeat)->setDuration(beatDur);
}


void Track::decDuration() {
    std::uint8_t beatDur = at(_cursor)->at(_cursorBeat)->getDuration();
    ReversableCommand command(ReversableType::SetDuration,beatDur);
    command.setPosition(0, _cursor, _cursorBeat);
    commandSequence.push_back(std::move(command));
    if (beatDur < 6)
        ++beatDur;
    at(_cursor)->at(_cursorBeat)->setDuration(beatDur);
}


void Track::saveFromTrack() {
    GmyFile gmyFile;
    std::string gfilename =  std::string(AConfig::getInst().testsLocation)  + "first.gmy";
    std::cerr << "Test loc " << AConfig::getInst().testsLocation << std::endl;
    std::ofstream file(gfilename.c_str());
    Tab* pTab =  parent; //TODO инкапсулировать в обычный класс, а не шаблон
    gmyFile.saveToFile(file, pTab);
    //TODO just get parent tab
    file.close();
    return;
}


void Track::newBar() {
    auto addition = std::make_unique<Bar>();
    auto& bOrigin = at(_cursor);
    addition->flush();
    addition->setSignDenum(bOrigin->getSignDenum());
    addition->setSignNum(bOrigin->getSignNum());

    auto addBeat = std::make_unique<Beat>();
    addBeat->setDuration(3);
    addBeat->setDotted(0);
    addBeat->setDurationDetail(0);
    addBeat->setPause(true);
    addition->push_back(std::move(addBeat));

    ReversableCommand command(ReversableType::InsertNewBar);
    command.setPosition(0,_cursor,0);
    commandSequence.push_back(std::move(command));
    insertBefore(std::move(addition), _cursor);
    connectAll();
    _cursorBeat = 0;//poits to new
    return;
}


void Track::setDotOnBeat() {
    auto& beat = at(_cursor)->at(_cursorBeat);
    std::uint8_t dotted = beat->getDotted();
    ReversableCommand command(ReversableType::SetDot, dotted);
    command.setPosition(0,_cursor, _cursorBeat);
    commandSequence.push_back(std::move(command));
    if (dotted & 1)
        beat->setDotted(0);
    else
        beat->setDotted(1);
}


void Track::setTriolOnBeat() {
    auto& beat = at(_cursor)->at(_cursorBeat);
    std::uint8_t curDetail = beat->getDurationDetail();
    ReversableCommand command(ReversableType::SetDurationDetail,curDetail);
    command.setPosition(0,_cursor,_cursorBeat);
    commandSequence.push_back(std::move(command));
    if (curDetail == 3)
        beat->setDurationDetail(0);
    else
        beat->setDurationDetail(3);
}


void Track::setTextOnBeat(std::string newText) {
    parent->addMacro(StringCommand<TrackCommand>{TrackCommand::Text, newText});
    auto& beat = at(_cursor)->at(_cursorBeat);
    beat->setGPCOMPText(newText);
}


void Track::clipboardCopyBar() {
    auto& bar = at(_cursor);
    if (_selectionBarFirst == -1) {
        Bar *cloner = new Bar; //TODO memory leak - переделать весь буфер обмена на unique
        cloner->flush();
        cloner->clone(bar.get());

        AClipboard::current()->setPtr(cloner); //Move сюда и потом от сюда К строчке выше!!!
        AClipboard::current()->setClipboardType(ClipboardType::BarPointer);
    }
    else {
        Tab* pTab = parent;
        int trackInd = pTab->getLastOpenedTrack();
        AClipboard::current()->setBeginIndexes(trackInd, _selectionBarFirst, _selectionBeatFirst);
        AClipboard::current()->setClipboardType(ClipboardType::SingleBeatCopy); //copy single beat
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
        AClipboard::current()->setClipboardType(ClipboardType::SingleBeatCopy); //copy single beat
        AClipboard::current()->setEndIndexes(trackInd,_cursor,_cursorBeat);
    }
    else
    {
        AClipboard::current()->setBeginIndexes(trackInd,_selectionBarFirst, _selectionBeatFirst);
        AClipboard::current()->setClipboardType(ClipboardType::SingleBeatCopy); //copy single beat
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
        AClipboard::current()->setClipboardType(ClipboardType::SingleBarCopy); //copy single bar
    }
    else {
        AClipboard::current()->setBeginIndexes(trackInd, _selectionBarFirst);
        AClipboard::current()->setClipboardType(ClipboardType::BarsCopy); //copy single bar
        AClipboard::current()->setEndIndexes(trackInd, _selectionBarLast);
    }
    _selectionBarFirst=_selectionBarLast=_selectionBeatFirst=_selectionBeatLast=-1;
    return;
}


void Track::clipboardCutBar() {
    auto& bar = at(_cursor);
    if (_selectionBarFirst == -1) {
        //int trackInd=tabParrent->getLastOpenedTrack();
        Bar *cloner = new Bar; //TODO memory leak - переделать весь буфер обмена на unique
        cloner->flush();
        cloner->clone(bar.get());
        AClipboard::current()->setPtr(cloner); //TODO move - иначе утечка (выше ещё 1 пример)
        AClipboard::current()->setClipboardType(ClipboardType::BarPointer);
        deleteBar();
    }
} //Вызов команды функцией


void Track::clipboardPaste() {
    if (AClipboard::current()->getClipboardType() != ClipboardType::NotSet){
        if (AClipboard::current()->getClipboardType() == ClipboardType::BarPointer) {
            auto addition = std::make_unique<Bar>();
            Bar *bOrigin = AClipboard::current()->getPtr();
            addition->clone(bOrigin);
            insertBefore(std::move(addition), _cursor);
            connectAll();
            //AClipboard::current()->setType(-1); //refact attention
            ReversableCommand command(ReversableType::InsertNewBar);
            command.setPosition(0, _cursor,0);
            commandSequence.push_back(std::move(command));
            return;
        }

        //TODO tab
        Tab* tab = parent;
        auto& track = tab->at(AClipboard::current()->getTrackIndex());

        if (AClipboard::current()->getClipboardType() == ClipboardType::SingleBarCopy) {
            Bar *origin = track->at(AClipboard::current()->getBarIndex()).get(); //pTrack->getV(copyIndex);
            auto addition = std::make_unique<Bar>();
            addition->clone(origin);

            track->insertBefore(std::move(addition), _cursor);
            track->connectAll();
            AClipboard::current()->setClipboardType(ClipboardType::NotSet); //refact attention

            ReversableCommand command(ReversableType::InsertNewBar);
            command.setPosition(0, _cursor,0);
            commandSequence.push_back(std::move(command));

            return;
        }

        if (AClipboard::current()->getClipboardType() == ClipboardType::SingleBeatCopy) {
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

                ReversableCommand command(ReversableType::InsertNewBar);
                command.setPosition(0, _cursor,0);
                commandSequence.push_back(std::move(command));

            }
            else
                for (int bars = AClipboard::current()->getSecondBarI();
                     bars >= AClipboard::current()->getBarIndex(); --bars)
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
            ReversableCommand command(ReversableType::InsertNewBar);
            command.setPosition(0, _cursor,barsRange);
            commandSequence.push_back(std::move(command));
            //tricke mech
            //will be able isert many times
            AClipboard::current()->setClipboardType(ClipboardType::NotSet); //refact attention
            return;
        }
        if (AClipboard::current()->getClipboardType() == ClipboardType::BarsCopy)
        {
            for (int bars=AClipboard::current()->getSecondBarI(); bars >= AClipboard::current()->getBarIndex(); --bars)
            {
                Bar *origin = track->at(bars).get();
                auto addition = std::make_unique<Bar>();
                addition->clone(origin);

                track->insertBefore(std::move(addition), _cursor);
            }

            int barsRange = AClipboard::current()->getSecondBarI() - AClipboard::current()->getBarIndex();
            ReversableCommand command(ReversableType::InsertNewBar);
            command.setPosition(0, _cursor,barsRange+1);
            commandSequence.push_back(std::move(command));

            track->connectAll();
            AClipboard::current()->setClipboardType(ClipboardType::NotSet);
            return;
        }
    }
    AClipboard::current()->setClipboardType(ClipboardType::NotSet);
}


void Track::undoOnTrack() {
    if (commandSequence.size()) {
        ReversableCommand lastCommand = std::move(commandSequence[commandSequence.size()-1]);
        commandSequence.pop_back();
        reverseCommand(std::move(lastCommand));
    }
}



void Track::onTrackCommand(TrackCommand command) {
    parent->addMacro(command);
    if (handlers.count(command))
        (this->*handlers.at(command))();
}


void Track::changeBarSigns(size_t newNum, size_t newDen) {
    parent->addMacro(TwoIntCommand<TrackCommand>{TrackCommand::SetSignForSelected, newNum, newDen});
    if ((_selectionBarFirst != -1) && (_selectionBarLast != -1))
       for (int i = _selectionBarFirst; i <= _selectionBarLast; ++i) {
           at(i)->setSignNum(newNum);
           at(i)->setSignDenum(newDen);
           //TODO undo option?
       }
}


void Track::setBarSign(size_t newNum, size_t newDen) {
    parent->addMacro(TwoIntCommand<TrackCommand>{TrackCommand::SetBarSign, newNum, newDen});
    auto& bar = at(_cursor);
    std::uint8_t oldDen = bar->getSignDenum();
    std::uint8_t oldNum = bar->getSignNum();
    bar->setSignNum(newNum);
    bar->setSignDenum(newDen);
    if ((bar->getSignDenum() != oldDen) ||
        (bar->getSignNum() != oldNum)) {
        ReversableCommand command(ReversableType::SetSign);
        command.setPosition(0,_cursor,0);
        command.setValue(oldDen);
        command.setValue2(oldNum);
        commandSequence.push_back(std::move(command));
    }
}

void Track::LeegNote() {
    switchNoteState(NoteStates::Leeg); //TODO enum
    _digitPress = -1;
}
void Track::DeadNote() {
    switchNoteState(NoteStates::Dead);
    _digitPress = -1;
}
void Track::Vibratto() {
    switchEffect(Effect::Vibrato);
}
void Track::Slide() {
    switchEffect(Effect::Slide);
}
void Track::Hammer() {
    switchEffect(Effect::Hammer);
}
void Track::LetRing() {
    switchEffect(Effect::LetRing);
}
void Track::PalmMute() {
    switchEffect(Effect::PalmMute);
}
void Track::Harmonics() {
    switchEffect(Effect::Harmonics);
}
void Track::TremoloPicking() {
    switchEffect(Effect::TremoloPick);
}
void Track::Trill() {
    switchEffect(Effect::TremoloPick); //What is real difference?
}
void Track::Stokatto() {
    switchEffect(Effect::Stokatto);
}
void Track::FadeIn() { //Todo fade out
    switchBeatEffect(Effect::FadeIn);
}
void Track::Accent() {
    switchEffect(Effect::HeavyAccented);
}
void Track::HeavyAccent() {
    switchEffect(Effect::HeavyAccented); //TODO real & new
}
void Track::UpStroke() {
    switchBeatEffect(Effect::UpStroke);
}
void Track::DownStroke() {
    switchBeatEffect(Effect::DownStroke);
}
