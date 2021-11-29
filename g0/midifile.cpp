#include "midifile.h"


#include <iostream>
#include <fstream>

#include <QDebug>

//TODO logs turned off yet


bool midiLog = false;

void reverseEndian(void *p,int s) { //TODO remove
    char *bytes = (char*)p;
    if (s == 4){
        char b = bytes[0];
        bytes[0] = bytes[3];
        bytes[3] = b;
        b = bytes[1];
        bytes[1] = bytes[2];
        bytes[2] = b;
    }
    if (s == 2) {
        char b = bytes[0];
        bytes[0] = bytes[1];
        bytes[1] = b;
    }
}

void writeReversedEndian(const char* in, size_t len, std::ofstream& file) {
    char buf[4];
    if (len == 2) {
        buf[0] = in[1];
        buf[1] = in[0];
    }
    else if (len == 4) {
        buf[0] = in[3];
        buf[1] = in[2];
        buf[2] = in[1];
        buf[3] = in[0];
    }
    file.write(buf, len);
}



bool MidiFile::readStream(std::ifstream & ifile)
{
    ifile.read((char*)midiHeader.chunkId, 4);
    ifile.read((char*)&(midiHeader.chunkSize), 4);
    reverseEndian(&(midiHeader.chunkSize), 4);
    ifile.read((char*)&(midiHeader.formatType), 2);
    reverseEndian(&(midiHeader.formatType), 2);
    ifile.read((char*)&(midiHeader.nTracks), 2);
    reverseEndian(&(midiHeader.nTracks), 2);
    ifile.read((char*)&(midiHeader.timeDevision), 2);
    reverseEndian(&(midiHeader.timeDevision), 2);
	
	//allocate tracks:
	for (int nT=0; nT < midiHeader.nTracks; ++nT)
	{
        auto midiTrack = std::make_unique<MidiTrack>();
        this->push_back(std::move(midiTrack));
	}
	// debug output header
	// init number of trackds ???
    if (midiLog)  qDebug() << "MidiHeader filled : " << midiHeader.chunkId;
    if (midiLog)  qDebug() << " ch size: " << midiHeader.
        chunkSize << "; tracks n:" << midiHeader.nTracks;
    if (midiLog)  qDebug() << " time devision: " << midiHeader.
        timeDevision << "; ftype:" << midiHeader.formatType;
	// some special check like size == 0;
    // end of debug, cover under the if (midiLog)  log operations

	if (midiHeader.nTracks > 0)
		for (int i = 0; i < midiHeader.nTracks; ++i)	// why was 1!!!&&
		{
			//REFACTORING-NOTICE: move to MidiTrack class
			
            // to if (midiLog)  logging
            if (midiLog)  qDebug() << "Reading track " << i;

            ifile.read((char*)at(i)->trackHeader.chunkId, 4);
            at(i)->trackHeader.chunkId[4] = 0;

            // to if (midiLog)  logging
            if (midiLog)  qDebug() << "Track " << i << " header " << at(i)->trackHeader.chunkId;

            size_t trackSize = 0;
            ifile.read((char*)&trackSize, 4);
            reverseEndian(&trackSize, 4);

            // to if (midiLog)  logging
            if (midiLog)  qDebug() << "Size of track " << trackSize;

            at(i)->trackHeader.trackSize = trackSize;
			//memcpy for header chunkie

            size_t totalRead = 0;
			
			while (totalRead < trackSize)
			{
				// SIGNAL READING CUT DONE!
                auto singleSignal = std::make_unique<MidiSignal>(); //for those who have troubles in speach
                size_t signalBytesRead = singleSignal->readStream(ifile);
				totalRead += signalBytesRead;
                at(i)->push_back(std::move(singleSignal));
                if (midiLog)  qDebug() << "Cycle of events. Read " << totalRead << " of " << trackSize;
			}

            if (midiLog)  qDebug() <<"Track reading finished. "<<totalRead<<" from "<<trackSize;

		}//cycle  for of tracks

    return true;
}


void MidiTrack::printToStream(std::ostream &stream)
{
	stream << "Output MidiTrack.";
    //stream << "chunky = " << trackHeader.chunkId <<std::endl;
    stream << "Track Size = " << trackHeader.trackSize << std::endl;
	
    //size_t signalsAmount = size();
    //for (size_t i = 0; i < signalsAmount; ++i)
        //at(i)->printToStream(stream); //message printing diabled
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
            at(i)->printToStream(stream);
	
	stream << "Printing finished for MidiFile" << std::endl;
}
//PRINT END

//WRITE STREAMS



