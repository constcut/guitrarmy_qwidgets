#include "tabcommands.h"

#include "tabviews.h"

#include "mainviews.h"

//other dep
#include "g0/midifile.h"
#include "g0/gmyfile.h"
#include "g0/aexpimp.h"
#include "g0/gtpfiles.h"
#include "g0/amusic.h"

#include <QInputDialog>


#include "g0/astreaming.h"

#include <QMutex>
#include <QFileDialog>
#include <QApplication>
#include <QScreen>

#include "midiengine.h"

#include "libtim/miditopcm.h"

//REFACT CLEAN ALL QT FUNCTIONS IN ONE QTHELPER!!!

static AStreaming logger("commands");


void TrackView::reverseCommand(SingleCommand &command)
{
    byte type = command.getType();
    byte value = command.getValue();
    byte value2 = command.getValue2();

    byte trackN = command.getTrackNum();

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
            for (int i = 0; i < command.storedNotes->size(); ++i)
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
        for (;addition != firstBar; addition=addition->getPrev())
        {
            if (addition == 0)
                break;

            LOG( << "Addition addr "<<(int)(addition));

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
                    curBeat = curBeat->getPrev();
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

            Bar *curBar = lastPa->getPrev();

            if (value2)
            {
                //last pa if full
                pTrack->insertBefore(lastPa,barN);
            }
            else
            {
                int indexInBar = -1;

                while(curBeat->getParent() == lastPa)
                {
                    pTrack->getV(barN)->insertBefore(curBeat,0);
                    curBeat = curBeat->getPrev();
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

                    currentBar = currentBar->getPrev();
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
                   curBeat = curBeat->getNext();
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


void TrackView::keyevent(std::string press)
{
    if (press.substr(0,4)=="com:")
    {
        std::string rest = press.substr(4);
        size_t separator = rest.find(":");
        std::string combo = rest.substr(0,separator);
        std::string item = rest.substr(separator+1);

        int itemNum = atoi(item.c_str());

        if (combo=="0")
        {
            char mini[2]={0};
            mini[0] = itemNum + 49;

            tabParrent->getMaster()->pushForceKey("esc");
            tabParrent->getMaster()->pushForceKey(mini);
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
        return;
    }

    if (press == "playFromStart")
    {
        cursor = displayIndex = 0;
        cursorBeat = 0;
        keyevent(CONF_PARAM("TrackView.playMidi"));
    }

    if (press == "goToStart")
    {
        cursorBeat = 0;
        cursor = displayIndex = 0;
    }

    if (press == "set for selected")
    {
        bool ok=false;
        int newNum = QInputDialog::getInt(0,"Input",
                             "New Num:", QLineEdit::Normal,
                             1,128,1,&ok);
        if (!ok)
            return;

        ok=false;

        //GET ITEM
        int newDen = QInputDialog::getInt(0,"Input",
                             "New Denum(1,2,4,8,16):", QLineEdit::Normal,
                             1,128,1,&ok);
        if (ok)
        {
                if ((selectionBarFirst != -1) && (selectionBarLast != -1))
                {
                   for (int i = selectionBarFirst; i <= selectionBarLast; ++i)
                   {
                       pTrack->getV(i)->setSignNum(newNum);
                       pTrack->getV(i)->setSignDenum(newDen);
                   }
                }
        }
    }

    if (press == "select <")
    {
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
        return;
    }
    if (press == "select >")
    {
        if (selectionBarLast >= 0)
        {
            if (selectionBeatLast <
                    (pTrack->getV(selectionBarLast)->len()-1))
            {
                ++selectionBeatLast;
            }
            else
            {
                if (selectionBarLast < (pTrack->len()-1) )
                {
                    ++selectionBarLast;
                    selectionBeatLast = 0;
                }
            }
        }
        return;
    }

    if((press == CONF_PARAM("TrackView.quickOpen"))||(press=="quickopen"))
    {
        //yet blocked
    }



    if (press == "ins")
    {
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

    if (isdigit(press[0]))
    {
        int digit = press[0]-48;

        //group operations
        if (digitPress>=0)
        {
            if (digitPress<10)
            {
                int pre = digitPress;
                digitPress*=10;
                digitPress+=digit;

                if (digitPress > pTrack->getGPCOMPInts(3)) //hate this refact
                {
                    digitPress = digit;
                    if (digit == pre)
                        return; //no changes
                }
            }
            else
            {
                digitPress =digit;
            }
        }
        else
            digitPress = digit;

        //chose where to change or what to add
        //pTrack->getV(cursor).getV(cursorBeat).getV(0).setFret(digitPress);

        if ( pTrack->getV(cursor)->len() > cursorBeat )
        {
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
        return;
    }


    if (press == CONF_PARAM("TrackView.nextBar")) // => //bar walk
    {
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
        return;
    }
    if (press == CONF_PARAM("TrackView.prevBar")) // <= //bar walk
    {
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
        return;
    }

    if (press == "prevPage")
    {
        if (displayIndex > 7)
        {
            displayIndex -= 7; //not real paging
            cursor = displayIndex;
            cursorBeat = 0;
            digitPress = -1;
            tabParrent->setCurrentBar(cursor);
        }
        return;
    }

    if (press == "nextPage")
    {
        if ((lastSeen+1) <= pTrack->len())
        {
            displayIndex = lastSeen+1;
            cursor = displayIndex;
            cursorBeat = 0;
            digitPress = -1;
            tabParrent->setCurrentBar(cursor);
        }
        return;
    }

    if (press == "nextTrack")
    {
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

    if (press == "prevTrack")
    {
        int trackInd = tabParrent->getLastOpenedTrack();

        --trackInd;

        char pressImit[2]={0};
        pressImit[0]=trackInd+49;
        std::string pressIm = pressImit;
        //refact for open function
        tabParrent->keyevent(pressIm);

        digitPress = -1;
        return;
    }


    if (press==CONF_PARAM("TrackView.stringDown")) // down
    {

        if ((stringCursor+1) < pTrack->tuning.getStringsAmount())
        ++stringCursor;

        digitPress=-1; // flush input after movement
        return;

    }

    if (press==CONF_PARAM("TrackView.stringUp")) // up
    {
        if (stringCursor > 0)
        --stringCursor;

        digitPress=-1; // flush input after movement
        return;
    }


    //synonime to make without mode
    if (press == CONF_PARAM("TrackView.prevBeat"))
    {
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
        return;
    }

    if (press == CONF_PARAM("TrackView.nextBeat"))
    {
        ++cursorBeat;


        //if incomplete if compl or exeed +2
        if ((cursorBeat) >= pTrack->getV(cursor)->len())
        {
            //edit mode

            if (1) //pan->isOpenned())
            {
                static int lastDur = 4;

                if (cursorBeat)
                {
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

                        ///ADD COMMAND          - TASK!!!!!!!!!!!!


                        SingleCommand command(16);
                        command.setPosition(0,cursor+1,0);
                        commandSequence.push_back(command);

                        ul trackLen = pTrack->len();
                        //++cursor;
                        /*
                        if (cursor > (lastSeen-1))
                            displayIndex = cursor; */

                        //cursorBeat = 0;
                        //--cursorBeat;

                        ++lastSeen;

                        cursorBeat = 0;
                        ++cursor;

                    }
                    else
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
            }
            else
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

    //commands
    if (press == CONF_PARAM("TrackView.setPause"))
    {
        SingleCommand command(7);
        command.setPosition(0,cursor,cursorBeat);
        command.requestStoredNotes();

        for (ul i = 0; i < pTrack->getV(cursor)->getV(cursorBeat)->len(); ++i)
        {
            Note *note = pTrack->getV(cursor)->getV(cursorBeat)->getV(i);
            command.storedNotes->push_back(note);
        }

        commandSequence.push_back(command);

        pTrack->getV(cursor)->getV(cursorBeat)->setPause(true);
        pTrack->getV(cursor)->getV(cursorBeat)->clear();

        digitPress = -1;
        return;
    }

    if (press == "delete bar")
    {
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

    if (press == "delete selected bars")
    {
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

    if (press == "delete selected beats")
    {
        if (selectionBarFirst != -1)
        {
            pTrack->connectAll();

            Beat *firstBeat,*lastBeat;

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

    if (press == CONF_PARAM("TrackView.deleteNote"))
    {
        if (pTrack->getV(cursor)->getV(cursorBeat)->len())
        {

            SingleCommand command(8);
            command.setPosition(0,cursor,cursorBeat);
            command.requestStoredNotes();

            Note *note = pTrack->getV(cursor)->getV(cursorBeat)->getNoteInstance(stringCursor+1);
            command.storedNotes->push_back(note);

            if (note->getFret()!=255)
            {
                //delete one note
                pTrack->getV(cursor)->getV(cursorBeat)->deleteNote(stringCursor+1);//shift from 0 to 1

                commandSequence.push_back(command);
            }
            else
                delete note;

        }
        else
        {
            //delete pause
            if (pTrack->getV(cursor)->len() > 1)
            {
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

    if (press == CONF_PARAM("TrackView.increaceDuration"))
    {
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

    if (press == CONF_PARAM("TrackView.decreaceDuration"))
    {
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

    /*
    if (press == "i")
    {
        //insert pause in the end (later cover under navigation)
        //attention
        //refact
        byte dur = pTrack->getV(cursor).getV(0).getDuration();
        Beat areat;
        newBeat.setDuration(dur);
        newBeat.setPause(true);
        pTrack->getV(cursor).add(newBeat);
    }
    */

    if ((press == CONF_PARAM("TrackView.playAMusic")) || (press == CONF_PARAM("TrackView.playMidi"))
            || (press=="playMerge"))
    {




        //pre action for repeat
        //AUTO END WHEN ANIMATION STOPED

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

            if (cursor != 0)
            {
                Bar *barPtr = pTrack->getV(cursor);

                for (ul i = 0; i < pTrack->timeLoop.len();++i)
                {
                     if (pTrack->timeLoop.getV(i) == barPtr)
                     {
                          shiftTheCursor = i;
                         break;
                     }
                }
            }

            //CHECK GENERATION NEEDED?


            //REPAIR CHAINS
            clock_t beforeT = getTime();
            pTrack->connectAll();
            clock_t afterT = getTime();
            int diffT = afterT - beforeT;

            LOG( <<"Repair chains "<<diffT);



            Tab *tab = tabParrent->getTab();

            tab->connectTracks();

            MidiFile generatedMidi;

            if (press == CONF_PARAM("TrackView.playAMusic"))
            {
                AMusic exporter;
                exporter.readFromTab(tab,shiftTheCursor);
                generatedMidi.generateFromAMusic(exporter);
            }
            else
            {
                generatedMidi.fromTab(tabParrent->getTab(),shiftTheCursor);
            }


            if ((CONF_PARAM("mergeMidiTracks")=="1") || (press=="playMerge"))
            {
                MidiTrack *newTrack = MidiEngine::uniteFileToTrack(&generatedMidi);
                generatedMidi.clear();
                generatedMidi.add(newTrack);
                //return; //MidiEngine::playTrack(newTrack);
            }

//WHEN NEED TO DEBUGMIDI:
            //good to add config value then
            //generatedMidi.calculateHeader();
            //generatedMidi.printToStream(std::cout);


            clock_t after2T = getTime();
            diffT = after2T - afterT;
            diffT /= (CLOCKS_PER_SEC/1000);
            LOG( <<"Generate midi "<<diffT);

            MidiEngine::closeDefaultFile();
            std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");

            AFile outFile2;

            if (!outFile2.open(fullOutName,false))
                logger << "Failed to open out file :(";
            else
                LOG( <<"File opened "<<fullOutName.c_str());


            generatedMidi.writeStream(outFile2);
            outFile2.close();

            //check time line
            //generatedMidi.getV(0)->printToStream(std::cout);

            if (CONF_PARAM("midi.config").empty() == false)
            {
                MidiToPcm generator(CONF_PARAM("midi.config"));

                std::string outputSound = getTestsLocation() + std::string("waveOutput.wav");

                generator.convert(fullOutName,outputSound);

            }


            //prepareThread();

            //IS BUG HERE:
            tabParrent->prepareAllThreads(shiftTheCursor);

            //connect current thread - not best
            //getMaster()->connectThread(localThr);
            //BYC TOO
            tabParrent->connectAllThreadsSignal(getMaster());

            std::string midiConfig = CONF_PARAM("midi.config");


            if (CONF_PARAM("midi.config").empty() == false)
            {
                ///NEED TO SEND start_record_output waveOutput.wav
                getMaster()->pushForceKey("start_record_output waveOutput.wav");

            }
            else
            {
                MidiEngine::openDefaultFile();
                MidiEngine::startDefaultFile();
            }

            //STARTS a little bit earlier then playback with pcm

            ///STATE changed could be watched (SIGNAL OF QAUDIOINPUT)

            tabParrent->launchAllThreads();

            tabParrent->setPlaying(true);

            //mute_x.unlock();

        }
        else
        {
            if (CONF_PARAM("midi.config").empty() == false)
            {
                    getMaster()->pushForceKey("stop_record_output");
            }
            else
            MidiEngine::stopDefaultFile();

            tabParrent->stopAllThreads();
            tabParrent->setPlaying(false);
        }
        return;
    }

    if (press == CONF_PARAM("TrackView.save")||(press == "quicksave"))
    {
        //save
        GmyFile gmyFile; //fkldsjfkldsj

        AFile file;
        stringExtended gfileName;
        gfileName <<  getTestsLocation() <<"first.gmy";
        file.open(gfileName.c_str(), false);

        gmyFile.saveToFile(&file,tabParrent->getTab());

        file.close();
        return;

    }

    if (press == "save as")
    {

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

            AFile file;
            std::string  gfileName = saveFileName.toStdString();
            file.open(gfileName.c_str(), false);

            gmyFile.saveToFile(&file,tabParrent->getTab());

            file.close();
            return;
    }

    if (press == "newBar")
    {
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

    if (press == "dot") //link
    {
        byte dotted = pTrack->getV(cursor)->getV(cursorBeat)->getDotted();

        SingleCommand command(6,dotted);
        command.setPosition(0,cursor,cursorBeat);
        commandSequence.push_back(command);

        if (dotted & 1)
            pTrack->getV(cursor)->getV(cursorBeat)->setDotted(0);
        else
            pTrack->getV(cursor)->getV(cursorBeat)->setDotted(1);

        return;
    }

    if (press == "-3-")
    {
        byte curDetail = pTrack->getV(cursor)->getV(cursorBeat)->getDurationDetail();

        SingleCommand command(5,curDetail);
        command.setPosition(0,cursor,cursorBeat);
        commandSequence.push_back(command);

        if (curDetail == 3)
        {
            pTrack->getV(cursor)->getV(cursorBeat)->setDurationDetail(0);
        }
        else
            pTrack->getV(cursor)->getV(cursorBeat)->setDurationDetail(3);
        return;
    }

    if (press == "leeg")
    {   
        switchNoteState(2);
        digitPress = -1;
        return;
    }

    if (press == "dead")
    {
        switchNoteState(3);
        digitPress = -1;
        return;
    }

    //EFFECTS - have yet the same problem

    if (press == CONF_PARAM("effects.vibrato"))
        switchEffect(1);

    if (press == CONF_PARAM("effects.slide"))
{
        switchEffect(4);
        return;
        }

    if (press == CONF_PARAM("effects.hammer"))
{
        switchEffect(10);
        return;
        }

    if (press == CONF_PARAM("effects.letring"))
{
        switchEffect(18);
        return;
        }

    if (press == CONF_PARAM("effects.palmmute"))
{
        switchEffect(2);
        return;
    }

    if (press == CONF_PARAM("effects.harmonics"))
    {
        switchEffect(14);
        return;
    }

    if (press == "trem")
    {
        switchEffect(24); //tremlo picking
        return;//
    }

    if (press == CONF_PARAM("effects.trill"))
{
        switchEffect(24);

return;
}

    if (press == CONF_PARAM("effects.stokatto"))
{
        switchEffect(23);
return;
}

    if (press == "tapp") //link
    return;
    if (press == "slap")
    return;
    if (press == "pop")
    return;

    if (press == CONF_PARAM("effects.fadein"))
    {
        switchBeatEffect(20);
        return;
    }


    if (press == "up m")
    {
return;
    }
    if (press == "down m")
    {
return;
    }

    if (press == CONF_PARAM("effects.accent"))
    {
        switchEffect(27);
        return;
    }

    if (press == "h acc")
    {
            switchEffect(27); ///should be another
    return;
    }

    //with imputs

    if (press == "bend")
    {
        Note *currentNote = pTrack->getV(cursor)->getV(cursorBeat)
        ->getNote(stringCursor+1);


       if (currentNote->effPack.get(17))
       {
           Package *bendPack = currentNote->effPack.getPack(17);
           if (bendPack) //attention possible errors escaped
           {
            BendPoints *bend = (BendPoints*) bendPack->getPointer();
            BendInput::setPtrNote(currentNote);
            BendInput::setPtrBend(bend);
           }
           else
           {

               BendInput::setPtrBend(0);
               BendInput::setPtrNote(currentNote);
           }
       }
       else
       {
            BendInput::setPtrBend(0);
            BendInput::setPtrNote(currentNote);
       }

        if (getMaster())
            getMaster()->pushForceKey("bend_view");
    }
    if (press == "chord")
    {

        if (getMaster())
            getMaster()->pushForceKey("chord_view");
            return;
    }
    if (press == "text")
    {
        Beat *beat = pTrack->getV(cursor)->getV(cursorBeat);

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
    if (press == "changes")
    {
        Beat *beat = pTrack->getV(cursor)->getV(cursorBeat);

        ChangesInput::setPtrBeat(beat);


    if (getMaster())
        getMaster()->pushForceKey("change_view");
        return;
    }
    if (press == "fing")
    {
        return;
    }

    if (press == "upstroke")
    {
        switchBeatEffect(25);
        return;
    }

    if (press == "downstroke")
    {
        switchBeatEffect(26);
        return;
    }

    if (press == "signs")
    {
        bool ok=false;

        byte oldDen = pTrack->getV(cursor)->getSignDenum();
        byte oldNum = pTrack->getV(cursor)->getSignNum();

        int newNum = QInputDialog::getInt(0,"Input",
                             "New Num:", QLineEdit::Normal,
                             1,128,1,&ok);

        bool thereWasChange = false;

        if (ok)
        {
            pTrack->getV(cursor)->setSignNum(newNum);
            thereWasChange = true;
        }

        ok=false;

        //GET ITEM
        int newDen = QInputDialog::getInt(0,"Input",
                             "New Denum(1,2,4,8,16):", QLineEdit::Normal,
                             1,128,1,&ok);
        if (ok)
        {
            pTrack->getV(cursor)->setSignDenum(newDen);
            thereWasChange = true;
        }

        if (thereWasChange)
        {
            //also could set to all here if turned on such flag

            if ((pTrack->getV(cursor)->getSignDenum() != oldDen) ||
                (pTrack->getV(cursor)->getSignNum() != oldNum))
            {
                SingleCommand command(19);
                command.setPosition(0,cursor,0);
                command.setValue(oldDen);
                command.setValue2(oldNum);
                commandSequence.push_back(command);
            }
        }
    }




    if (press == "cut")
    {
        //oups - yet works only without selection for 1 bar
        if (selectionBarFirst == -1)
        {
            //int trackInd=tabParrent->getLastOpenedTrack();

            Bar *cloner = new Bar;
            cloner->flush();
            cloner->clone(pTrack->getV(cursor));

            AClipboard::current()->setPtr(cloner);
            AClipboard::current()->setType(4);

            keyevent("delete bar");

        }
    }

    if (press == "copy") //1 bar
    {
        //copyIndex = cursor;

        if (selectionBarFirst == -1)
        {
            //int trackInd=tabParrent->getLastOpenedTrack();
            /*
            AClipboard::current()->setBeginIndexes(trackInd,cursor);
            AClipboard::current()->setType(0); //copy single bar
            */
            Bar *cloner = new Bar;
            cloner->flush();
            cloner->clone(pTrack->getV(cursor));

            AClipboard::current()->setPtr(cloner);
            AClipboard::current()->setType(4);
        }
        else
        {
            int trackInd=tabParrent->getLastOpenedTrack();
            AClipboard::current()->setBeginIndexes(trackInd,selectionBarFirst,selectionBeatFirst);
            AClipboard::current()->setType(1); //copy single beat
            AClipboard::current()->setEndIndexes(trackInd,selectionBarLast,selectionBeatLast);
        }
        selectionBarFirst=selectionBarLast=selectionBeatFirst=selectionBeatLast=-1;


        //AClipboard::current()->setType(4);
        //AClipboard::current()->setPtr();

        return;
    }

    if (press == "copyBeat")
    {
    //replace selected if selectionBarFirst ==-1
        int trackInd=tabParrent->getLastOpenedTrack();

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

    if (press == "copyBars") //copybaryy
    {
    int trackInd=tabParrent->getLastOpenedTrack();
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

    if (press == "paste")
    {
        if (AClipboard::current()->getType() >= 0)
        {
            Tab *tab = tabParrent->getTab();

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
                        for (int beats = AClipboard::current()->getBeatIndex();
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

    if (press == "undo")
    {
        if (commandSequence.size())
        {
            SingleCommand lastCommand = commandSequence[commandSequence.size()-1];
            commandSequence.pop_back();
            reverseCommand(lastCommand);
        }
        return;
    }

    //pannels switching
/*
    if (press == "eff")
    {
        if (pan == trackPan)
            pan = effPan;
        else
            pan = trackPan;

        pan->setW(getMaster()->getWidth());
        pan->setH(getMaster()->getHeist());
        pan->resetButtons();
    }

    if (press == "clip")
    {
        if (pan == trackPan)
            pan = clipPan;
        else
            pan = trackPan;

        pan->setW(getMaster()->getWidth());
        pan->setH(getMaster()->getHeight());
        pan->resetButtons();
    }
    */

    tabParrent->keyevent(press);
}

void TabView::keyevent(std::string press)
{
    if (press == "set till the end")
    {

        bool ok=false;
        int newNum = QInputDialog::getInt(0,"Input",
                             "New Num:", QLineEdit::Normal,
                             1,128,1,&ok);
        if (!ok)
            return;

        ok=false;

        //GET ITEM
        int newDen = QInputDialog::getInt(0,"Input",
                             "New Denum(1,2,4,8,16):", QLineEdit::Normal,
                             1,128,1,&ok);
        if (ok)
        {
            //how to make undo? list of index old num old denum?
            for (int i = currentBar; i < pTab->getV(0)->len(); ++i)
            {
                pTab->getV(0)->getV(i)->setSignDenum(newDen);
                pTab->getV(0)->getV(i)->setSignNum(newNum);
            }
        }
    }



    if (press == "save as")
    {

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

            AFile file;
            std::string  gfileName = saveFileName.toStdString();
            file.open(gfileName.c_str(), false);

            gmyFile.saveToFile(&file,getTab());

            file.close();
    }

    if (press=="export_midi")
    {
        Tab *tab = getTab();

        if (tab)
        {
            tab->connectTracks();

            MidiFile generatedMidi;
            generatedMidi.fromTab(tab,0);

            std::string preFN;

    #ifdef __ANDROID_API__
            preFN = "/sdcard/" ;
    #else
            preFN = getTestsLocation();
    #endif

            std::string fullOutName = preFN + std::string("midiOutput.mid");

            AFile outFile;

            if (!outFile.open(fullOutName,false))
                logger << "Failed to open out file :(";
            else
                LOG( <<"File opened "<<fullOutName.c_str(););

            generatedMidi.writeStream(outFile);
            outFile.close();
        }
    }

    if (press=="mute")
    {
        byte curStat = pTab->getV(displayTrack)->getStatus();

        if (curStat==1)
            pTab->getV(displayTrack)->setStatus(0);
        else
            pTab->getV(displayTrack)->setStatus(1);
    }

    if (press =="solo")
    {
        byte curStat = pTab->getV(displayTrack)->getStatus();

        if (curStat==2)
            pTab->getV(displayTrack)->setStatus(0);
        else
            pTab->getV(displayTrack)->setStatus(2);
    }

    if (press==">>>")
    {
        if (displayBar < (pTab->getV(0)->len()-1))
            ++displayBar;
    }
    if (press=="<<<")
    {
        if (displayBar > 0)
            --displayBar;
    }
    if (press=="^^^")
    {
        if (displayTrack > 0)
        {
            --displayTrack;
            //if not seen
            currentTrack = displayTrack; //escape fine condition
            //still no errors
        }
    }
    if (press=="vvv")
    {
        if (displayTrack < (pTab->len()-1))
        {
            ++displayTrack;

            if (currentTrack < displayTrack)
                currentTrack = displayTrack;
        }
    }

    if (press=="drums")
    {
        bool drums = pTab->getV(displayTrack)->isDrums();
        drums = !drums;
        pTab->getV(displayTrack)->setDrums(drums);
    }

    if (press=="instr")
    {
    std::string instruments[]=
            {
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
                "Gunshot"
            };

     bool ok=false;
        QStringList items;
        for (int i = 0 ; i < 128; ++i)
        {
            stringExtended fullLine;
            fullLine << i <<" - "<<instruments[i];
            items.push_back(fullLine.c_str());
        }

        int curInstr = pTab->getV(displayTrack)->getInstrument();

        QString result = QInputDialog::getItem(0,"Input",
                                     "New Instrument:",items, curInstr,false,&ok);
        if (ok)
        {
            int backToNumber = -1;
            for (int i = 0 ; i < 128; ++i)
            {
                if (result == items[i])
                {
                    backToNumber = i;
                    break;
                }
            }

            if (backToNumber>=0)
            pTab->getV(displayTrack)->setInstrument(backToNumber);

            //shh
            int centerX = 0;
            getMaster()->setComboBox(1,"instruments",240+centerX,5,200,30,backToNumber);

            ///NEED UPDATE HERE combobox
        }
          //
    }

    if (press=="pan")
    {
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

        int curPan = pTab->getV(displayTrack)->getPan();
        QString result = QInputDialog::getItem(0,"Input",
                                     "New Panoram:",items, curPan,false,&ok);

        if (ok)
        {
            int backToNumber = -1;
            for (int i = 0 ; i < 128; ++i)
            {
                if (result == items[i])
                {
                    backToNumber = i;
                    break;
                }
            }

           if (backToNumber >= 0)
            pTab->getV(displayTrack)->setPan(backToNumber);

           int centerX = 0;
           getMaster()->setComboBox(6,"pan",570+centerX,5,50,30,backToNumber);

        }
    }

    if (press=="volume")
    {
        bool ok=false;

        int newVol = QInputDialog::getInt(0,"Input",
                             "Vol Instrument:", QLineEdit::Normal,
                             0,16,1,&ok);
        if (ok)
           pTab->getV(displayTrack)->setVolume(newVol);
    }

    if (press=="name")
    {
        bool ok=false;

        //refact inputs to gview
        QString newName = QInputDialog::getText(0,"Input",
                             "New Instrument name:", QLineEdit::Normal,"untitled",&ok);


        std::string stdName = newName.toStdString();

        if (ok)
           pTab->getV(displayTrack)->setName(stdName);
    }

    if (press=="bpm")
    {
        bool ok=false;

        int newBpm = QInputDialog::getInt(0,"Input",
                             "New Bpm:", QLineEdit::Normal,
                             1,999,1,&ok);
        if (ok)
        {
            pTab->setBPM(newBpm);

            stringExtended sX;
            sX<<"bpm="<<newBpm;
            bpmLabel->setText(sX.c_str());

            stringExtended statusBar2;

            statusBar2 << "BPM = "<<pTab->getBPM();

            getMaster()->setStatusBarMessage(2,statusBar2.c_str());
        }
    }

    if (press=="opentrack")
    {
        ul digit = displayTrack + 1;

        if (digit)
        if (digit <= pTab->len())
        {
            TrackView *trackView = tracksView[digit-1];

            lastOpenedTrack = digit-1;

            MainView *mainView = (MainView*)getMaster()->getFirstChild();
            mainView->changeCurrentView(trackView);
        }
    }

    if (press=="newTrack")
    {
        //hmmm ;)
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


        for (ul barI=0; barI < pTab->getV(0)->len(); ++barI)
        {
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

        //track.connectAll();

        pTab->add(track);

        pTab->connectTracks();

        //addSingleTrack( &(pTab->getV(pTab->len()-1)));


        this->setTab(pTab);
    }

    if (press=="deleteTrack")
    {
        bool ok=false;
        int inp = QInputDialog::getInt(0,"Delete track","Delete track",0,0,1,1,&ok);

        if ((ok) && (inp))
        {
            pTab->remove(displayTrack);

            if (displayTrack)
                --displayTrack;
        }
    }


    if ((press == CONF_PARAM("TrackView.playAMusic")) || (press == CONF_PARAM("TrackView.playMidi")))
    {
        //pre action for repeat
        if (getPlaying()==true)
        {
            if (localThr)
            {
                if (localThr->getStatus())
                {
                    //animation stopped
                    setPlaying(false);
                    //cursor = displayIndex; //auto repeat from page
                    //cursorBeat = 0;
                }
            }
        }


        if (getPlaying() == false)
        {

            //to start not from begin always
            ul shiftTheCursor = 0;


            if (currentBar != 0)
            {
                Bar *barPtr = pTab->getV(0)->getV(currentBar);

                for (ul i = 0; i < pTab->getV(0)->timeLoop.len();++i)
                {
                     if (pTab->getV(0)->timeLoop.getV(i) == barPtr)
                     {
                          shiftTheCursor = i;
                         break;
                     }
                }
            }

            //CHECK GENERATION NEEDED?


            //REPAIR CHAINS
            //clock_t beforeT = getTime();
            //pTrack->connectAll();
            //clock_t afterT = getTime();
            //int diffT = afterT - beforeT;

            //logger <<"Repair chains "<<diffT;



            Tab *tab = getTab();
            tab->connectTracks();

            MidiFile generatedMidi;

            if (press == CONF_PARAM("TrackView.playAMusic"))
            {
                AMusic exporter;
                exporter.readFromTab(tab,shiftTheCursor);
                generatedMidi.generateFromAMusic(exporter);
            }
            else
            {
                generatedMidi.fromTab(getTab(),shiftTheCursor);
            }

            //clock_t after2T = getTime();
            //diffT = after2T - afterT;
            //logger <<"Generate midi "<<diffT;

            MidiEngine::closeDefaultFile();
            std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");

            AFile outFile2;

            if (!outFile2.open(fullOutName,false))
                logger << "Failed to open out file :(";
            else
                LOG( <<"File opened "<<fullOutName.c_str());


            generatedMidi.writeStream(outFile2);
            outFile2.close();

            //prepareThread();
            prepareAllThreads(shiftTheCursor);

            //connect current thread - not best
            //getMaster()->connectThread(localThr);
            connectAllThreadsSignal(getMaster());

            MidiEngine::openDefaultFile();
            MidiEngine::startDefaultFile();

            launchAllThreads();

            setPlaying(true);

        }
        else
        {
            MidiEngine::stopDefaultFile();

            stopAllThreads();
            setPlaying(false);
        }
    }

    if ((press == CONF_PARAM("TrackView.playAMusic"))
|| (press == CONF_PARAM("TrackView.playMidi"))
|| (press=="playMerge"))
    {
        return;
        if (isPlaying)
        {
            MidiEngine::stopDefaultFile();
            isPlaying = false;
            return;
        }
        AMusic exporter;


        Tab *tab = pTab;
        MidiFile generatedMidi;

        if (press == CONF_PARAM("TrackView.playAMusic"))
        {
            exporter.readFromTab(tab);
            generatedMidi.generateFromAMusic(exporter);
        }
        else
        {
            generatedMidi.fromTab(pTab);
        }

        if ((CONF_PARAM("mergeMidiTracks")=="1") || (press=="playMerge"))
        {
            MidiTrack *newTrack = MidiEngine::uniteFileToTrack(&generatedMidi);
            generatedMidi.clear();
            generatedMidi.add(newTrack);
            //return; //MidiEngine::playTrack(newTrack);
        }

        MidiEngine::closeDefaultFile();
        std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");

        AFile outFile2;

        if (! outFile2.open(fullOutName,false))
        {
            //if (gViewLog)
            logger << "Failed to open out file :(";

            statusLabel->setText("failed to open generated");
        }
        ul outFileSize2 = generatedMidi.writeStream(outFile2);
        LOG( << "File wroten. " << outFileSize2 << " bytes. ");
        outFile2.close();
        press = "p"; //autoplay
    }


    if ((press==CONF_PARAM("TabView.genAMusic"))||(press==CONF_PARAM("TabView.genMidi")) ) //|| (press == "spc"))
    {
        //generate

        AMusic exporter;


        Tab *tab = pTab;
        MidiFile generatedMidi;

        if (press == CONF_PARAM("TabView.genAMusic"))
        {
            exporter.readFromTab(tab);
            generatedMidi.generateFromAMusic(exporter);
        }
        else
        {
            generatedMidi.fromTab(pTab);
        }

        MidiEngine::closeDefaultFile();
        std::string fullOutName = getTestsLocation() + std::string("midiOutput.mid");

        AFile outFile2;

        if (! outFile2.open(fullOutName,false))
        {
            //if (gViewLog)
            logger << "Failed to open out file :(";

            statusLabel->setText("failed to open generated");
        }


        //ul outFileSize = midiFile.writeStream(outFile);
        ul outFileSize2 = generatedMidi.writeStream(outFile2);

        LOG( << "File wroten. " << outFileSize2 << " bytes. ");

        outFile2.close();

        generatedMidi.printToStream(std::cout);

        statusLabel->setText("generation done. p for play");

        //press == "p";
    }


    if (press=="p")
    {
        if (isPlaying == false)
        {
            MidiEngine::closeDefaultFile();
            MidiEngine::openDefaultFile();
            MidiEngine::startDefaultFile();
            isPlaying = true;
        }
        else
        {
            MidiEngine::stopDefaultFile();
            isPlaying = false;
        }
    }

    if (isdigit(*(press.c_str())))
    {
        ul digit = press.c_str()[0]-48;

        if (digit)
        if (digit <= pTab->len())
        {
            TrackView *trackView = tracksView[digit-1];//new TrackView(&pTab->getV(digit-1));

            lastOpenedTrack = digit-1;



            MainView *mainView = (MainView*)getMaster()->getFirstChild();
            mainView->changeCurrentView(trackView);
        }
        //store in pull
    }

    if (press == "marker")
    {
        //GET text + - color

        bool ok=false;
        QString markerText= QInputDialog::getText(0,"Input",
                             "Marker:", QLineEdit::Normal,"untitled",&ok);

        if (ok)
        {
            std::string stdMarkerText = markerText.toStdString();
            pTab->getV(0)->getV(currentBar)->setGPCOMPMarker(stdMarkerText,0);
        }
    }

    //MUST MOVE OUT FOR UNDO

    if (press == "|:")
    {
       byte repeat = pTab->getV(0)->getV(currentBar)->getRepeat();

       byte repeatOpens = repeat & 1;
       byte repeatCloses = repeat & 2;

       if (repeatOpens)
       {
        pTab->getV(0)->getV(currentBar)->setRepeat(0); //flush
        pTab->getV(0)->getV(currentBar)->setRepeat(repeatCloses);
       }
       else
        pTab->getV(0)->getV(currentBar)->setRepeat(1);
    }

    if (press == ":|")
    {
        byte repeat = pTab->getV(0)->getV(currentBar)->getRepeat();

        byte repeatOpens = repeat & 1;
        byte repeatCloses = repeat & 2;

        if (repeatCloses)
        {
            pTab->getV(0)->getV(currentBar)->setRepeat(0); //flush
            pTab->getV(0)->getV(currentBar)->setRepeat(repeatOpens);
          //must refact
        }
        else
        {
            bool ok=false;
            int newTimes = QInputDialog::getInt(0,"Input",
                                 "Repeat times:", QLineEdit::Normal,
                                 2,99,1,&ok);

            if ((ok)&&(newTimes))
                pTab->getV(0)->getV(currentBar)->setRepeat(2,newTimes);

        }
    }

    if (press == "goToN")
    {
        bool ok=false;
        int newTimes = QInputDialog::getInt(0,"Input",
                             "Bar to jump:", QLineEdit::Normal,
                             1,pTab->getV(0)->len(),1,&ok);

        if (ok)
        {
            --newTimes;
            currentBar = newTimes;
            displayBar = newTimes;

        }
    }

    if (press == "alt")
    {
        //GET ITEMS?
    }

    if (press == "tune")
    {
        //GET ITEM
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


    for (int i = 0; i < 8; ++i)
    {
        //octaves

        iBuf[0] = i +49;
        iBuf[1] = 0;
        std::string octave = iBuf;
        std::string note;


        int midiNote = 0;

        for (int j = 0; j < 12; ++j)
        {
            //Notes
            switch (j)
            {
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
            stringExtended freq;
            freq<<theFreq;

            midiNote = 12+j + 12*i;


            stringExtended fullLine;
            fullLine << note <<octave << " - "<<midiNote<<" - "<<freq.c_str();
            items.push_back(fullLine.c_str());
        }


    }

    //items.push_back("another thesr");
    //items.push_back("once_more");



        for (int i = 0; i < pTab->getV(currentTrack)->tuning.getStringsAmount(); ++i)
        {
            stringExtended label;
            label << "String #"<<(i+1);

            int preValue = pTab->getV(currentTrack)->tuning.getTune(i)-12;

            QString resp = QInputDialog::getItem(0,"Input tune",
                                         label.c_str(),items,preValue,false,&ok);

            int respIndex = -1;

            for (int j = 0; j < items.size(); ++j)
            {
                if (items.at(j)==resp)
                {
                    respIndex = j;
                    break;
                }
            }

            if (ok)
            {
                if (respIndex>=0)
                    pTab->getV(currentTrack)->tuning.setTune(i,respIndex+12);
            }
        }
    }

}
