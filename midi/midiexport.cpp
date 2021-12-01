#include "midiexport.h"
#include "g0/config.h"

bool midiExportLog = false;

#include <QDebug>



std::unique_ptr<MidiFile> exportMidi(Tab* tab, size_t shiftTheCursor) {
    auto output = std::make_unique<MidiFile>();
    //time line track
    {
        auto timeLineTrack = std::make_unique<MidiTrack>();
        tab->createTimeLine(shiftTheCursor);
        //std::cout << tab->timeLine.size() << " is size of timeLine" <<std::endl;
        size_t tlAccum = 0;
        for (size_t i = 0; i < tab->timeLine.size(); ++i) {
            if (tab->timeLine[i].type==1) {
                //changing bpm
                //std::cout << "Push bpm to "<<tab->timeLine[i].value<<std::endl;
                //refact to cover under another layer
                short int rhyBase = 120;
                short int power2 = 2<<(3);
                int preRValue = rhyBase*power2/4;
                preRValue *= tlAccum;
                preRValue /= 1000;
                timeLineTrack->pushChangeBPM(tab->timeLine[i].value, preRValue);
                tlAccum = 0;
            }

            if (tab->timeLine[i].type==0) {
                tlAccum += tab->timeLine[i].value;
            }

            if (tab->timeLine[i].type==2) {
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
        output->push_back(std::move(timeLineTrack));
    }

    if (CONF_PARAM("metronome")=="1") {
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

                    int num = newDen = bar->getSignNum(); //TODO shouldn't be -1
                    for (int i = 0; i < num; ++i)
                        if (firstRun){
                            auto noteOn = std::make_unique<MidiSignal>(0x90 | 9, 33, 127,0);
                            metronomeClickTrack->push_back(std::move(noteOn));
                            firstRun = false;
                        }
                        else{
                            auto noteOn = std::make_unique<MidiSignal>(0x90 | 9, 33, 127,metronomeClickSize);
                            metronomeClickTrack->push_back(std::move(noteOn));
                        }
                }
                metronomeClickTrack->pushEvent47();
                output->push_back(std::move(metronomeClickTrack));
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
            for (size_t i2 = 0; i2 < indecesToSkip.size(); ++i2)
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
                //realInd = 10; //TODO review this part? WOuld it be broken if we have many tracks?
            exportTrack(track.get(), mTrack.get(), realInd,startCursorBar);
        }
        else
            exportTrack(track.get(), mTrack.get(), i, startCursorBar);

        //clock_t afterT3 = getTime();
        output->push_back(std::move(mTrack));
        //clock_t afterT4 = getTime();
        //int addDiff = afterT4-afterT3;
    }

    return output;
}



void exportTrack(Track* track, MidiTrack* midiTrack, size_t channel, size_t shiftCursorBar) {

    size_t instrument = track->getInstrument();
    std::uint8_t midiPan = midiTrack->calcMidiPanoramGP(track->getPan());
    std::uint8_t midiVol = midiTrack->calcMidiVolumeGP(track->getVolume());
    midiTrack->pushChangeInstrument(instrument, channel);
    midiTrack->pushChangePanoram(midiPan, channel);
    midiTrack->pushChangeVolume(midiVol, channel);

    std::uint8_t theTunes[11];
    for (size_t i = 0; i < 10; ++i)
        if (track->isDrums())
            theTunes[i] = 0;
        else
            theTunes[i] = track->tuning.getTune(i);

    midiTrack->setTunes(theTunes);


    size_t trackLen = track->timeLoop.size();

    for (size_t i = shiftCursorBar ; i < trackLen; ++i)
    {
        Bar *bar = track->timeLoop.at(i);

        //BAR STATUS
        std::uint8_t completeStatus = bar->getCompleteStatus();

        size_t barLen = bar->size();
        short specialLast = 0;
        size_t completeIndex = 0;

        if (completeStatus == 2) {
           //barLen = bar->getCompleteIndex();
            completeIndex = bar->getCompleteIndex();
           specialLast = bar->getCompleteAbs();
        }

        //Signature

        for (size_t j = 0; j < barLen; ++j) {
            Beat *beat = bar->at(j).get();
            if ((completeStatus == 2) && (j == completeIndex)) {
                exportBeat(beat, midiTrack, channel, specialLast);
                break;
            }
               else
                   exportBeat(beat, midiTrack, channel);
        }

       midiTrack->closeLetRings(channel);

       if (completeStatus == 1)
           midiTrack->finishIncomplete(bar->getCompleteAbs()); //short

    }
    midiTrack->pushEvent47();
}