size_t MidiFile::writeStream(std::ofstream &ofile)
{
    size_t bytesWritten = 0;

    //attention penetration

    calculateHeader(); //also fills header of tracks
		

    ofile.write((const char*)midiHeader.chunkId,4);
    writeReversedEndian((const char*)&midiHeader.chunkSize, 4, ofile);
    writeReversedEndian((const char*)&midiHeader.formatType,2, ofile);
    writeReversedEndian((const char*)&midiHeader.nTracks,2, ofile);
    writeReversedEndian((const char*)&midiHeader.timeDevision,2, ofile);


		
	bytesWritten += 14;
		
	for (short int i = 0; i < midiHeader.nTracks; ++i)
    {

        ofile.write((const char*)at(i)->trackHeader.chunkId,4);
        writeReversedEndian((const char*)&at(i)->trackHeader.trackSize, 4, ofile);
        //reverseEndian(&getV(i)->trackHeader.trackSize, 4);
		
		bytesWritten += 8;
		
        size_t amountOfEvents = at(i)->size();
		
		for (size_t j = 0; j < amountOfEvents; ++j)
		{
            bytesWritten += at(i)->at(j)->writeStream(ofile);
		}
	}
		
	return bytesWritten;		
}




size_t MidiFile::noMetricsTest(std::ofstream &ofile)
{
    size_t bytesWritten = 0;

    int tracks = 1;

    //write header
    ofile.write((const char*)midiHeader.chunkId,4);
    writeReversedEndian((const char*)&midiHeader.chunkSize, 4, ofile);
    writeReversedEndian((const char*)&midiHeader.formatType,2, ofile);
    writeReversedEndian((const char*)&tracks,2, ofile);
    writeReversedEndian((const char*)&midiHeader.timeDevision,2, ofile);

    bytesWritten += 14;

    calculateHeader(true);//for tracks mostly

    //don't skip first track ??
    for (short int i = 1; i < 2; ++i) //shit condition
    {
        ofile.write((const char*)at(i)->trackHeader.chunkId,4);
        writeReversedEndian((const char*)&at(i)->trackHeader.trackSize, 2, ofile);

        bytesWritten += 8;

        size_t amountOfEvents = at(i)->size();

        for (size_t j = 0; j < amountOfEvents; ++j)
        {
            bytesWritten += at(i)->at(j)->writeStream(ofile,true);
        }
    }

    return bytesWritten;
}

//WRITE END

//RECOUNTERS BEGIN

bool MidiFile::calculateHeader(bool skip)
{
     size_t calculatedTracks = this->size();
     //NOTE this will work only with previusly loaded file
     if (midiLog)  qDebug() << "Calculating headers "<<calculatedTracks<<"-tracks.";
     midiHeader.nTracks = calculatedTracks; //NOW PUSED

     //and here we fill header
     midiHeader.chunkSize = 6;
     midiHeader.formatType = 1;
     midiHeader.timeDevision = 480; //LOT OF ATTENTION HERE WAS 480
            //and then  bpm*4;
     memcpy(midiHeader.chunkId,"MThd",5);


     for (size_t i = 0; i < calculatedTracks; ++i)
     {
        at(i)->calculateHeader(skip);
     }

     return true;
}

bool MidiTrack::calculateHeader(bool skip)
{
    //this function responsable for calculations of data stored inside MidiTrack

    size_t calculatedSize = 0;

    for (size_t i =0; i < size(); ++i)
    {
        //seams to be easiest option
        calculatedSize += at(i)->calculateSize(skip);
    }

    if (midiLog)  qDebug() <<"Calculating track size : "<<calculatedSize;
    if (midiLog)  qDebug() <<"Previously stored : "<<trackHeader.trackSize;

    trackHeader.trackSize=calculatedSize;//NOW PUSED

    //and header defaults
    memcpy(trackHeader.chunkId,"MTrk",5); //some attention here if normal wouldnt go

    return true;
}

///GENERATOR begin


//HELPERS BEGIN

void MidiTrack::pushChangeInstrument(std::uint8_t newInstr, std::uint8_t channel, size_t timeShift)
{
    if (midiLog)  qDebug() << "Change instrument "<<newInstr<<" on CH "<<channel;
    auto instrumentChange = std::make_unique<MidiSignal>(0xC0|channel,newInstr,0,timeShift);
    this->push_back(std::move(instrumentChange));
}

void MidiTrack::pushMetrSignature(std::uint8_t num, std::uint8_t den,size_t timeShift=0, std::uint8_t metr, std::uint8_t perQuat)
{
    auto signatureEvent  = std::make_unique<MidiSignal>(0xff,88,0,timeShift);

    signatureEvent->metaBufer.push_back(num);

    std::uint8_t transDur=0;
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

    signatureEvent->metaBufer.push_back(transDur);
    signatureEvent->metaBufer.push_back(metr);
    signatureEvent->metaBufer.push_back(perQuat);

    std::uint8_t metaSize = 4;
    signatureEvent->metaLen.push_back(metaSize);

    push_back(std::move(signatureEvent));
}

