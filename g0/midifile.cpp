#include "midifile.h"
#include "amusic.h"

#include "aconfig.h"

//used only for changes now - push somewhere else please(to effects) refact
#include "tab.h"

// ALL of the debug if (midiLog)  logging operations should be replaced with normal << >>
// if (midiLog)  logging
// this will allow still have full information, but get clean stdout
// if (midiLog)  logFile(MidiFileCpp)

#include <iostream>

//if (midiLog)  logging
#include "g0/astreaming.h"
static AStreaming  logger("midifile");

bool midiLog = false;

ul VariableInt::readStream(AFile & ifile)
{
	ul totalBytesRead = 0;

	byte lastByte = 0;
	do
	{
		ifile.read(&lastByte, 1);

		byte valueByte = lastByte & 127;
		this->add(valueByte);

        if (midiLog)  LOG( << "V[" << totalBytesRead << "] value = " << valueByte);

		if (lastByte & 128)
        {
            if (midiLog)  logger << " not last! " ;
        }
		else
            if (midiLog)  logger << " last one!" ;

		// end of debugging

		++totalBytesRead;

	}
	while (lastByte & 128);

	return totalBytesRead;
}

ul VariableInt::getValue()
{
	// collect value from poly
	ul responseValue = 0;

	size_t length = len();

	for (size_t i = 0; i < length; ++i)
	{
		responseValue <<= 7;
		responseValue += getV(i);
	}

	return responseValue;
}

ul MidiSignal::readStream(AFile & ifile)
{
	ul totalRead = 0;

	// main place for current refactoring
	// move into signal
	VariableInt varInt;
	ul varIntBytesRead = varInt.readStream(ifile);

	totalRead += varIntBytesRead;

	byte p00 = 0;				// 2 parts of 4bit params
	byte p1 = 0;				// param 1
	byte p2 = 0;				// param 2;

	ifile.read(&p00, 1);
	totalRead += 1;

	//first fill the structure
	this->byte0 = p00; //channel + event type
	this->time = varInt;

	// debuging

	if (p00 == 0xff)
	{
        if (midiLog)  logger << "This is a meta event! " ;

		byte metaType;
		ifile.read(&metaType, 1);
		int metaEvType = metaType;
		totalRead += 1;

		//or in own place?
		this->param1 = metaType;

		VariableInt metaLength;
		ul mLenBytesRead = metaLength.readStream(ifile);
		totalRead += mLenBytesRead;
		ul bytesToSkip = metaLength.getValue();

		this->metaStore.metaLen = metaLength;

        if (midiLog)  LOG( << "Meta type = " << metaEvType << " to skip - " << bytesToSkip);

		byte bBufer;

		for (ul k = 0; k < bytesToSkip; ++k)
		{
			ifile.read(&bBufer, 1);
			this->metaStore.bufer.add(bBufer);
		}
		
		totalRead += bytesToSkip;

	}
	else
	{

		int evValue = p00;
        if (midiLog)  LOG( << "Not meta event. value = " << evValue);

		ifile.read(&p1, 1);
		totalRead += 1;
		
		this->param1 = p1;

		//CLEAN THIS	MARKER	ONLY	AFTER	FILLING	FUNCTIONS
		int midiChannel = p00 & (0xf);	// (0x1 + 0x2 + 0x4 + 0x8)
        int eventType = p00 & (0xf0);	// (16 + 32 + 64 + 128);
		eventType >>= 4;
		//JUST OUTPUT	FOR	HERE, BUT ALWAYS	FOR	FUNCTIONS

		if ((eventType != 0xC) && (eventType != 0xD))
		{
			ifile.read(&p2, 1);
			totalRead += 1;
			this->param2 = p2;

		}

		int param1 = p1;
		int param2 = p2;

        if (midiLog)  LOG( << " Event# " << eventType << "; midiChan# " << midiChannel << "; p1 = " << param1 << "; p2 = " << param2);

        //if (eventType == 0) ; //just a reminder of system events

	}

	return totalRead;
}

bool MidiFile::readStream(AFile & ifile)
{
    //char headerData[14] = { 0 };

    //long long lldata;
	ifile.read(midiHeader.chunkId, 4);	// chunk header // aS Abstract String
	midiHeader.chunkId[4] = 0;

	// ifile >>
	ifile.r(&(midiHeader.chunkSize), 4);
	ifile.r(&(midiHeader.formatType), 2);
	ifile.r(&(midiHeader.nTracks), 2);
	ifile.r(&(midiHeader.timeDevision), 2);
	
	//allocate tracks:
	for (int nT=0; nT < midiHeader.nTracks; ++nT)
	{
        MidiTrack *midiTrack=new MidiTrack();
		this->add(midiTrack);
	}
	// debug output header
	// init number of trackds ???
    if (midiLog)  LOG( << "MidiHeader filled : " << midiHeader.chunkId);
    if (midiLog)  LOG( << " ch size: " << midiHeader.
        chunkSize << "; tracks n:" << midiHeader.nTracks);
    if (midiLog)  LOG( << " time devision: " << midiHeader.
        timeDevision << "; ftype:" << midiHeader.formatType);
	// some special check like size == 0;
    // end of debug, cover under the if (midiLog)  log operations

	if (midiHeader.nTracks > 0)
		for (int i = 0; i < midiHeader.nTracks; ++i)	// why was 1!!!&&
		{
			//REFACTORING-NOTICE: move to MidiTrack class
			
            // to if (midiLog)  logging
            if (midiLog)  LOG( << "Reading track " << i);

            ifile.read(getV(i)->trackHeader.chunkId, 4);
            getV(i)->trackHeader.chunkId[4] = 0;

            // to if (midiLog)  logging
            if (midiLog)  LOG( << "Track " << i << " header " << getV(i)->trackHeader.chunkId);

			ul trackSize = 0;
			ifile.r(&trackSize, 4);

            // to if (midiLog)  logging
            if (midiLog)  LOG( << "Size of track " << trackSize);

            getV(i)->trackHeader.trackSize = trackSize;
			//memcpy for header chunkie

			ul totalRead = 0;
			
			while (totalRead < trackSize)
			{
				// SIGNAL READING CUT DONE!
                MidiSignal *singleSignal=new MidiSignal(); //for those who have troubles in speach
                ul signalBytesRead = singleSignal->readStream(ifile);
				totalRead += signalBytesRead;
                getV(i)->add(singleSignal);
                if (midiLog)  LOG( << "Cycle of events. Read " << totalRead << " of " << trackSize);
			}

            if (midiLog)  LOG( <<"Track reading finished. "<<totalRead<<" from "<<trackSize);

		}//cycle  for of tracks

    return true;
}

//Here appear stream printing operaions
// PRINT BEGIN
void MidiSignal::printToStream(std::ostream &stream)
{
	if (byte0 == 0xff)
	{
		int metaType = param1;
		ul metaLen = metaStore.metaLen.getValue();
		stream <<"MetaType "<<metaType<<"; MetaLen " <<metaLen;
        for (ul i = 0; i < metaLen; ++i)
        {
            stream << std::endl<< "Meta byte "<<i<<" = "<<(int) metaStore.bufer.getV(i);
        }
	}
	else
	{
		int midiChannel = byte0 & (0xf);
        int eventType = byte0 & (0xf0);
		eventType >>= 4;
        stream <<"EventType "<<eventType<<"("<<(int)byte0<<"); channel "<<midiChannel<<"; abs="<<absValue;

		int p1 = param1; //its shame to output this way, must correct this
		int p2 = param2;
	
		stream << "; p1 " << p1;
	
		if ((eventType != 0xC) && (eventType != 0xD))
			stream << "; p2 " << p2;		
	}
	
	ul time = this->time.getValue();
	stream <<"; t " <<time<<std::endl;
}


//ANOTHER VERSION FOR ASTREAMING
void MidiTrack::printToStream(std::ostream &stream)
{
	stream << "Output MidiTrack.";
    //stream << "chunky = " << trackHeader.chunkId <<std::endl;
    stream << "Track Size = " << trackHeader.trackSize << std::endl;
	
	ul signalsAmount = len();
	for (ul i = 0; i < signalsAmount; ++i)
        getV(i)->printToStream(stream);
}

void MidiFile::printToStream(std::ostream &stream)
{
	stream << "Output MidiFile.";
	stream << "chunky = " << midiHeader.chunkId <<std::endl;
	stream << "Chunk Size = " << midiHeader.chunkSize;
	stream << "; Format type = " << midiHeader.formatType <<std::endl;
	stream << "Tracks amount = " << midiHeader.nTracks;
	stream << "; TimeD = " << midiHeader.timeDevision << std::endl;
	
	//then all the tracks
	for (short i = 0 ; i < midiHeader.nTracks; ++i)
            getV(i)->printToStream(stream);
	
	stream << "Printing finished for MidiFile" << std::endl;
}
//PRINT END

