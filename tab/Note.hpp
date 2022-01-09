#ifndef NOTE_H
#define NOTE_H

#include "Types.hpp"
#include "Effects.hpp"

#include <vector>

class Beat;


//old version of bends used on early moments for
struct BendPointGPOld
{
    size_t absolutePosition;
    size_t heightPosition;
    std::uint8_t vibratoFlag;
};

class BendPointsGPOld : public std::vector<BendPointGPOld>
{
protected:
    size_t bendHeight;
    std::uint8_t bendType;
public:

    std::uint8_t getType() { return bendType; }
    void setType(std::uint8_t newBendType) { bendType = newBendType; }

    size_t getHeight() { return bendHeight; }
    void setHeight(size_t newBendHeight) { bendHeight=newBendHeight;}

};

struct BendPoint
{
    std::uint8_t vertical; //:4
    std::uint8_t horizontal; //:5
    std::uint8_t vFlag; //:2
};

class BendPoints : public std::vector<BendPoint>
{
protected:
    std::uint8_t bendType;
public:
    std::uint8_t getType() { return bendType; }
    void setType(std::uint8_t newBendType) { bendType = newBendType; }

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
    std::uint8_t fret; //[0-63]; some last values used for special coding {pause, empty, leege note... ghost note(x)}
    std::uint8_t volume;//[0-64]]-1);  values less than 14 used for ppp pp p m f ff fff - before -is precents for 2 per step
    std::uint8_t fingering; //store information about finger to play - //fingers {none, L1,L2,L3,L4,L5, R1,R2,R3,R4,R5 +(12)(13)(14)(15)(16) ) - pressure hand for another there
    ///are pick effects

    std::uint8_t noteState;//NoteState


    std::uint8_t effectsA; // inEffects:[slide in up\down; grace; none] outEffects:[side out up\down; legato; bend; ghost; ] [let ring] [palm mute] [vibro]
    std::uint8_t effectsB; // picking [tap slap pop up down stoks and non] [trill 0 2 4 8]

    //moved to set get
   //short int *bendPoints;



    //to set sure  - if possible escape - then do it
    std::uint8_t GpCompFing1;
    std::uint8_t GpCompFing2;


    std::uint8_t stringNumber;

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
    std::uint8_t graceNote[4];//compitability with guitar pro requires later migration

    BendPoints bend;
    //[EFFECTS]
    ABitArray effPack;

    //SET GET operations
    void setStringNumber(std::uint8_t num) {stringNumber = num;}
    std::uint8_t getStringNumber() { return stringNumber; }

    void setFret(std::uint8_t fValue) { fret = fValue; }
    std::uint8_t getFret() { return fret; }


    int getMidiNote(int tune=0) //get tune from tracks hard chain
    {
        return fret + tune;
    }

    void setState(std::uint8_t nState) { noteState = nState; }
    void signStateLeeged() { if (noteState<=1) noteState=4; if (noteState==2) noteState=6; }
    std::uint8_t getState() { return noteState;}

    void setVolume(std::uint8_t vValue) { volume = vValue; }
    std::uint8_t getVolume() { return volume; }

    void setFingering(std::uint8_t fValue) {fingering = fValue;}
    std::uint8_t getFingering() { return fingering; }

    void setEffect(Effect eff);// { effectsA = eff; }
    ABitArray getEffects();// { return effectsA; }
    void addEffects(ABitArray &append) { effPack.mergeWith(append); }

    //effects need review
    //MISSING
    //INCLUDING	BEND

    void setGPCOMPFing1(std::uint8_t fValue) { GpCompFing1 = fValue; }
    void setGPCOMPFing2(std::uint8_t fValue) { GpCompFing2 = fValue; }
    std::uint8_t getGPCOMPFing1() { return GpCompFing1;}
    std::uint8_t getGPCOMPFing2() { return GpCompFing2;}

    //NO	CHECK	REMINDS	ABOUT	FUTURE	CHANGES
    void setGPCOMPGrace(size_t index, std::uint8_t gValue) { graceNote[index] = gValue;}
    std::uint8_t getGPCOMPGrace(size_t index) { return graceNote[index]; }

    void clone(Note *from);
    //pack function
};



#endif // NOTE_H
