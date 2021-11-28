#ifndef BEAT_H
#define BEAT_H

#include <memory.h>  //TODO find modern way
#include <vector>

#include "types.h"
#include "note.h"



class Bar;


class Beat : public ChainContainer<Note, Bar> {

public:
    Beat()
    {}

    virtual ~Beat()
    {
        for (ul i=0; i < size(); ++i)
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
    class ChangesList : public std::vector<SingleChange>
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
        for (ul i = 0; i < size(); ++i)
        {
            if (string == at(i)->getStringNumber())
            {
                remove(i);

                if (size() == 0)
                    setPause(true);

                return;
            }
        }
    }

    Note *getNote(int string)
    {
        for (ul i = 0; i < size(); ++i)

            if (at(i)->getStringNumber()==string)
                return at(i);

        return 0;
    }

    Note *getNoteInstance(int string)
    {
        /*
        for (ul i = 0; i < size(); ++i)

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

        if (size() == 0)
        {
            //paused
            Note *newNote = new Note();
            newNote->setFret(fret);
            newNote->setStringNumber(string);
            newNote->setState(0);

            //DEFAULT NOTE VALUES??
            push_back(newNote);

            setPause(false);
            return;
        }

        for (ul i = 0; i < size(); ++i)
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


        int lastStringN = at(size()-1)->getStringNumber();
        if (lastStringN < string)
        {
            Note *newNote=new Note();
            newNote->setFret(fret);
            newNote->setStringNumber(string);
            newNote->setState(0);

            //DEFAULT NOTE VALUES??
            push_back(newNote);
            return;
        }
        //we got here - that means we need insert fret
    }


    byte getFret(int string)
    {
        if (size() == 0)
            return 255;

        for (ul i = 0; i < size(); ++i)
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

#endif // BEAT_H