//WRITE STREAMS

ul VariableInt::writeStream(AFile &file)
{
	size_t amountOfBytes = len();
	
	byte byteToWrite = 0;

	
	for (size_t i = 0; i < amountOfBytes; ++i)
	{
		byteToWrite = getV(i);

    //    if (midiLog)  logger <<"PRE WrOtE"<<byteToWrite;
		
        if (i != (amountOfBytes - 1))
				byteToWrite |= 128;	
		
		file.write(&byteToWrite,1);

      // if (midiLog)  logger <<"V WROTE "<<byteToWrite;
    }
						
	return amountOfBytes;	
}

ul MidiSignal::writeStream(AFile &ofile,bool skip)
{
    ul bytesWritten = 0;

    if (skip)
    {
        if (skipThat())
            return 0;
    }
	
    bytesWritten += time.writeStream(ofile); //CHECK 0
	ofile.write(&byte0,1);
    //if (midiLog)  logger <<"WROTE "<<byte0;

    //int zFuck = 00; //check???? OUPS
    //ofile.write(&zFuck,1);
    ofile.write(&param1,1);
    //if (midiLog)  logger <<"WROTE "<<param1;

	bytesWritten += 2;
	//ATTENTION finish bytes written

	if (isMetaEvent())
    {
        //if (midiLog)  logger << "META EVENT WROTEN!";

		//ofile.write(&metaStore.metaType,1);
		bytesWritten += metaStore.metaLen.writeStream(ofile);
		
		size_t metaBufLen = metaStore.bufer.len();
		for (size_t i = 0; i < metaBufLen; ++i)
			ofile.write(&metaStore.bufer.getV(i),1);
		
		bytesWritten += metaBufLen;
	}
	else
    { //normal event
        byte eventType = getEventType();
        if ((eventType != 0xC) && (eventType != 0xD))
		{
              ofile.write(&param2,1);
              //if (midiLog)  logger <<"WROTE "<<param2;
              bytesWritten += 1;
        }
	}
									
	return bytesWritten;	
}

ul MidiFile::writeStream(AFile &ofile)
{
	ul bytesWritten = 0;

    //attention penetration

    calculateHeader(); //also fills header of tracks
		
	//write header
	ofile.write(midiHeader.chunkId,4);
	ofile.w(&midiHeader.chunkSize,4);
	ofile.w(&midiHeader.formatType,2);
	ofile.w(&midiHeader.nTracks,2);
	ofile.w(&midiHeader.timeDevision,2);
		
	bytesWritten += 14;
		
	for (short int i = 0; i < midiHeader.nTracks; ++i)
    {

        ofile.write(getV(i)->trackHeader.chunkId,4);
        ofile.w(&getV(i)->trackHeader.trackSize,4);
		
		bytesWritten += 8;
		
        size_t amountOfEvents = getV(i)->len();
		
		for (size_t j = 0; j < amountOfEvents; ++j)
		{
            bytesWritten += getV(i)->getV(j)->writeStream(ofile);
		}
	}
		
	return bytesWritten;		
}


ul MidiFile::noMetricsTest(AFile &ofile)
{
    ul bytesWritten = 0;

    int tracks = 1;

    //write header
    ofile.write(midiHeader.chunkId,4);
    ofile.w(&midiHeader.chunkSize,4);
    ofile.w(&midiHeader.formatType,2);
    ofile.w(&tracks,2);
    ofile.w(&midiHeader.timeDevision,2);

    bytesWritten += 14;

    calculateHeader(true);//for tracks mostly

    //don't skip first track ??
    for (short int i = 1; i < 2; ++i) //shit condition
    {
        ofile.write(getV(i)->trackHeader.chunkId,4);
        ofile.w(&getV(i)->trackHeader.trackSize,4);

        bytesWritten += 8;

        size_t amountOfEvents = getV(i)->len();

        for (size_t j = 0; j < amountOfEvents; ++j)
        {
            bytesWritten += getV(i)->getV(j)->writeStream(ofile,true);
        }
    }

    return bytesWritten;
}

//WRITE END

//RECOUNTERS BEGIN

bool MidiFile::calculateHeader(bool skip)
{
     //this function used to be called each time we try to write midi file
     //its responsible for headers fill

     //but as a very first option - most values peace - just set amount of tracks
     ul calculatedTracks = this->len();
     //NOTE this will work only with previusly loaded file
     if (midiLog)  LOG( << "Calculating headers "<<calculatedTracks<<"-tracks.");
     midiHeader.nTracks = calculatedTracks; //NOW PUSED

     //and here we fill header
     midiHeader.chunkSize = 6;
     midiHeader.formatType = 1;
     midiHeader.timeDevision = 480; //LOT OF ATTENTION HERE WAS 480
            //and then  bpm*4;
     memcpy(midiHeader.chunkId,"MThd",5);


     for (ul i = 0; i < calculatedTracks; ++i)
     {
        getV(i)->calculateHeader(skip);
     }

     return true;
}

bool MidiTrack::calculateHeader(bool skip)
{
    //this function responsable for calculations of data stored inside MidiTrack

    ul calculatedSize = 0;

    for (ul i =0; i < len(); ++i)
    {
        //seams to be easiest option
        calculatedSize += getV(i)->calcSize(skip);
    }

    if (midiLog)  LOG( <<"Calculating track size : "<<calculatedSize);
    if (midiLog)  LOG( <<"Previously stored : "<<trackHeader.trackSize);

    trackHeader.trackSize=calculatedSize;//NOW PUSED

    //and header defaults
    memcpy(trackHeader.chunkId,"MTrk",5); //some attention here if normal wouldnt go

    return true;
}

ul MidiSignal::calcSize(bool skip)
{
    ul localSize = 0;

    if (skip)
    {
        if (skipThat())
            return 0;
    }

    localSize += time.getLen(); //delta time value
    localSize += 1; //byte0 always here

    if (byte0 != 0xff) //not meta event(normal one)
    {
        //DEPENDS ON BYTE0 type stored
        localSize += 1;
        int eventType = byte0; //get FROM BYTE0 !!
        eventType &= 0xf0;
        eventType >>= 4;

        if ((eventType != 0xC) && (eventType != 0xD))
            localSize += 1;

    }
    else
    {
        //Meta event
        localSize += 1; // meta type
        localSize += metaStore.metaLen.getLen(); //unhide len?
        localSize += metaStore.bufer.len();
    }



    return localSize;
}

//RECOUNTERS END

///GENERATOR begin

bool MidiSignal::skipThat()
{
    byte evT = getEventType();


    if (byte0 == 0xff)
    {
        //GET meta type

        //3 or 47???  SKIP NAME attempt
        if (param1 == 47)
        return false; //stay for meta event

        if (param1 == 81) //change tempo
           return false;
            //255 (0xFF) 	81 (0x51) 	3
    }

    //if (evT == 12) return false; //and skip value key (instrument type)

    if (evT == 8) return false; //note on
    if (evT == 9) return false; //note off



    //if (byte0 == 176) return true; //fine for smalles
    //if (byte0 == 177) return true; //fine for smalles

    return true;
}

MidiSignal::MidiSignal(byte b0, byte b1, byte b2, ul timeShift):byte0(b0),param1(b1),param2(b2)
{
    absValue=0;
    if (timeShift == 0)
    {
        byte noShift = 0;
        time.add(noShift);
    }
    else
    {
        //attention - cover ander variableInt class
        short int first = timeShift / 128;
        short int second =  timeShift % 128;
        short int third = 0;
        //HERE ATTENTION
        if(first > 127)
        {
            first = (timeShift / 128) / 128;
            second = (timeShift / 128) % 128;
            third = timeShift % 128;

            byte firstB = first;
            byte secondB = second;
            byte thirdB = third;

            time.add(firstB);
            time.add(secondB);
            time.add(thirdB);
        }
        else
        {
            byte firstB = first;
            byte secondB = second;

            time.add(firstB);
            time.add(secondB);
        }
    }
}

