#include "tab.h"

#include "aexpimp.h"

//if (tabLog)  loging
#include "g0/astreaming.h"
static AStreaming   logger("tab");

bool tabLog = false;

#include "g0/amusic.h"

#include <algorithm>

#include <map>

//BEND points fun

void BendPoints::insertNewPoint(BendPoint bendPoint)
{
    for (ul i = 0; i < len()-1; ++i)
    {
        if (getV(i).horizontal < bendPoint.horizontal)
            if (getV(i+1).horizontal > bendPoint.horizontal)
            {
                this->insertBefore(bendPoint,i+1);
                return;
            }
        if (getV(i).horizontal == bendPoint.horizontal)
        {
            getV(i).vertical = bendPoint.vertical;
            return;
        }
    }
    add(bendPoint);
}


////////////////////////////////////////////////////////////////////////////
////EFFECTS SET

void Beat::setEffects(byte eValue)
{
    if (eValue==0)
    {
         effPack.flush();
    }
        else
    effPack.set(eValue,true);
}
EffectsPack Beat::getEffects()
{
    return effPack;
}


void Note::setEffect(byte eff)
{
    if (eff==0)
    {
        effPack.flush();
    }
        else
    effPack.set(eff,true);
}

EffectsPack Note::getEffects()
{
    return effPack;
}