void MidiTrack::pushChangeBPM(int bpm, size_t timeShift)
{
    if (midiLog)  qDebug() << "We change midi temp to "<<bpm; //attention

    auto changeTempEvent = std::make_unique<MidiSignal>(0xff,81,0,timeShift);

    //changeTempEvent.byte0 = 0xff;
    //changeTempEvent.param1 = 81;

    size_t MCount = 60000000/bpm;

    std::uint8_t tempB1 = (MCount>>16)&0xff; //0x7
    std::uint8_t tempB2 = (MCount>>8)&0xff; //0xa1
    std::uint8_t tempB3 = MCount&0xff; //0x20

    changeTempEvent->metaBufer.push_back(tempB1);
    changeTempEvent->metaBufer.push_back(tempB2);
    changeTempEvent->metaBufer.push_back(tempB3);

    std::uint8_t lenMeta = 3;
    changeTempEvent->metaLen.push_back(lenMeta);

    //byte timeZero = 0;
    //changeTempEvent.param2 = 0;
    //changeTempEvent.time.
    this->push_back(std::move(changeTempEvent));
}


void MidiTrack::pushChangeVolume(std::uint8_t newVolume, std::uint8_t channel)
{
    auto volumeChange = std::make_unique<MidiSignal>();
    if (newVolume > 127)
         newVolume = 127;

    volumeChange->byte0 = 0xB0 | channel;
    volumeChange->param1 = 7; //volume change
    volumeChange->param2 = newVolume;
    std::uint8_t timeZero = 0;
    volumeChange->timeStamp.push_back(timeZero);

    push_back(std::move(volumeChange));
}

void MidiTrack::pushChangePanoram(std::uint8_t newPanoram, std::uint8_t channel)
{
    auto panoramChange = std::make_unique<MidiSignal>();
    panoramChange->byte0 = 0xB0 | channel;
    panoramChange->param1 = 0xA; //change panoram
    panoramChange->param2 = newPanoram;
    std::uint8_t timeZero = 0;
    panoramChange->timeStamp.push_back(timeZero);

    this->push_back(std::move(panoramChange));
}

void MidiTrack::pushVibration(std::uint8_t channel, std::uint8_t depth, short int step, std::uint8_t stepsCount) {
    std::uint8_t shiftDown = 64-depth;
    std::uint8_t shiftUp = 64+depth;
    std::uint8_t signalKey = 0xE0 + channel;

    for (size_t vibroInd=0; vibroInd <stepsCount; ++vibroInd) {
         auto mSignalVibOn = std::make_unique<MidiSignal>(signalKey,0,shiftDown,step);
         auto mSignalVibOff = std::make_unique<MidiSignal>(signalKey,0,shiftUp,step);
         push_back(std::move(mSignalVibOn));
         push_back(std::move(mSignalVibOff));
    }

    auto mSignalVibOn = std::make_unique<MidiSignal>(signalKey,0,64,0);
    this->push_back(std::move(mSignalVibOn));
}

void MidiTrack::pushSlideUp(std::uint8_t channel, std::uint8_t shift, short int step, std::uint8_t stepsCount)
{
    std::uint8_t pitchShift = 64;
    std::uint8_t signalKey = 0xE0 + channel;

    for (size_t slideInd=0; slideInd <stepsCount; ++slideInd)
    {
         auto mSignalSlideOn = std::make_unique<MidiSignal>(signalKey,0,pitchShift,step);
         this->push_back(std::move(mSignalSlideOn));
         pitchShift+=shift;
    }
    auto mSignalSlideOff = std::make_unique<MidiSignal>(signalKey,0,64,0);
    this->push_back(std::move(mSignalSlideOff));
}

void MidiTrack::pushSlideDown(std::uint8_t channel, std::uint8_t shift, short int step, std::uint8_t stepsCount)
{
    std::uint8_t pitchShift = 64;
    std::uint8_t signalKey = 0xE0 + channel;

    for (size_t slideInd=0; slideInd <stepsCount; ++slideInd)
    {
         auto mSignalSlideOn = std::make_unique<MidiSignal>(signalKey,0,pitchShift,step);
         this->push_back(std::move(mSignalSlideOn));
         pitchShift-=shift;
    }
    auto mSignalSlideOff = std::make_unique<MidiSignal>(signalKey,0,64,0);
    this->push_back(std::move(mSignalSlideOff));
}



