#include "gmyfile.h"

#include "astreaming.h"

//temp action for bends

#include "g0/gtpfiles.h"

AStreaming logger("gmyfile");

GmyFile::GmyFile()
{
}

//bends operation

void writeBendGMY(AFile *file, BendPoints *bend)
{
    byte bendType = bend->getType();
    file->write(&bendType,1);

    byte pointsCount = bend->len();
    file->write(&pointsCount,1);

    for (ul pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        BendPoint *point = &bend->getV(pointInd);

        byte absolutePosition = point->horizontal;
        byte verticalPosition = point->vertical;
        byte vibratoFlag = point->vFlag;

        file->write(&absolutePosition,1);
        file->write(&verticalPosition,1); //could be packed more
        file->write(&vibratoFlag,1); //but don't optimize before there is a need
    }
}


void readBendGMY(AFile *file, BendPoints *bend)
{
    byte bendType=0;
    file->read(&bendType,1);

    ul pointsCount = 0;
    file->read(&pointsCount,1); //cannot be more then 255

    LOG(<< "Type "<<bendType<<"; N= "<<pointsCount);
    bend->setType(bendType); //trem yet not handled anyway

    for (ul pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        byte absolutePosition = 0;
        byte verticalPosition = 0;
        byte vibratoFlag = 0;

        file->read(&absolutePosition,1);
        file->read(&verticalPosition,1);
        file->read(&vibratoFlag,1);

        LOG( << "Point# "<< pointInd << "; absPos="<<absolutePosition<<"; vertPos="
               <<verticalPosition<<"; vibrato- "<<vibratoFlag);

        BendPoint point;
        point.vertical = verticalPosition; //25 - 1/12 of 3 tones
        point.horizontal = absolutePosition;
        point.vFlag = vibratoFlag;
        bend->add(point);
    }

    LOG( << "Beng if (gtpLog)  logging finished with "<<(int)bend->len());
}



    //from the tab gp originally
