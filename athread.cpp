#include "athread.h"

#include "g0/tab.h"

#include "g0/astreaming.h"

static AStreaming logger("thread");

AThread::AThread()
{

}

void WaveMoveThr::threadRun()
{
   /*
   int quarter = 60000/bpm;
   int firstTakt = (quarter/2)*21;
   int fullNote = 240000/bpm;
   int waitValue = (fullNote/denum)*num;
   */
   int indexWait = 0;

   status = 0;
   pleaseStop =false;

   while (pleaseStop == false)
   {
        if (incrementA)
        {
            static int counter=0;
            ++counter;

            int toAdd = 110;
            if(counter%40==0)
                ++toAdd; //*10

            *incrementA=*incrementA+toAdd;
            if (*incrementA >= limit)
                break;

            callUpdate();
            sleepThread(10);
        }
   }



   status = 1;
}

void PlayAnimationThr::threadRun()
{
   /*
   int quarter = 60000/bpm;
   int firstTakt = (quarter/2)*21;
   int fullNote = 240000/bpm;
   int waitValue = (fullNote/denum)*num;
   */
   int indexWait = 0;

   status = 0;

   //while ((*increment +1) < limit)
   for (ul i = 0 ; i < waitTimes.size(); ++i)
   {
         (*incrementA) = waitIndexes[indexWait];
         int nowWait = waitTimes[indexWait];

         if (incrementB)
            (*incrementB) = 0;

         if (pleaseStop)
         {
             status = 1;
             break;
         }

         //sleepThread(nowWait); //msleep(nowWait);
         callUpdate();


         if (beatTimes.size() > i)
         for(ul j = 0; j < beatTimes[i].size(); ++j)
         {
             int beatWait = beatTimes[i][j];

             nowWait-=beatWait;
             if (nowWait < 0)
                 beatWait += nowWait;

             if (pleaseStop)
             {
                 status = 1;
                 break;
             }

             if (beatWait > 0)
                sleepThread(beatWait);

             (*incrementB) = (*incrementB) +1;
             callUpdate();
         }
         //check for beats times - for cycle
         //a) sleep each beat
         //b) move beat cursor
         //c) update screen
         //d) decreace nowWait
         //e) escape nowWaite

         if (nowWait>0)
         {
             if (incrementB != 0)
             {
             (*incrementB) = (*incrementB) - 1;

             callUpdate();
             }
             sleepThread(nowWait);
         }


         //(*increment) + 1; // ++
         //callUpdate(); //emit updateUI();
         ++indexWait;
   }

   /* //issue with stepback attention debugging
   if (incrementB != 0)
    (*incrementB) = (*incrementB) - 1; //step back
    */

   status = 1;
   if (incrementB == 0)
       noticeFinished();
}


struct BpmWaitNode
{
    int waitTime;
    int newBpm;
};


