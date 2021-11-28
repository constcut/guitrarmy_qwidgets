#include "track.h"

#include <QDebug>

bool trackLog = false;


void Track::printToStream(std::ostream &stream)
{
    stream << "Outputing #"<<len()<<" Bars."<<std::endl;
    for (ul ind = 0; ind < len(); ++ind)
            at(ind)->printToStream(stream);
}


void Track::connectAll()
{
    if (trackLog)
        qDebug() <<"Connection initiated for track "<<name.c_str();

    connectBars();
    connectBeats(); //uncomment for
    connectNotes(); //uncomment for effects refactoring
    connectTimeLoop();

    if (trackLog)
        qDebug() <<"Connection finished for track "<<name.c_str();

    return;
}



ul Track::connectNotes() //for let ring
{
   Bar* curBar;
   Beat *curBeat;
   ul index=0;

   if (len() == 0)
       return 0;

   curBar = at(0);

   if (curBar->len())
   curBeat = curBar->at(0);
   else
   {
       while (curBar && curBar->len()==0)
            curBar = (Bar*)curBar->getNext();

       if (curBar==0) return 0;

       curBeat = curBar->at(0);

   }

   Note *ringRay[16] = {0}; //let this remember to set max strings up to 16
   ul indRay[10] = {0};
   //byte  fretRay[16] = {0};
   ul notesCount[16] = {0};

   ul count = 0;
   ul beatIndex = 0;

   while (index < beatsAmount)
   {
       //connect simmiliar note values - good for let ring and leeg
      //ring ray - here to move


       for (ul noteI=0; noteI < curBeat->len(); ++noteI)
       {
           Note *curNote = curBeat->at(noteI);

           byte stringN = curNote->getStringNumber();
           Note *prevNote = ringRay[stringN];
           ul prevInd = indRay[stringN];

           if (prevNote)
           {
               prevNote->setNext(curNote);
               curNote->setPrev(prevNote);
           }

           /*
           byte noteState = curNote->getState();

           if (curNote->getFret() == 63||
                   (noteState==2|| //enum NoteState
                    noteState==4||
                    noteState==6))
           {
               byte prevFret = 64;

               if (prevNote)
               {
                   prevFret = prevNote->getFret();
                   curNote->setFret(prevFret);
                   curNote->signStateLeeged();

                   //EFFECTS COPY!
                   //AND MAYBE MORE WIDE CONDITION
                   EffectsPack prevEff = prevNote->getEffects();
                   curNote->addEffects(prevEff);
               }

               if (prevFret == 64)
               {
                   qDebug() <<"Prev Note "<<(int)prevNote<<" sN= "<<stringN;
                       qDebug() <<"Is Pause "<<(int)curBeat->getPause();
                       qDebug() <<"Count="<<count<<"; notesCount="<<notesCount[stringN];
               }

               qDebug() <<"Fret was 63 now "<<prevFret;
           }

           */

           //SOME GUITAR PRO ACTION




           byte noteState = curNote->getState();

           if (noteState == 2)
           if (prevNote)
           {
               if ((index-prevInd)>1)
                   curNote->setState(3); //dead it
               else
               {
                   byte prevFret = prevNote->getFret();
                   if (trackLog)
                   qDebug() << "Prev found "<<prevNote->getStringNumber()<<
                          " "<<prevFret;

                   //curBeat

                   prevNote->signStateLeeged();


                   //Full copy not yet used well
                   EffectsPack prevEff = prevNote->getEffects();
                   curNote->addEffects(prevEff);


                   curNote->setFret(prevFret);

                   if (prevFret==63)
                   {
                       //prevNote->setState(3); //dead
                       //curNote->setState(3);
                   }
               }
           }
           else
               curNote->setState(3); //dead it

           byte newNoteState = curNote->getState();
           byte nowFret = curNote->getFret();
           if (nowFret == 63)
               curNote->setState(3); //dead it


           if (trackLog)
               qDebug() <<stringN<<"ZFret "<<nowFret<<"; newState= "<<newNoteState<<"; oldS= "<<noteState;


           //if (curNote != 0)
           {
               ringRay[stringN] = curNote;
               indRay[stringN] = index;

               ++count;
               ++notesCount[stringN];
           }

       }


       ++index;
       curBeat = (Beat*)curBeat->getNext(); // like ++;
       if (curBeat == 0)
           break; //attention could miss the errors
   }

   if (trackLog)
   qDebug() << "Connect notes done "<<count<<"; S1="<<notesCount[1]<<
          "; S2="<<notesCount[2]<<"; S3="<<notesCount[3]<<"; S4="<<notesCount[4]<<
          "; S5="<<notesCount[5]<<"; S6="<<notesCount[6];

  return count;
}


