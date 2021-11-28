#include "tab.h"

#include "aexpimp.h"



bool tabLog = false;

#include <QDebug>

//TODO

#include <algorithm>

#include <map>

//BEND points fun

void BendPoints::insertNewPoint(BendPoint bendPoint)
{
    for (ul i = 0; i < len()-1; ++i)
    {
        if (at(i).horizontal < bendPoint.horizontal)
            if (at(i+1).horizontal > bendPoint.horizontal)
            {
                this->insertBefore(bendPoint,i+1);
                return;
            }
        if (at(i).horizontal == bendPoint.horizontal)
        {
            at(i).vertical = bendPoint.vertical;
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
            at(ind)->printToStream(stream);

    //as alrternative there
    //SHOULD APPEAR CLASS
    //TextTabList
    //that creates tab lists with numbers
 }



 void Bar::printToStream(std::ostream &stream)
 {
     stream << "Outputing #"<<len()<<" Beats."<<std::endl;
     for (ul ind = 0; ind < len(); ++ind)
             at(ind)->printToStream(stream);
 }

 void Beat::printToStream(std::ostream &stream)
 {
     stream << "Outputing #"<<len()<<" Notes."<<std::endl;
     for (ul ind = 0; ind < len(); ++ind)
             at(ind)->printToStream(stream);
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
         ul duration = at(beatInd)->getDuration();
         ul detail = at(beatInd)->getDurationDetail();

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
             if (tabLog)  qDebug()<<"Shit in duration "<<duration;
         }

         addition *=2; //dots on 32
         addition *= 3; //triplets

         if (detail==3)
         {
             addition *= 2;
             addition /= 3;
         }

         bool byteDote = at(beatInd)->getDotted();
         if (byteDote==1)
         {
             addition += addition/2;
         }

         //recalculations for addition
          num+=addition;
         if (tabLog)  qDebug()<<"Addition is "<<addition<<"; det= "<<detail
              <<" dur= "<<duration<<" full "<<num<<"; dot "<<byteDote;
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
        byte beatDur = at(i)->getDuration();
        byte durDetail = at(i)->getDurationDetail();
        byte isDotted = at(i)->getDotted();

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

        //qDebug() << "On "<<i<<" used = "<<usedAbs;

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
            qDebug() << "EXCEED abs "<<exceedAbs <<" localAbs "<<localAbs<<" Ind "<<completeIndex;

            short newNoteAbs = localAbs - exceedAbs;

            if (tabLog)
            qDebug() << "New note "<<newNoteAbs;

            if (newNoteAbs > 10)
                lastAbs = newNoteAbs;
            else
            {
               --completeIndex; //to small for new note
                qDebug() <<"SHIFTBACK";
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

    //qDebug() << "Bar abs len "<< barAbs<<"; used "<<usedAbs;
    //qDebug() <<"Complete status="<<completeStatus<<"; lastAbs="<<lastAbs<<"; cInd="<<completeIndex;

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
     //qDebug() << "Creating alt ray NORM: "<<normalW<<"; BACK: "<<backW;
     //qDebug() << "Alt a : "<<(int)a<< " alt b :"<<(int)b;

     ul localInd = 0;
     for (Bar *barI=a; barI!=b; barI=(Bar*)barI->getNext()
          ,++localInd)
     {
        if (tabLog)
         qDebug() << "ALTRAY Bar ptr "<<barI;
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


 //---------------------------------Chains-----------------------------




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
            Bar *bar = at(i)->at(barN);

            if (bar->len())
            {
                if (bar->at(0)->changes.len())
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
    for (int i = 0; i < timeLine.size(); ++i){
        //TODO
    }
 }

 void Tab::createTimeLine(ul shiftTheCursor)
 {
     timeLine.clear();

     //INITIAL value
     TimeLineKnot initKnot(1,getBPM());

     timeLine.push_back(initKnot);//fuck loo

     int lastNumDen=0;

     ul barsAmount = at(0)->timeLoop.len(); //should search longest
     for (ul barsI = shiftTheCursor; barsI < barsAmount; ++barsI)
     {
         std::vector<BpmChangeKnot> timeChanges;

         for (ul tracksI = 0; tracksI < len(); ++tracksI)
         {
            short int localAccumulate = 0;///tracksI

            if (at(tracksI)->timeLoop.len() <= barsI)
                continue;

            Bar *currentBar = at(tracksI)->timeLoop[barsI]; //attention refact fix

            for (ul beatI = 0; beatI < currentBar->len(); ++beatI)
            {
                if (currentBar->at(beatI)->effPack == 28) //changes
                {
                    //search for bpm changes
                    Package *changePack = currentBar->at(beatI)->effPack.getPack(28);
                    Beat::ChangesList *changes = (Beat::ChangesList*)changePack->getPointer();

                    for (ul indexChange = 0; indexChange != changes->len(); ++indexChange)
                      if (changes->at(indexChange).changeType==8)
                      {
                          ul newBPM = changes->at(indexChange).changeValue;

                          BpmChangeKnot newChangeBpm(newBPM,localAccumulate);

                          timeChanges.push_back(newChangeBpm);
                          break;
                      }

                }

                byte beatDur = currentBar->at(beatI)->getDuration();
                byte durDetail = currentBar->at(beatI)->getDurationDetail();
                byte isDotted = currentBar->at(beatI)->getDotted();

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
         byte thatNum = at(0)->timeLoop.at(barsI)->getSignNum();
         byte thatDen = at(0)->timeLoop.at(barsI)->getSignDenum();


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
        Beat *beat = from->at(i);
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
        Note *note = from->at(i);
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


