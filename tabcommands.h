#ifndef TABCOMMANDS_H
#define TABCOMMANDS_H

#include "g0/types.h"

#include "g0/tab.h"

class CommandRunner
{
public:
    virtual void run() = 0;
};

//Comand types:

//0 - empty
//1 - switch effect note
//2 - switch effect beat
//3 - set fret

//4 - set duration (includes both inc and dec)
//5 - set duration detail
//6 - set dot
//7 - set pause
//8 - delete note

//9 - pasty
//10 - cut (same for delete sequence)

//11 - instrument
//12 - pan
//13 - volume
//14 - drums

//15 - bpm

//16 - insert new bar or more

//17 - change note state

//18 - insert new pause (note)

//19 - set sign

//20 - insert beat

//24 - delete bar
//25 - delete range of bars (could be groupped but yet no reason)

class SingleCommand
{
    byte commandType;

    byte commandValue;
    byte commandValue2;
    //position

    //yet set exceed - optimize to file format
    byte track;
    int bar;
    int beat;
    byte string; //note

public:

    //special store
    std::vector<Note*> *storedNotes;

    //REFACT to variable amount and types
    void *outerPtr;
    void *outerPtrEnd;

    //could be refacted
    Bar *startBar;
    Bar *endBar;

    void requestStoredNotes()
    {
        storedNotes = new std::vector<Note*>;
    }

    void releaseStoredNotes()
    {
        delete storedNotes;
    }

    SingleCommand():commandType(0),commandValue(0),track(0),bar(0),beat(0),string(0),
        storedNotes(0),outerPtr(0),outerPtrEnd(0),startBar(0),endBar(0)
    {}

    SingleCommand(byte newType, byte newValue=0):
        commandType(newType),commandValue(newValue),track(0),bar(0),beat(0),string(0),
        storedNotes(0),outerPtr(0),outerPtrEnd(0),startBar(0),endBar(0)
    {
    }

    void setType(byte newType) { commandType = newType; }
    byte getType() { return commandType; }

    void setValue(byte newValue) { commandValue = newValue; }
    byte getValue() { return commandValue; }

    void setValue2(byte newValue) { commandValue2 = newValue; }
    byte getValue2() { return commandValue2; }


    void setPosition(byte trackN, int barN, int beatN, byte stringN=255)
    {
        track = trackN;
        bar = barN;
        beat = beatN;
        string = stringN;
    }

    byte getTrackNum() { return track; }
    int getBarNum() { return bar; }
    int getBeatNum() { return beat; }
    byte getStringNum() { return string; }

};

#endif // TABCOMMANDS_H