ul Track::connectBeats()
{
   Bar* curBar;
   Beat *curBeat;

   if (len() == 0)
       return 0;

   curBar = at(0);
   ul trackLen = len();
   ul fullCount=0;

   if (trackLog)
       qDebug() <<"ConnectingBeats";


   for (ul barI = 0; barI < trackLen; ++barI)
   {
       curBar = at(barI);

       if (curBar->len())
       for (ul beatI = 0; beatI < (curBar->len()-1); ++beatI)
       {
           curBeat = curBar->at(beatI);
           Beat *nextBeat = curBar->at(beatI+1);

           if ((nextBeat)&&(curBeat))
           {
               nextBeat->setPrev(curBeat);
               curBeat->setNext(nextBeat);
               ++fullCount;
           }
           else
               qDebug() <<"Issue connecting beats";
       }

       if (barI+1 != trackLen)
       { //not the lastBar
           Bar *nextBar = at(barI+1);

           if (nextBar->len()==0)
               continue;

           /*

           qDebug() << "Next len "<<(int)nextBar->len()<<
                     "Cur len "<<(int)curBar->len()<<" i "<<barI)

           qDebug() << "-";
           */


           curBeat = 0;

           if (curBar->len() == 0)
           {
               Bar *prevBar = curBar;

               /*

               while (1)
               {
                   if (prevBar == 0)
                   {
                       qDebug()<<"Exited extrimly! ";
                       return; ///ehh
                   }

                   prevBar = prevBar->getPrev();

                   if ((prevBar) && (prevBar->len()))
                   {
                       curBeat = prevBar->getV(prevBar->len());
                       break;
                   }


               }

               */
           }
           else
           {
               curBeat = curBar->at(curBar->len()-1);
           }

           Beat *nextBeat = nextBar->at(0);

           if ((nextBeat)&&(curBeat))
           {
               nextBeat->setPrev(curBeat);
               curBeat->setNext(nextBeat);
               ++fullCount;
           }
           else
               qDebug() <<"Issue connecting beatz";
       }
   }

   if (trackLog)
   qDebug() << "Full count of the beats is "<<fullCount;
   beatsAmount = fullCount;

   return fullCount;
}



ul Track::connectBars()
{
    if (trackLog)
        qDebug() <<"ConnectingBars "<<(int)len() ;

    if (len() == 0)
        return 0;

    byte currentNum = at(0)->getSignNum();
    byte currentDen = at(0)->getSignDenum();


    ul trackLen = len();

    for(ul barsI=1; barsI < trackLen; ++barsI)
    {
        at(barsI)->setPrev(at(barsI-1));
        at(barsI-1)->setNext(at(barsI));

        byte thatNum = at(barsI)->getSignNum();
        byte thatDen = at(barsI)->getSignDenum();

        if (thatNum==0)
            at(barsI)->setSignNum(currentNum);
        else
            currentNum = thatNum;

        if (thatDen==0)
            at(barsI)->setSignDenum(currentDen);
        else
            currentDen = thatDen;

    }

    return trackLen;
}