bool GmyFile::saveToFile(AFile *file, Tab *tab)
{
    file->write("GA",2);
    char byteZero = 0;
    file->write(&byteZero,1);
    //fill file with 8bit 0


    byte effectsSet = 0; //development one
    byte tracksByteLen = 1; //1 byte for tracks
    byte barsByteLen = 2;
    byte noteByteLen = 1; //attention - all this fields yet not used directly

    file->write(&effectsSet,1);
    file->write(&tracksByteLen,1);
    file->write(&barsByteLen,1);
    file->write(&noteByteLen,1);

    //must not exceed the value!!
    //IDEA REFACT ATTENTION
    //make poly as double template
    //Poly<ul, Bar>
    //Poly<byte, Track>
    //Poly<int, Beat>
    ul tracksCount = tab->len();//attention please
    ul barsCount = tab->getV(0)->len(); //search largerst or not do such thing? refact

    file->write(&tracksCount,1); //256 tracks are insane
    file->write(&barsCount,2); //65 535 bars are insace
    //first 8 bytes of format


    //General tab information

    //1 bpm
    ul bpm = tab->getBPM();
    file->write(&bpm,2); //values more then 10 bits used for the 0.025 of bpm


    for (ul i = 0; i < tracksCount; ++i)
    {
        //Each track
        Track *track = tab->getV(i);

        //Name
        std::string trackName = track->getName();
        //for Track#1 could be used empty
        saveString(file,trackName);


        //strings N
        byte stringsCount = track->tuning.getStringsAmount();
        file->write(&stringsCount,1);
        //tuning

        LOG( << "Write strings count "<<stringsCount);

        for (byte sI=0; sI < stringsCount; ++ sI)
        {
            byte tune = track->tuning.getTune(sI);
            file->write(&tune,1);

            LOG( <<"Write tune "<<tune<<" for i="<<sI);
        }

        //maximum frets
        byte fretsLimit = track->getGPCOMPInts(3); //frets
        file->write(&fretsLimit,1);
        //prepare for capo
        byte capoSet = track->getGPCOMPInts(4);
        file->write(&capoSet,1);

        LOG( << "Write limits fret "<<fretsLimit<<"; capo "<<capoSet);


        int portIndex = track->getGPCOMPInts(0);
        int channelIndex = track->getGPCOMPInts(1);
        int midiChanPortIndex = (portIndex-1)*16 + (channelIndex-1);

        //extract from midi table -
            /// instrument

            //isDrums
        bool isDrums = track->isDrums();

        file->write(&isDrums,1);

        int instr = track->getInstrument();
        byte pan = track->getPan();
        byte vol = track->getVolume();

        file->write(&instr,2);

        //cover under track functions refact: getVolume getPan GeInstrument
            /// pan
        file->write(&pan,1);
            /// volume
        file->write(&vol,1);

        LOG( <<"IsD "<<isDrums<<"; instr-"<<
(int)track->getInstrument()<<
              "; +2more sets ");



        byte lastSignNum = 0;
        byte lastSignDen = 0;

        for (ul j = 0; j < barsCount; ++j)
        {
            //Each bar for that track
            Bar *bar;

            if (j < track->len())
                bar = track->getV(j);
            else
                bar = new Bar;



            if (i == 0) //first track
            {
                LOG( <<"First track bar "<<j);

                //work over repeats signs etc - set them only from first track
                //On POLY tab format this would be applied to each track
                ///HEADED: 1) repeats 2) signatures 3) Marker
                /// 1 - change num 2 - change denum
                /// 4 - begin repeat 8 - end repeat
                /// 16- alt     32 marker - could be stored in 3 bits (+2 reserved values)

                byte barHead = 0;

                byte barNum = bar->getSignNum();
                byte barDen = bar->getSignDenum();

                if (barNum != lastSignNum)
                {
                    barHead |= 1;
                    lastSignNum = barNum;
                }

                if (barDen != lastSignDen)
                {
                    barHead |= 2;
                    lastSignDen = barDen;
                }

                if (bar->getRepeat() & 1)
                    barHead |= 4;
                if (bar->getRepeat() & 2)
                    barHead |= 8;

                byte repeatTimes = bar->getRepeatTimes();

                byte altRepeat = bar->getAltRepeat();
                if (altRepeat)
                {
                    barHead |= 16;
                }

                std::string markerText;
                ul markerColor = 0;
                //change to bool
                bar->getGPCOMPMarker(markerText, markerColor);

                if (markerColor != 0)
                {
                    barHead |= 32;
                }

                //Now according to barHead WRITE DOWN information

                LOG(<< "Bar head "<<barHead);
                file->write(&barHead,1);

                if (barHead & 1)
                    file->write(&barNum,1);

                if (barHead & 2)
                    file->write(&barDen,1);

                if (barHead & 8)
                    file->write(&repeatTimes,1);

                if (barHead & 16)
                    file->write(&altRepeat,1);

                if (barHead & 32)
                {
                   //WRITE TEXT function;
                   //saveString(file,markerText);
                   file->write(&markerColor,4);
                }
            }


            ul barLen = bar->len();
            file->write(&barLen,1); //more then 256 notes in beat are strange

            for (ul k = 0; k < bar->len(); ++k)
            {
                //Each beat
                Beat *beat = (bar->getV(k));

                bool isPause = beat->getPause();
                //file->write(&isPause,1);

                byte dur = beat->getDuration();
                //file->write(&dur,1);

                byte dot = beat->getDotted();
                //file->write(&dot,1);

                byte det = beat->getDurationDetail();
                //file->write(&det,1);

                std::string beatText;
                beat->getGPCOMPText(beatText);

                byte beatHead = 0;


                beatHead = dur; //3bits
                beatHead += dot << 3;

                if (det)
                    beatHead += 1 << 5;


                EffectsPack effPackBeat = beat->getEffects();
                ul effPackBeatValue = effPackBeat.takeBits();

                if (effPackBeatValue)
                    beatHead += 1 << 6;

                if (isPause)
                    beatHead += 1 << 7;

                file->write(&beatHead,1);


                if (det)
                    file->write(&det,1);

                if (effPackBeatValue)
                    file->write(&effPackBeatValue,4);


                if (effPackBeat == 28)
                {
                    //Package *changePack = effPackBeat.getPack(28);

                    //if (changePack)
                    {
                        Beat::ChangesList *changes = &beat->changes;//(Beat::ChangesList*)changePack->getPointer();

                        ul amountOfChanges = changes->len();

                        file->write(&amountOfChanges,1);

                        for (ul indexChange=0; indexChange != amountOfChanges; ++indexChange)
                        {
                            byte changeType = (changes->getV(indexChange)).changeType;
                            ul changeValue = (changes->getV(indexChange)).changeValue;

                            byte changeDur = (changes->getV(indexChange)).changeCount;
                            //change count (apply effect on few beats later)

                            file->write(&changeType,1);
                            file->write(&changeValue,2); //pack it
                            file->write(&changeDur,1);
                        }
                    }

                }


                //tremolo, chord etc should be stored here

                ul beatLen = beat->len();
                file->write(&beatLen,1); //256 notes in beat are too much

                //and notes inside
                for (ul el=0; el < beat->len(); ++el)
                {
                    Note *note = beat->getV(el);

                    EffectsPack effPackNote = note->getEffects();                  

                    byte fret = note->getFret();
                    //merge
                    byte stringNum = note->getStringNumber();

                    byte packFret=0;


                    if (isDrums)
                    {
                        fret -= 35;
                        packFret = (fret);
                        byte stringSh = stringNum<<5;
                        packFret += stringSh;
                    }
                    else
                    {
                       //4bit for fret 0-31 and string num 0-15
                       packFret = (fret);
                       byte stringSh = stringNum<<5;
                       packFret += stringSh;
                    }


                    //file->write(&fret,1);
                    //file->write(&stringNum,1);
                    file->write(&packFret,1);

                    byte noteSpec = 0;

                    byte vol = note->getVolume();
                    //file->write(&vol,1);

                    byte state = note->getState();
                    byte effectsPrec = effPackNote.empty() == true ? 0 : 1;

                    noteSpec = (vol & 0xF) + ((state&7)<<4) + (effectsPrec<<7);

                    file->write(&noteSpec,1);

                    if (effectsPrec)
                    {
                        ul noteEffValue = effPackNote.takeBits();
                        file->write(&noteEffValue,4);

                        if (effPackNote == 17) //bend
                        {
                            Package *bendPack = effPackNote.getPack(17);
                            BendPoints *bend = (BendPoints*) bendPack->getPointer();
                            writeBendGMY(file,bend);
                        }
                    }
                }
                //data then effects
            }

        }
    }


    return true;
}