void exportBeat(Beat* beat, MidiTrack* midiTrack, size_t channel, short specialRhy) {
    std::uint8_t dur,det,dot; //rhythm value

    dur = beat->getDuration();
    det = beat->getDurationDetail();
    dot = beat->getDotted();

    const int baseAmount = 120;

    int rOffset = 0;
    if (specialRhy == 0)
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
            rOffset = midiTrack->calcRhythmDetail(det,rOffset); //FEW MISSING
    }
    else
    {
        //constant refact
         short int rhyBase = 120;

        short int power2 = 2<<(3);
        int preRValue = rhyBase*power2/4;

        preRValue *= specialRhy;
        preRValue /= 1000;

        rOffset=preRValue;
    }

    if (beat->getPause())
    {
        midiTrack->closeLetRings(channel);
        midiTrack->accumulate(rOffset);
        return;
    }

    //BEAT EFFECTS:
    if (beat->effPack.getEffectAt(Effect::Changes))
    {
        //changes

        {
            Beat::ChangesList *changes = &beat->changes;

            for (size_t indexChange = 0; indexChange != changes->size(); ++indexChange)
            {
              if (changes->at(indexChange).changeType==8) {
                  size_t newBPM = changes->at(indexChange).changeValue; //skipped according to time line
                  //pushChangeBPM(newBPM,accum);
                  //takeAccum();
              }

              if (changes->at(indexChange).changeType==1) {
                 size_t newInstr = changes->at(indexChange).changeValue;
                 midiTrack->pushChangeInstrument(newInstr,channel, midiTrack->accum);
                 midiTrack->takeAccum();
              }

              if (changes->at(indexChange).changeType==2) {
                  std::uint8_t newVol = changes->at(indexChange).changeValue;
                  std::uint8_t midiNewVolume = newVol*8;
                  if (midiNewVolume > 127)
                      midiNewVolume = 127;
                  midiTrack->pushChangeVolume(midiNewVolume,channel); //must take accum
              }

              if (changes->at(indexChange).changeType==3) {
                    std::uint8_t newPan = changes->at(indexChange).changeValue;
                    std::uint8_t midiNewPanoram = newPan*8;
                    if (midiNewPanoram > 127)
                        midiNewPanoram = 127;
                    midiTrack->pushChangePanoram(midiNewPanoram,channel); //must take accum
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

        if (beat->effPack.getEffectAt(Effect::DownStroke)) //down
            trueIndex = (beatLen - i - 1);

        if (beat->effPack.inRange(Effect::UpStroke,Effect::DownStroke)) //up down strokes
        {
            midiTrack->accumulate(strokeStep);
            rOffset -= strokeStep;
        }

        Note *note = beat->at(trueIndex).get();
        exportSingalsFromNoteOn(note, midiTrack, channel);
    }

    //ACCUMULATE BEFORE EFFECTS AND OFFS
    midiTrack->accumulate(rOffset);

    if (beat->effPack.getEffectAt(Effect::FadeIn)) //fade in
    {
        midiTrack->pushFadeIn(midiTrack->accum, channel);
        midiTrack->takeAccum();
    }

    if (beat->effPack.getEffectAt(Effect::Tremolo)) //tremolo
    {
        midiTrack->pushTremolo(midiTrack->accum);
        midiTrack->takeAccum();
    }

    exportPostEffect(beat, midiTrack, channel);

    for (size_t i =0; i < beat->size(); ++i)
    {
        Note *note = beat->at(i).get();
        exportSingalsFromNoteOff(note, midiTrack, channel);
    }
}




bool exportSingalsFromNoteOn(Note* note, MidiTrack* midiTrack, std::uint8_t channel) {
    std::uint8_t noteState = note->getState();

    if ((noteState==Note::leegNote) || (noteState==Note::leegedLeeg))
        return false;


    std::uint8_t fret = note->getFret();
    std::uint8_t stringN = note->getStringNumber();
    std::uint8_t midiNote = fret + midiTrack->tunes[stringN-1]; //TODO убрать потом в класс генератор

    std::uint8_t volume = note->getVolume();
    std::uint8_t midiVelocy = volume*15; //calcMidiVolumeGP(volume);

    std::uint8_t lastVelocy = 95; //not real last one yet

    if (midiVelocy > 127)
        midiVelocy = 127;

    if (midiVelocy == 0)
        midiVelocy = lastVelocy;

    //PRE-effects
    if (note->effPack.getEffectAt(Effect::LetRing)) //let ring
    {
        midiTrack->openLetRing(stringN,midiNote,midiVelocy,channel);
        return true;
    }

    if (note->effPack.getEffectAt(Effect::PalmMute)) //palm mute
        midiVelocy = midiTrack->calcPalmMuteVelocy(midiVelocy);

    if (note->effPack.getEffectAt(Effect::GhostNote)) //ghost note
        midiVelocy = midiVelocy > 10 ? midiVelocy - 10 : 1 ;

    if (note->effPack.getEffectAt(Effect::HeavyAccented)) //heavy accented
        midiVelocy = midiVelocy < 110 ? midiVelocy+midiVelocy/10 : 127;

    if (note->effPack.getEffectAt(Effect::GraceNote)) //grace note
    {   //attention - deadcode - realize
        midiNote += 2;
    }

    if (note->effPack.inRange(Effect::Harmonics,Effect::HarmonicsV6))
    { //11-16 - harmonics
      if (note->effPack.getEffectAt(Effect::Harmonics))
      {
          if (fret==7) midiNote += 12;
          if (fret==5) midiNote += 19;
      }
      if (note->effPack.getEffectAt(Effect::HarmonicsV4))
      {
          midiNote += 12;
      }
      //2 artif+5; 3 artif+7; 6 artif+12;
      //3 - tapped; 5 - semi
    }

    //up down stroke 29+

    midiTrack->pushNoteOn(midiNote,midiVelocy,channel);

    return true;
}



bool exportSingalsFromNoteOff(Note* note, MidiTrack* midiTrack, std::uint8_t channel) {
    if (note->effPack.getEffectAt(Effect::LetRing)) //let ring
        //skip let ring
        return false;

    std::uint8_t noteState = note->getState();

    if ((noteState == Note::leegedLeeg)||
            (noteState == Note::leegedNormal)) //refact note stat
        //skip - next is leeg
        return false;

    if (note->effPack.getEffectAt(Effect::Stokatto))
        return false; //skip stokkato

    std::uint8_t fret = note->getFret();
    std::uint8_t stringN = note->getStringNumber();
    std::uint8_t midiNote = fret + midiTrack->tunes[stringN-1];

    std::uint8_t volume = note->getVolume();
    std::uint8_t midiVelocy = 80; //calcMidiVolumeGP(volume);


    if (note->effPack.inRange(Effect::Harmonics,Effect::HarmonicsV6))
    { //11-16 - harmonics
        //update calculations for harmonics
      if (note->effPack.getEffectAt(Effect::Harmonics))
      {
          if (fret==7) midiNote += 12;
          if (fret==5) midiNote += 19;
      }
      if (note->effPack.getEffectAt(Effect::HarmonicsV4))
      {
          midiNote += 12;
      }
      //2 artif+5; 3 artif+7; 6 artif+12;
      //3 - tapped; 5 - semi
    }

    midiTrack->pushNoteOff(midiNote,midiVelocy,channel);
    return true;
}

void exportPostEffect(Beat* beat, MidiTrack* midiTrack, std::uint8_t channel) {
    //POST-effects

    for (size_t i =0; i < beat->size(); ++i)
    {
        auto& note = beat->at(i);

        std::uint8_t fret = note->getFret();
        std::uint8_t stringN = note->getStringNumber();
        std::uint8_t midiNote = fret + midiTrack->tunes[stringN-1];

        //std::uint8_t volume = note->getVolume();
        std::uint8_t midiVelocy = 95; //calcMidiVolumeGP(volume);


        std::uint8_t noteState = note->getState();

        if (noteState == Note::deadNote) //dead note
        {
            short tempAccum = midiTrack->accum;
            midiTrack->accum = 20;
            midiTrack->pushNoteOff(midiNote,midiVelocy,channel);

            if (tempAccum > 20)
                tempAccum -= 20;
            else
                tempAccum = 0;

            midiTrack->accum = tempAccum;
            //DEAD NOTE
        }

        if (note->effPack.getEffectAt(Effect::GraceNote))
        {
            short int graceLen = (midiTrack->accum/8);
            if (graceLen == 0)
                graceLen = 1;

            auto mSignalGraceOff = std::make_unique<MidiSignal>(0x80  | channel,midiNote+2,80,graceLen-1);
            midiTrack->push_back(std::move(mSignalGraceOff));
            auto mSignalOn = std::make_unique<MidiSignal>(0x90  | channel,midiNote,midiVelocy,1);
            midiTrack->push_back(std::move(mSignalOn));
            midiTrack->accum -= graceLen;
        }

        if (note->effPack.inRange(Effect::Hammer, Effect::Legato))
        {
            if ( (note->effPack == Effect::Slide) || (note->effPack == Effect::LegatoSlide))
            {
                   //if (effects==5) velocyShift=19; //set decreace legatto slide
                    short int slideStep = midiTrack->accum/8;
                    midiTrack->pushSlideUp(channel,2,slideStep);//channel + shift
                    midiTrack->takeAccum();
            }
            else if ((note->effPack == Effect::SlideDownV2) || (note->effPack == Effect::SlideDownV1))
            {
                short int slideStep = midiTrack->accum/8;
                midiTrack->pushSlideDown(channel,7,slideStep);//channel + shift
                midiTrack->takeAccum();
            }
            else if ((note->effPack == Effect::SlideUpV2)|| (note->effPack == Effect::SlideUpV1))
            {
                short int slideStep = midiTrack->accum/8;
                midiTrack->pushSlideUp(channel,7,slideStep);//channel + shift
                midiTrack->takeAccum();

            }
            else if (note->effPack == Effect::Legato)
            {   //legato - as normal one should decreace sound of next note
                //velocyShift=19; //set decreace
            }
        }

        if (note->effPack.getEffectAt(Effect::Bend)) { //bend
            BendPoints *bend = &note->bend;
            pushBendToTrack(bend, midiTrack,channel);
        }

        if (note->effPack.getEffectAt(Effect::TremoloPick)) { //tremolo pick {
            //pushTremoloPick - tremolo pick - trills
            short int tremoloStep = midiTrack->accum/4;
            for (size_t i = 0; i < 3; ++i) {
                auto mSignalOff = std::make_unique<MidiSignal>(0x80 | channel,midiNote,midiVelocy,tremoloStep);
                midiTrack->push_back(std::move(mSignalOff));
                auto mSignalOn = std::make_unique<MidiSignal>(0x90 | channel,midiNote,midiVelocy,0);
                midiTrack->push_back(std::move(mSignalOn));
            }
            midiTrack->accum = tremoloStep;
            //takeAccum();
        }

        if (note->effPack.getEffectAt(Effect::Stokatto)) {
            //stokato - stop earlier
            short halfAccum = midiTrack->accum/2;
            auto mSignalOff = std::make_unique<MidiSignal>(0x80 | channel,midiNote,midiVelocy,halfAccum);
            midiTrack->push_back(std::move(mSignalOff));
            midiTrack->accum = halfAccum;
        }

        if (note->effPack.getEffectAt(Effect::Vibrato)) {
            short int vibroStep = midiTrack->accum/6;
            midiTrack->pushVibration(channel,3,vibroStep);
            midiTrack->takeAccum();
        }

    }
}


void pushBendToTrack(BendPoints* bend, MidiTrack* midiTrack, std::uint8_t channel) {
    short rOffset = midiTrack->accum;
    midiTrack->takeAccum();

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
            midiTrack->push_back(std::move(mSignalBendOpen));
            rAccum = 0;

            double stepsDone = 0.0;
            double shiftDone = 0.0;

            for (size_t i = 0; i < 10; ++i) {
                double thisStep = rhyStep*(i+1) - stepsDone;
                stepsDone += thisStep;
                double thisShift = pitchStep*(i+1) - shiftDone;
                shiftDone += thisShift;
                short thisROffset = thisStep;
                short thisMidiShift = thisShift;
                short totalShiftNow = lastShift + shiftDone;

                auto mSignalBend = std::make_unique<MidiSignal>(0xE0 | channel,0,totalShiftNow,thisROffset);
                midiTrack->push_back(std::move(mSignalBend));

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
    midiTrack->push_back(std::move(mSignalBendLast));
    auto mSignalBendClose = std::make_unique<MidiSignal>(0xE0 | channel,0,64,0);
    midiTrack->push_back(std::move(mSignalBendClose));

    if (midiExportLog)
    qDebug() << "done";
}