void PlayAnimationThr::setupValues(void *tab_ptr, void *track_ptr, ul shiftTheCursor)
{
    Tab *tab=(Tab*)tab_ptr;

    int startBPM = tab->getBPM();
    //beter from timeline


    Track *track = (Track*)track_ptr;
    ul timeLoopLen = track->timeLoop.len();

    //INCREMENTS set OUTSIDE
    int localWait = 0;

    std::vector<BpmWaitNode> bpmChangeList;

    for (int ind = 0 ; ind < tab->timeLine.size(); ++ind)
    {
       if (tab->timeLine[ind].type == 0)
       {
            //wait
           localWait += tab->timeLine[ind].value;
       }

       if (tab->timeLine[ind].type == 1)
       {
            //change
           BpmWaitNode newNode;
           newNode.newBpm = tab->timeLine[ind].value;
           newNode.waitTime = localWait;
           localWait = 0;
           bpmChangeList.push_back(newNode);
       }
    }

    int changeIndex = 0;

    setBPM(bpmChangeList[changeIndex].newBpm);
    beatTimes.clear();
    waitTimes.clear();
    waitIndexes.clear();

    ++changeIndex;

    int toTheNextWait = bpmChangeList[changeIndex].waitTime;
    //logger << "Waiting for next: "<<toTheNextWait;

    std::vector<int> barMoments;

    //MAIN CYCLE
    for (ul barI = shiftTheCursor; barI < timeLoopLen; ++barI)
    {
        Bar *bar = track->timeLoop.getV(barI);
        barMoments.clear();

        //addBeatTimes(bar);

        for (ul beatI = 0; beatI < bar->len(); ++beatI)
        {
           Beat *beat = bar->getV(beatI);

           byte dur = beat->getDuration();
           byte dot = beat->getDotted();
           byte det = beat->getDurationDetail();


           int beatAbs = translaeDuration(dur);

            if (dot == 1)
            {
                beatAbs *= 3;
                beatAbs /= 2;
            }

            if (det)
                beatAbs = updateDurationWithDetail(det,beatAbs);

            toTheNextWait -= beatAbs;

            if (toTheNextWait <= 1) //maybe more check
            {
                if (changeIndex < bpmChangeList.size())
                {
                    //logger << "Next wait arhived "<<toTheNextWait<<"; switch to "<<bpmChangeList[changeIndex].newBpm;

                    //all bpm switch position
                    bpm =  bpmChangeList[changeIndex].newBpm;
                    //logger << "Changed bpm to "<<bpm;

                    {
                        ++changeIndex;
                        toTheNextWait = bpmChangeList[changeIndex].waitTime;

                        //logger << "Waiting for next: "<<toTheNextWait;
                    }
                }
            }

               int noteTime = 2400000/bpm; //full note

               if (dot == 1)
               {
                   noteTime *= 3;
                   noteTime /= 2;
               }

               switch (dur)
               {
                   case 1: noteTime /= 2; break;
                   case 2: noteTime /= 4; break;
                   case 3: noteTime /= 8; break;
                   case 4: noteTime /= 16; break;
                   case 5: noteTime /= 32; break;
                   case 6: noteTime /= 64; break;
               }

               if (det)
                noteTime = updateDurationWithDetail(det,noteTime);

               noteTime /= 10;

               barMoments.push_back(noteTime);

        }
        beatTimes.push_back(barMoments);

        addNumDenum(bar->getSignNum(), bar->getSignDenum(), track->timeLoopIndexStore[barI]);
    }

    setLimit(track->timeLoop.len());

    //logger << "prepare thread done";
}

void PlayAnimationThr::addNumDenum(byte nu, byte de, ul nextIndex)
{
    int fullNote = 240000/bpm;
    int waitValue = (fullNote/de)*nu;
    waitTimes.push_back(waitValue);
    waitIndexes.push_back(nextIndex);

    //log << "Adding n="<<nu<<"; d="<<de<<"; ind="<<nextIndex;
}

int PlayAnimationThr::calculateSeconds()
{
    ul totalSumm = 0;

    for (int i = 0; i < waitTimes.size(); ++i)
    {
        totalSumm += waitTimes[i];
    }

    int seconds = totalSumm/1000;
    return seconds;
}


void PlayAnimationThr::addBeatTimes(void *bar)
{
    Bar *curBar = (Bar*)bar;

    std::vector<int> barMoments;

    //int toNextChange =

    for (ul i = 0; i < curBar->len(); ++i)
    {
        Beat *beat = curBar->getV(i);


        //CHANGE BPM
        if (beat->effPack.get(28))
        {
            Package *changePack =beat->effPack.getPack(28);

            if (changePack)
            {
                Beat::ChangesList *changes = (Beat::ChangesList*)changePack->getPointer();

                for (ul indexChange=0; indexChange <changes->len(); ++indexChange)
                if (changes->getV(indexChange).changeType==8)
                {
                    ul newBPM = changes->getV(indexChange).changeValue;

                    //CHANGING BPM from mix table!

                    bpm = newBPM;
                    break;
                }
            }
        }
        //animation

        int noteTime = 2400000/bpm; //full note

        byte dur = beat->getDuration();
        byte dot = beat->getDotted();
        byte det = beat->getDurationDetail();

        if (dot == 1)
        {
            noteTime *= 3;
            noteTime /= 2;
        }

        switch (dur)
        {
            case 1: noteTime /= 2; break;
            case 2: noteTime /= 4; break;
            case 3: noteTime /= 8; break;
            case 4: noteTime /= 16; break;
            case 5: noteTime /= 32; break;
            case 6: noteTime /= 64; break;
        }

        noteTime = updateDurationWithDetail(det,noteTime);

        noteTime /= 10;

        barMoments.push_back(noteTime);
    }

    beatTimes.push_back(barMoments);
}
