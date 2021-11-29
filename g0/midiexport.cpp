#include "midifile.h"
#include "tab.h"
#include "aconfig.h"

bool midiExportLog = false;

#include <QDebug>

bool MidiTrack::fromTrack(Track *track, std::uint8_t channel, size_t shiftCursorBar)
{

 clock_t afterT = getTime();
 size_t instrument = track->getInstrument();
 /*
 bool isDrums = track->isDrums();
 if (isDrums)
 {  //or any else
     instrument = 25;
 }
 */

 std::uint8_t midiPan = calcMidiPanoramGP(track->getPan());
 std::uint8_t midiVol = calcMidiVolumeGP(track->getVolume());

 //byte channel = 0;
 pushChangeInstrument(instrument,channel);
 pushChangePanoram(midiPan,channel);
 pushChangeVolume(midiVol,channel);

 std::uint8_t theTunes[11];
 for (int i = 0; i < 10; ++i)
     if (track->isDrums())
         theTunes[i] = 0;
     else
         theTunes[i] = track->tuning.getTune(i);

 setTunes(theTunes);


 size_t trackLen = track->timeLoop.size();

 for (size_t i = shiftCursorBar ; i < trackLen; ++i)
 {
     Bar *bar = track->timeLoop.at(i);

     //BAR STATUS
     std::uint8_t completeStatus = bar->getCompleteStatus();

     size_t barLen = bar->size();
     short specialLast = 0;
     size_t completeIndex = 0;

     if (completeStatus == 2)
     {
        //barLen = bar->getCompleteIndex();
         completeIndex = bar->getCompleteIndex();
        specialLast = bar->getCompleteAbs();
     }

     //Signature

     for (size_t j = 0; j < barLen; ++j)
     {
         //if (specialLast)
        Beat *beat = bar->at(j).get();

         if ((completeStatus == 2) && (j == completeIndex))
         {
                addSignalsFromBeat(beat,channel,specialLast);
                break;
         }
            else
                addSignalsFromBeat(beat,channel);
     }

    closeLetRings(channel);

    if (completeStatus == 1)
        finishIncomplete(bar->getCompleteAbs()); //short

 }

 pushEvent47();

 clock_t after2T = getTime();
 int diffT = after2T - afterT;

 //qDebug() <<"Generate track "<<diffT;

  return 0; //TODO?
}


