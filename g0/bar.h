#ifndef BAR_H
#define BAR_H

#include "types.h"

#include <memory.h> //TODO remove
#include <unordered_map>
#include "abitarray.h"
#include "tabcommands.h" //move into g0

class Track;
class Bar;
class Beat;
class Note;


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


//STARTING LARGE REFACTORING OVER THE TAB

class Beat : public ChainContainer<Note*, Bar>
{

public:
    Beat()
    {}

    virtual ~Beat()
    {
        for (ul i=0; i < len(); ++i)
                   delete at(i);
    }
    //usually size reserved

     void printToStream(std::ostream &stream);

    struct SingleChange
    {
        byte changeType;
        ul changeValue;
        byte changeCount;
    };

    //need inner functions for analtics of packing
    class ChangesList : public ChainContainer<SingleChange, void>
    {
    public:
        //search functions
    };

    //used for compatibility with guitar pro
    struct GPChordDiagram
    {
        byte header;
        byte sharp;
        byte blank1;
        byte blank2;
        byte blank3;

        byte root;
        byte minMaj;
        byte steps9x;

        ul bass;
        ul deminush;

        byte add;
        char name[20];

        byte blank4;
        byte blank5;

        byte s5,s9,s11;

        ul baseFrets;
        ul frets[7];

        byte baresNum;
        byte fretBare[5];
        byte startBare[5];
        byte endBare[5];

        byte o1,o3,o5,o7,o9,o11,o13;

        byte blank6;

        byte fingering[7];
        byte showFing;
    } gpCompChordDiagram;


    struct ChangeTable
    {
       byte newInstr;
       byte newVolume;
       byte newPan;
       byte newChorus;
       byte newReverb;
       byte newPhaser; //6 total + 4
       byte newTremolo;
       ul newTempo; //OR IT IS INT?
       //10
       byte instrDur;
       byte volumeDur;
       byte panDur;
       byte chorusDur;
       byte reverbDur;
       byte phaserDur;
       byte tremoloDur;
       byte tempoDur;

       byte changesTo; //one track or all of them
       byte postFix;
    };

    EffectsPack effPack;

    Beat &operator=(Beat *another)
    {
        clone(another);
        return *this;
    }

protected:

    byte duration = 0; // 2 1 . 2 4 8 16 32 64 [8 values] - 3 bits
    byte durationDetail = 0; // none, dot, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 [4 bits] //one of 2-15 means empty(together with pause only!)
    //1 bit determines pause
    bool isPaused;

    byte dotted = 0; //0 1 2


    byte effects = 0; //tremolo precene - 1bit, (upstrings, downstrings+x) - 2 bits, bookmark - 1 bit, notice (text or elst) - 1 bit,
    //then 3 left for change - first bit of those 3 shows that there is change
    //+1 chord diagram
    //+1 reserved

    //moved to set get
    //short int *tremoloPoints;

    std::string bookmarkName;
    std::string noticeText;


    public:

    //pub are part of set and get
    ChangesList changes;
    BendPoints tremolo;



    //SET GET operations
    void setPause(bool pause) {isPaused = pause;}
    bool getPause() { return isPaused; }

    void deleteNote(int string)
    {
        for (ul i = 0; i < len(); ++i)
        {
            if (string == at(i)->getStringNumber())
            {
                remove(i);

                if (len() == 0)
                    setPause(true);

                return;
            }
        }
    }

    Note *getNote(int string)
    {
        for (ul i = 0; i < len(); ++i)

            if (at(i)->getStringNumber()==string)
                return at(i);

        return 0;
    }

    Note *getNoteInstance(int string)
    {
        /*
        for (ul i = 0; i < len(); ++i)

            if (getV(i).getStringNumber()==string)
                return getV(i);
        */

        Note *notePtr = getNote(string);
        if (notePtr)
        {
            Note *theClone = new Note();
            theClone->clone(notePtr);
            return theClone;
        }

        Note *emptyNote = new Note;
        emptyNote->setFret(255);
        return emptyNote;
    }