ul Track::connectTimeLoop()
{
   if (len() == 0)
       return 0;

   timeLoop.clear();
   timeLoopIndexStore.clear();

   //FEW WORDS:
    //in guitar pro only 1 takt works after reprize!
    //so no need to search for after tail,
    //and only 1 goes fine in the en - so pretail is - the only :|

   Bar* curBar;

   ul lastIndex = len();
   ul curIndex = 0;
   curBar = at(0);

   Bar *lastBeginRepeat = curBar;

   Bar *beginRepeat=0;
   Bar *endRepeat=0;

   ul beginIndex=0;
   ul endIndex=0;


   Bar *tailEnd=0;
   Bar *tailBegin=0;

   ul tailEndIndex=0;
   ul tailBeginIndex=0;

   if (trackLog)
       qDebug() << "Start connecting time-loop ";

   //THERE IS AUTO SET - to check new way of setting tails:
   /*
   while (curIndex < lastIndex) // curBar != .end() ?
   {
       timeLoop.add(curBar);
       ++curBar; ++curIndex;
   }
   return 0;*/
   //END OF AUTOSET
   //now we start change

   while (curIndex < lastIndex) // curBar != .end() ?
   {

       //minifix attention
       if (curBar==0)
           break;

       if (curBar->getRepeat() & 1)
       {
           if (beginRepeat)
           {
               while (beginRepeat != curBar)
               {
                   timeLoop.add(beginRepeat);
                   timeLoopIndexStore.push_back(beginIndex);

                   ++beginIndex;
                   beginRepeat = (Bar*)beginRepeat->getNext();
               }
           }

           beginRepeat=curBar;
           beginIndex=curIndex;

           if (curBar->getRepeat() & 2)
           {
               //one bar reprize
               endIndex = curIndex;
               endRepeat = curBar;

               pushReprise(beginRepeat,endRepeat,
                           0,0,0, beginIndex, endIndex);

               beginRepeat = endRepeat = 0;
               curBar = (Bar*)curBar->getNext();
               ++curIndex;
               continue;
           }
       }

       if (curBar->getRepeat() & 2)
       {
           endRepeat = curBar;
           endIndex = curIndex;

           //then search for alternative tail
           curBar = (Bar*)curBar->getNext();
           ++curIndex;

           if (curBar)
           {
               if (curBar->getAltRepeat() != 0)
               {
                       tailBegin = curBar;
                       //qDebug() <<"Tail begin set to "<<(int)tailBegin;
                       tailEnd = curBar;
                       tailBeginIndex = curIndex;
               }

               if ((curBar->getRepeat() & 1) == 0)
               {
                   while( curBar->getAltRepeat() != 0 ) //there is alternative
                   {
                       //could be there appear an error later (if there possible not marked alt bars) attention
                       tailEnd = curBar;
                       tailEndIndex = curIndex;
                       curBar = (Bar*)curBar->getNext();
                       ++curIndex;

                       if (curIndex>=lastIndex)
                           break;

                      // qDebug() <<"Tail END reset to "<<(int)tailEnd;
                   }
               }
               else
               {
                   //alt is a start of a new repeat
               }
               //Post-Tail complete
           }


           if (beginRepeat == 0)
               beginRepeat = lastBeginRepeat;

           //Now search for pre-tail in the endings
           Bar *preTail = 0;
           ul preTailIndex = 0;

           //qDebug() <<"RUN PRE TAIL SEARCH "<<(int)beginRepeat<<" to "<<(int)endRepeat;

           int indX=0;
           for (Bar *barI=endRepeat; barI != beginRepeat; barI=(Bar*)barI->getPrev()
                ,++indX)//--barI
           {
               //qDebug() <<"pre-tail search "<<(int)barI;
               if (barI->getAltRepeat() != 0)
               {
                   preTail = barI;
                   preTailIndex = endIndex-indX;
                   //qDebug() <<" got alt ";
               }
           }

           //Now we collect all:
           //BEGIN-END(pre-Tail)-tailBegin-tailEnd
           //and we can push it into timeloop

           pushReprise(beginRepeat,endRepeat,
                       preTail,tailBegin,tailEnd, beginIndex, endIndex,
                       preTailIndex,tailBeginIndex,tailEndIndex);
           //there is issue in tail - it could be only 1 takt long, but here not only


           lastBeginRepeat = beginRepeat;

           //then flush begin and end repeat
           beginRepeat = endRepeat = 0;
           tailBegin = tailEnd = 0;
           preTail = 0;
       }

       //new reprise
       if (curBar)
       {
           if (curBar->getRepeat() & 1)
           {
               beginRepeat=curBar;
               beginIndex=curIndex;
           }

           if (curIndex < lastIndex) //not get out of
           if (beginRepeat == 0)
           {
               timeLoop.add(curBar);
               timeLoopIndexStore.push_back(curIndex);
           }

           curBar = (Bar*)curBar->getNext();
           ++curIndex;
       }
   }

   if (trackLog)
   qDebug() << "TIME LOOP size is "<<(int)timeLoop.len();

   return timeLoop.len();
}




