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






 //move code to caclCompleteStatus




 ///////////////////////////////////////////////////////////////////////




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


