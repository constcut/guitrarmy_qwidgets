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



size_t VariableInt::readStream(std::ifstream & ifile)
{
    size_t totalBytesRead = 0;

    std::uint8_t lastByte = 0;
	do
	{
        ifile.read((char*)&lastByte, 1);

        std::uint8_t valueByte = lastByte & 127;
        this->push_back(valueByte);

        if (midiLog)  qDebug() << "V[" << totalBytesRead << "] value = " << valueByte;

		if (lastByte & 128)
        {
            if (midiLog)  qDebug() << " not last! " ;
        }
		else
            if (midiLog)  qDebug() << " last one!" ;

		// end of debugging

		++totalBytesRead;

	}
	while (lastByte & 128);

	return totalBytesRead;
}

size_t VariableInt::getValue()
{
	// collect value from poly
    size_t responseValue = 0;

    size_t length = size();

	for (size_t i = 0; i < length; ++i)
	{
		responseValue <<= 7;
        responseValue += at(i);
	}

	return responseValue;
}

size_t MidiSignal::readStream(std::ifstream & ifile)
{
    size_t totalRead = 0;

	// main place for current refactoring
	// move into signal
	VariableInt varInt;
    size_t varIntBytesRead = varInt.readStream(ifile);

	totalRead += varIntBytesRead;

    std::uint8_t p00 = 0;				// 2 parts of 4bit params
    std::uint8_t p1 = 0;				// param 1
    std::uint8_t p2 = 0;				// param 2;

    ifile.read((char*)&p00, 1);
	totalRead += 1;

	//first fill the structure
	this->byte0 = p00; //channel + event type
	this->time = varInt;

	// debuging

	if (p00 == 0xff)
	{
        if (midiLog)  qDebug() << "This is a meta event! " ;

        std::uint8_t metaType;
        ifile.read((char*)&metaType, 1);
		int metaEvType = metaType;
		totalRead += 1;

		//or in own place?
		this->param1 = metaType;

		VariableInt metaLength;
        size_t mLenBytesRead = metaLength.readStream(ifile);
		totalRead += mLenBytesRead;
        size_t bytesToSkip = metaLength.getValue();

		this->metaStore.metaLen = metaLength;

        if (midiLog)  qDebug() << "Meta type = " << metaEvType << " to skip - " << bytesToSkip;

        std::uint8_t bBufer;

        for (size_t k = 0; k < bytesToSkip; ++k)
		{
            ifile.read((char*)&bBufer, 1);
            this->metaStore.bufer.push_back(bBufer);
		}
		
		totalRead += bytesToSkip;

	}
	else
	{

		int evValue = p00;
        if (midiLog)  qDebug() << "Not meta event. value = " << evValue;

        ifile.read((char*)&p1, 1);
		totalRead += 1;
		
		this->param1 = p1;

		//CLEAN THIS	MARKER	ONLY	AFTER	FILLING	FUNCTIONS
		int midiChannel = p00 & (0xf);	// (0x1 + 0x2 + 0x4 + 0x8)
        int eventType = p00 & (0xf0);	// (16 + 32 + 64 + 128);
		eventType >>= 4;
		//JUST OUTPUT	FOR	HERE, BUT ALWAYS	FOR	FUNCTIONS

		if ((eventType != 0xC) && (eventType != 0xD))
		{
            ifile.read((char*)&p2, 1);
			totalRead += 1;
			this->param2 = p2;

		}

		int param1 = p1;
		int param2 = p2;

        if (midiLog)  qDebug() << " Event# " << eventType << "; midiChan# "
            << midiChannel << "; p1 = " << param1 << "; p2 = " << param2;

        //if (eventType == 0) ; //just a reminder of system events

	}

	return totalRead;
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

//Here appear stream printing operaions
// PRINT BEGIN
void MidiSignal::printToStream(std::ostream &stream)
{
	if (byte0 == 0xff)
	{
		int metaType = param1;
        size_t metaLen = metaStore.metaLen.getValue();
		stream <<"MetaType "<<metaType<<"; MetaLen " <<metaLen;
        for (size_t i = 0; i < metaLen; ++i)
        {
            stream << std::endl<< "Meta byte "<<i<<" = "<<(int) metaStore.bufer.at(i);
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
	
    size_t time = this->time.getValue();
	stream <<"; t " <<time<<std::endl;
}


void MidiTrack::printToStream(std::ostream &stream)
{
	stream << "Output MidiTrack.";
    //stream << "chunky = " << trackHeader.chunkId <<std::endl;
    stream << "Track Size = " << trackHeader.trackSize << std::endl;
	
    size_t signalsAmount = size();
    for (size_t i = 0; i < signalsAmount; ++i)
        at(i)->printToStream(stream);
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

size_t VariableInt::writeStream(std::ofstream &file)
{
    size_t amountOfBytes = size();
	
    std::uint8_t byteToWrite = 0;

	
	for (size_t i = 0; i < amountOfBytes; ++i)
	{
        byteToWrite = at(i);

    //    if (midiLog)  qDebug() <<"PRE WrOtE"<<byteToWrite;
		
        if (i != (amountOfBytes - 1))
				byteToWrite |= 128;	
		
        file.write((const char*)&byteToWrite,1);

      // if (midiLog)  qDebug() <<"V WROTE "<<byteToWrite;
    }
						
	return amountOfBytes;	
}

size_t MidiSignal::writeStream(std::ofstream &ofile,bool skip)
{
    size_t bytesWritten = 0;

    if (skip)
    {
        if (skipThat())
            return 0;
    }
	
    bytesWritten += time.writeStream(ofile); //CHECK 0
    ofile.write((const char*)&byte0,1);
    //if (midiLog)  qDebug() <<"WROTE "<<byte0;

    //int zFuck = 00; //check???? OUPS
    //ofile.write((const char*)&zFuck,1);
    ofile.write((const char*)&param1,1);
    //if (midiLog)  qDebug() <<"WROTE "<<param1;

	bytesWritten += 2;
	//ATTENTION finish bytes written

	if (isMetaEvent())
    {
        //if (midiLog)  qDebug() << "META EVENT WROTEN!";

        //ofile.write((const char*)&metaStore.metaType,1);
		bytesWritten += metaStore.metaLen.writeStream(ofile);
		
        size_t metaBufLen = metaStore.bufer.size();
		for (size_t i = 0; i < metaBufLen; ++i)
            ofile.write((const char*)&metaStore.bufer.at(i),1);
		
		bytesWritten += metaBufLen;
	}
	else
    { //normal event
        std::uint8_t eventType = getEventType();
        if ((eventType != 0xC) && (eventType != 0xD))
		{
              ofile.write((const char*)&param2,1);
              //if (midiLog)  qDebug() <<"WROTE "<<param2;
              bytesWritten += 1;
        }
	}
									
	return bytesWritten;	
}




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
        calculatedSize += at(i)->calcSize(skip);
    }

    if (midiLog)  qDebug() <<"Calculating track size : "<<calculatedSize;
    if (midiLog)  qDebug() <<"Previously stored : "<<trackHeader.trackSize;

    trackHeader.trackSize=calculatedSize;//NOW PUSED

    //and header defaults
    memcpy(trackHeader.chunkId,"MTrk",5); //some attention here if normal wouldnt go

    return true;
}

size_t MidiSignal::calcSize(bool skip)
{
    size_t localSize = 0;

    if (skip)
    {
        if (skipThat())
            return 0;
    }

    localSize += time.getsize(); //delta time value
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
        localSize += metaStore.metaLen.getsize(); //unhide len?
        localSize += metaStore.bufer.size();
    }



    return localSize;
}

//RECOUNTERS END

///GENERATOR begin

bool MidiSignal::skipThat()
{
    std::uint8_t evT = getEventType();


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

MidiSignal::MidiSignal(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2, size_t timeShift):byte0(b0),param1(b1),param2(b2)
{
    absValue=0;
    if (timeShift == 0)
    {
        std::uint8_t noShift = 0;
        time.push_back(noShift);
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

            std::uint8_t firstB = first;
            std::uint8_t secondB = second;
            std::uint8_t thirdB = third;

            time.push_back(firstB);
            time.push_back(secondB);
            time.push_back(thirdB);
        }
        else
        {
            std::uint8_t firstB = first;
            std::uint8_t secondB = second;

            time.push_back(firstB);
            time.push_back(secondB);
        }
    }
}


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

    signatureEvent->metaStore.bufer.push_back(num);

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

    signatureEvent->metaStore.bufer.push_back(transDur);
    signatureEvent->metaStore.bufer.push_back(metr);
    signatureEvent->metaStore.bufer.push_back(perQuat);

    std::uint8_t metaSize = 4;
    signatureEvent->metaStore.metaLen.push_back(metaSize);

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

    changeTempEvent->metaStore.bufer.push_back(tempB1);
    changeTempEvent->metaStore.bufer.push_back(tempB2);
    changeTempEvent->metaStore.bufer.push_back(tempB3);

    std::uint8_t lenMeta = 3;
    changeTempEvent->metaStore.metaLen.push_back(lenMeta);

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
    volumeChange->time.push_back(timeZero);

    push_back(std::move(volumeChange));
}

void MidiTrack::pushChangePanoram(std::uint8_t newPanoram, std::uint8_t channel)
{
    auto panoramChange = std::make_unique<MidiSignal>();
    panoramChange->byte0 = 0xB0 | channel;
    panoramChange->param1 = 0xA; //change panoram
    panoramChange->param2 = newPanoram;
    std::uint8_t timeZero = 0;
    panoramChange->time.push_back(timeZero);

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
    event47->metaStore.metaLen.push_back(lenZero);
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