bool MidiFile::generateFromAMusic(AMusic &music)
{
    setBPM(music.getBPM());

    ul tracksAmount = music.lenR();

    ul melodyIndex = 0;

    for (ul trackIndex = 0; trackIndex != tracksAmount; ++trackIndex)
    //single track midi!
    {
        MidiTrack *newMidiTrack=new MidiTrack(); //yet a single track

        ul notesCount = music.gR(trackIndex).len();

        ul pauseAccumulator = 0;
        ul rythmAccumulator = 0;

        ul pauseCounter = 0;
        byte velocyShift = 0;
        byte lastVelocy = 95; //default one

        bool drumsTrack = false;

        byte normalChannel = 0;
        byte effectChannel = 1;
        //attention isn't inside subfunctions,
        //but drums shouldn live there
        //yet shifted
        //not just to add another value into functions call

        //track selection
        if (tracksAmount <= 4)
        {
            //we don't have too much tracks and can sepparate them all
            byte channelsShift = trackIndex*2; //temp way
            normalChannel += channelsShift;
            effectChannel += channelsShift;
        }
        else
        {
            if (trackIndex <=16)
            {
                byte channelsShift = trackIndex; //temp way

                if ((trackIndex==9)||(trackIndex==10))
                    channelsShift += 5;

                normalChannel = effectChannel = channelsShift;

                /*
                normalChannel += channelsShift;
                effectChannel += channelsShift;
                */

                //if ((trackIndex==9)||(trackIndex==10))
            }
            else
            {
                /*
                byte channelsShift = (trackIndex+1)*2; //temp way
                normalChannel += channelsShift;
                effectChannel += channelsShift;
                */
            }
            if (midiLog)  logger <<"Too many tracks!";
        }

        unsigned long instrument = music.gM(trackIndex).instrument;

        if (instrument == 666)
        {
            drumsTrack = true;
            instrument = 25;
        }

        if (drumsTrack)
            normalChannel = effectChannel = 9;
// ------------------------------------------------------
        /////SET UP MIDI CHANNELS VALUES -refact awaits


        {                                       //FIRST SET
            newMidiTrack->pushChangeBPM(bpm);

            newMidiTrack->pushChangeInstrument(instrument,normalChannel);
            newMidiTrack->pushChangeInstrument(instrument,effectChannel);

            byte midiPanoram = newMidiTrack->calcMidiPanoramGP(music.gM(trackIndex).panoram);
            newMidiTrack->pushChangePanoram(midiPanoram,normalChannel); //ch 0 also add effect please

            byte midiVolume = newMidiTrack->calcMidiVolumeGP(music.gM(trackIndex).volume);
            newMidiTrack->pushChangeVolume(midiVolume,normalChannel);
        }

        melodyIndex = 0;

        for (ul i = 0; i < notesCount; ++i)
        {

            ul rhythmValue = music.gR(trackIndex).getV(i);
            if (midiLog)  LOG( << "Rhythm value is "<<rhythmValue);

            byte baseRValue = rhythmValue & 7;

            bool lastNoteInSeq = false;
            bool nextIsPause = false;

            if ((i+1) == notesCount)
                lastNoteInSeq = true;
            else
            {
               ul rhythmValueNext = music.gR(trackIndex).getV(i+1);
               if ((rhythmValueNext & 128) > 0)
                   nextIsPause = true;
               //empty taks 3 5 after let ring
            }

            //MAIN rhythm value
            ul rOffset = 0;
            byte special = (rhythmValue>>15) & 1;

            short int rhyBase = 120; //constant for midi values

            if (special)
            {
                ul specialValue = (rhythmValue>>16) & 0xffff;

                if (midiLog)
                LOG( << "MIDI_SPECIAL value "<<specialValue );

                short int power2 = 2<<(3);
                ul preRValue = rhyBase*power2/4;

                preRValue *= specialValue;
                preRValue /= 1000;

                rOffset = preRValue;

                if (midiLog)
                LOG( <<"GOT R OFFSET "<<rOffset );
            }
            else
            {
                byte rhythmDetail = rhythmValue & 0x78; //4 bits after first 3
                rhythmDetail>>=3;

                byte dotAppear = (rhythmValue>>8) & 3;

                //2 is 4th 3 is 8th 4 is 16th etc
                byte powOfTwo = 6 - baseRValue;
                short int power2 = 2<<(powOfTwo-1);//-1 because 2 is 1 pow itself
                rOffset = rhyBase*power2/4;


                if (dotAppear==1) //dotted
                {
                    if (midiLog)  logger << "Dot did appear";
                    rOffset *= 3;
                    rOffset /= 2;
                    //BETTER TO BE COVERED UNDER CAL RHY DETAIL
                }

                if (rhythmDetail)
                    rOffset = newMidiTrack->calcRhythmDetail(rhythmDetail,rOffset);
            }



            bool isPause = false;

            if ((rhythmValue & 128) > 0)
                isPause = true;

            if (isPause)
            {
                pauseAccumulator += rOffset;
                if (midiLog)  LOG( << "Pause in AMUSIC to MIDI "<<pauseAccumulator );
                ++pauseCounter;
            }
            else
            {
                //Melody handling
                //ul melodyIndex = i-pauseCounter;
                Melody::mnote noteValue = music.gM(trackIndex).getMNote(melodyIndex); //i

                ul roughMelodyValue = music.gM(trackIndex).getV(melodyIndex);
                ul hasHarmony = (roughMelodyValue>>24) & 1;
                byte noteState = (roughMelodyValue>>8) & 7;
                byte effectsOnNote = (roughMelodyValue>>16)&0xff;
                byte noteStateHarm = 0;
                byte velocyValue = (roughMelodyValue>>11)&15;

                byte midiVelocy = velocyValue*15;
                if (midiLog)  LOG( << "Velocity value "<<velocyValue <<" midi "<<midiVelocy);
                if (velocyShift)
                {
                    velocyShift = 0;
                    midiVelocy = newMidiTrack->calcLeggatoVelocy(midiVelocy);
                    if (midiLog)  LOG( <<"SHIFT velocy "<<midiVelocy);
                }
                if (midiVelocy == 0)
                    midiVelocy = lastVelocy;

                if (midiLog)  LOG( <<"AM note state = "<<noteState);
                if (midiLog)  LOG( <<"Effects are: "<<effectsOnNote);
                if (midiLog)  LOG( <<"Has harmony "<<hasHarmony);

                EffectsPack effects;

                if (effectsOnNote == 1)
                {
                    effects = music.gM(trackIndex).mapOfEffects.getEffect(melodyIndex);

                    effects.set(29,false); //tap
                    effects.set(30,false); //slap
                    effects.set(31,false); //pop
                }
                else
                {
                    if (midiLog)  logger << "No effects";
                }

                //HANDLING CHANGES
                if (effectsOnNote==0)
                    effects = music.gM(trackIndex).mapOfEffects.getEffect(melodyIndex);

                Package *changePack = effects.getPack(28);

                if (changePack)
                {
                    Beat::ChangesList *changes = (Beat::ChangesList*)changePack->getPointer();

                    for (ul indexChange = 0; indexChange != changes->len(); ++indexChange)
                    {
                      if (changes->getV(indexChange).changeType==8)
                      {
                          ul newBPM = changes->getV(indexChange).changeValue;

                          if (midiLog)  LOG( << "New BPM "<<newBPM);

                          pauseAccumulator += rythmAccumulator; ///ATTENTION!!!
                          rythmAccumulator = 0; //should fix this

                          newMidiTrack->pushChangeBPM(newBPM,pauseAccumulator); //PARAMZ

                          pauseAccumulator = 0;
                      }

                      if (changes->getV(indexChange).changeType==1)
                      {
                          ul newInstr = changes->getV(indexChange).changeValue;

                          if (midiLog)  LOG( << "New instr "<<newInstr);

                          pauseAccumulator += rythmAccumulator; ///ATTENTION!!!
                          rythmAccumulator = 0;

                         newMidiTrack->pushChangeInstrument(newInstr,normalChannel,pauseAccumulator); //PARAMZ (ch 1?) refact to channel

                         pauseAccumulator = 0;
                      }

                      if (changes->getV(indexChange).changeType==2)
                      {
                          ul newVol = changes->getV(indexChange).changeValue;

                          if (midiLog)  LOG( << "New volume "<<newVol);

                          byte midiNewVolume = newVol*8;
                          if (midiNewVolume > 127)
                              midiNewVolume = 127;

                          newMidiTrack->pushChangeVolume(midiNewVolume,normalChannel); //PARAMZ (ch 1 2?)
                      }

                      if (changes->getV(indexChange).changeType==3)
                      {
                            ul newPan = changes->getV(indexChange).changeValue;

                            if (midiLog)  LOG( << "New panoram "<<newPan);

                            byte midiNewPanoram = newPan*8;
                            if (midiNewPanoram > 127)
                                midiNewPanoram = 127;

                            newMidiTrack->pushChangePanoram(midiNewPanoram,normalChannel); //PARAMZ (ch 1 2?)
                      }
                    }
                }


                effects.set(28,false); //turn off changes for now


                if (midiLog)  LOG( <<"Velocuty "<<velocyValue);

                Poly<ul> harmonyVertical;

                byte harmonyNote = 255; //later vector or Poly!!
                //struct from hNote, noteState
                ul strokeStep = 0; //updown stroke

                while (hasHarmony)
                {
                    //here should be collected data about other notes
                    //but first we just skip harmonis
                    ++melodyIndex;
                    roughMelodyValue = music.gM(trackIndex).getV(melodyIndex);
                    hasHarmony = (roughMelodyValue>>24) & 1;

                    noteStateHarm = (roughMelodyValue>>8) & 7; //too will go to vector..

                    Melody::mnote noteHarmValue = music.gM(trackIndex).getMNote(melodyIndex);
                    harmonyNote = noteHarmValue.octave*12 + noteHarmValue.key;

                    harmonyVertical.add(melodyIndex);
                }

                ++melodyIndex;


                if (noteState == Melody::continingNote)
                {
                     rythmAccumulator += rOffset;
                     rOffset = 0;
                     continue;
                }


                byte midiNote = noteValue.octave*12 + noteValue.key;


                bool alreadyFinished = false;

                //MAIN NOTE POSITION ON
                if ((noteState != Melody::endingNote))
                {
                    if (noteState != Melody::deadNote)
                    {

                        if (effects == 2)
                        { //palm mute
                            byte palmMuteVel = newMidiTrack->calcPalmMuteVelocy(midiVelocy);
                            midiVelocy = palmMuteVel;
                            //MidiSignal mSignalOn(0x90  | normalChannel,midiNote,palmMuteVel,pauseAccumulator);
                            //newMidiTrack->add(mSignalOn);
                            //pauseAccumulator = 0; //flush accumulated
                            effects.set(2,false);
                        }

                        //NORMAL NOTE
                        if (effects.empty())
                        {
                            //LOT ATTENTION HERE
                            pauseAccumulator += rythmAccumulator; ///ATTENTION!!!
                            rythmAccumulator = 0; //should fix this
                            //FUCKEN SHIT

                            MidiSignal *mSignalOn=new MidiSignal(0x90 | normalChannel,midiNote,midiVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalOn);
                            pauseAccumulator = 0; //flush accumulated
                        }


                        if  (effects.inRange(3,10)) //((effects >= 3) && (effects <= 10))
                        { //slides
                            if ( (effects == 4) || (effects == 5))
                            {
                                   if (effects==5) velocyShift=19; //set decreace legatto slide

                                    MidiSignal *mSignalOn=new MidiSignal(0x90 | effectChannel,midiNote,midiVelocy,pauseAccumulator);
                                    newMidiTrack->add(mSignalOn);
                                    pauseAccumulator = 0;
                                    short int slideStep = rOffset/8;
                                    //real function depends on next note...
                                    newMidiTrack->pushSlideUp(effectChannel,2,slideStep);//channel + shift
                                    rOffset = 0;
                            }
                            else if ((effects == 8) || (effects == 6))
                            {
                                /*/slide /. + // slide . \
                                //not really same but generates somth */

                                MidiSignal *mSignalOn=new MidiSignal(0x90 | effectChannel,midiNote,midiVelocy,pauseAccumulator);
                                newMidiTrack->add(mSignalOn);
                                pauseAccumulator = 0;
                                short int slideStep = rOffset/8;

                                newMidiTrack->pushSlideDown(effectChannel,7,slideStep);//channel + shift
                                rOffset = 0;
                            }
                            else if ((effects == 9)|| (effects == 7))
                            {
                                MidiSignal *mSignalOn=new MidiSignal(0x90 | effectChannel,midiNote,midiVelocy,pauseAccumulator);
                                newMidiTrack->add(mSignalOn);
                                pauseAccumulator = 0;
                                short int slideStep = rOffset/8;

                                newMidiTrack->pushSlideUp(effectChannel,7,slideStep);//channel + shift
                                rOffset = 0;

                            }
                            else if (effects == 10)
                            {   //legato - as normal one should decreace sound of next note
                                MidiSignal *mSignalOn=new MidiSignal(0x90 | effectChannel,midiNote,midiVelocy,pauseAccumulator);
                                newMidiTrack->add(mSignalOn);
                                pauseAccumulator = 0; //flush accumulated
                                velocyShift=19; //set decreace
                            }
                        }//slides
                        else if (effects == 17) //bend default pattern
                        {
                            MidiSignal *mSignalOn=new MidiSignal(0x90 | effectChannel,midiNote,midiVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalOn);
                            pauseAccumulator = 0;

                            //effects = music.gM(trackIndex).mapOfEffects.getEffect(melodyIndex);
                            Package *bendPack = effects.getPack(17);
                            BendPoints *bend = (BendPoints*) bendPack->getPointer();

                            if (midiLog)
                            LOG( <<(int)bend<< "Bend h "<<"; len "<<(int)bend->len()<<"; type"<<bend->getType());

                            newMidiTrack->pushBend(rOffset,bend,effectChannel);
                            rOffset = 0; //attention for chords
                        }
                        else if (effects == 19)
                        { //TREMOLO default pattern
                            MidiSignal *mSignalOn=new MidiSignal(0x90 | effectChannel,midiNote,midiVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalOn);
                            pauseAccumulator = 0;

                            newMidiTrack->pushTremolo(rOffset);
                            rOffset = 0; //attention for chords
                            //is closed well???
                        }
                        else if (effects == 20)
                        { //fade in
                            MidiSignal *mSignalOn=new MidiSignal(0x90 | normalChannel,midiNote,midiVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalOn);
                            pauseAccumulator = 0;

                            //fade in as not effect
                            newMidiTrack->pushFadeIn(rOffset, normalChannel);
                            //ATTENTION - must return it back later
                            rOffset = 0;

                            effectsOnNote = 0; // to finish well
                        }
                        else if (effects == 21)
                        {
                            //ghost note
                            byte ghostVelocy = midiVelocy - 10;
                            MidiSignal *mSignalOn=new MidiSignal(0x90 | normalChannel,midiNote,ghostVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalOn);
                            pauseAccumulator = 0;

                            effectsOnNote = 0; // to finish well
                        }
                        else if (effects == 22)
                        {
                            //GRACE NOTE
                            //dead code of value
                            MidiSignal *mSignalGraceOn=new MidiSignal(0x90  | normalChannel,midiNote+2,midiVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalGraceOn);
                            pauseAccumulator = 0;

                            short int graceLen = (rOffset/8);
                            MidiSignal *mSignalGraceOff=new MidiSignal(0x80  | normalChannel,midiNote+2,80,graceLen-1);
                            newMidiTrack->add(mSignalGraceOff);
                            MidiSignal *mSignalOn=new MidiSignal(0x90  | normalChannel,midiNote,midiVelocy,1);
                            newMidiTrack->add(mSignalOn);
                            rOffset -= graceLen;

                            effectsOnNote = 0; // to finish well

                        }
                        else if (effects == 23)
                        {
                            //staccato
                            //LOT ATTENTION HERE
                            pauseAccumulator += rythmAccumulator; ///ATTENTION!!!
                            rythmAccumulator = 0; //should fix this
                            //FUCKEN SHIT

                            MidiSignal *mSignalOn=new MidiSignal(0x90  | normalChannel,midiNote,midiVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalOn);
                            pauseAccumulator = 0; //flush accumulated

                            pauseAccumulator = rOffset/2;
                            rOffset-=rOffset/2;

                            effectsOnNote = 0;
                        }
                        else if (effects == 24)
                        {
                            //tremolo pick - trills
                            //LOT ATTENTION HERE
                            pauseAccumulator += rythmAccumulator; ///ATTENTION!!!
                            rythmAccumulator = 0; //should fix this
                            //FUCKEN SHIT

                            MidiSignal *mSignalOn=new MidiSignal(0x90  | normalChannel,midiNote,midiVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalOn);
                            pauseAccumulator = 0; //flush accumulated

                            short int tremoloStep = rOffset/4;

                            for (int i = 0; i < 3; ++i)
                            {
                                MidiSignal *mSignalOff=new MidiSignal(0x80 | normalChannel,midiNote,midiVelocy,tremoloStep);
                                newMidiTrack->add(mSignalOff);
                                MidiSignal *mSignalOn=new MidiSignal(0x90 | normalChannel,midiNote,midiVelocy,0);
                                newMidiTrack->add(mSignalOn);
                            }

                            rOffset = tremoloStep;

                            effectsOnNote = 0;

                        }
                        else if ((effects == 25) || (effects == 26))
                        {
                            //upstroke + down stroke effects

                            //LOT ATTENTION HERE
                            pauseAccumulator += rythmAccumulator; ///ATTENTION!!!
                            rythmAccumulator = 0; //should fix this
                            //FUCKEN SHIT

                            strokeStep = rOffset/12;

                            if (effects == 25)
                            {
                                pauseAccumulator += strokeStep;
                                rOffset -= strokeStep;
                                MidiSignal *mSignalOn=new MidiSignal(0x90 | normalChannel,midiNote,midiVelocy,pauseAccumulator);
                                newMidiTrack->add(mSignalOn);
                                pauseAccumulator = 0; //flush accumulated
                            }

                            //FIRST UPDOWN STROKE WORK AS UPSTROKE
                            //rOffset-=rOffset/4;
                        }
                        else if (effects == 27)
                        {
                            //LOT ATTENTION HERE
                            pauseAccumulator += rythmAccumulator; ///ATTENTION!!!
                            rythmAccumulator = 0; //should fix this
                            //FUCKEN SHIT

                            byte accentVelocy = midiVelocy + midiVelocy/10;//cali)
                            if (accentVelocy > 127) accentVelocy = 127;

                            MidiSignal *mSignalOn=new MidiSignal(0x90 | normalChannel,midiNote,accentVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalOn);
                            pauseAccumulator = 0; //flush accumulated
                            effectsOnNote = 0; //other cases
                        }

                        if (effects == 1)
                        {   //vibratto

                            MidiSignal *mSignalOn=new MidiSignal(0x90 | effectChannel,midiNote,midiVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalOn);
                            pauseAccumulator = 0;

                            short int vibroStep = rOffset/6;
                            newMidiTrack->pushVibration(effectChannel,3,vibroStep); //channel + depth

                            rOffset = 0;
                        }

                        bool needAfterAdd = false;

                        if (effects == 18)
                                needAfterAdd = true;

                        if (effects.inRange(11,16))
                            needAfterAdd = true;

                        if (effects.inRange(28,32))
                            needAfterAdd = true;


                        if (needAfterAdd) //and many other cases
                        { //ANY OTHER CASE !!!

                            //LOT ATTENTION HERE
                            pauseAccumulator += rythmAccumulator; ///ATTENTION!!!
                            rythmAccumulator = 0; //should fix this
                            //FUCKEN SHIT

                            MidiSignal *mSignalOn=new MidiSignal(0x90 | normalChannel,midiNote,midiVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalOn);
                            pauseAccumulator = 0; //flush accumulated
                            effectsOnNote = 0; //other cases
                        }


                    } //not DEAD
                    else
                    { //X NOTE - DEAD NOTE by conditions

                       MidiSignal *mSignalOn=new MidiSignal(0x90 | normalChannel,midiNote,midiVelocy,pauseAccumulator);
                       newMidiTrack->add(mSignalOn);
                       pauseAccumulator = rOffset-20;
                       rOffset = 20;//let of ghost note
                      //ATTENTION COULD BE BUGS IN ACCORDS WITH GHOSTS - check
                    }
                } //not ending note
                else
                {
                    //ENDING NOTE YEARH
                    if (midiLog)  logger << "We run on ending note";

                    //rOffset += rythmAccumulator;
                    //rythmAccumulator = 0;

                    //MidiSignal *mSignalOff=new MidiSignal(0x80 | normalChannel,midiNote,80,rOffset);
                    //newMidiTrack->add(mSignalOff);
                    //rOffset = 0;
                    //alreadyFinished = true;



                }


                //ON Cycle
                for (ul hInd = 0; hInd < harmonyVertical.len(); ++hInd)
                {
                    ul hIndReal = hInd;

                    if (effects==26)
                        hIndReal = harmonyVertical.len()-1-hInd;//upside down

                    ul mHarmonyIndex = harmonyVertical.getV(hIndReal);
                    ul roughHarmony = music.gM(trackIndex).getV(mHarmonyIndex);
                    byte harmStat = (roughHarmony>>8) & 7; //too will go to vector..
                    Melody::mnote harmValNote = music.gM(trackIndex).getMNote(mHarmonyIndex);
                    byte harmVal = harmValNote.octave*12 + harmValNote.key;

                    byte HvelocyValue = (roughHarmony>>11)&15;
                    byte HmidiVelocy = HvelocyValue*15; //velocy HERE !!!
                    if (HmidiVelocy == 0) HmidiVelocy = 95;//default

                    noteStateHarm = harmStat; //to old
                    harmonyNote = harmVal;

                    if ((effects == 25)||(effects == 26))
                    {
                        pauseAccumulator += strokeStep;
                        rOffset -= strokeStep;
                    }

                    if ((noteStateHarm != Melody::endingNote))
                    {

                        {
                            MidiSignal *mSignalHarmOn=new MidiSignal(0x90 | normalChannel,harmonyNote,HmidiVelocy,pauseAccumulator);
                            newMidiTrack->add(mSignalHarmOn);
                            pauseAccumulator = 0; //flush accumulated
                        }
                    }
                }

                if ((effects == 25) ||  (effects == 26))
                {
                    if (effects == 26)
                    {
                        //pushFirstNote!
                        pauseAccumulator += strokeStep;
                        rOffset -= strokeStep;
                        MidiSignal *mSignalOn=new MidiSignal(0x90 | normalChannel,midiNote,midiVelocy,pauseAccumulator);
                        newMidiTrack->add(mSignalOn);
                        pauseAccumulator = 0; //flush accumulated
                    }

                    //rOffset-=rOffset/4;

                    effectsOnNote =  0;
                }



                //MAIN NOTE POSITION OFF
                //if (alreadyFinished==false)
                {

                    if ((noteState != Melody::startingNote))
                    {

                        rOffset += rythmAccumulator;
                        if (midiLog)  LOG( << "rOffset in the End "<<rOffset);
                        rythmAccumulator = 0;





                        if (effects.empty() || (effectsOnNote == 0))
                        {
                            MidiSignal *mSignalOff=new MidiSignal(0x80 | normalChannel,midiNote,80,rOffset);
                            newMidiTrack->add(mSignalOff);
                            rOffset = 0;

                            if (noteState == Melody::endingNote) //fix bad one - attention refact
                            {
                                MidiSignal *mSignalOff=new MidiSignal(0x80 | effectChannel,midiNote,80,rOffset);
                                newMidiTrack->add(mSignalOff);
                                rOffset = 0;
                            }

                        }
                        else
                        {
                            if (noteState == Melody::endingNote) //fix
                            {
                                MidiSignal *mSignalOff=new MidiSignal(0x80 | normalChannel,midiNote,80,rOffset);
                                newMidiTrack->add(mSignalOff);
                                rOffset = 0;
                            }
                        }

                        if (effects == 1)
                        {
                            MidiSignal *mSignalOff=new MidiSignal(0x80 | effectChannel,midiNote,80,rOffset);
                            newMidiTrack->add(mSignalOff);
                            rOffset = 0;
                        }
                        if (effects == 2)
                        { //palm mute
                            short int shiftTime = rOffset/4;
                            pauseAccumulator += shiftTime;
                            rOffset -= shiftTime;
                            MidiSignal *mSignalOff=new MidiSignal(0x80 | normalChannel,midiNote,80,rOffset);
                            newMidiTrack->add(mSignalOff);
                            rOffset = 0;

                        }
                        if  (effects.inRange(3,10)) //if ((effects >= 3)&&(effects <=10))
                        {
                            //slides +
                            MidiSignal *mSignalOff=new MidiSignal(0x80 | effectChannel,midiNote,80,rOffset);
                            newMidiTrack->add(mSignalOff);

                            rOffset = 0;
                        }
                        if  (effects==17) //if ((effects >10) && (effects <=17)) //17 is bend
                        {
                            //slide + bend
                            MidiSignal *mSignalOff=new MidiSignal(0x80 | effectChannel,midiNote,80,rOffset);
                            newMidiTrack->add(mSignalOff);
                            rOffset = 0;

                        } //error herer

                    }
                    else
                    {   //double code refact


                        if (lastNoteInSeq)
                        {
                            rOffset += rythmAccumulator;
                            if (midiLog)  LOG( << "rOffset in the End "<<rOffset);
                            rythmAccumulator = 0;
                            //if (effects.empty()||(effects == 18))
                            {
                                MidiSignal *mSignalOff=new MidiSignal(0x80 | normalChannel,midiNote,80,rOffset);
                                newMidiTrack->add(mSignalOff);
                            }
                            rOffset=0;
                        }
                    }

                }

                //OFF cycle
                for (ul hInd = 0; hInd < harmonyVertical.len(); ++hInd)
                {
                    ul mHarmonyIndex = harmonyVertical.getV(hInd);
                    ul roughHarmony = music.gM(trackIndex).getV(mHarmonyIndex);
                    byte harmStat = (roughHarmony>>8) & 7; //too will go to vector..
                    Melody::mnote harmValNote = music.gM(trackIndex).getMNote(mHarmonyIndex);
                    byte harmVal = harmValNote.octave*12 + harmValNote.key;

                    //to old
                    noteStateHarm = harmStat;
                    harmonyNote = harmVal;

                    if (midiLog)  LOG( << "Harmony index "<<hInd<<" harmonyState "<<harmonyNote);

                    if ((noteStateHarm != Melody::startingNote))
                    {
                       if (harmonyNote != midiNote) //let ring fix
                       {
                           rOffset += rythmAccumulator;
                           rythmAccumulator = 0;
                           MidiSignal *mSignalHarmOff=new MidiSignal(0x80 | normalChannel,harmonyNote,80,rOffset);
                           newMidiTrack->add(mSignalHarmOff);

                           //std::cout<<std::endl;
                           //mSignalHarmOff.printToStream(std::cout);
                           rOffset=0;
                       }
                    }
                    else
                    { //double code refact
                        if (lastNoteInSeq)
                            //next is pause?
                        {
                            rOffset += rythmAccumulator;
                            rythmAccumulator = 0;
                            MidiSignal *mSignalHarmOff=new MidiSignal(0x80 | normalChannel,harmonyNote,80,rOffset);
                            newMidiTrack->add(mSignalHarmOff);
                            rOffset=0;
                        }
                    }
                }

                rythmAccumulator += rOffset;
                rOffset = 0;

            }



        }

        newMidiTrack->pushEvent47();
        add(newMidiTrack);

    }

    return true;
}