//-----------------OUTPUTING TO STREAM--------------------------------

 void Tab::printToStream(std::ostream &stream)
 {
    stream << "Outputing #"<<len()<<" Tracks."<<std::endl;
    for (ul ind = 0; ind < len(); ++ind)
            getV(ind)->printToStream(stream);

    //as alrternative there
    //SHOULD APPEAR CLASS
    //TextTabList
    //that creates tab lists with numbers
 }

 void Track::printToStream(std::ostream &stream)
 {
     stream << "Outputing #"<<len()<<" Bars."<<std::endl;
     for (ul ind = 0; ind < len(); ++ind)
             getV(ind)->printToStream(stream);
 }

 void Bar::printToStream(std::ostream &stream)
 {
     stream << "Outputing #"<<len()<<" Beats."<<std::endl;
     for (ul ind = 0; ind < len(); ++ind)
             getV(ind)->printToStream(stream);
 }

 void Beat::printToStream(std::ostream &stream)
 {
     stream << "Outputing #"<<len()<<" Notes."<<std::endl;
     for (ul ind = 0; ind < len(); ++ind)
             getV(ind)->printToStream(stream);
 }

 void Note::printToStream(std::ostream &stream)
 {
    stream << "Fret = " <<(int)this->fret << std::endl;
 }

 //----------END-OF------OUTPUTING TO STREAM---------------------------
 ///////////////////////////////////////////////////////////////////////





 ////////////Count bar operations ///////////////////////////////////

 void Bar::countUsedSigns(byte &numGet, byte &denumGet)
 {
     ul num = 0;
     ul denum = 32;

     denum *= 3;//temp action for triplets (for n trumplets better multipleyer must)
     denum *= 2;

     for (ul beatInd = 0; beatInd < len(); ++beatInd)
     {
         ul duration = getV(beatInd)->getDuration();
         ul detail = getV(beatInd)->getDurationDetail();

         ul addition = 0;

         switch (duration)
         { //remember 8 is 8
             case 5: addition=1; break;
             case 4: addition=2; break;
             case 3: addition=4; break;
             case 2: addition=8; break;
             case 1: addition=16; break;
             case 0: addition=32; break;
             default:
             if (tabLog)  LOG(<<"Shit in duration "<<duration);
         }

         addition *=2; //dots on 32
         addition *= 3; //triplets

         if (detail==3)
         {
             addition *= 2;
             addition /= 3;
         }

         bool byteDote = getV(beatInd)->getDotted();
         if (byteDote==1)
         {
             addition += addition/2;
         }

         //recalculations for addition
          num+=addition;
         if (tabLog)  LOG(<<"Addition is "<<addition<<"; det= "<<detail
              <<" dur= "<<duration<<" full "<<num<<"; dot "<<byteDote);
     }

     //decre

     while ( (num%2==0)&&(denum%2==0) )
     {
         num /= 2;
         denum /= 2;
     }

     while ( (num%3==0)&&(denum%3==0) )
     {
         num /= 3;
         denum /= 3;
     }

     numGet = num;
     denumGet = denum;
 }


 //move code to caclCompleteStatus

 int translateDenum(byte den)
 {
     switch (den)
     {
        case 1: return 4000;
        case 2: return 2000;
        case 4: return 1000;
        case 8: return 500;
        case 16: return 250;
        case 32: return 125;
     }

     return 0;
 }

 int translaeDuration(byte dur)
 {
     switch (dur)
     {
        case 0: return 4000;
        case 1: return 2000;
        case 2: return 1000;
        case 3: return 500;
        case 4: return 250;
        case 5: return 125;
     }

     return 0;
 }

 int updateDurationWithDetail(byte detail, int base)
 {
     int result = base;

     switch (detail)
     {
        case 3:
        case 6:
        case 12:
            result = base*2.0/3.0;
         break;

        case 5:
        case 10:
            result = base*4.0/5.0;
         break;

        case 7:
            result = base*4.0/7.0;
         break;

        case 9:
            result = base*8.0/9.0;
         break;

        case 11:
            result = base*8.0/11.0;
         break;

        case 13:
            result = base*8.0/13.0;
         break;
     }

     return result;
 }

 //REPLACE with name CALCULATE COMPLETE status
 //and add just getCompleteStatus

 byte Bar::getCompleteStatus()
 {
     byte completeStatus = 255; //calculation failed

     byte thatNum = getSignNum();
     byte thatDen = getSignDenum();

     int barAbs = translateDenum(thatDen)*thatNum;

     int usedAbs = 0;
     int lastAbs = 0;

     completeIndex = 0;

     ul barSize = len();

     for (ul i = 0; i < barSize; ++i)
     {
        byte beatDur = getV(i)->getDuration();
        byte durDetail = getV(i)->getDurationDetail();
        byte isDotted = getV(i)->getDotted();

        int localAbs = translaeDuration(beatDur);

        if (durDetail)
            localAbs = updateDurationWithDetail(durDetail,localAbs);

        if (isDotted&1)
        {
            //first only one dot
            localAbs *= 3;
            localAbs /= 2;
        }

        usedAbs += localAbs;

        //logger << "On "<<i<<" used = "<<usedAbs;

        if ((i+1) != barSize) //without case of last
        if (usedAbs == barAbs)
        {
            completeStatus=2; //exceed
            completeIndex=i;
            completeAbs=localAbs;
            return completeStatus;
        }

        if (usedAbs > barAbs)
        {
            completeStatus=2; //exceed
            completeIndex=i;

            int exceedAbs = usedAbs - barAbs;

            if (tabLog)
            LOG( << "EXCEED abs "<<exceedAbs <<" localAbs "<<localAbs<<" Ind "<<completeIndex);

            short newNoteAbs = localAbs - exceedAbs;

            if (tabLog)
            LOG( << "New note "<<newNoteAbs);

            if (newNoteAbs > 10)
                lastAbs = newNoteAbs;
            else
            {
               --completeIndex; //to small for new note
                logger <<"SHIFTBACK";
            }

            completeAbs = lastAbs;

            break;
        }

        lastAbs = localAbs;

     }


    if (usedAbs < barAbs)
    {
        completeIndex=0; //just for logs
        completeStatus=1; //incomplete
        lastAbs=barAbs-usedAbs;

        completeAbs = lastAbs;

        if (lastAbs < 50)
            completeStatus = 0;
    }

    if (usedAbs == barAbs)
    {
        completeStatus = 0;
    }

    //logger << "Bar abs len "<< barAbs<<"; used "<<usedAbs;
    //logger <<"Complete status="<<completeStatus<<"; lastAbs="<<lastAbs<<"; cInd="<<completeIndex;

     return completeStatus;
 }

 double Bar::getCompleteAbs()
 {
     return completeAbs;
 }

 ul Bar::getCompleteIndex()
 {
    return completeIndex;
 }

 ///////////////////////////////////////////////////////////////////////



 //REFACT - cover under Track operations
 typedef std::map<byte,PolyBar> AltRay;
 typedef std::map<byte,std::vector<int> > AltRayInd;

 void createAltRay(AltRay &altRay, AltRayInd &altRayInd, Bar *a, Bar *b, ul indA, ul indB)
 {
     //possible we will need value for the default repeat
     byte currentAlt = 0;

     //int normalW = (int)b - (int)a;
     //int backW = (int)a - (int)b;
     //logger << "Creating alt ray NORM: "<<normalW<<"; BACK: "<<backW;
     //logger << "Alt a : "<<(int)a<< " alt b :"<<(int)b;

     ul localInd = 0;
     for (Bar *barI=a; barI!=b; barI=barI->getNext()
          ,++localInd)
     {
        if (tabLog)
         LOG( << "ALTRAY Bar ptr "<<(int)barI);
        if (barI->getAltRepeat() != 0)
            currentAlt = barI->getAltRepeat();

        for (byte i=0; i < 8; ++i)
        {
            byte altMaskI = currentAlt & (1<<i);

            if (altMaskI)
            {
                altRay[i].add(barI);
                //altRayInd[i].push_back()
                altRayInd[i].push_back(localInd+indA);
            }
        }
     }

     currentAlt=b->getAltRepeat();
     for (byte i=0; i < 8; ++i)
     {
         byte altMaskI = currentAlt & (1<<i);
         if (altMaskI)
         {     
                altRay[i].add(b);
                altRayInd[i].push_back(indB);
         }
     }

 }

 void Track::pushReprise(Bar *beginRepeat, Bar *endRepeat,
                  Bar *preTail, Bar *tailBegin, Bar *tailEnd, ul beginIndex, ul endIndex,
                         ul preTailIndex, ul tailBeginIndex, ul tailEndIndex)
 {
     //Poly<Bar*> timeLoop;

     //if (tabLog)
     //logger << "Push reprise called; "<<(int)beginRepeat<<"; "<<(int)endRepeat; //exend with values
     //logger << "Pre: "<<(int)preTail<<"; tailBegin: "<<(int)tailBegin<<"; tE: "<<(int)tailEnd;

     AltRay altRay;
     AltRayInd altRayInd;

     if (preTail == 0)
     { //no alt ending in begin-end
         if (tailBegin == 0)
         { //no alt at all
             for (byte i = 0; i < endRepeat->getRepeatTimes(); ++i)
             {
                 ul localIndex = 0;
                 for (Bar *barI=beginRepeat; barI != endRepeat; barI=barI->getNext())
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
                for (Bar *barI=beginRepeat; barI != endRepeat; barI=barI->getNext())
                {
                    timeLoop.add(barI);
                    timeLoopIndexStore.push_back(beginIndex+localIndex);
                    ++localIndex;
                }

                if (altRay.find(i) != altRay.end())
                { //add alt ray from a tail
                    PolyBar *thatEnd = &altRay[i];
                    std::vector<int> thatRayInd = altRayInd[i];

                    if ((thatEnd->getV(0)->getRepeat() & 1)==0)
                    { //don't repeat alts that begin

                        for (ul j = 0; j < thatEnd->len(); ++j)
                        {
                            timeLoop.add(thatEnd->getV(j));
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
                for (Bar *barI=beginRepeat; barI != preTail; barI=barI->getNext())
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
                        timeLoop.add(thatEnd->getV(j));
                        timeLoopIndexStore.push_back(thatRayInd[j]);
                    }
                        //timeLoop += thatEnd;
                }
                else //add default value
                {
                    timeLoop.add(endRepeat);
                    timeLoopIndexStore.push_back(endIndex);
                    //POSSIBLE THERE would be ISSUE - hard test it
                    if (tabLog)
                    LOG( << "Tail begin=0, pre tail=1?; attention");
                }
            }
     }

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
    curBar = getV(0);

    Bar *lastBeginRepeat = curBar;

    Bar *beginRepeat=0;
    Bar *endRepeat=0;

    ul beginIndex=0;
    ul endIndex=0;


    Bar *tailEnd=0;
    Bar *tailBegin=0;

    ul tailEndIndex=0;
    ul tailBeginIndex=0;

    if (tabLog)
        LOG( << "Start connecting time-loop ");

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
                    beginRepeat = beginRepeat->getNext();
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
                curBar = curBar->getNext();
                ++curIndex;
                continue;
            }
        }

        if (curBar->getRepeat() & 2)
        {
            endRepeat = curBar;
            endIndex = curIndex;

            //then search for alternative tail
            curBar = curBar->getNext();
            ++curIndex;

            if (curBar)
            {
                if (curBar->getAltRepeat() != 0)
                {
                        tailBegin = curBar;
                        //logger <<"Tail begin set to "<<(int)tailBegin;
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
                        curBar = curBar->getNext();
                        ++curIndex;

                        if (curIndex>=lastIndex)
                            break;

                       // logger <<"Tail END reset to "<<(int)tailEnd;
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

            //logger <<"RUN PRE TAIL SEARCH "<<(int)beginRepeat<<" to "<<(int)endRepeat;

            int indX=0;
            for (Bar *barI=endRepeat; barI != beginRepeat; barI=barI->getPrev()
                 ,++indX)//--barI
            {
                //logger <<"pre-tail search "<<(int)barI;
                if (barI->getAltRepeat() != 0)
                {
                    preTail = barI;
                    preTailIndex = endIndex-indX;
                    //logger <<" got alt ";
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

            curBar = curBar->getNext();
            ++curIndex;
        }
    }

    if (tabLog)
    LOG( << "TIME LOOP size is "<<(int)timeLoop.len());

    return timeLoop.len();
 }


 //---------------------------------Chains-----------------------------

 ul Track::connectBars()
 {
     if (tabLog)
         LOG( <<"ConnectingBars "<<(int)len()) ;

     if (len() == 0)
         return 0;

     byte currentNum = getV(0)->getSignNum();
     byte currentDen = getV(0)->getSignDenum();


     ul trackLen = len();

     for(ul barsI=1; barsI < trackLen; ++barsI)
     {
         getV(barsI)->setPrev(getV(barsI-1));
         getV(barsI-1)->setNext(getV(barsI));

         byte thatNum = getV(barsI)->getSignNum();
         byte thatDen = getV(barsI)->getSignDenum();

         if (thatNum==0)
             getV(barsI)->setSignNum(currentNum);
         else
             currentNum = thatNum;

         if (thatDen==0)
             getV(barsI)->setSignDenum(currentDen);
         else
             currentDen = thatDen;

     }

     return trackLen;
 }

 ul Track::connectBeats()
 {
    Bar* curBar;
    Beat *curBeat;

    if (len() == 0)
        return 0;

    curBar = getV(0);
    ul trackLen = len();
    ul fullCount=0;

    if (tabLog)
        LOG( <<"ConnectingBeats");


    for (ul barI = 0; barI < trackLen; ++barI)
    {
        curBar = getV(barI);

        if (curBar->len())
        for (ul beatI = 0; beatI < (curBar->len()-1); ++beatI)
        {
            curBeat = curBar->getV(beatI);
            Beat *nextBeat = curBar->getV(beatI+1);

            if ((nextBeat)&&(curBeat))
            {
                nextBeat->setPrev(curBeat);
                curBeat->setNext(nextBeat);
                ++fullCount;
            }
            else
                LOG( <<"Issue connecting beats");
        }

        if (barI+1 != trackLen)
        { //not the lastBar
            Bar *nextBar = getV(barI+1);

            if (nextBar->len()==0)
                continue;

            /*

            LOG( << "Next len "<<(int)nextBar->len()<<
                      "Cur len "<<(int)curBar->len()<<" i "<<barI)

            logger << "-";
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
                        logger<<"Exited extrimly! ";
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
                curBeat = curBar->getV(curBar->len()-1);
            }

            Beat *nextBeat = nextBar->getV(0);

            if ((nextBeat)&&(curBeat))
            {
                nextBeat->setPrev(curBeat);
                curBeat->setNext(nextBeat);
                ++fullCount;
            }
            else
                logger <<"Issue connecting beatz";
        }
    }

    if (tabLog)
    logger << "Full count of the beats is "<<fullCount;
    beatsAmount = fullCount;

    return fullCount;
 }

 ul Track::connectNotes() //for let ring
 {
    Bar* curBar;
    Beat *curBeat;
    ul index=0;

    if (len() == 0)
        return 0;

    curBar = getV(0);

    if (curBar->len())
    curBeat = curBar->getV(0);
    else
    {
        while (curBar && curBar->len()==0)
             curBar = curBar->getNext();

        if (curBar==0) return;

        curBeat = curBar->getV(0);

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
            Note *curNote = curBeat->getV(noteI);

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
                    logger <<"Prev Note "<<(int)prevNote<<" sN= "<<stringN;
                        logger <<"Is Pause "<<(int)curBeat->getPause();
                        logger <<"Count="<<count<<"; notesCount="<<notesCount[stringN];
                }

                logger <<"Fret was 63 now "<<prevFret;
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
                    if (tabLog)
                    LOG( << "Prev found "<<prevNote->getStringNumber()<<
                           " "<<prevFret);

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


            if (tabLog)
                LOG( <<stringN<<"ZFret "<<nowFret<<"; newState= "<<newNoteState<<"; oldS= "<<noteState);


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

    if (tabLog)
    LOG( << "Connect notes done "<<count<<"; S1="<<notesCount[1]<<
           "; S2="<<notesCount[2]<<"; S3="<<notesCount[3]<<"; S4="<<notesCount[4]<<
           "; S5="<<notesCount[5]<<"; S6="<<notesCount[6]);

   return count;
 }

 void Track::connectAll()
 {
     if (tabLog)
     LOG( <<"Connection initiated for track "<<name.c_str());

     connectBars();
     connectBeats(); //uncomment for
     connectNotes(); //uncomment for effects refactoring
     connectTimeLoop();

     if (tabLog)
     LOG( <<"Connection finished for track "<<name.c_str());

     return;
 }


 struct BpmChangeKnot
 {
     int bpm;
     int time;

     BpmChangeKnot(int newBpm, int newTime):bpm(newBpm),time(newTime){}

     bool operator<(const BpmChangeKnot& another){  return time < another.time; }
 };



 byte Tab::getBPMStatusOnBar(ul barN)
 {
        for (int i = 0 ; i < len(); ++i)
        {
            Bar *bar = getV(i)->getV(barN);

            if (bar->len())
            {
                if (bar->getV(0)->changes.len())
                {
                    //bar->getV(0)->changes.getV(0)->changeType;
                }
            }
            bar->len();
        }
 }

 int Tab::getBpmOnBar(ul barN)
 {
    int bpmTrace = bpmTemp;
    for (int i = 0; i < timeLine.size(); ++i)
    {

    }
 }

 void Tab::createTimeLine(ul shiftTheCursor)
 {
     timeLine.clear();

     //INITIAL value
     TimeLineKnot initKnot(1,getBPM());

     timeLine.push_back(initKnot);//fuck loo

     int lastNumDen=0;

     ul barsAmount = getV(0)->timeLoop.len(); //should search longest
     for (ul barsI = shiftTheCursor; barsI < barsAmount; ++barsI)
     {
         std::vector<BpmChangeKnot> timeChanges;

         for (ul tracksI = 0; tracksI < len(); ++tracksI)
         {
            short int localAccumulate = 0;///tracksI

            if (getV(tracksI)->timeLoop.len() <= barsI)
                continue;

            Bar *currentBar = getV(tracksI)->timeLoop[barsI]; //attention refact fix

            for (ul beatI = 0; beatI < currentBar->len(); ++beatI)
            {
                if (currentBar->getV(beatI)->effPack == 28) //changes
                {
                    //search for bpm changes
                    Package *changePack = currentBar->getV(beatI)->effPack.getPack(28);
                    Beat::ChangesList *changes = (Beat::ChangesList*)changePack->getPointer();

                    for (ul indexChange = 0; indexChange != changes->len(); ++indexChange)
                      if (changes->getV(indexChange).changeType==8)
                      {
                          ul newBPM = changes->getV(indexChange).changeValue;

                          BpmChangeKnot newChangeBpm(newBPM,localAccumulate);

                          timeChanges.push_back(newChangeBpm);
                          break;
                      }

                }

                byte beatDur = currentBar->getV(beatI)->getDuration();
                byte durDetail = currentBar->getV(beatI)->getDurationDetail();
                byte isDotted = currentBar->getV(beatI)->getDotted();

                int localAbs = translaeDuration(beatDur);

                if (durDetail)
                    localAbs = updateDurationWithDetail(durDetail,localAbs);

                if (isDotted&1)
                {
                    //first only one dot
                    localAbs *= 3;
                    localAbs /= 2;
                }

                localAccumulate += localAbs;

            }

         }


         //NOT POLY YET
         byte thatNum = getV(0)->timeLoop.getV(barsI)->getSignNum();
         byte thatDen = getV(0)->timeLoop.getV(barsI)->getSignDenum();


         int packedMeter=0;
         packedMeter=(thatNum<<8) + thatDen;

         if (packedMeter != lastNumDen)
         {
            TimeLineKnot changeNumDen(2,packedMeter);
            timeLine.push_back(changeNumDen);
            lastNumDen = packedMeter;
         }

         int barAbs = translateDenum(thatDen)*thatNum;

         if (timeChanges.empty())
         {
            TimeLineKnot noChangeBar(0,barAbs);

            timeLine.push_back(noChangeBar);
         }
         else
         {
             std::sort(timeChanges.begin(),timeChanges.end());

             short int lastChange = -1;

             for (int i = 0; i < timeChanges.size(); ++i)
             {
                if (timeChanges[i].time != lastChange)
                {

                    int currentChange = timeChanges[i].time-lastChange;
                    lastChange = timeChanges[i].time;

                    TimeLineKnot timeWait(0,currentChange);
                    TimeLineKnot bpmChange(1,timeChanges[i].bpm);

                    timeLine.push_back(timeWait);
                    timeLine.push_back(bpmChange);
                }
             }

             if (lastChange < barAbs)
             {
                 TimeLineKnot timeWait(0,barAbs-lastChange);
                 timeLine.push_back(timeWait);
             }
         }

         timeChanges.clear();
     }
 }


 void Bar::clone(Bar *from)
 {
    flush();

    signatureDenum = from->signatureDenum;
    signatureNum = from->signatureNum;

    for (ul i = 0; i < from->len(); ++i)
    {
        Beat *beat = from->getV(i);
        Beat *newBeat=new Beat();

        newBeat->clone(beat);
        add(newBeat);
    }
 }

 void Beat::clone(Beat *from)
 {
    effPack.mergeWith(from->effPack);
    duration = from->duration;
    durationDetail = from->durationDetail;
    dotted = from->dotted;
    isPaused = from->isPaused;
    noticeText = from->noticeText;

    bookmarkName = from->bookmarkName;

    for (ul i = 0; i < from->len(); ++i)
    {
        Note *note = from->getV(i);
        Note *newNote=new Note();
        newNote->clone(note);
        add(newNote);
    }
 }

 void Note::clone(Note *from)
 {
     fret = from->fret;
     volume = from->volume;
     noteState = from->noteState;
     stringNumber = from->stringNumber;
     effPack.mergeWith(from->effPack);
 }
