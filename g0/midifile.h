#ifndef MIDIFILE_H
#define MIDIFILE_H

#include <vector>
#include <iostream>

#include "types.h"

class Tab;
class Track;
class Bar;
class Beat;
class Note;


//protected looks great but should be designed for manual use
class VariableInt : public std::vector<byte> //or public, or fullfit
{
public:
    VariableInt &operator =(VariableInt &v) 
    {
    	//clear - to POLY ! ATTENTION !
        size_t copyLen = v.size();
    	for (size_t i = 0; i < copyLen; ++i)
            push_back(v.at(i));

        return *this;
    }

    size_t readStream(std::ifstream &file); //reads from file and returns amount of bytes stored inside
    size_t writeStream(std::ofstream &file);
    size_t getValue();
    size_t getsize() { return this->size(); } //cover

	//for future - get extended values for many many parts
	//for future - ul writeStream(AF ile &file);
    //operator = int, char, long long, VariableInt
    //operator += all of them
    //operator -= same
};

struct MidiSignal
{
	//for each of type
    VariableInt time;
    size_t absValue;
    //for each of type
	//byte type; //replace with getEventType()
    //byte channel; //replace with getChannel()
	byte byte0;
	//for the normal events
	byte param1; //and for meta event(b0==0xff) here will lay type 
    byte param2;

    class ByteBufer : public std::vector<byte>
	{
	};

	struct MetaStore
	{
		VariableInt metaLen;
		ByteBufer bufer;
			
		//possible function parse(byte) used for unpack bytes data
		//for determined events - param is byte0
	} metaStore;
	
	//system events should lay simmiliar as meta ones

    size_t readStream(std::ifstream &ifile);
    size_t writeStream(std::ofstream &ofile, bool skip=false);
    bool skipThat();
		
	byte getEventType()
	{
        byte eventType = byte0 & (0xf0);	// (16 + 32 + 64 + 128);
		eventType >>= 4;
		return eventType;
	}
	
	byte getChannel()
	{
		byte midiChannel = byte0 & (0xf);	// (0x1 + 0x2 + 0x4 + 0x8)
		return midiChannel;
	}
	
	bool isMetaEvent(){
		return (byte0 == 0xff);
	}
	
	bool isNormalEvent(){
		return !isMetaEvent();
	}
	
    bool isSystemEvent() {
		return false; //yet unknown
	}
	
	void printToStream(std::ostream &stream);
    size_t calcSize(bool skip=false);

    MidiSignal(byte b0, byte b1, byte b2, size_t timeShift);
    MidiSignal(){absValue=0;}

protected:
    //any shit put here from format

};


struct midiHeader
{
    char chunkId[5]; //4
    long chunkSize; //4b
    short formatType; //2
    short nTracks; //2
    short timeDevision; //2

    //read from stream or string
};

struct midiTrackHeader
{
    char chunkId[5];
    size_t trackSize;
};

class MidiTrack : public ChainContainer<MidiSignal, void>
{
protected:

public:
    midiTrackHeader trackHeader; //think about it!
    MidiTrack() {} //ou ou..

    virtual ~MidiTrack()
    {
        for (size_t i=0; i < size(); ++i)
            delete at(i);
    }
    
    bool calculateHeader(bool skip=false);
	void printToStream(std::ostream &stream);

	//ul writeStream(AF ile &ofile);
	//ul readStream(AF ile &ifile); //missing from refactoing

    bool fromTrack(Track *track, byte channel=0, size_t shiftCursorBar=0);

    //void add(MidiSignal &val);

    //HELPERS functions from AMUSIC generation

    //no time versions
    void pushChangeInstrument(byte newInstr, byte channel, size_t timeShift=0);
    void pushChangeBPM(int bpm, size_t timeShift=0); //same way others
    void pushChangeVolume(byte newVolume, byte channel);
    void pushChangePanoram(byte newPanoram, byte channel);

    void pushMetrSignature(byte num, byte den, size_t timeShift, byte metr=24, byte perQuat=8);

    void pushVibration(byte channel, byte depth, short int step, byte stepsCount=3);
    void pushSlideUp(byte channel, byte shift, short int step, byte stepsCount=8);
    void pushSlideDown(byte channel, byte shift, short int step, byte stepsCount=8);

    void pushBend(short int rOffset, void *bendP, byte channel); //yet deadcoded one
    void pushTremolo(short int rOffset); //yet deadcoded one

    void pushFadeIn(short int rOffset, byte channel);


    void pushEvent47();

    //END of HELPERS

    short int calcRhythmDetail(byte RDValue, short int rhythmOffset);
    byte calcMidiPanoramGP(byte pan); //GUITAR P OPTION
    byte calcMidiVolumeGP(byte vol); //GUITAR P OPTION
    byte calcPalmMuteVelocy(byte vel);
    byte calcLeggatoVelocy(byte vel);

protected:

    int accum;
    byte tunes[10];
    byte ringRay[10];

    ///! CHENNELS FOR RING RAY

    bool addSignalsFromNoteOn(Note *note, byte channel);
    bool addSignalsFromNoteOff(Note *note, byte channel);

    bool addPostEffects(Beat *beat, byte channel);

    bool addSignalsFromBeat(Beat *beat, byte channel, short specialR=0);

    void closeLetRings(byte channel);		//-
    void openLetRing(byte stringN, byte midiNote, byte velocity, byte channel); //-
    void closeLetRing(byte stringN, byte channel);  //-

    void finishIncomplete(short specialR); //-

    void pushNoteOn(byte midiNote, byte velocity, byte channel); //acummulated already inside
    void pushNoteOff(byte midiNote, byte velocity, byte channel); //-

    void startLeeg(byte stringN, byte channel);
    void stopLeeg(byte stringN, byte channel);

    bool checkForLeegFails(); //-

public:

    int accumulate(int value) { accum += value; return accum; }
    int getAccum() { return accum; }
    void takeAccum() { accum=0; }

    void setTunes(byte *from)
    {
        for (byte i=0; i < 10; ++i)
        {
            tunes[i]=from[i];
            ringRay[i]=255;
        }

    }


};

class MidiFile : public ChainContainer<MidiTrack, void>
{
protected:
    struct midiHeader midiHeader;
    
    //inner function for reading signals structure

    int bpm;
    
public:
    //each as track - 0 for defaults signals
    MidiFile():bpm(120){

    }

    virtual ~MidiFile()
    {
        for (size_t i=0; i < size(); ++i)
            delete at(i);
    }

    //bool generateFromAMusic(AMusic &music);
    bool fromTab(Tab *tab, size_t shiftTheCursor=0);

    //calculation helpers


    //used for generation
    
    bool calculateHeader(bool skip=false);
    void printToStream(std::ostream &stream);

    bool readStream(std::ifstream &ifile);
    size_t writeStream(std::ofstream &ofile);

    size_t noMetricsTest(std::ofstream &ofile);

    //set get
    void setBPM(int bpmNew) { bpm = bpmNew; }
    int  getBPM() { return bpm; }

};

#endif // MIDIFILE_H