///GENERATOR END

//HELPERS BEGIN

void MidiTrack::pushChangeInstrument(byte newInstr, byte channel, ul timeShift)
{
    if (midiLog)  LOG( << "Change instrument "<<newInstr<<" on CH "<<channel);

    MidiSignal *instrumentChange=new MidiSignal(0xC0|channel,newInstr,0,timeShift);

    this->add(instrumentChange);
}

void MidiTrack::pushMetrSignature(byte num, byte den,ul timeShift=0, byte metr=24, byte perQuat=8)
{
    MidiSignal *signatureEvent = new MidiSignal(0xff,88,0,timeShift);

    signatureEvent->metaStore.bufer.add(num);

    byte transDur=0;
    switch (den)
    {
        case 1: transDur = 0; break;
        case 2: transDur = 1; break;
        case 4: transDur = 2; break;
        case 8: transDur = 3; break;
        case 16: transDur = 4; break;
        case 32: transDur = 5; break;
        case 64: transDur = 6; break;
    default:
        transDur=6;
    }

    signatureEvent->metaStore.bufer.add(transDur);
    signatureEvent->metaStore.bufer.add(metr);
    signatureEvent->metaStore.bufer.add(perQuat);

    byte metaSize = 4;
    signatureEvent->metaStore.metaLen.add(metaSize);

    add(signatureEvent);
}