void Track::pushReprise(Bar *beginRepeat, Bar *endRepeat,
                 Bar *preTail, Bar *tailBegin, Bar *tailEnd, ul beginIndex, ul endIndex,
                        ul preTailIndex, ul tailBeginIndex, ul tailEndIndex)
{
    //ChainContainer<Bar*> timeLoop;

    //if (trackLog)
    //qDebug() << "Push reprise called; "<<(int)beginRepeat<<"; "<<(int)endRepeat; //exend with values
    //qDebug() << "Pre: "<<(int)preTail<<"; tailBegin: "<<(int)tailBegin<<"; tE: "<<(int)tailEnd;

    AltRay altRay;
    AltRayInd altRayInd;

    if (preTail == 0)
    { //no alt ending in begin-end
        if (tailBegin == 0)
        { //no alt at all
            for (byte i = 0; i < endRepeat->getRepeatTimes(); ++i)
            {
                ul localIndex = 0;
                for (Bar *barI=beginRepeat; barI != endRepeat; barI=(Bar*)barI->getNext())
                {
                    timeLoop.add(barI);
                    timeLoopIndexStore.push_back(beginIndex+localIndex);
                    ++localIndex;
                }

                timeLoop.add(endRepeat);
                timeLoopIndexStore.push_back(endIndex);
            }
        }
        else
        {
           createAltRay(altRay, altRayInd, tailBegin,tailEnd,tailBeginIndex,tailEndIndex);
           //there is a tail after

           for (byte i = 0; i < endRepeat->getRepeatTimes(); ++i)
           {
               ul localIndex = 0;
               for (Bar *barI=beginRepeat; barI != endRepeat; barI=(Bar*)barI->getNext())
               {
                   timeLoop.add(barI);
                   timeLoopIndexStore.push_back(beginIndex+localIndex);
                   ++localIndex;
               }

               if (altRay.find(i) != altRay.end())
               { //add alt ray from a tail
                   PolyBar *thatEnd = &altRay[i];
                   std::vector<int> thatRayInd = altRayInd[i];

                   if ((thatEnd->at(0)->getRepeat() & 1)==0)
                   { //don't repeat alts that begin

                       for (ul j = 0; j < thatEnd->len(); ++j)
                       {
                           timeLoop.add(thatEnd->at(j));
                           timeLoopIndexStore.push_back(thatRayInd[j]);
                       }
                           //timeLoop += thatEnd;
                   }
               }
               else //add default value
               {
                   timeLoop.add(endRepeat);
                   timeLoopIndexStore.push_back(endIndex);
               }
           }
        }

    }
    else
    {
       //alt endings in  - worst case

       if (tailBegin == 0)
           createAltRay(altRay, altRayInd, preTail,endRepeat,preTailIndex,endIndex);
       else
       {
           createAltRay(altRay, altRayInd, preTail,tailEnd,preTailIndex,tailEndIndex);
       }

           for (byte i = 0; i < endRepeat->getRepeatTimes(); ++i)
           {
               ul localIndex = 0;
               for (Bar *barI=beginRepeat; barI != preTail; barI=(Bar*)barI->getNext())
               {
                   timeLoop.add(barI);
                   timeLoopIndexStore.push_back(beginIndex+localIndex);
                   ++localIndex;
               }

               if (altRay.find(i) != altRay.end())
               { //add alt ray from a tail
                   PolyBar *thatEnd = &altRay[i];
                   std::vector<int> thatRayInd = altRayInd[i];

                   for (ul j = 0; j < thatEnd->len(); ++j)
                   {
                       timeLoop.add(thatEnd->at(j));
                       timeLoopIndexStore.push_back(thatRayInd[j]);
                   }
                       //timeLoop += thatEnd;
               }
               else //add default value
               {
                   timeLoop.add(endRepeat);
                   timeLoopIndexStore.push_back(endIndex);
                   //POSSIBLE THERE would be ISSUE - hard test it
                   if (trackLog)
                   qDebug() << "Tail begin=0, pre tail=1?; attention";
               }
           }
    }

}
