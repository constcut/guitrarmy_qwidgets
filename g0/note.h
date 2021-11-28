#ifndef NOTE_H
#define NOTE_H

#include "types.h"
#include <unordered_map>
#include "abitarray.h"

class Beat;


//old version of bends used on early moments for
struct BendPointGPOld
{
    ul absolutePosition;
    ul heightPosition;
    byte vibratoFlag;
};

class BendPointsGPOld : public ChainContainer<BendPointGPOld, void>
{
protected:
    ul bendHeight;
    byte bendType;
public:

    byte getType() { return bendType; }
    void setType(byte newBendType) { bendType = newBendType; }

    ul getHeight() { return bendHeight; }
    void setHeight(ul newBendHeight) { bendHeight=newBendHeight;}

};

struct BendPoint
{
    byte vertical; //:4
    byte horizontal; //:5
    byte vFlag; //:2
};

class BendPoints : public ChainContainer<BendPoint, void>
{
protected:
    byte bendType;
public:
    byte getType() { return bendType; }
    void setType(byte newBendType) { bendType = newBendType; }

    void insertNewPoint(BendPoint bendPoint);
};



class Note
{
    //class implements single note, a sound on one string
    public:

    void printToStream(std::ostream &stream);

    enum FretValues
    {
        emptyFret = 63,
        pauseFret = 62,
        ghostFret = 61,
        leegFret = 60
    };
        //check is upper replaced?

    enum NoteState
    {
        normalNote = 0,
        leegNote = 2,
        deadNote = 3,
        leegedNormal = 4,
        leegedLeeg = 6
    };


    Note():
    fret(emptyFret),volume(0),fingering(0),effectsA(0),effectsB(0),
          graceIsHere(false) {}
    virtual ~Note() {}

    Note &operator=(Note *anotherNote)
    {
        clone(anotherNote);
        return *this;
    }

    protected:
    //[BASE]
    byte fret; //[0-63]; some last values used for special coding {pause, empty, leege note... ghost note(x)}
    byte volume;//[0-64]]-1);  values less than 14 used for ppp pp p m f ff fff - before -is precents for 2 per step
    byte fingering; //store information about finger to play - //fingers {none, L1,L2,L3,L4,L5, R1,R2,R3,R4,R5 +(12)(13)(14)(15)(16) ) - pressure hand for another there
    ///are pick effects

    byte noteState;//NoteState


    byte effectsA; // inEffects:[slide in up\down; grace; none] outEffects:[side out up\down; legato; bend; ghost; ] [let ring] [palm mute] [vibro]
    byte effectsB; // picking [tap slap pop up down stoks and non] [trill 0 2 4 8]

    //moved to set get
   //short int *bendPoints;



    //to set sure  - if possible escape - then do it
    byte GpCompFing1;
    byte GpCompFing2;


    byte stringNumber;

    Note *prevNote;
    Note *nextNote;

    public:

    void setNext(Note *nextOne) { nextNote = nextOne; }
    void setPrev(Note *prevOne) { prevNote = prevOne; }

    Note *getNext() { return nextNote; }
    Note *getPrev() { return prevNote; }

    //For GP Compatibility left:
    //Grace note - 4bytes
    bool graceIsHere;
    byte graceNote[4];//compitability with guitar pro requires later migration

    BendPoints bend;
    //[EFFECTS]
    EffectsPack effPack;

    //SET GET operations
    void setStringNumber(byte num) {stringNumber = num;}
    byte getStringNumber() { return stringNumber; }

    void setFret(byte fValue) { fret = fValue; }
    byte getFret() { return fret; }


    int getMidiNote(int tune=0) //get tune from tracks hard chain
    {
        return fret + tune;
    }

    void setState(byte nState) { noteState = nState; }
    void signStateLeeged() { if (noteState<=1) noteState=4; if (noteState==2) noteState=6; }
    byte getState() { return noteState;}

    void setVolume(byte vValue) { volume = vValue; }
    byte getVolume() { return volume; }

    void setFingering(byte fValue) {fingering = fValue;}
    byte getFingering() { return fingering; }

    void setEffect(byte eff);// { effectsA = eff; }
    EffectsPack getEffects();// { return effectsA; }
    void addEffects(EffectsPack &append) { effPack.mergeWith(append); }

    //effects need review
    //MISSING
    //INCLUDING	BEND

    void setGPCOMPFing1(byte fValue) { GpCompFing1 = fValue; }
    void setGPCOMPFing2(byte fValue) { GpCompFing2 = fValue; }
    byte getGPCOMPFing1() { return GpCompFing1;}
    byte getGPCOMPFing2() { return GpCompFing2;}

    //NO	CHECK	REMINDS	ABOUT	FUTURE	CHANGES
    void setGPCOMPGrace(size_t index, byte gValue) { graceNote[index] = gValue;}
    byte getGPCOMPGrace(size_t index) { return graceNote[index]; }

    void clone(Note *from);
    //pack function
};



#endif // NOTE_H