void MidiTrack::pushChangeBPM(int bpm, ul timeShift)
{
    if (midiLog)  LOG( << "We change midi temp to "<<bpm); //attention

    MidiSignal *changeTempEvent=new MidiSignal(0xff,81,0,timeShift);

    //changeTempEvent.byte0 = 0xff;
    //changeTempEvent.param1 = 81;

    ul MCount = 60000000/bpm;

    byte tempB1 = (MCount>>16)&0xff; //0x7
    byte tempB2 = (MCount>>8)&0xff; //0xa1
    byte tempB3 = MCount&0xff; //0x20

    changeTempEvent->metaStore.bufer.add(tempB1);
    changeTempEvent->metaStore.bufer.add(tempB2);
    changeTempEvent->metaStore.bufer.add(tempB3);

    byte lenMeta = 3;
    changeTempEvent->metaStore.metaLen.add(lenMeta);

    //byte timeZero = 0;
    //changeTempEvent.param2 = 0;
    //changeTempEvent.time.

    this->add(changeTempEvent);
}
void MidiTrack::pushChangeVolume(byte newVolume, byte channel)
{
    MidiSignal *volumeChange=new MidiSignal;

    if (newVolume > 127) newVolume = 127;

    volumeChange->byte0 = 0xB0 | channel;
    volumeChange->param1 = 7; //volume change
    volumeChange->param2 = newVolume;
    byte timeZero = 0;
    volumeChange->time.add(timeZero);

    this->add(volumeChange);
}
void MidiTrack::pushChangePanoram(byte newPanoram, byte channel)
{
    MidiSignal *panoramChange=new MidiSignal;

    panoramChange->byte0 = 0xB0 | channel;
    panoramChange->param1 = 0xA; //change panoram
    panoramChange->param2 = newPanoram;
    byte timeZero = 0;
    panoramChange->time.add(timeZero);

    this->add(panoramChange);
}
void MidiTrack::pushVibration(byte channel, byte depth, short int step, byte stepsCount)
{
    byte shiftDown = 64-depth;
    byte shiftUp = 64+depth;
    byte signalKey = 0xE0 + channel;

    for (ul vibroInd=0; vibroInd <stepsCount; ++vibroInd)
    {
         MidiSignal *mSignalVibOn=new MidiSignal(signalKey,0,shiftDown,step);
         MidiSignal *mSignalVibOff=new MidiSignal(signalKey,0,shiftUp,step);
         this->add(mSignalVibOn);
         this->add(mSignalVibOff);
    }

    MidiSignal *mSignalVibOn=new MidiSignal(signalKey,0,64,0);
    this->add(mSignalVibOn);
}
void MidiTrack::pushSlideUp(byte channel, byte shift, short int step, byte stepsCount)
{
    byte pitchShift = 64;
    byte signalKey = 0xE0 + channel;

    for (ul slideInd=0; slideInd <stepsCount; ++slideInd)
    {
         MidiSignal *mSignalSlideOn=new MidiSignal(signalKey,0,pitchShift,step);
         this->add(mSignalSlideOn);
         pitchShift+=shift;
    }
    MidiSignal *mSignalSlideOff=new MidiSignal(signalKey,0,64,0);
    this->add(mSignalSlideOff);
}

