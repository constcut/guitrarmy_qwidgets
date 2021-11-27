#include "tab.h"

#include "g0/midifile.h"
#include "g0/gmyfile.h"
#include "g0/aexpimp.h"
#include "g0/gtpfiles.h"
#include "midiengine.h"

#include <fstream>
#include <QDebug>

void Track::switchEffect(int effIndex) {

    if (this->getV(_cursor)->getV(_cursorBeat)->getPause())
        return;

    if (this->getV(_cursor)->getV(_cursorBeat)->len()==0)
        return;

    auto pa = (Tab*) parent; //TODO get rid of every void* void *
    if (pa->playing())
        return;

    int ind = effIndex;

    Note *theNote = this->getV(_cursor)->getV(_cursorBeat)->getNote(_stringCursor+1);
    if (theNote) {
        bool effect = theNote->effPack.get(ind);
        effect = !effect;
        this->getV(_cursor)->getV(_cursorBeat)->getNote(_stringCursor+1)->effPack.set(ind,effect);

        SingleCommand command(1,effIndex); //note effect
        command.setPosition(0, _cursor, _cursorBeat, _stringCursor+1);
        commandSequence.push_back(command);
    }
}

void Track::switchBeatEffect(int effIndex) {

    if (this->getV(_cursor)->getV(_cursorBeat)->getPause())
        return;
    auto pa = (Tab*) parent; //TODO get rid of every void* void *
    if (pa->playing())
        return;
    int ind = effIndex;
    //check for pause
    bool effect = this->getV(_cursor)->getV(_cursorBeat)->effPack.get(ind);
    effect = !effect;
    this->getV(_cursor)->getV(_cursorBeat)->effPack.set(ind,effect);

    SingleCommand command(2,effIndex); //beat effect
    command.setPosition(0, _cursor, _cursorBeat);
    commandSequence.push_back(command);
}

void Track::switchNoteState(byte changeState)
{
    size_t& cursor = this->cursor(); //TODO _
    size_t& cursorBeat = this->cursorBeat();
    size_t& stringCursor = this->stringCursor();

    Note *note = (this->getV(cursor)->getV(cursorBeat)->getNote(stringCursor+1));

    if ((this->getV(cursor)->getV(cursorBeat)->getPause()) ||
        (this->getV(cursor)->getV(cursorBeat)->len()==0) ||(note==0)) {

        this->getV(cursor)->getV(cursorBeat)->setPause(false);
        Note *newNote=new Note();
        newNote->setState(changeState);
        newNote->setFret(0);
        newNote->setStringNumber(stringCursor+1);
        this->getV(cursor)->getV(cursorBeat)->add(newNote);
        SingleCommand command(3,255);
        command.setPosition(0,cursor,cursorBeat,stringCursor+1);
        commandSequence.push_back(command);
        return;
    }

    byte state = note->getState();
    if (state == changeState)
        note->setState(0);
    else
        note->setState(changeState);

    SingleCommand command(17,state);
    command.setPosition(0,cursor,cursorBeat,stringCursor+1);
    commandSequence.push_back(command);
}


void Track::reverseCommand(SingleCommand &command) //TODO get rid of this->cursor() etc
{
    byte type = command.getType();
    byte value = command.getValue();
    byte value2 = command.getValue2();

    int barN = command.getBarNum();
    int beatN = command.getBeatNum();
    byte stringN = command.getStringNum();

    if (type == 1) //eff
    {
        int ind = value;
        bool effect = this->getV(barN)->getV(beatN)->getNote(stringN)->effPack.get(ind);
        effect = !effect;
        this->getV(barN)->getV(beatN)->getNote(stringN)->effPack.set(ind,effect);
    }

    if (type == 2) //beat eff
    {
        int ind = value;
        bool effect = this->getV(barN)->getV(beatN)->effPack.get(ind);
        effect = !effect;
        this->getV(barN)->getV(beatN)->effPack.set(ind,effect);
    }

    if (type == 3) //fret
    {
        if (value != 255)
            this->getV(barN)->getV(beatN)->setFret(value,stringN);
        else
            this->getV(barN)->getV(beatN)->deleteNote(stringN);

    }

    if (type == 4) //duration
    {
        this->getV(barN)->getV(beatN)->setDuration(value);
    }

    if (type == 5) //detail
    {
        this->getV(barN)->getV(beatN)->setDurationDetail(value);
    }

    if (type == 6) //dot
    {
        this->getV(barN)->getV(beatN)->setDotted(value);
    }

    if (type == 7) //pause
    {
        if (command.storedNotes)
        {
            for (size_t i = 0; i < command.storedNotes->size(); ++i)
            {
                Note *note = command.storedNotes->operator [](i);
                this->getV(barN)->getV(beatN)->add(note);
            }

            this->getV(barN)->getV(beatN)->setPause(false);

            command.releaseStoredNotes();
        }
    }

    if (type == 8)
    {
        if (command.storedNotes) //delete note
        {
            Note *note = command.storedNotes->operator [](0);
            this->getV(barN)->getV(beatN)->add(note);

            this->getV(barN)->getV(beatN)->setPause(false);

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

            this->getV(barN)->insertBefore(beat,beatN);
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
        Note *note = (this->getV(barN)->getV(beatN)->getNote(stringN));
        note->setState(value);
    }

    if (type == 18)
    {
        this->getV(barN)->remove(beatN);
        this->connectAll();
        if (this->cursorBeat())
            --this->cursorBeat();
        //may be shift cursorBeat (when activate <> undo)
    }

    if (type == 19)
    {
        this->getV(barN)->setSignDenum(value);
        this->getV(barN)->setSignNum(value2);
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
                    this->getV(barN)->insertBefore(curBeat,beatN);
                    curBeat = (Beat*)curBeat->getPrev();
                    if (curBeat == 0)
                        break;
                }
                if (curBeat==firstBeat)
                    this->getV(barN)->insertBefore(curBeat,beatN);
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
                    this->getV(barN)->insertBefore(curBeat,0);
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
                   this->getV(barN)->
                           insertBefore(curBeat,
                                        this->getV(barN)->len()
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