    void setFret(byte fret, int string)
    {

        if (len() == 0)
        {
            //paused
            Note *newNote = new Note();
            newNote->setFret(fret);
            newNote->setStringNumber(string);
            newNote->setState(0);

            //DEFAULT NOTE VALUES??
            add(newNote);

            setPause(false);
            return;
        }

        for (ul i = 0; i < len(); ++i)
        {
            if (at(i)->getStringNumber()==string)
            {
                at(i)->setFret(fret);
                return; //function done
            }

            if (at(i)->getStringNumber() > string)
            {
                Note *newNote=new Note();
                newNote->setFret(fret);
                newNote->setStringNumber(string);
                newNote->setState(0);


                insertBefore(newNote,i);
                return;
            }

        }


        int lastStringN = at(len()-1)->getStringNumber();
        if (lastStringN < string)
        {
            Note *newNote=new Note();
            newNote->setFret(fret);
            newNote->setStringNumber(string);
            newNote->setState(0);

            //DEFAULT NOTE VALUES??
            add(newNote);
            return;
        }
        //we got here - that means we need insert fret
    }


    byte getFret(int string)
    {
        if (len() == 0)
            return 255;

        for (ul i = 0; i < len(); ++i)
            if (at(i)->getStringNumber()==string)
            {
                byte fretValue = at(i)->getFret();
                return fretValue; //function done
            }

        return 255;
    }

    void setDuration(byte dValue) { duration = dValue; }
    void setDurationDetail(byte dValue) {	durationDetail = dValue; }
    void setDotted(byte dottedValue) { dotted = dottedValue; }

    byte getDuration() { return duration;}
    byte getDurationDetail () { return durationDetail; }
    byte getDotted() { return dotted; }

    void setEffects(byte eValue);// { effects = eValue; }
    EffectsPack getEffects();// { return effects; }

    //TREMOLO missing

    //BOOKMARK - goes  for future

    void setGPCOMPChordDiagram(char *area) { memcpy(&gpCompChordDiagram,area,sizeof(gpCompChordDiagram)); }
    bool getGPCOMPChordDiagram(char *to) { memcpy(to,&gpCompChordDiagram,sizeof(gpCompChordDiagram)); return true; }

    void setGPCOMPText(std::string &value) { noticeText = value; }
    void getGPCOMPText(std::string &value) { value = noticeText; }

    //pack function

    void clone(Beat *from);
};

class Bar : public ChainContainer<Beat*, Track>
{

public:
    Bar() {
        flush();
    }
    virtual ~Bar()
    {
        for (ul i=0; i < len(); ++i)
                   delete at(i);
    }

    void printToStream(std::ostream &stream);

    void flush()
    {
        signatureNum = signatureDenum = 0;
        repeat = repeatTimes = altRepeat = 0;
        markerColor = 0;
        completeStatus = 0;
    }

    Bar &operator=(Bar *another)
    {
        clone(another);
        return *this;
    }

    virtual void add(Beat *&val)
    {
        if (val)
        {
            val->setParent(this);
            ChainContainer<Beat*, Track>::add(val);
        }
    }

    virtual void insertBefore(Beat* &val, int index=0)
    {
        if (val)
        {
            val->setParent(this);
            ChainContainer<Beat*, Track>::insertBefore(val,index);
        }
    }


protected:

    byte signatureNum;
    byte signatureDenum;

    byte repeat;//reprease options
    byte repeatTimes;
    byte altRepeat;

    //GP comp - marker, tonality
    byte gpCompTonality;
    std::string markerText;
    ul markerColor; //white byte == 1 if empty

    byte completeStatus;
    short completeAbs;
    ul completeIndex;

    public:
    //SET GET operations

    void setSignNum(byte num) { signatureNum = num; }
    void setSignDenum(byte denum) { signatureDenum = denum; }

    byte getSignNum() { return signatureNum; }
    byte getSignDenum() { return signatureDenum; }

    //!completeStatus !!! that should go private and opt
    void countUsedSigns(byte &numGet, byte &denumGet);

    byte getCompleteStatus();
    double getCompleteAbs();
    ul   getCompleteIndex();

    void setRepeat(byte rValue, byte times=0)
    {
        if (rValue == 0) repeat = 0;
        else repeat |= rValue;
        if(times) repeatTimes=times;
    }
    byte getRepeat() { return repeat; }
    byte getRepeatTimes() { return repeatTimes; }

    void setAltRepeat(byte number) { altRepeat = number; }
    byte getAltRepeat() { return altRepeat; }

    void setGPCOMPTonality(byte tValue) { gpCompTonality = tValue; }
    byte getGPCOMPTonality() { return gpCompTonality; }

    void setGPCOMPMarker(std::string &text, ul color) { markerText = text; markerColor = color; }
    void getGPCOMPMarker(std::string &text, ul &color) { text = markerText; color = markerColor; }

    //pack function
     void clone(Bar *from);
};


#endif // BAR_H
