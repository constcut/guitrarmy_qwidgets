



#include "g0/amusic.h"



#include "g0/astreaming.h"
static AStreaming logger("amusic");

//#include <cmath>

bool museLog=false;

#include "g0/midifile.h"
#include "g0/tab.h"
//R


//first bit determines pause or signal
//64,32,16,8,4,2,1,11 - 3 bits
//next 4 bits used for
//0 - normal 1 - dot. 2-14 trumplets. 15 - reserved

bool Rythm::isPause(size_t ind)
{
    byte val = operator [](ind);

    return val > 127; // check
}

void Rythm::countSize()
{
    //calc over all size as number of minimum elements;
    //then if possible go them to the /4 for num denum if they not set
}


Rythm::MLen Rythm::getMLen(size_t ind)
{
   byte val = this->getV(ind);
   byte maskedMLen = val & MASK_MLEN;
   maskedMLen >>=4; //check

   Rythm::MLen mlen=(Rythm::MLen)maskedMLen;
   return mlen;
}

Rythm::TimeMetric Rythm::getTimeMetric(size_t ind)
{
    byte val = getV(ind);
    byte maskedTimeMetr = val & MASK_TIME_METRIC;

    Rythm::TimeMetric timeMetr =(Rythm::TimeMetric) maskedTimeMetr;
    return timeMetr;
}

//then there will appear numerate/denumerate in progress

//M

//hieghst byte used for tone value coding:
//3 bits of them for the octave value
//4 bits for the key value (12 values)
//another bit thats left is for the effects flag;

//in next byte we hold sequent infromation
//2 bit - 4 values used for:
//0-normal, 1-not finished, 2-continued, 3-finished
//harmonic presence bit also lays there
//else 5 bits used for the values of volume
//like fff ff etc, with few additions

//left 2 bytes could be used for some effects store,
//or it could be managed to use only 2 bytes there


int getOctave(ul value)
{
    value &= MASK_OCTAVE;
    value >>= SHIFT_OCTAVE;
    return value;
}

int getKey(ul value)
{
    value &= MASK_KEY;
    value >>= SHIFT_KEY;
    return value;;
}

Melody::mnote Melody::getMNote(size_t ind)
{
    ul val = getV(ind);

    Melody::mnote mn;
    mn.key = getKey(val);
    mn.octave = getOctave(val);

    return mn;
}

bool Melody::effectPresent(size_t ind)
{
     ul val = getV(ind);

     return val & MASK_EFFECTS;
}

Melody::NoteState Melody::getNoteState(size_t ind)
{
     ul val = getV(ind);

     ul state = (val & MASK_NOTE_STATE) << SHIFT_NOTE_STATE;
     Melody::NoteState st = (Melody::NoteState)state;

     return st;
}


Melody::Volume Melody::getVolume(size_t ind)
{
     ul val = getV(ind);

     ul volume = (val & MASK_VOLUME) << SHIFT_VOLUME;
     Melody::Volume vol = (Melody::Volume)volume;

     return vol;
}

bool Melody::isHarmonic(size_t ind)
{
     ul val = getV(ind);

     //implement
     return val & MASK_HARMONY;
}


//AMu

AMusic::AMusic()
{
}

//Extending
///////MIDI file formats and Tab
///
///
///
/// AMusic::

void AMusic::test()
{
   if (museLog)  logger << "Testing if (museLog)  logs!";
}

bool packRythm(Beat *currentBeat, Rythm &rythm, short special=0, bool forcePause=false)
{
    ul globalRythm = 0;  //the result

    bool paused = false;

    if (currentBeat)
        paused = currentBeat->getPause();

    if (special)
    {
        globalRythm |= (1)<<15; //1st moved to 15 is 16th
        ul ulSpecial = special;
        ulSpecial <<=16;
        globalRythm |= ulSpecial;

        if (forcePause)
            paused = true;
    }
    else
    {
        byte dur = currentBeat->getDuration();
        byte durT = currentBeat->getDurationDetail();
        byte dotted = currentBeat->getDotted();

        globalRythm |= dur;
        globalRythm |= durT<<3;
        globalRythm |= dotted<<8;

        if (museLog)  LOG( << "Beat options "<<dur<<" "<<durT<<" "<<globalRythm);
    }

    if (paused)
    {
        globalRythm |= 128;
        if (museLog)  logger << "Paused";
    }

    rythm.add(globalRythm);

    return false; //returns true if continues (like pause)
}