bool MidiFile::fromTab(Tab *tab, size_t shiftTheCursor)
{
    clock_t afterT = getTime();
    //time line track

    {
        auto timeLineTrack = std::make_unique<MidiTrack>();
        tab->createTimeLine(shiftTheCursor);

        std::cout << tab->timeLine.size() << " is size of timeLine" <<std::endl;

        size_t tlAccum = 0;

        for (size_t i = 0; i < tab->timeLine.size(); ++i)
        {

           // std::cout <<"Time line out "<<(int)tab->timeLine[i].type
             //        <<"; "<<tab->timeLine[i].value<<std::endl;


            if (tab->timeLine[i].type==1)
            {
                //changing bpm
                std::cout << "Push bpm to "<<tab->timeLine[i].value<<std::endl;

                //refact to cover under another layer
                short int rhyBase = 120;

                short int power2 = 2<<(3);
                int preRValue = rhyBase*power2/4;

                preRValue *= tlAccum;
                preRValue /= 1000;


                timeLineTrack->pushChangeBPM(tab->timeLine[i].value, preRValue);
                tlAccum = 0;
            }

            if (tab->timeLine[i].type==0)
            {
                tlAccum += tab->timeLine[i].value;



            }

            if (tab->timeLine[i].type==2)
            {
                int packedMeter = tab->timeLine[i].value;
                int newDen = packedMeter & 0xff;


                packedMeter -= newDen;
                int newNum = (packedMeter>>8);


                short int rhyBase = 120;

                short int power2 = 2<<(3);
                int preRValue = rhyBase*power2/4;

                preRValue *= tlAccum;
                preRValue /= 1000;


                timeLineTrack->pushMetrSignature(newNum,newDen,preRValue);
                tlAccum=0;
            }
        }

        timeLineTrack->pushEvent47();
        push_back(std::move(timeLineTrack));
    }

    if (CONF_PARAM("metronome")=="1")
    {
     //metronome track
        int metronomeClickSize = 0;
        bool metronomeTurnedOn = true;

        if (metronomeTurnedOn)
        {
            auto metronomeClickTrack = std::make_unique<MidiTrack>();
            bool firstRun = true;

            for (int barI=shiftTheCursor; barI< tab->at(0)->timeLoop.size(); ++barI)
            {
                    //for bars
                    Bar *bar = tab->at(0)->timeLoop.at(barI);

                    int newDen = bar->getSignDenum(); //nextbar;

                    if (newDen==32)
                             metronomeClickSize=60;
                    if (newDen==16)
                             metronomeClickSize=120;
                    if (newDen == 8)
                        metronomeClickSize=240;
                    else if (newDen==4)
                             metronomeClickSize=480;
                    else if (newDen==2)
                             metronomeClickSize=960;
                    else if (newDen==1)
                             metronomeClickSize=1920;
                    else if (newDen==0)
                             metronomeClickSize=3840;


                    int num = newDen = bar->getSignNum();;

                    for (int i = 0; i < num; ++i)
                    {
                        if (firstRun)
                        {
                            auto noteOn = std::make_unique<MidiSignal>(0x90 | 9, 33, 127,0);
                            metronomeClickTrack->push_back(std::move(noteOn));
                            firstRun = false;
                        }
                        else
                        {
                            auto noteOn = std::make_unique<MidiSignal>(0x90 | 9, 33, 127,metronomeClickSize);
                            metronomeClickTrack->push_back(std::move(noteOn));
                        }
                    }

                    }
                    //tlAccum=0;

                metronomeClickTrack->pushEvent47();
                push_back(std::move(metronomeClickTrack));
            }

    }

    size_t tabLen = tab->size();

    int drumsTrack=0;

    size_t startCursorBar = shiftTheCursor;

    //Get aware of solo tracks
    std::vector<size_t> indecesToSkip;
    bool soloTurnedOn = false;

    for (size_t trackIndex = 0; trackIndex < tabLen; ++trackIndex)
    {
        auto& currentTrack = tab->at(trackIndex);
        std::uint8_t trackStatus = currentTrack->getStatus();

        if (trackStatus != 2)
            indecesToSkip.push_back(trackIndex);
        else
            soloTurnedOn=true;
    }

    if (soloTurnedOn==false)
        indecesToSkip.clear();

    clock_t after2T = getTime();
    int diffT = after2T - afterT;
    qDebug() <<"File pre-generation "<<diffT;

    //Main generation

    for (size_t i=0; i < tabLen; ++i)
    {
        //qDebug() << "0 Tab is "<<(int)tab;
        //qDebug() <<"pushed "<<tabLen;

        auto& track = tab->at(i);

        std::uint8_t trackStatus = track->getStatus();

        if (trackStatus==1) //mute
            continue;

        if (soloTurnedOn)
        {
            bool skipThatTrack = false;
            for (int i2 = 0; i2 < indecesToSkip.size(); ++i2)
                if (indecesToSkip[i2]==i)
                {
                    skipThatTrack=true;
                    break;
                }
            if (skipThatTrack)
                continue;
        }


        auto mTrack = std::make_unique<MidiTrack>();
        mTrack->takeAccum();
        //if (i==0)
        //mTrack->pushChangeBPM(tab->getBPM());


        if (track->isDrums())
        {
            ++drumsTrack;
            size_t realInd = 9; //8 + drumsTrack; //9 and 10
            //if (realInd > 10)
                //realInd = 10;

            mTrack->fromTrack(track.get(),realInd,startCursorBar);
        }
        else
            mTrack->fromTrack(track.get(),i,startCursorBar); //1 chan per track


        //clock_t afterT3 = getTime();
        push_back(std::move(mTrack));
        //clock_t afterT4 = getTime();
        //int addDiff = afterT4-afterT3;
    }
}


