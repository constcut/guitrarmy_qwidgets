#include "tab.h"
#include "tabloader.h"

bool tabLog = false;

#include <QDebug>
#include <algorithm>
#include <map>


 void Tab::printToStream(std::ostream &stream)
 {
    stream << "Outputing #"<<size()<<" Tracks."<<std::endl;
    for (size_t ind = 0; ind < size(); ++ind)
            at(ind)->printToStream(stream);

 }

 struct BpmChangeKnot
 {
     int bpm;
     int time;

     BpmChangeKnot(int newBpm, int newTime):bpm(newBpm),time(newTime){}

     bool operator<(const BpmChangeKnot& another){  return time < another.time; }
 };



 std::uint8_t Tab::getBPMStatusOnBar(size_t barN)
 {
        for (size_t i = 0 ; i < size(); ++i)
        {
            auto& bar = at(i)->at(barN);

            if (bar->size())
            {
                if (bar->at(0)->changes.size())
                {
                    //bar->getV(0)->changes.getV(0)->changeType;
                }
            }
            bar->size();
        }
 }

 int Tab::getBpmOnBar(size_t barN)
 {
    int bpmTrace = bpmTemp;
    for (int i = 0; i < timeLine.size(); ++i){
        //TODO
    }
 }

 void Tab::createTimeLine(size_t shiftTheCursor)
 {
     timeLine.clear();

     //INITIAL value
     TimeLineKnot initKnot(1,getBPM());

     timeLine.push_back(initKnot);//fuck loo

     int lastNumDen=0;

     size_t barsAmount = at(0)->timeLoop.size(); //should search longest
     for (size_t barsI = shiftTheCursor; barsI < barsAmount; ++barsI)
     {
         std::vector<BpmChangeKnot> timeChanges;

         for (size_t tracksI = 0; tracksI < size(); ++tracksI)
         {
            short int localAccumulate = 0;///tracksI

            if (at(tracksI)->timeLoop.size() <= barsI)
                continue;

            Bar *currentBar = at(tracksI)->timeLoop[barsI]; //attention refact fix

            for (size_t beatI = 0; beatI < currentBar->size(); ++beatI)
            {
                if (currentBar->at(beatI)->effPack == 28) //changes
                {
                    //search for bpm changes
                    Package *changePack = currentBar->at(beatI)->effPack.getPack(28);
                    ChangesList *changes = (ChangesList*)changePack->getPointer();

                    for (size_t indexChange = 0; indexChange != changes->size(); ++indexChange)
                      if (changes->at(indexChange).changeType==8)
                      {
                          size_t newBPM = changes->at(indexChange).changeValue;

                          BpmChangeKnot newChangeBpm(newBPM,localAccumulate);

                          timeChanges.push_back(newChangeBpm);
                          break;
                      }

                }

                std::uint8_t beatDur = currentBar->at(beatI)->getDuration();
                std::uint8_t durDetail = currentBar->at(beatI)->getDurationDetail();
                std::uint8_t isDotted = currentBar->at(beatI)->getDotted();

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
         std::uint8_t thatNum = at(0)->timeLoop.at(barsI)->getSignNum();
         std::uint8_t thatDen = at(0)->timeLoop.at(barsI)->getSignDenum();


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






