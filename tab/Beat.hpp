#ifndef BEAT_H
#define BEAT_H

#include <memory.h>  //TODO find modern way
#include <vector>
#include <string>

#include "Types.hpp"
#include "Note.hpp"



class Bar;


class Beat : public ChainContainer<Note, Bar> {

public:
    Beat() = default;
    virtual ~Beat() = default;

    void printToStream(std::ostream &stream);

    struct SingleChange {
        std::uint8_t changeType;
        size_t changeValue;
        std::uint8_t changeCount;
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
        std::uint8_t header;
        std::uint8_t sharp;
        std::uint8_t blank1;
        std::uint8_t blank2;
        std::uint8_t blank3;

        std::uint8_t root;
        std::uint8_t minMaj;
        std::uint8_t steps9x;

        size_t bass;
        size_t deminush;

        std::uint8_t add;
        char name[20];

        std::uint8_t blank4;
        std::uint8_t blank5;

        std::uint8_t s5,s9,s11;

        size_t baseFrets;
        size_t frets[7];

        std::uint8_t baresNum;
        std::uint8_t fretBare[5];
        std::uint8_t startBare[5];
        std::uint8_t endBare[5];

        std::uint8_t o1,o3,o5,o7,o9,o11,o13;

        std::uint8_t blank6;

        std::uint8_t fingering[7];
        std::uint8_t showFing;
    } gpCompChordDiagram;


    struct ChangeTable
    {
       std::uint8_t newInstr;
       std::uint8_t newVolume;
       std::uint8_t newPan;
       std::uint8_t newChorus;
       std::uint8_t newReverb;
       std::uint8_t newPhaser; //6 total + 4
       std::uint8_t newTremolo;
       size_t newTempo; //OR IT IS INT?
       //10
       std::uint8_t instrDur;
       std::uint8_t volumeDur;
       std::uint8_t panDur;
       std::uint8_t chorusDur;
       std::uint8_t reverbDur;
       std::uint8_t phaserDur;
       std::uint8_t tremoloDur;
       std::uint8_t tempoDur;

       std::uint8_t changesTo; //one track or all of them
       std::uint8_t postFix;
    };

    ABitArray effPack;

    Beat &operator=(Beat *another)
    {
        clone(another);
        return *this;
    }

protected:

    std::uint8_t duration = 0; // 2 1 . 2 4 8 16 32 64 [8 values] - 3 bits
    std::uint8_t durationDetail = 0; // none, dot, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 [4 bits] //one of 2-15 means empty(together with pause only!)
    //1 bit determines pause
    bool isPaused;

    std::uint8_t dotted = 0; //0 1 2


    std::uint8_t effects = 0; //tremolo precene - 1bit, (upstrings, downstrings+x) - 2 bits, bookmark - 1 bit, notice (text or elst) - 1 bit,
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
        for (size_t i = 0; i < size(); ++i)
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

    Note *getNote(int string) {
        for (size_t i = 0; i < size(); ++i)
            if (at(i)->getStringNumber()==string)
                return at(i).get();
        return nullptr;
    }


    void setFret(std::uint8_t fret, int string)
    {

        if (size() == 0) {
            auto newNote = std::make_unique<Note>();
            newNote->setFret(fret);
            newNote->setStringNumber(string);
            newNote->setState(0);
            push_back(std::move(newNote));
            setPause(false);
            return;
        }

        for (size_t i = 0; i < size(); ++i)
        {
            if (at(i)->getStringNumber()==string)
            {
                at(i)->setFret(fret);
                return; //function done
            }

            if (at(i)->getStringNumber() > string)
            {
                auto newNote = std::make_unique<Note>();
                newNote->setFret(fret);
                newNote->setStringNumber(string);
                newNote->setState(0);


                insertBefore(std::move(newNote),i);
                return;
            }

        }


        int lastStringN = at(size()-1)->getStringNumber();
        if (lastStringN < string)
        {
            auto newNote = std::make_unique<Note>();
            newNote->setFret(fret);
            newNote->setStringNumber(string);
            newNote->setState(0);

            //DEFAULT NOTE VALUES??
            push_back(std::move(newNote));
            return;
        }
        //we got here - that means we need insert fret
    }


    std::uint8_t getFret(int string)
    {
        if (size() == 0)
            return 255;

        for (size_t i = 0; i < size(); ++i)
            if (at(i)->getStringNumber()==string)
            {
                std::uint8_t fretValue = at(i)->getFret();
                return fretValue; //function done
            }

        return 255;
    }

    void setDuration(std::uint8_t dValue) { duration = dValue; }
    void setDurationDetail(std::uint8_t dValue) {	durationDetail = dValue; }
    void setDotted(std::uint8_t dottedValue) { dotted = dottedValue; }

    std::uint8_t getDuration() { return duration;}
    std::uint8_t getDurationDetail () { return durationDetail; }
    std::uint8_t getDotted() { return dotted; }

    void setEffects(Effect eValue);// { effects = eValue; }
    ABitArray getEffects();// { return effects; }

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