bool MidiTrack::addSignalsFromBeat(Beat *beat, std::uint8_t channel, short specialR)
{
    std::uint8_t dur,det,dot; //rhythm value

    dur = beat->getDuration();
    det = beat->getDurationDetail();
    dot = beat->getDotted();

    const int baseAmount = 120;

    int rOffset = 0;
    if (specialR == 0)
    {
      std::uint8_t powOfTwo = 6 - dur;
       short int power2 = 2<<(powOfTwo-1);//-1 because 2 is 1 pow itself
        rOffset = baseAmount*power2/4;

     if (dot&1) //dotted
     {
          rOffset *= 3;
           rOffset /= 2;
        }

       if (det)
            rOffset = calcRhythmDetail(det,rOffset); //FEW MISSING
    }
    else
    {
        //constant refact
         short int rhyBase = 120;

        short int power2 = 2<<(3);
        int preRValue = rhyBase*power2/4;

        preRValue *= specialR;
        preRValue /= 1000;

        rOffset=preRValue;
    }

    if (beat->getPause())
    {
        closeLetRings(channel);
        accumulate(rOffset);
        return true;
    }

    //BEAT EFFECTS:
    if (beat->effPack.get(28))
    {
        //changes
        Package *changePack = beat->effPack.getPack(28);

        if (changePack)
        {
            Beat::ChangesList *changes = (Beat::ChangesList*)changePack->getPointer();

            for (size_t indexChange = 0; indexChange != changes->size(); ++indexChange)
            {
              if (changes->at(indexChange).changeType==8)
              {
                  size_t newBPM = changes->at(indexChange).changeValue;
                  //pushChangeBPM(newBPM,accum);
                  //takeAccum();

                  //skipped according to time line
              }

              if (changes->at(indexChange).changeType==1)
              {
                 size_t newInstr = changes->at(indexChange).changeValue;
                 pushChangeInstrument(newInstr,channel,accum);
                 takeAccum();
              }

              if (changes->at(indexChange).changeType==2)
              {
                  std::uint8_t newVol = changes->at(indexChange).changeValue;

                  std::uint8_t midiNewVolume = newVol*8;
                  if (midiNewVolume > 127)
                      midiNewVolume = 127;

                  pushChangeVolume(midiNewVolume,channel); //must take accum
              }

              if (changes->at(indexChange).changeType==3)
              {
                    std::uint8_t newPan = changes->at(indexChange).changeValue;

                    std::uint8_t midiNewPanoram = newPan*8;
                    if (midiNewPanoram > 127)
                        midiNewPanoram = 127;

                    pushChangePanoram(midiNewPanoram,channel); //must take accum
              }
            }
        }
    }


    short int strokeStep  = rOffset/12;

    size_t beatLen = beat->size();
    for (size_t i =0; i < beatLen; ++i)
    {
        //reverse indexation
        size_t trueIndex = i;

        if (beat->effPack.get(26)) //down
            trueIndex = (beatLen - i - 1);

        if (beat->effPack.inRange(25,26)) //up down strokes
        {
            accumulate(strokeStep);
            rOffset -= strokeStep;
        }

        Note *note = beat->at(trueIndex).get();
        addSignalsFromNoteOn(note,channel);
    }

    //ACCUMULATE BEFORE EFFECTS AND OFFS
    accumulate(rOffset);

    if (beat->effPack.get(20)) //fade in
    {
        pushFadeIn(accum, channel);
        takeAccum();
    }

    if (beat->effPack.get(19)) //tremolo
    {
        pushTremolo(accum);
        takeAccum();
    }



    addPostEffects(beat,channel);

    for (size_t i =0; i < beat->size(); ++i)
    {
        Note *note = beat->at(i).get();
        addSignalsFromNoteOff(note,channel);
    }
}


bool MidiTrack::addSignalsFromNoteOn(Note *note, std::uint8_t channel)
{
    std::uint8_t noteState = note->getState();

    if ((noteState==Note::leegNote) ||
            (noteState==Note::leegedLeeg))
    {
            //this are leegs
        return false;
    }


    std::uint8_t fret = note->getFret();
    std::uint8_t stringN = note->getStringNumber();
    std::uint8_t midiNote = fret + tunes[stringN-1];

    std::uint8_t volume = note->getVolume();
    std::uint8_t midiVelocy = volume*15; //calcMidiVolumeGP(volume);

    std::uint8_t lastVelocy = 95; //not real last one yet

    if (midiVelocy > 127)
        midiVelocy = 127;

    if (midiVelocy == 0)
        midiVelocy = lastVelocy;

    //PRE-effects
    if (note->effPack.get(18)) //let ring
    {
        openLetRing(stringN,midiNote,midiVelocy,channel);
        return true;
    }

    if (note->effPack.get(2)) //palm mute
        midiVelocy = calcPalmMuteVelocy(midiVelocy);

    if (note->effPack.get(21)) //ghost note
        midiVelocy = midiVelocy > 10 ? midiVelocy - 10 : 1 ;

    if (note->effPack.get(27)) //heavy accented
        midiVelocy = midiVelocy < 110 ? midiVelocy+midiVelocy/10 : 127;

    if (note->effPack.get(22)) //grace note
    {   //attention - deadcode - realize
        midiNote += 2;
    }

    if (note->effPack.inRange(11,16))
    { //11-16 - harmonics
      if (note->effPack.get(11))
      {
          if (fret==7) midiNote += 12;
          if (fret==5) midiNote += 19;
      }
      if (note->effPack.get(14))
      {
          midiNote += 12;
      }
      //2 artif+5; 3 artif+7; 6 artif+12;
      //3 - tapped; 5 - semi
    }

    //up down stroke 29+

    pushNoteOn(midiNote,midiVelocy,channel);

    return true;
}

