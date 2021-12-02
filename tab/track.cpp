#include "track.h"

#include <iostream>
#include <QDebug>

bool trackLog = false;


void Track::printToStream(std::ostream& stream)
{
    stream << "Outputing #" << size() << " Bars."<< std::endl;
    for (size_t ind = 0; ind < size(); ++ind)
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



size_t Track::connectNotes() //for let ring
{
   Bar* curBar;
   Beat *curBeat;
   size_t index=0;

   if (size() == 0)
       return 0;

   curBar = at(0).get();

   if (curBar->size())
    curBeat = curBar->at(0).get();
   else {
       while (curBar && curBar->size()==0)
            curBar = (Bar*)curBar->getNext();

       if (curBar==0)
            return 0;
       curBeat = curBar->at(0).get();
   }

   Note *ringRay[16] = {0}; //let this remember to set max strings up to 16
   size_t indRay[10] = {0};
   //byte  fretRay[16] = {0};
   size_t notesCount[16] = {0};

   size_t count = 0;
   size_t beatIndex = 0;

   while (index < beatsAmount)
   {
       //connect simmiliar note values - good for let ring and leeg
      //ring ray - here to move


       for (size_t noteI=0; noteI < curBeat->size(); ++noteI)
       {
           Note *curNote = curBeat->at(noteI).get();

           std::uint8_t stringN = curNote->getStringNumber();
           Note *prevNote = ringRay[stringN];
           size_t prevInd = indRay[stringN];

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
                   ABitArray prevEff = prevNote->getEffects();
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




           std::uint8_t noteState = curNote->getState();

           if (noteState == 2)
           if (prevNote)
           {
               if ((index-prevInd)>1)
                   curNote->setState(3); //dead it
               else
               {
                   std::uint8_t prevFret = prevNote->getFret();
                   if (trackLog)
                   qDebug() << "Prev found "<<prevNote->getStringNumber()<<
                          " "<<prevFret;

                   //curBeat

                   prevNote->signStateLeeged();


                   //Full copy not yet used well
                   ABitArray prevEff = prevNote->getEffects();
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

           std::uint8_t newNoteState = curNote->getState();
           std::uint8_t nowFret = curNote->getFret();
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


size_t Track::connectBeats()
{
   Bar* curBar;
   Beat *curBeat;

   if (size() == 0)
       return 0;

   curBar = at(0).get();
   size_t trackLen = size();
   size_t fullCount=0;

   if (trackLog)
       qDebug() <<"ConnectingBeats";


   for (size_t barI = 0; barI < trackLen; ++barI)
   {
       curBar = at(barI).get();

       if (curBar->size())
       for (size_t beatI = 0; beatI < (curBar->size()-1); ++beatI)
       {
           curBeat = curBar->at(beatI).get();
           Beat *nextBeat = curBar->at(beatI+1).get();

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
           Bar *nextBar = at(barI+1).get();

           if (nextBar->size()==0)
               continue;

           /*

           qDebug() << "Next len "<<(int)nextBar->size()<<
                     "Cur len "<<(int)curBar->size()<<" i "<<barI)

           qDebug() << "-";
           */


           curBeat = 0;

           if (curBar->size() == 0)
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

                   if ((prevBar) && (prevBar->size()))
                   {
                       curBeat = prevBar->getV(prevBar->size());
                       break;
                   }


               }

               */
           }
           else
           {
               curBeat = curBar->at(curBar->size()-1).get();
           }

           Beat *nextBeat = nextBar->at(0).get();

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



size_t Track::connectBars()
{
    if (trackLog)
        qDebug() <<"ConnectingBars "<<(int)size() ;

    if (size() == 0)
        return 0;

    std::uint8_t currentNum = at(0)->getSignNum();
    std::uint8_t currentDen = at(0)->getSignDenum();


    size_t trackLen = size();

    for(size_t barsI=1; barsI < trackLen; ++barsI)
    {
        at(barsI)->setPrev(at(barsI-1).get());
        at(barsI-1)->setNext(at(barsI).get());

        std::uint8_t thatNum = at(barsI)->getSignNum();
        std::uint8_t thatDen = at(barsI)->getSignDenum();

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




size_t Track::connectTimeLoop()
{
   if (size() == 0)
       return 0;

   timeLoop.clear();
   timeLoopIndexStore.clear();

   //FEW WORDS:
    //in guitar pro only 1 bar works after reprize!
    //so no need to search for after tail,
    //and only 1 goes fine in the en - so pretail is - the only :|

   size_t lastIndex = size();
   size_t curIndex = 0;
   Bar* curBar = at(0).get();

   Bar *lastBeginRepeat = curBar;

   Bar *beginRepeat=0;
   Bar *endRepeat=0;

   size_t beginIndex=0;
   size_t endIndex=0;


   Bar *tailEnd=0;
   Bar *tailBegin=0;

   size_t tailEndIndex=0;
   size_t tailBeginIndex=0;

   if (trackLog)
       qDebug() << "Start connecting time-loop ";

   while (curIndex < lastIndex)  {

       if (curBar==0) //minifix attention
           break;

       if (curBar->getRepeat() & 1)
       {
           if (beginRepeat)
           {
               while (beginRepeat != curBar)
               {
                   timeLoop.push_back(beginRepeat);
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
                   //alt is a start of a ne w repeat
               }
               //Post-Tail complete
           }


           if (beginRepeat == 0)
               beginRepeat = lastBeginRepeat;

           //Now search for pre-tail in the endings
           Bar *preTail = 0;
           size_t preTailIndex = 0;

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
               timeLoop.push_back(curBar);
               timeLoopIndexStore.push_back(curIndex);
           }

           curBar = (Bar*)curBar->getNext();
           ++curIndex;
       }
   }

   if (trackLog)
   qDebug() << "TIME LOOP size is "<<(int)timeLoop.size();

   return timeLoop.size();
}


//REFACT - cover under Track operations
typedef std::map<std::uint8_t, PolyBar> AltRay;
typedef std::map<std::uint8_t, std::vector<int> > AltRayInd;

void createAltRay(AltRay &altRay, AltRayInd &altRayInd, Bar *a, Bar *b, size_t indA, size_t indB)
{
    //possible we will need value for the default repeat
    std::uint8_t currentAlt = 0;

    //int normalW = (int)b - (int)a;
    //int backW = (int)a - (int)b;
    //qDebug() << "Creating alt ray NORM: "<<normalW<<"; BACK: "<<backW;
    //qDebug() << "Alt a : "<<(int)a<< " alt b :"<<(int)b;

    size_t localInd = 0;
    for (Bar *barI=a; barI!=b; barI=(Bar*)barI->getNext()
         ,++localInd)
    {
       if (trackLog)
        qDebug() << "ALTRAY Bar ptr "<<barI;
       if (barI->getAltRepeat() != 0)
           currentAlt = barI->getAltRepeat();

       for (std::uint8_t i=0; i < 8; ++i) {
           std::uint8_t altMaskI = currentAlt & (1<<i);
           if (altMaskI) {
               altRay[i].push_back(barI);
               //altRayInd[i].push_back()
               altRayInd[i].push_back(localInd+indA);
           }
       }
    }

    currentAlt=b->getAltRepeat();
    for (std::uint8_t i=0; i < 8; ++i) {
        std::uint8_t altMaskI = currentAlt & (1<<i);
        if (altMaskI) {
               altRay[i].push_back(b);
               altRayInd[i].push_back(indB);
        }
    }

}



void Track::pushReprise(Bar *beginRepeat, Bar *endRepeat,
                 Bar *preTail, Bar *tailBegin, Bar *tailEnd, size_t beginIndex, size_t endIndex,
                        size_t preTailIndex, size_t tailBeginIndex, size_t tailEndIndex)
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
            for (std::uint8_t i = 0; i < endRepeat->getRepeatTimes(); ++i)
            {
                size_t localIndex = 0;
                for (Bar *barI=beginRepeat; barI != endRepeat; barI=(Bar*)barI->getNext())
                {
                    timeLoop.push_back(barI);
                    timeLoopIndexStore.push_back(beginIndex+localIndex);
                    ++localIndex;
                }

                timeLoop.push_back(endRepeat);
                timeLoopIndexStore.push_back(endIndex);
            }
        }
        else
        {
           createAltRay(altRay, altRayInd, tailBegin,tailEnd,tailBeginIndex,tailEndIndex);
           //there is a tail after

           for (std::uint8_t i = 0; i < endRepeat->getRepeatTimes(); ++i)
           {
               size_t localIndex = 0;
               for (Bar *barI=beginRepeat; barI != endRepeat; barI=(Bar*)barI->getNext())
               {
                   timeLoop.push_back(barI);
                   timeLoopIndexStore.push_back(beginIndex+localIndex);
                   ++localIndex;
               }

               if (altRay.find(i) != altRay.end())
               { //add alt ray from a tail
                   PolyBar *thatEnd = &altRay[i];
                   std::vector<int> thatRayInd = altRayInd[i];

                   if ((thatEnd->at(0)->getRepeat() & 1)==0)
                   { //don't repeat alts that begin

                       for (size_t j = 0; j < thatEnd->size(); ++j)
                       {
                           timeLoop.push_back(thatEnd->at(j));
                           timeLoopIndexStore.push_back(thatRayInd[j]);
                       }
                           //timeLoop += thatEnd;
                   }
               }
               else //add default value
               {
                   timeLoop.push_back(endRepeat);
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

           for (std::uint8_t i = 0; i < endRepeat->getRepeatTimes(); ++i)
           {
               size_t localIndex = 0;
               for (Bar *barI=beginRepeat; barI != preTail; barI=(Bar*)barI->getNext())
               {
                   timeLoop.push_back(barI);
                   timeLoopIndexStore.push_back(beginIndex+localIndex);
                   ++localIndex;
               }

               if (altRay.find(i) != altRay.end())
               { //add alt ray from a tail
                   PolyBar *thatEnd = &altRay[i];
                   std::vector<int> thatRayInd = altRayInd[i];

                   for (size_t j = 0; j < thatEnd->size(); ++j)
                   {
                       timeLoop.push_back(thatEnd->at(j));
                       timeLoopIndexStore.push_back(thatRayInd[j]);
                   }
                       //timeLoop += thatEnd;
               }
               else //add default value
               {
                   timeLoop.push_back(endRepeat);
                   timeLoopIndexStore.push_back(endIndex);
                   //POSSIBLE THERE would be ISSUE - hard test it
                   if (trackLog)
                   qDebug() << "Tail begin=0, pre tail=1?; attention";
               }
           }
    }

}