void packMelody(Note *currentNote, Beat *currentBeat, Bar *currentBar,
                Track* currentTrack,ul beatIndex, ul noteIndex, Melody &melody,short int *ringRay)
{
    ul ind2 = beatIndex; //refact
    ul noteInd = noteIndex;

    EffectsPack beatEffects = currentBeat->getEffects();
    if (museLog)  logger << "Beat effects ";
    beatEffects.logIt();

    EffectsPack effects = currentNote->getEffects();
    if (museLog)  logger<<"Normal effects ";
    effects.logIt();

    if (!beatEffects.empty())
        effects.mergeWith(beatEffects);

    if (museLog)  logger<<"After merge";
    effects.logIt();

    byte nextFret=255;
    byte nextStringN=255;
    ul nextTune=0;

    Beat *nextBeat = currentBeat->getNext();


    ul barLen = currentBar->len();

    //! LET RING !!! ATTENTION - could be one of tab operations
    //===================
    if (effects.get(18))
    if ((ind2+1) != barLen)
    {
       Beat *nextBeat = currentBar->getV(ind2+1);

       if (museLog)  LOG( << "NextBear ptr "<<(int)nextBeat);
       if (museLog)  LOG( << "Next beat len "<<(int)nextBeat->len());

       Note *nextNote = 0;

       if (nextBeat->len())
        nextNote = nextBeat->getV(0);

       if (nextBeat->len() > 0)
       if (museLog)  LOG( << "Next note state "<<nextNote->getState());

       if (nextBeat->len() > 0)
       if ((nextNote->getState() ==  Melody::normalNote) || (nextNote->getState() == Melody::startingNote))
       {
        nextFret = nextNote->getFret();
        nextStringN = nextNote->getStringNumber();
        --nextStringN;
        nextTune = currentTrack->tuning.getTune(nextStringN-1);
       }
    }
    //===================
    //! LETR


    //AND SHOULD SKIP melody when paused
    byte fret = currentNote->getFret();
    byte stringN = currentNote->getStringNumber(); //values from 1 to 7
    stringN--; //decreace for indexing
    ul stringTune = currentTrack->tuning.getTune(stringN); //index
    //attention error over here ??
    if (currentTrack->isDrums())
        stringTune = 0;

    if (museLog)  LOG( << "Melody fret="<<fret<<" tune="<<stringTune);


    byte midiNoteValue = fret + stringTune;


    ul globalMelody = 0; //here we pack


    //Harmonics checkers
    //byte effects = currentNote->getEffect(); upper
    if (effects.inRange(11,16))// (effects>10)&&(effects<17))
    {
        if (effects.get(11))
        { //natural
            if (fret==7) midiNoteValue += 12;
            if (fret==5) midiNoteValue += 19; //check?
            //and 12 later
        }
        if (effects.get(14))
        { //pitch
            midiNoteValue += 12;
        }
        //2 artif+5; 3 artif+7; 6 artif+12;
        //3 - tapped; 5 - semi
    }


    byte noteValue = midiNoteValue % 12; //C-0.....B-11
    byte octaveValue = midiNoteValue / 12;
    noteValue <<=4;
    globalMelody = octaveValue + noteValue;

    if (museLog)  LOG( <<"Midi value = "<<midiNoteValue);
    //should appear as midi note value from 0 to 127

    if (museLog)  LOG( <<"Packing "<<octaveValue<<" "<<noteValue);


    ul noteVolume = currentNote->getVolume();

    byte tabNoteState = currentNote->getState();

    if (museLog)  LOG(<<"Tab note state "<<tabNoteState<<" ; volume "<<noteVolume);
    ul noteState = 0; //check for leags or let ring flag

    if (tabNoteState<=1)
        noteState = Melody::normalNote;
    if (tabNoteState==2)
        noteState = Melody::endingNote;

    if (tabNoteState==3)
        noteState = Melody::deadNote;

    if (tabNoteState==4)
        noteState = Melody::startingNote;
    if (tabNoteState==6)
        noteState = Melody::continingNote;


    ul postLetRingPush = 0;
    std::vector<ul> postLetRing;
    if (effects.get(18))
    { //let ring

        if (noteState != Melody::endingNote) //leeg end
        noteState = Melody::startingNote;

        if (currentNote->getStringNumber() != 255) //NOT NEXT STRING N AS IN NEXT CONDITION
        {
            if (ringRay[currentNote->getStringNumber()] != -1) //would have strange technics - could be later repaired
            {
                if (museLog)  LOG( << "String "<<currentNote->getStringNumber()<<" ; tune "<<nextTune<<" ; fret "<<nextFret);
                byte midiNoteValueOld =  ringRay[currentNote->getStringNumber()]; //+nextTune;
                byte noteValueOld = midiNoteValueOld % 12; //C-0.....B-11
                byte octaveValueOld = midiNoteValueOld / 12;
                noteValueOld <<=4;
                ul globalMelodyOld = octaveValueOld + noteValueOld;
                ul noteStateOld = Melody::endingNote;
                globalMelodyOld += (noteStateOld<<8) + (noteVolume<<11); //fuck volume off


                 postLetRingPush = globalMelodyOld;
                postLetRing.push_back(postLetRingPush);
                //TO PREVIOUS LZPlane
            }

            //ringRay[nextStringN]=nextFret;
        }

        ringRay[currentNote->getStringNumber()] = midiNoteValue;//currentNote->getFret(); //not -1
    }

    globalMelody += (noteState<<8) + (noteVolume<<11);

    if (!effects.empty())
    {
         globalMelody += (1<<16);
    }

    byte closeLetRings = 0; //false;

    if ((noteInd+1) != currentBeat->len()) //this Note has folowing harmony
        globalMelody |= 1<<24; //turn on 1st bit on 3rd byte
    else
    {
        //last note in harmony or the only one in beat
        if ((ind2+1) == barLen)
        {
            //and last one in bar
            for (int i = 0; i < 10; ++i)
                if (ringRay[i]!=-1)
                {
                    ++closeLetRings;
                     globalMelody |= 1<<24;
                    //break;
                }
        }
    }

    if (closeLetRings==0)
    {
        if (nextBeat)
        if (nextBeat->getPause())
        {
            for (int i = 0; i < 10; ++i)
                if (ringRay[i]!=-1)
                {
                    ++closeLetRings;
                     globalMelody |= 1<<24;
                    //break;
                }
        }
    }

    //CLEAN IT LATER
    if (postLetRingPush)
        globalMelody |= 1<<24;



    ul nextIndex = melody.len();
    melody.add(globalMelody);

    if (!effects.empty())
    {//must switch
        effects.set(18,false);
        melody.mapOfEffects.setEffect(nextIndex,effects);
    }


    //before too late
    if (postLetRingPush)
    {
       for (ul indLR = 0; indLR < postLetRing.size(); ++indLR)
       {
           postLetRingPush = postLetRing[indLR];

           if (((noteInd+1) != currentBeat->len())
               ||
               (closeLetRings))
            postLetRingPush |= 1<<24; //always has normal harmony

         melody.add(postLetRingPush);
       }
       postLetRing.clear();
    }


    if (closeLetRings!=0)
    {
        byte closedCounter = 0;


        for (int i = 0; i < 9; ++i)
        {
            short int fromRingRay=ringRay[i];
            if (fromRingRay!=-1)
            {
                //byte noteTune = currentTrack->tuning.getTune(i+1); //HERE?
                byte midiNoteValueOld =  fromRingRay; //+noteTune;
                byte noteValueOld = midiNoteValueOld % 12; //C-0.....B-11
                byte octaveValueOld = midiNoteValueOld / 12;
                noteValueOld <<=4;
                ul globalMelodyOld = octaveValueOld + noteValueOld;
                ul noteStateOld = Melody::endingNote;
                globalMelodyOld += (noteStateOld<<8) + (noteVolume<<11);

                if (museLog)
                LOG( <<"Close LetRing on "<<i<<"; fret "<<ringRay[i]);//<<"; tune "<<noteTune);

                ++closedCounter;
                if (closedCounter != closeLetRings)
                    globalMelodyOld |= 1<<24;

                melody.add(globalMelodyOld);

                ringRay[i]=-1;
            }
        }
    }
}