bool MidiTrack::addPostEffects(Beat *beat, std::uint8_t channel)
{
    //POST-effects

    for (size_t i =0; i < beat->size(); ++i)
    {
        auto& note = beat->at(i);

        std::uint8_t fret = note->getFret();
        std::uint8_t stringN = note->getStringNumber();
        std::uint8_t midiNote = fret + tunes[stringN-1];

        //std::uint8_t volume = note->getVolume();
        std::uint8_t midiVelocy = 95; //calcMidiVolumeGP(volume);


        std::uint8_t noteState = note->getState();

        if (noteState == Note::deadNote) //dead note
        {
            short tempAccum = accum;
            accum = 20;
            pushNoteOff(midiNote,midiVelocy,channel);

            if (tempAccum > 20)
                tempAccum -= 20;
            else
                tempAccum = 0;

            accum = tempAccum;
            //DEAD NOTE
        }

        if (note->effPack.get(22))
        {
            short int graceLen = (accum/8);
            if (graceLen == 0)
                graceLen = 1;

            auto mSignalGraceOff = std::make_unique<MidiSignal>(0x80  | channel,midiNote+2,80,graceLen-1);
            push_back(std::move(mSignalGraceOff));
            auto mSignalOn = std::make_unique<MidiSignal>(0x90  | channel,midiNote,midiVelocy,1);
            push_back(std::move(mSignalOn));
            accum -= graceLen;
        }

        if (note->effPack.inRange(3,10))
        {
            if ( (note->effPack == 4) || (note->effPack == 5))
            {
                   //if (effects==5) velocyShift=19; //set decreace legatto slide
                    short int slideStep = accum/8;
                    pushSlideUp(channel,2,slideStep);//channel + shift
                    takeAccum();
            }
            else if ((note->effPack == 8) || (note->effPack == 6))
            {
                /*/slide /. + // slide . \
                //not really same but generates somth */
                short int slideStep = accum/8;
                pushSlideDown(channel,7,slideStep);//channel + shift
                takeAccum();
            }
            else if ((note->effPack == 9)|| (note->effPack == 7))
            {
                short int slideStep = accum/8;
                pushSlideUp(channel,7,slideStep);//channel + shift
                takeAccum();

            }
            else if (note->effPack == 10)
            {   //legato - as normal one should decreace sound of next note
                //velocyShift=19; //set decreace
            }
        }

        if (note->effPack.get(17)) //bend
        {
            Package *bendPack = note->effPack.getPack(17);

            if (bendPack) //attention possible errors escaped
            {
                BendPoints *bend = (BendPoints*) bendPack->getPointer();
                pushBend(accum,bend,channel); //channel !!!!
                takeAccum();
            }
        }

        if (note->effPack.get(24)) //tremolo pick
        {
            //pushTremoloPick - tremolo pick - trills

            short int tremoloStep = accum/4;

            for (int i = 0; i < 3; ++i) {
                auto mSignalOff = std::make_unique<MidiSignal>(0x80 | channel,midiNote,midiVelocy,tremoloStep);
                push_back(std::move(mSignalOff));
                auto mSignalOn = std::make_unique<MidiSignal>(0x90 | channel,midiNote,midiVelocy,0);
                push_back(std::move(mSignalOn));
            }

            accum = tremoloStep;
            //takeAccum();
        }

        if (note->effPack.get(23)) {
            //stokato - stop earlier
            short halfAccum = accum/2;

            auto mSignalOff = std::make_unique<MidiSignal>(0x80 | channel,midiNote,midiVelocy,halfAccum);
            push_back(std::move(mSignalOff));

            accum = halfAccum;
        }

        if (note->effPack.get(1))
        {
            //vibrato
            //push vibration

            short int vibroStep = accum/6;
            pushVibration(channel,3,vibroStep);
            takeAccum();
        }

    }
}