void MidiTrack::pushTremolo(short int rOffset)
{
    short int slideStep = rOffset/40; //10 steps of 1/4

    std::uint8_t pitchShift = 64;
    for (size_t slideInd=0; slideInd <10; ++slideInd)
    {
         auto mSignalBend = std::make_unique<MidiSignal>(0xE1,0,pitchShift,slideStep);
         this->push_back(std::move(mSignalBend));
         pitchShift-=3;//calibrate
    }

    rOffset -= rOffset/4;
    //last point
    auto mSignalBendLast = std::make_unique<MidiSignal>(0xE1,0,pitchShift, rOffset);
    this->push_back(std::move(mSignalBendLast));
    auto mSignalBendClose = std::make_unique<MidiSignal>(0xE1,0,64,0);
    this->push_back(std::move(mSignalBendClose));
}

void MidiTrack::pushFadeIn(short int rOffset, std::uint8_t channel)
{

    std::uint8_t newVolume = 27;
    short int fadeInStep = rOffset/20;

    auto volumeChangeFirst = std::make_unique<MidiSignal>(0xB0 | channel,7,newVolume,0);
    this->push_back(std::move(volumeChangeFirst));

    for (int i = 0; i < 20; ++i)
    {
        newVolume += 5;
        auto volumeChange = std::make_unique<MidiSignal>(0xB0 | channel,7,newVolume,fadeInStep);
        this->push_back(std::move(volumeChange));

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

    auto event47 = std::make_unique<MidiSignal>(0xff,47,0,0);
    std::uint8_t lenZero = 0;
    event47->metaLen.push_back(lenZero);
    this->push_back(std::move(event47));
}

//CALC helpers
short int MidiTrack::calcRhythmDetail(std::uint8_t RDValue, short int rhythmOffset)
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

std::uint8_t MidiTrack::calcMidiPanoramGP(std::uint8_t pan)
{
    if (midiLog)  qDebug() << "Panoram value = "<<pan;
    std::uint8_t midiPanoram = pan*8;
    if (midiPanoram>=128) midiPanoram=128;
    return midiPanoram;
}
std::uint8_t MidiTrack::calcMidiVolumeGP(std::uint8_t vol)
{
    if (midiLog)  qDebug() <<"Volume is "<<vol;
    std::uint8_t midiVolume = vol*8;
    if (midiVolume >= 128) midiVolume=128;
    return midiVolume;
}
std::uint8_t MidiTrack::calcPalmMuteVelocy(std::uint8_t vel)
{
    std::uint8_t outputVelocy = vel;
    std::uint8_t decreaceVelocy = vel/5;
    outputVelocy -= decreaceVelocy;
    return outputVelocy;
}
std::uint8_t MidiTrack::calcLeggatoVelocy(std::uint8_t vel)
{
    return calcPalmMuteVelocy(vel);//for possible difference
}
//HELPERS END

/*
void MidiTrack::add(MidiSignal &val)
{
    ChainContainer::add(val);
  //  std::cout<<std::endl;
  //  val.printToStream(std::cout);
   // if (midiLog)  qDebug() <<"lo";
}
*/



//new fun
void MidiTrack::closeLetRings(std::uint8_t channel)
{
    for (int i = 0; i < 10; ++i)
    {
        if (ringRay[i] != 255)
        {
            closeLetRing(i,channel);
        }
    }
}

void MidiTrack::closeLetRing(std::uint8_t stringN, std::uint8_t channel)
{
    if (stringN > 8)
    {
        qDebug() <<"String issue "<<stringN;
        return;
    }

    std::uint8_t ringNote = ringRay[stringN];
    ringRay[stringN]=255;

    std::uint8_t ringVelocy=80;

    if (ringNote != 255)
    pushNoteOff(ringNote,ringVelocy,channel);

}

void MidiTrack::openLetRing(std::uint8_t stringN, std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel)
{
    if (stringN > 8)
    {
        qDebug() <<"String issue "<<stringN;
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

void MidiTrack::pushNoteOn(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel)
{
    auto noteOn = std::make_unique<MidiSignal>(0x90 | channel, midiNote, velocity,accum);
    takeAccum();
    push_back(std::move(noteOn));
}

void MidiTrack::pushNoteOff(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel)
{
    auto noteOn = std::make_unique<MidiSignal>(0x80 | channel, midiNote, velocity,accum);
    takeAccum();
    push_back(std::move(noteOn));
}

bool MidiTrack::checkForLeegFails()
{
}

void MidiTrack::startLeeg(std::uint8_t stringN, std::uint8_t channel)
{
}

void MidiTrack::stopLeeg(std::uint8_t stringN, std::uint8_t channel)
{
}