bool AMusic::readFromTab(Tab *tab, ul barIndex)
{
    ul polyLen = tab->len();

    if (museLog)  LOG( << "Starting reading music abstraction from tablature. ");
    if (museLog)  LOG( << "Amount of tracks "<<polyLen<<" !;");

    bpm = tab->getBPM();

    if (museLog)  LOG( <<"Base bpm "<<bpm);

    //Get aware of solo tracks
    std::vector<ul> indecesToSkip;
    bool soloTurnedOn = false;

    for (ul trackIndex = 0; trackIndex < polyLen; ++trackIndex)
    {
        Track *currentTrack = tab->getV(trackIndex);
        byte trackStatus = currentTrack->getStatus();

        if (trackStatus != 2)
            indecesToSkip.push_back(trackIndex);
        else
            soloTurnedOn=true;
    }

    if (soloTurnedOn==false)
        indecesToSkip.clear();

    //Main generation

    for (ul trackIndex = 0; trackIndex < polyLen; ++trackIndex)
    {
        Rythm rythm; //each track should
        Melody melody;

        short int ringRay[10]; //from string size
        for (int i=0; i < 10; ++i) ringRay[i]=-1;

        Track *currentTrack = tab->getV(trackIndex); //only for the first
        ul trackLen = currentTrack->len();

        byte trackStatus = currentTrack->getStatus();

        if (trackStatus==1) //mute
            continue;

        if (soloTurnedOn)
        {
            bool skipThatTrack = false;
            for (int i = 0; i < indecesToSkip.size(); ++i)
                if (indecesToSkip[i]==trackIndex)
                {
                    skipThatTrack=true;
                    break;
                }
            if (skipThatTrack)
                continue;
        }

        int portIndex = currentTrack->getGPCOMPInts(0);
        int channelIndex = currentTrack->getGPCOMPInts(1);
        int midiChanPortIndex = (portIndex-1)*16 + (channelIndex-1);

        bool drumsTrack = currentTrack->isDrums();
        //HEREATTENTION: set for other tracks
        melody.instrument = currentTrack->getInstrument();//tab.GpCompMidiChannels[midiChanPortIndex].instrument;
        melody.panoram = currentTrack->getPan();//tab.GpCompMidiChannels[midiChanPortIndex].balance;
        melody.volume = currentTrack->getVolume();//tab.GpCompMidiChannels[midiChanPortIndex].volume;
        //volume+pan is for the rhythm

        if (drumsTrack)
        {
            melody.instrument = 666;//magic drum number attention
        }

        if (museLog)  LOG( <<"Port "<<portIndex<<" instr "<<(int)melody.instrument<<"; ");
        if (museLog)  LOG( <<"Chan "<<channelIndex<<" full index "<<midiChanPortIndex);

        if (museLog)  logger<<"if (museLog)  log";


        trackLen = currentTrack->timeLoop.len();

        //barIndex used to start not from the first
        for (ul ind = barIndex; ind < trackLen; ++ind)
        {

            //BEFORE TIMELOOP: Bar *currentBar = &currentTrack->getV(ind);
            Bar *currentBar = currentTrack->timeLoop.getV(ind);


            //COMPLETE STATUS (check for incomplete or exceed Bars)
            byte completeStatus = currentBar->getCompleteStatus();
            short specialRhythm = 0;
            ul exceedIndex = 0;

            if (completeStatus == 2)//exceed
            {
                specialRhythm = currentBar->getCompleteAbs();
                exceedIndex = currentBar->getCompleteIndex();

                if (museLog)
                LOG( << "Exceed special "<<specialRhythm<<" exceedIndex "<<exceedIndex);
            }

            if (completeStatus == 1)
            {
                 specialRhythm = currentBar->getCompleteAbs();
                 if(museLog)
                 LOG( << "Incomplete special "<<specialRhythm);
            }

            if (museLog)  LOG( << "Trace in bar #"<<ind<<" of "<<trackLen);
            ul barLen = currentBar->len();
            if (museLog)  LOG( << "Bar size in beats " << barLen);


            Note *firstNote = 0;

            if (barLen)
                if (currentBar->getV(0)->len())
                firstNote = currentBar->getV(0)->getV(0);

            if ((barLen == 1) && (firstNote == 0))
            {   //empty Bar

                if(museLog)
                logger <<"EMPTY BAR found! ";

                //COULD PUSH SINGLE ONE!

                byte barDenum =  currentBar->getSignDenum();
                byte barNum =  currentBar->getSignNum();

                ul rValue = 0;

                if (barDenum == 1) rValue = 0; //attention check
                if (barDenum == 2) rValue = 1;
                if (barDenum == 4) rValue = 2;
                if (barDenum == 8) rValue = 3;
                if (barDenum == 16) rValue = 4;
                if (barDenum == 32) rValue = 5;
                if (barDenum == 64) rValue = 6;

                if (museLog)  LOG(<<"Empty bar num "<<barNum<<" empty bar den "<<barDenum<<"; "<<rValue);

                rValue |= 128;

                for (byte pausesInd = 0; pausesInd <barNum; ++pausesInd)
                {
                   ul localRValue = rValue;
                   rythm.add(localRValue);
                }

                continue;
            }
            else
                //normal Bar
                for (ul ind2 = 0; ind2 < barLen; ++ind2)
                {
                    if (museLog)  LOG( << "Trace beat #"<<ind2<<" of "<<barLen);

                    Beat *currentBeat = 0;
                    Note *currentNote = 0;

                    if (currentBar->len())
                    {
                        currentBeat = currentBar->getV(ind2);

                        if (currentBeat->len())
                            currentNote = currentBeat->getV(0);
                    }

                    if ((completeStatus==2) && (exceedIndex == ind2))
                        packRythm(currentBeat, rythm ,specialRhythm);

                    else
                       //normal way
                       packRythm(currentBeat,rythm);

                    if (museLog)  LOG( << "Current note pointer "<<(int)currentNote);

                    if (currentNote)
                        for (ul noteInd = 0; noteInd < currentBeat->len(); ++noteInd)
                        {
                            //Rythm packed
                            //Now we pack melody
                            //In fact this block should appear in CYCLE
                            currentNote = currentBeat->getV(noteInd);

                            packMelody(currentNote,currentBeat,currentBar,
                                       currentTrack,ind2,noteInd,melody,ringRay); //refact
                        } //Melody packed

                    if ((completeStatus==2) && (exceedIndex == ind2))
                        break;
                }

                //clean ring ray after bar
                int closeLetR = 0;
                int closedCounter = 0;
                for (int i=0; i < 10; ++i)
                {
                    short int fromRingRay  = ringRay[i]; //=-1;

                    if (fromRingRay != -1)
                    ++closeLetR;

                    //ringRay[i]=-1;
                }

                if (closeLetR)
                {
                    for (int i=0; i < 10; ++i)
                    {
                        short int fromRingRay  = ringRay[i]; //=-1;

                        if (fromRingRay!=-1)
                        {
                            byte midiNoteValueOld =  fromRingRay; //+noteTune;
                            byte noteValueOld = midiNoteValueOld % 12; //C-0.....B-11
                            byte octaveValueOld = midiNoteValueOld / 12;
                            noteValueOld <<=4;
                            ul globalMelodyOld = octaveValueOld + noteValueOld;
                            ul noteStateOld = Melody::endingNote;
                            globalMelodyOld += (noteStateOld<<8) + (1<<11);

                            if (museLog)
                            LOG( <<"Close LetRing on "<<i<<"; fret "<<ringRay[i]);

                            ++closedCounter;
                            if (closedCounter != closeLetR)
                                globalMelodyOld |= 1<<24;

                            melody.add(globalMelodyOld);

                            ringRay[i]=-1;
                        }
                    }
                }


            if (completeStatus==1) //incomplete action
            {
                //PUSH pause with value of special
                if (museLog)
                logger << "PUSHING incomplete";
                packRythm(0,rythm, specialRhythm,true);
            }

        }

        if (museLog)  LOG( << "Track writen: "<<(int)rythm.len()<<" rythm and "<<(int)melody.len()<<" melody elements.");

        rSection.add(rythm);
        mSection.add(melody);

        if (museLog)  logger << "AMusic track loading from tab done";
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
///-------------Printing to stream---------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//
void AMusic::printToStream(std::ostream &stream)
{
    //for r section for m section

    stream << "Outputing Music Abstraction"<<std::endl;

    //first output just in sequences

    Rythm *currentRythm = &rSection.getV(0);
    Melody *currentMelody = &mSection.getV(0);

    //while (currentRythm)
    {
        //++currentRythm; ++currentMelody;
        ul melodyI = 0;
        ul melodyLen = currentMelody->len();

        for (ul i = 0; i < currentRythm->len(); ++i)
        {
            byte rByte = currentRythm->getV(i);
            //could fail if no melody at all - attention

            stream << "R["<<(int)rByte << "]";

            if (melodyLen > melodyI)
            {
                ul mWord = currentMelody->getV(melodyI ); //another index should apper
                byte mOctave = mWord & 0xf;
                byte mNote = mWord & 0xf0;
                mNote>>=4;

                ul hasHarmony = (mWord>>24) & 1;
                //and extra check on pause

               if ((rByte&128)==0)
               {
               stream<<"  M[" <<mWord<<" : "<<(int)mOctave
                      <<" : "<<(int)mNote<<" : "<<hasHarmony<<"];"<<std::endl;

               ++melodyI;
               }
               else
                   stream<<std::endl;
            }
            else
                stream<<std::endl;
            //and more information should follow
        }


    }


    stream << "Outputing Music Abstraction done"<<std::endl;

}