bool MidiTrack::addSignalsFromNoteOff(Note *note, std::uint8_t channel)
{
    if (note->effPack.get(18)) //let ring
        //skip let ring
        return false;


    std::uint8_t noteState = note->getState();

    if ((noteState == Note::leegedLeeg)||
            (noteState == Note::leegedNormal)) //refact note stat
        //skip - next is leeg
        return false;


    if (note->effPack.get(23))
        return false; //skip stokkato





    std::uint8_t fret = note->getFret();
    std::uint8_t stringN = note->getStringNumber();
    std::uint8_t midiNote = fret + tunes[stringN-1];

    std::uint8_t volume = note->getVolume();
    std::uint8_t midiVelocy = 80; //calcMidiVolumeGP(volume);


    if (note->effPack.inRange(11,16))
    { //11-16 - harmonics
        //update calculations for harmonics
      if (note->effPack.get(11))
      {
          if (fret==7) midiNote += 12;
          if (fret==5) midiNote += 19;
      }
      if (note->effPack.get(14))
      {
          midiNote += 12;
      }
      //2 artif+5; 3 artif+7; 6 artif+12;
      //3 - tapped; 5 - semi
    }

    pushNoteOff(midiNote,midiVelocy,channel);

    return true;
}


void MidiTrack::pushBend(short rOffset, void *bendP, std::uint8_t channel)
{
    BendPoints *bend = (BendPoints*)bendP;

    size_t lastAbs = 0;
    size_t lastH = 0;

    short rAccum = 0;

    if (midiExportLog)
    qDebug() << "Bend rOffset="<<rOffset;

    //something did changed after moving to more own format of bends

    for (size_t i = 0 ; i < bend->size(); ++i)
    {
        size_t curAbs = bend->at(i).horizontal;
        size_t curH = bend->at(i).vertical;

        size_t shiftAbs = curAbs - lastAbs;
        size_t shiftH = curH - lastH;

        if (midiExportLog)
        qDebug() << "AbsShift="<<shiftAbs<<"; HShift="<<shiftH;

        if (shiftH != 0)
        {
            std::uint8_t lastShift = 64+(lastH*32)/4; //decreased from 100 to 4
            std::uint8_t curShift = 64+(curH*32)/4; //next

            if (midiExportLog)
            qDebug() <<"lastShiftPitch="<<lastShift<<"; curShiftPitch="<<curShift;

            double rhyStep = (shiftAbs*rOffset)/600.0; //10 steps
            double pitchStep = (curShift-lastShift)/10.0;

            if (midiExportLog)
            qDebug() <<"rStep="<<rhyStep<<"; rAccum="<<rAccum<<"; pSh="<<pitchStep;

            auto mSignalBendOpen = std::make_unique<MidiSignal>(0xE0 |channel,0,lastShift, rAccum);
            this->push_back(std::move(mSignalBendOpen));
            rAccum = 0;

            double stepsDone = 0.0;
            double shiftDone = 0.0;

            for (int i = 0; i < 10; ++i) {
                double thisStep = rhyStep*(i+1) - stepsDone;
                stepsDone += thisStep;
                double thisShift = pitchStep*(i+1) - shiftDone;
                shiftDone += thisShift;
                short thisROffset = thisStep;
                short thisMidiShift = thisShift;
                short totalShiftNow = lastShift + shiftDone;

                auto mSignalBend = std::make_unique<MidiSignal>(0xE0 | channel,0,totalShiftNow,thisROffset);
                this->push_back(std::move(mSignalBend));

                if (midiExportLog)
                    qDebug() <<"rD="<<thisStep<<"; sD="<<thisShift<<" shiftDone="<<shiftDone;
                if (midiExportLog)
                    qDebug() <<"rOff="<<thisROffset<<"; midiSh="<<thisMidiShift;
            }
        }
        else
        {
            //no change - just calculate rOffset
            rAccum += shiftAbs*rOffset/60;
        }

        lastAbs = curAbs;
        lastH = curH;
    }

    //last point push
    std::uint8_t lastShift = 64+(lastH*32)/4; //decreased from 100
    auto mSignalBendLast = std::make_unique<MidiSignal>(0xE0 | channel,0,lastShift, rAccum);
    this->push_back(std::move(mSignalBendLast));
    auto mSignalBendClose = std::make_unique<MidiSignal>(0xE0 | channel,0,64,0);
    this->push_back(std::move(mSignalBendClose));

    if (midiExportLog)
    qDebug() << "done";
}