bool GmyFile::saveString(AFile *file, std::string &strValue)
{
    ul stringLen = strValue.size();

    file->write(&stringLen,2);

    file->write(strValue.c_str(),strValue.size());
    return true;
}


bool GmyFile::loadString(AFile *file, std::string &strValue)
{
    char bufer[2048];

    ul stringLen = 0;
    file->read(&stringLen,2);

    file->read(&bufer,stringLen);
    bufer[stringLen] = 0;

    strValue = bufer;
    return true;
}

bool GmyFile::loadFromFile(AFile *file, Tab *tab, bool skipVersion)
{
    //now reverse

    if (skipVersion==false)
    {
        char firstBytes[3]={0};
        file->read(firstBytes,2);

        if ((firstBytes[0]!='G') ||
            (firstBytes[1]!='A'))
        {
            logger << "Attempt to open not guitarmy file";
            return false;
        }
    }

    char byteZero = 0;
    file->read(&byteZero,1);
    if (byteZero != 0)
    {
        logger << "Not a base format version";
        return false;
    }

    byte effectsSet = 0; //development one
    byte tracksByteLen = 1; //1 byte for tracks
    byte barsByteLen = 2;
    byte noteByteLen = 1; //attention - all this fields yet not used directly

    file->read(&effectsSet,1);
    file->read(&tracksByteLen,1);
    file->read(&barsByteLen,1);
    file->read(&noteByteLen,1);


    ul tracksCount = 0;
    ul barsCount = 0;

    file->read(&tracksCount,1);
    file->read(&barsCount,2);
    //here could reserve values
    /*
    for (ul tI = 0; tI < tracksCount; ++tI)
    {
        for (ul bI = 0; bI < barsCount; ++bI)
        {
            //reserve
        }
    }
    */

    //General tab information

    //1 bpm
    ul bpm = 0;
    file->read(&bpm,2); //values more then 1200 could be used as parts of bpm

    tab->setBPM(bpm);

    for (ul i = 0; i < tracksCount; ++i)
    {
        //Each track
        Track *track=new Track();

        //Name
        std::string trackName="not yet";
        //for Track#1 could be used empty
        loadString(file,trackName);

        track->setName(trackName);
        //strings N
        byte stringsCount = 0; //track->tuning.getStringsAmount();
        file->read(&stringsCount,1);
        //tuning
        track->tuning.setStringsAmount(stringsCount);

        LOG(<<"Strings amount "<<stringsCount);
        for (byte sI=0; sI < stringsCount; ++sI)
        {
            byte tune = 0;
            file->read(&tune,1);
            track->tuning.setTune(sI,tune);
            LOG( << "Read tune "<<sI<<" "<<tune);
        }

        //maximum frets
        byte fretsLimit = 0; //frets
        file->read(&fretsLimit,1);
        track->setGPCOMPInts(3,fretsLimit);
        //prepare for capo
        byte capoSet = 0;
        file->read(&capoSet,1);
        track->setGPCOMPInts(4,capoSet);


        LOG( << "read fret limits "<<fretsLimit<<"; capo "<<capoSet);

        //extract from midi table -
            /// instrument

            //isDrums
        bool isDrums = false; //track->isDrums();

        file->read(&isDrums,1);
        track->setDrums(isDrums);

        //SET GET REFACT
        short int instr = 0;
        byte pan = 0;
        byte volume = 0;

        file->read(
 //&(tab->GpCompMidiChannels[midiChanPortIndex].instrument),
    &instr,
                    2);

        //cover under track functions refact: getVolume getPan GeInstrument
            /// pan
        file->read(
  //&(tab->GpCompMidiChannels[midiChanPortIndex].balance)
                    &pan,1);
            /// volume
        file->read(
 //&(tab->GpCompMidiChannels[midiChanPortIndex].volume)
                    &volume
                    ,1);

        track->setInstrument(instr);
        track->setVolume(volume);
        track->setPan(pan);


        LOG(<<" Read instr "<<instr<<" isD "<<isDrums<<"; pan "<<
              pan<<"; volume "<<volume);

        for (ul j = 0; j < barsCount; ++j)
        {
            //Each bar for that track
            //Bar *bar = &(track->getV(i));

            Bar *bar= new Bar();
            bar->flush();


            if (i == 0) //first track
            {
                //work over repeats signs etc - set them only from first track
                //On POLY tab format this would be applied to each track
                ///HEADED: 1) repeats 2) signatures 3) Marker
                /// 1 - change num 2 - change denum
                /// 4 - begin repeat 8 - end repeat
                /// 16- alt     32 marker - could be stored in 3 bits (+2 reserved values)


                byte barHead = 0;
                file->read(&barHead,1);

                LOG( << "Read bar head "<<barHead);

                byte barNum =0;
                byte barDen =0;

                byte repeatTimes = 0;
                byte altRepeat = 0;


                if (barHead & 1)
                {
                    file->read(&barNum,1);
                    bar->setSignNum(barNum);
                }

                if (barHead & 2)
                {
                    file->read(&barDen,1);
                    bar->setSignDenum(barDen);
                }

                if (barHead & 4)
                {
                    bar->setRepeat(1);
                }

                if (barHead & 8)
                {
                    file->read(&repeatTimes,1);
                    bar->setRepeat(2,repeatTimes);
                }

                if (barHead & 16)
                {
                    file->read(&altRepeat,1);
                    bar->setAltRepeat(altRepeat);
                }

                if (barHead & 32)
                {
                   std::string markerText;
                   //read TEXT function;
                   //loadString(file,markerText);
                   ul markerColor = 0;
                   file->read(&markerColor,4);
                }
            }
            else
            {
               //will be erased on time loop

               // not 0 bar
               Bar *ftBar = tab->getV(0)->getV(j); //firstTrackBar
               bar->setRepeat(ftBar->getRepeat(),ftBar->getRepeatTimes());
               bar->setSignDenum(ftBar->getSignDenum());
               bar->setSignNum(ftBar->getSignNum());
               bar->setAltRepeat(ftBar->getAltRepeat());
               //no markers here haha
            }


            ul barLen = 0;
            file->read(&barLen,1); //more then 256 notes in beat are strange

            if (barLen == 0)
            {
                logger << "Bars len";
                Beat *emptyBeat = new Beat();
                emptyBeat->setPause(true);
                emptyBeat->setDuration(3);
                bar->add(emptyBeat);
            }

            for (ul k = 0; k < barLen; ++k)
            {
                //Each beat
                //Beat *beat = &(bar->getV(k));
                Beat *beat = new Beat();

                bool isPause = false;

                //file->read(&isPause,1);

                byte dur =0;
                //file->read(&dur,1);


                byte dot = 0;
                //file->read(&dot,1);


                byte det = 0;
                //file->read(&det,1);


                byte beatHead = 0;
                file->read(&beatHead,1);

                dur = beatHead & 0x7;
                isPause = ((beatHead >> 7) & 1) > 0;
                byte gotEff = (beatHead >> 6) & 1;

                det = (beatHead >> 5) & 1;
                dot = (beatHead >> 3) & 3;

                if (det)
                {
                    file->read(&det,1);
                }

                /*
                std::string beatText;
                loadString(file,beatText);
                if (beatText.empty())
                {
                    logger <<"beatText "<< beatText.c_str()
                }
                */
                //EffectsPack effPackBeat = beat->getEffects();



                ul effPackBeatValue = 0;

                if (gotEff)
                    file->read(&effPackBeatValue,4);


                beat->setPause(isPause);
                beat->setDuration(dur);
                beat->setDotted(dot);
                beat->setDurationDetail(det);

                beat->effPack.putBits(effPackBeatValue);

                if (beat->effPack == 28)
                {
                    ul amountOfChanges = 0;

                    Beat::ChangesList *newChanges = new Beat::ChangesList();

                    file->read(&amountOfChanges,1);

                    for (ul indexChange=0; indexChange != amountOfChanges; ++indexChange)
                    {
                        byte changeType = 0;
                        ul changeValue = 0;

                        byte changeDur = 0;
                        //change count (apply effect on few beats later)

                        file->read(&changeType,1);
                        file->read(&changeValue,2); //pack it
                        file->read(&changeDur,1);

                        Beat::SingleChange change;
                        change.changeCount = changeDur;
                        change.changeType = changeType;
                        change.changeValue = changeValue;

                        newChanges->add(change);
                    }

                    beat->effPack.addPack(28,1,newChanges);
                }

                //tremolo, chord etc should be loaded here

                ul beatLen = 0;
                file->read(&beatLen,1); //256 notes are even too much

                //and notes inside
                for (ul el=0; el < beatLen; ++el)
                {
                   // Note *note = &(beat->getV(el));

                    // EffectsPack effPackNote = note->getEffects();
                    Note *note= new Note();

                    byte fret = 0;
                    //merge
                    byte stringNum = 0;

                    byte fretPack = 0;
                    //file->read(&fret,1);
                    //file->read(&stringNum,1);


                    file->read(&fretPack,1);

                    if (isDrums)
                    {
                        fret = 0x1F & fretPack;
                        fret += 35;
                        stringNum = (fretPack>>5) & 7;
                    }
                    else
                    {
                        fret = 0x1F & fretPack;
                        stringNum = (fretPack>>5) & 7;
                    }


                    note->setFret(fret);
                    note->setStringNumber(stringNum);

                    byte noteSpec = 0;

                    file->read(&noteSpec,1);

                    byte vol = noteSpec & 0xF;
                    //file->read(&vol,1);

                    byte effectState = 1&(noteSpec >> 7);
                    //file->read(&effectState,1);

                    byte state = (noteSpec >>4) &7;

                    note->setVolume(vol);
                    //note.setState(state);

                    if (effectState == 1)
                    {
                        ul noteEffValue = 0;
                        file->read(&noteEffValue,4);
                        note->effPack.putBits(noteEffValue);

                        //note.effPack.set(17,false); //turn off bend
                        if (note->effPack == 17)
                        {
                            BendPoints *bend = new BendPoints;
                            readBendGMY(file,bend);
                            note->effPack.addPack(17,2,bend);
                        }

                        note->effPack.set(19,false); //turn off tremolo
                    }

                    beat->add(note);
                }

                bar->add(beat);
                //data then effects
            }

            track->add(bar);

        }

        tab->add(track);
    }

    return true;
}
