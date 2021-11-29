#ifndef MIDIFILE_H
#define MIDIFILE_H

#include <vector>
#include <iostream>
#include <memory>

#include "midisignal.h"




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

class MidiTrack : public std::vector<std::unique_ptr<MidiSignal>>
{
protected:

public:
    midiTrackHeader trackHeader; //think about it!

    MidiTrack() = default;
    virtual ~MidiTrack() = default;
    
    bool calculateHeader(bool skip=false);
	void printToStream(std::ostream &stream);

	//ul writeStream(AF ile &ofile);
	//ul readStream(AF ile &ifile); //missing from refactoing

    //bool fromTrack(Track *track, std::uint8_t channel=0, size_t shiftCursorBar=0);

    //void add(MidiSignal &val);

    //HELPERS functions from AMUSIC generation

    //no time versions
    void pushChangeInstrument(std::uint8_t newInstr, std::uint8_t channel, size_t timeShift=0);
    void pushChangeBPM(int bpm, size_t timeShift=0); //same way others
    void pushChangeVolume(std::uint8_t newVolume, std::uint8_t channel);
    void pushChangePanoram(std::uint8_t newPanoram, std::uint8_t channel);

    void pushMetrSignature(std::uint8_t num, std::uint8_t den, size_t timeShift, std::uint8_t metr=24, std::uint8_t perQuat=8);

    void pushVibration(std::uint8_t channel, std::uint8_t depth, short int step, std::uint8_t stepsCount=3);
    void pushSlideUp(std::uint8_t channel, std::uint8_t shift, short int step, std::uint8_t stepsCount=8);
    void pushSlideDown(std::uint8_t channel, std::uint8_t shift, short int step, std::uint8_t stepsCount=8);

    void pushBend(short int rOffset, void *bendP, std::uint8_t channel); //yet deadcoded one
    void pushTremolo(short int rOffset); //yet deadcoded one

    void pushFadeIn(short int rOffset, std::uint8_t channel);


    void pushEvent47();

    //END of HELPERS

    short int calcRhythmDetail(std::uint8_t RDValue, short int rhythmOffset);
    std::uint8_t calcMidiPanoramGP(std::uint8_t pan); //GUITAR P OPTION
    std::uint8_t calcMidiVolumeGP(std::uint8_t vol); //GUITAR P OPTION
    std::uint8_t calcPalmMuteVelocy(std::uint8_t vel);
    std::uint8_t calcLeggatoVelocy(std::uint8_t vel);

public: //TODO review when midi generation is done


    std::uint8_t tunes[10];
    std::uint8_t ringRay[10];

    ///! CHENNELS FOR RING RAY

    //bool addSignalsFromNoteOn(Note *note, std::uint8_t channel);
    //bool addSignalsFromNoteOff(Note *note, std::uint8_t channel);

    //bool addPostEffects(Beat *beat, std::uint8_t channel);

    //bool addSignalsFromBeat(Beat *beat, std::uint8_t channel, short specialR=0);

    void closeLetRings(std::uint8_t channel);		//-
    void openLetRing(std::uint8_t stringN, std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel); //-
    void closeLetRing(std::uint8_t stringN, std::uint8_t channel);  //-

    void finishIncomplete(short specialR); //-

    void pushNoteOn(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel); //acummulated already inside
    void pushNoteOff(std::uint8_t midiNote, std::uint8_t velocity, std::uint8_t channel); //-

    void startLeeg(std::uint8_t stringN, std::uint8_t channel);
    void stopLeeg(std::uint8_t stringN, std::uint8_t channel);

    bool checkForLeegFails(); //-

public:

    int accum;//TODO ref fun or move?

    int accumulate(int value) { accum += value; return accum; }
    int getAccum() { return accum; }
    void takeAccum() { accum=0; }

    void setTunes(std::uint8_t *from)
    {
        for (std::uint8_t i=0; i < 10; ++i)
        {
            tunes[i]=from[i];
            ringRay[i]=255;
        }

    }


};

class MidiFile : public std::vector<std::unique_ptr<MidiTrack>>
{
protected:
    struct midiHeader midiHeader;
    
    //inner function for reading signals structure

    int bpm;
    
public:
    //each as track - 0 for defaults signals
    MidiFile():bpm(120) {}

    virtual ~MidiFile() = default;


    //bool generateFromAMusic(AMusic &music);
    ///bool fromTab(Tab *tab, size_t shiftTheCursor=0);

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