void MidiTrack::pushSlideDown(byte channel, byte shift, short int step, byte stepsCount)
{
    byte pitchShift = 64;
    byte signalKey = 0xE0 + channel;

    for (ul slideInd=0; slideInd <stepsCount; ++slideInd)
    {
         MidiSignal *mSignalSlideOn=new MidiSignal(signalKey,0,pitchShift,step);
         this->add(mSignalSlideOn);
         pitchShift-=shift;
    }
    MidiSignal *mSignalSlideOff=new MidiSignal(signalKey,0,64,0);
    this->add(mSignalSlideOff);
}

void MidiTrack::pushBend(short rOffset, void *bendP, byte channel)
{
    BendPoints *bend = (BendPoints*)bendP;

    ul lastAbs = 0;
    ul lastH = 0;

    short rAccum = 0;

    if (midiLog)
    LOG( << "Bend rOffset="<<rOffset);

    //something did changed after moving to more own format of bends

    for (ul i = 0 ; i < bend->len(); ++i)
    {
        ul curAbs = bend->getV(i).horizontal;
        ul curH = bend->getV(i).vertical;

        ul shiftAbs = curAbs - lastAbs;
        ul shiftH = curH - lastH;

        if (midiLog)
        LOG( << "AbsShift="<<shiftAbs<<"; HShift="<<shiftH);

        if (shiftH != 0)
        {
            byte lastShift = 64+(lastH*32)/4; //decreased from 100 to 4
            byte curShift = 64+(curH*32)/4; //next

            if (midiLog)
            LOG( <<"lastShiftPitch="<<lastShift<<"; curShiftPitch="<<curShift);

            double rhyStep = (shiftAbs*rOffset)/600.0; //10 steps
            double pitchStep = (curShift-lastShift)/10.0;

            if (midiLog)
            LOG( <<"rStep="<<rhyStep<<"; rAccum="<<rAccum<<"; pSh="<<pitchStep);

            MidiSignal *mSignalBendOpen=new MidiSignal(0xE0 |channel,0,lastShift, rAccum);
            this->add(mSignalBendOpen);
            rAccum = 0;

            double stepsDone = 0.0;
            double shiftDone = 0.0;

            for (int i = 0; i < 10; ++i)
            {
                double thisStep = rhyStep*(i+1) - stepsDone;
                stepsDone += thisStep;

                double thisShift = pitchStep*(i+1) - shiftDone;
                shiftDone += thisShift;

                short thisROffset = thisStep;
                short thisMidiShift = thisShift;

                short totalShiftNow = lastShift + shiftDone;

                MidiSignal *mSignalBend=new MidiSignal(0xE0 | channel,0,totalShiftNow,thisROffset);
                this->add(mSignalBend);

                if (midiLog)
                LOG( <<"rD="<<thisStep<<"; sD="<<thisShift<<" shiftDone="<<shiftDone);
                if (midiLog)
                LOG( <<"rOff="<<thisROffset<<"; midiSh="<<thisMidiShift);

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
    byte lastShift = 64+(lastH*32)/4; //decreased from 100
    MidiSignal *mSignalBendLast=new MidiSignal(0xE0 | channel,0,lastShift, rAccum);
    this->add(mSignalBendLast);
    MidiSignal *mSignalBendClose=new MidiSignal(0xE0 | channel,0,64,0);
    this->add(mSignalBendClose);

    if (midiLog)
    LOG( << "done");
}

void MidiTrack::pushTremolo(short int rOffset)
{
    short int slideStep = rOffset/40; //10 steps of 1/4

    byte pitchShift = 64;
    for (ul slideInd=0; slideInd <10; ++slideInd)
    {
         MidiSignal *mSignalBend=new MidiSignal(0xE1,0,pitchShift,slideStep);
         this->add(mSignalBend);
         pitchShift-=3;//calibrate
    }

    rOffset -= rOffset/4;
    //last point
    MidiSignal *mSignalBendLast=new MidiSignal(0xE1,0,pitchShift, rOffset);
    this->add(mSignalBendLast);
    MidiSignal *mSignalBendClose=new MidiSignal(0xE1,0,64,0);
    this->add(mSignalBendClose);
}

void MidiTrack::pushFadeIn(short int rOffset, byte channel)
{

    byte newVolume = 27;
    short int fadeInStep = rOffset/20;

    MidiSignal *volumeChangeFirst=new MidiSignal(0xB0 | channel,7,newVolume,0);
    this->add(volumeChangeFirst);

    for (int i = 0; i < 20; ++i)
    {
        newVolume += 5;
        MidiSignal *volumeChange=new MidiSignal(0xB0 | channel,7,newVolume,fadeInStep);
        this->add(volumeChange);

    }
}

void MidiTrack::pushEvent47()
{

#ifdef WIN32
/* terrible fix :)
    MidiSignal *trickA = new MidiSignal(0x90 , 64, 3 ,240);
    MidiSignal *trickB = new MidiSignal(0x80 , 64, 3 ,240);
//THERE IS SMALL FIX FOR WINDOWS PLAY MACHINE
    add(trickA);
    add(trickB);*/
#endif


    MidiSignal *event47=new MidiSignal(0xff,47,0,0);

    byte lenZero = 0;
    event47->metaStore.metaLen.add(lenZero);

    this->add(event47);
}

//CALC helpers
short int MidiTrack::calcRhythmDetail(byte RDValue, short int rhythmOffset)
{
    short int rOffset = rhythmOffset;
    if (RDValue == 3) //truplet
    {
        rOffset *= 2;
        rOffset /= 3;
    }
    if (RDValue == 5) //five-plet
    {
        rOffset *= 4;
        rOffset /= 5;
    }
    if (RDValue == 6) //five-plet
    {
        rOffset *= 5;
        rOffset /= 6;
    }
    if (RDValue == 7) //-plet
    {
        rOffset *= 4;
        rOffset /= 7;
    }
    if (RDValue == 9) //-plet
    {
        rOffset *= 8;
        rOffset /= 9;
    }

    //FEW MISSING

    //MAYBE NPLET posibility??
    //attention for other connections here
    //miss single double and tripple dot here
    return rOffset;
}

byte MidiTrack::calcMidiPanoramGP(byte pan)
{
    if (midiLog)  LOG( << "Panoram value = "<<pan);
    byte midiPanoram = pan*8;
    if (midiPanoram>=128) midiPanoram=128;
    return midiPanoram;
}
byte MidiTrack::calcMidiVolumeGP(byte vol)
{
    if (midiLog)  LOG( <<"Volume is "<<vol);
    byte midiVolume = vol*8;
    if (midiVolume >= 128) midiVolume=128;
    return midiVolume;
}
byte MidiTrack::calcPalmMuteVelocy(byte vel)
{
    byte outputVelocy = vel;
    byte decreaceVelocy = vel/5;
    outputVelocy -= decreaceVelocy;
    return outputVelocy;
}
byte MidiTrack::calcLeggatoVelocy(byte vel)
{
    return calcPalmMuteVelocy(vel);//for possible difference
}
//HELPERS END

/*
void MidiTrack::add(MidiSignal &val)
{
    Poly::add(val);
  //  std::cout<<std::endl;
  //  val.printToStream(std::cout);
   // if (midiLog)  logger <<"lo";
}
*/

/////////NEW WAY OF GENERATION FOR NOW BUT MOST SIMPLE EVER\\\\\\\
//(((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))


bool MidiTrack::addSignalsFromNoteOn(Note *note, byte channel)
{
    byte noteState = note->getState();

    if ((noteState==Note::leegNote) ||
            (noteState==Note::leegedLeeg))
    {
            //this are leegs
        return false;
    }


    byte fret = note->getFret();
    byte stringN = note->getStringNumber();
    byte midiNote = fret + tunes[stringN-1];

    byte volume = note->getVolume();
    byte midiVelocy = volume*15; //calcMidiVolumeGP(volume);

    byte lastVelocy = 95; //not real last one yet

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

bool MidiTrack::addPostEffects(Beat *beat, byte channel)
{
    //POST-effects

    for (ul i =0; i < beat->len(); ++i)
    {
        Note *note = beat->getV(i);

        byte fret = note->getFret();
        byte stringN = note->getStringNumber();
        byte midiNote = fret + tunes[stringN-1];

        byte volume = note->getVolume();
        byte midiVelocy = 95; //calcMidiVolumeGP(volume);


        byte noteState = note->getState();

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

            MidiSignal *mSignalGraceOff=new MidiSignal(0x80  | channel,midiNote+2,80,graceLen-1);
            add(mSignalGraceOff);
            MidiSignal *mSignalOn=new MidiSignal(0x90  | channel,midiNote,midiVelocy,1);
            add(mSignalOn);
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

            for (int i = 0; i < 3; ++i)
            {
                MidiSignal *mSignalOff=new MidiSignal(0x80 | channel,midiNote,midiVelocy,tremoloStep);
                add(mSignalOff);
                MidiSignal *mSignalOn=new MidiSignal(0x90 | channel,midiNote,midiVelocy,0);
                add(mSignalOn);
            }

            accum = tremoloStep;
            //takeAccum();
        }

        if (note->effPack.get(23))
        {
            //stokato - stop earlier
            short halfAccum = accum/2;

            MidiSignal *mSignalOff=new MidiSignal(0x80 | channel,midiNote,midiVelocy,halfAccum);
            add(mSignalOff);

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

bool MidiTrack::addSignalsFromNoteOff(Note *note, byte channel)
{
    if (note->effPack.get(18)) //let ring
        //skip let ring
        return false;


    byte noteState = note->getState();

    if ((noteState == Note::leegedLeeg)||
            (noteState == Note::leegedNormal)) //refact note stat
        //skip - next is leeg
        return false;


    if (note->effPack.get(23))
        return false; //skip stokkato





    byte fret = note->getFret();
    byte stringN = note->getStringNumber();
    byte midiNote = fret + tunes[stringN-1];

    byte volume = note->getVolume();
    byte midiVelocy = 80; //calcMidiVolumeGP(volume);


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

bool MidiTrack::addSignalsFromBeat(Beat *beat, byte channel, short specialR)
{
    byte dur,det,dot; //rhythm value

    dur = beat->getDuration();
    det = beat->getDurationDetail();
    dot = beat->getDotted();

    const int baseAmount = 120;

    int rOffset = 0;
    if (specialR == 0)
    {
      byte powOfTwo = 6 - dur;
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

            for (ul indexChange = 0; indexChange != changes->len(); ++indexChange)
            {
              if (changes->getV(indexChange).changeType==8)
              {
                  ul newBPM = changes->getV(indexChange).changeValue;
                  //pushChangeBPM(newBPM,accum);
                  //takeAccum();

                  //skipped according to time line
              }

              if (changes->getV(indexChange).changeType==1)
              {
                 ul newInstr = changes->getV(indexChange).changeValue;
                 pushChangeInstrument(newInstr,channel,accum);
                 takeAccum();
              }

              if (changes->getV(indexChange).changeType==2)
              {
                  byte newVol = changes->getV(indexChange).changeValue;

                  byte midiNewVolume = newVol*8;
                  if (midiNewVolume > 127)
                      midiNewVolume = 127;

                  pushChangeVolume(midiNewVolume,channel); //must take accum
              }

              if (changes->getV(indexChange).changeType==3)
              {
                    byte newPan = changes->getV(indexChange).changeValue;

                    byte midiNewPanoram = newPan*8;
                    if (midiNewPanoram > 127)
                        midiNewPanoram = 127;

                    pushChangePanoram(midiNewPanoram,channel); //must take accum
              }
            }
        }
    }


    short int strokeStep  = rOffset/12;

    ul beatLen = beat->len();
    for (ul i =0; i < beatLen; ++i)
    {
        //reverse indexation
        ul trueIndex = i;

        if (beat->effPack.get(26)) //down
            trueIndex = (beatLen - i - 1);

        if (beat->effPack.inRange(25,26)) //up down strokes
        {
            accumulate(strokeStep);
            rOffset -= strokeStep;
        }

        Note *note = beat->getV(trueIndex);


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

    for (ul i =0; i < beat->len(); ++i)
    {
        Note *note = beat->getV(i);
        addSignalsFromNoteOff(note,channel);
    }
}

bool MidiTrack::fromTrack(Track *track, byte channel, ul shiftCursorBar)
{
 //FIRST SET instruments pan volume

    clock_t afterT = getTime();

 ul instrument = track->getInstrument();

 /*
 bool isDrums = track->isDrums();
 if (isDrums)
 {  //or any else
     instrument = 25;
 }
 */

 byte midiPan = calcMidiPanoramGP(track->getPan());
 byte midiVol = calcMidiVolumeGP(track->getVolume());

 //byte channel = 0;
 pushChangeInstrument(instrument,channel);
 pushChangePanoram(midiPan,channel);
 pushChangeVolume(midiVol,channel);

 byte theTunes[11];
 for (int i = 0; i < 10; ++i)
     if (track->isDrums())
         theTunes[i] = 0;
     else
         theTunes[i] = track->tuning.getTune(i);

 setTunes(theTunes);


 ul trackLen = track->timeLoop.len();

 for (ul i = shiftCursorBar ; i < trackLen; ++i)
 {
     Bar *bar = track->timeLoop.getV(i);

     //BAR STATUS
     byte completeStatus = bar->getCompleteStatus();

     ul barLen = bar->len();
     short specialLast = 0;
     ul completeIndex = 0;

     if (completeStatus == 2)
     {
        //barLen = bar->getCompleteIndex();
         completeIndex = bar->getCompleteIndex();
        specialLast = bar->getCompleteAbs();
     }

     //Signature

     for (ul j = 0; j < barLen; ++j)
     {
         //if (specialLast)
        Beat *beat = bar->getV(j);

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

 //logger <<"Generate track "<<diffT;

}

//new fun
void MidiTrack::closeLetRings(byte channel)
{
    for (int i = 0; i < 10; ++i)
    {
        if (ringRay[i] != 255)
        {
            closeLetRing(i,channel);
        }
    }
}

void MidiTrack::closeLetRing(byte stringN, byte channel)
{
    if (stringN > 8)
    {
        LOG( <<"String issue "<<stringN);
        return;
    }

    byte ringNote = ringRay[stringN];
    ringRay[stringN]=255;

    byte ringVelocy=80;

    if (ringNote != 255)
    pushNoteOff(ringNote,ringVelocy,channel);

}

void MidiTrack::openLetRing(byte stringN, byte midiNote, byte velocity, byte channel)
{
    if (stringN > 8)
    {
        LOG( <<"String issue "<<stringN);
        return;
    }

    if (ringRay[stringN]!=255)
    {
        closeLetRing(stringN,channel);
    }
    ringRay[stringN]=midiNote;

    pushNoteOn(midiNote,velocity,channel);
}

void MidiTrack::finishIncomplete(short specialR)
{
    //constant refact
     short int rhyBase = 120;

    short int power2 = 2<<(3);
    int preRValue = rhyBase*power2/4;

    preRValue *= specialR;
    preRValue /= 1000;

    accumulate(preRValue);
}

void MidiTrack::pushNoteOn(byte midiNote, byte velocity, byte channel)
{
    MidiSignal *noteOn=new MidiSignal(0x90 | channel, midiNote, velocity,accum);
    takeAccum();
    add(noteOn);
}

void MidiTrack::pushNoteOff(byte midiNote, byte velocity, byte channel)
{
    MidiSignal *noteOn=new MidiSignal(0x80 | channel, midiNote, velocity,accum);
    takeAccum();
    add(noteOn);
}

bool MidiTrack::checkForLeegFails()
{
}

void MidiTrack::startLeeg(byte stringN, byte channel)
{
}

void MidiTrack::stopLeeg(byte stringN, byte channel)
{
}

bool MidiFile::fromTab(Tab *tab, ul shiftTheCursor)
{
    clock_t afterT = getTime();
    //time line track

    {

        MidiTrack *timeLineTrack=new MidiTrack();



        tab->createTimeLine(shiftTheCursor);

        std::cout << tab->timeLine.size() << " is size of timeLine" <<std::endl;

        ul tlAccum = 0;

        for (int i = 0; i < tab->timeLine.size(); ++i)
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
        add(timeLineTrack);

        //Please note it was debug
        //timeLineTrack->printToStream(std::cout);

    }

    if (CONF_PARAM("metronome")=="1")
    {
     //metronome track

        int metronomeClickSize = 0;
        bool metronomeTurnedOn = true;


        if (metronomeTurnedOn)
        {
            MidiTrack *metronomeClickTrack=new MidiTrack();
            bool firstRun = true;

            for (int barI=shiftTheCursor; barI< tab->getV(0)->timeLoop.len(); ++barI)
            {



                    //for bars
                    Bar *bar = tab->getV(0)->timeLoop.getV(barI);

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
                            MidiSignal *noteOn=new MidiSignal(0x90 | 9, 33, 127,0);
                            metronomeClickTrack->add(noteOn);
                            firstRun = false;
                        }
                        else
                        {
                            MidiSignal *noteOn=new MidiSignal(0x90 | 9, 33, 127,metronomeClickSize);
                            metronomeClickTrack->add(noteOn);
                        }
                    }

                    }
                    //tlAccum=0;

            metronomeClickTrack->pushEvent47();
            add(metronomeClickTrack);
            }

    }

    ul tabLen = tab->len();

    int drumsTrack=0;

    ul startCursorBar = shiftTheCursor;

    //Get aware of solo tracks
    std::vector<ul> indecesToSkip;
    bool soloTurnedOn = false;

    for (ul trackIndex = 0; trackIndex < tabLen; ++trackIndex)
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

    clock_t after2T = getTime();
    int diffT = after2T - afterT;
    LOG( <<"File pre-generation "<<diffT);

    //Main generation

    for (ul i=0; i < tabLen; ++i)
    {
        //logger << "0 Tab is "<<(int)tab;
        //logger <<"pushed "<<tabLen;

        Track *track = tab->getV(i);

        byte trackStatus = track->getStatus();

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


        MidiTrack *mTrack=new MidiTrack();
        mTrack->takeAccum();
        //if (i==0)
        //mTrack->pushChangeBPM(tab->getBPM());


        if (track->isDrums())
        {
            ++drumsTrack;
            ul realInd = 9; //8 + drumsTrack; //9 and 10
            //if (realInd > 10)
                //realInd = 10;

            mTrack->fromTrack(track,realInd,startCursorBar);
        }
        else
            mTrack->fromTrack(track,i,startCursorBar); //1 chan per track

        //logger << "1 Tab is "<<(int)tab;
        //logger <<"pushed";

        clock_t afterT3 = getTime();

        add(mTrack);
        clock_t afterT4 = getTime();

        int addDiff = afterT4-afterT3;
        //logger << "Difference on add to poly "<<addDiff;

        //logger << "2 Tab is "<<(int)tab;
        //logger <<"pushed";
    }
}
