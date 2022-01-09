#ifndef BEAT_H
#define BEAT_H

#include <memory.h>  //TODO find modern way
#include <vector>
#include <string>

#include "Types.hpp"
#include "Note.hpp"


namespace gtmy {


    class Bar;

    class Beat : public ChainContainer<Note, Bar> {

    public:
        Beat() = default;
        virtual ~Beat() = default;

        void printToStream(std::ostream &stream) const;

        struct SingleChange {
            std::uint8_t changeType;
            size_t changeValue;
            std::uint8_t changeCount;
        };


        class ChangesList : public std::vector<SingleChange> {
            //search functions
        };


        struct ChordDiagram { //used for compatibility
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
        } _chordDiagram;


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

    private:

        std::uint8_t _duration = 0; // 2 1 . 2 4 8 16 32 64 [8 values] - 3 bits
        std::uint8_t _durationDetail = 0; // none, dot, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 [4 bits] //one of 2-15 means empty(together with pause only!)
        //1 bit determines pause
        bool _isPaused;

        std::uint8_t _dotted = 0; //0 1 2

        std::string _bookmarkName;
        std::string _noticeText;

    public:

        ChangesList changes; //TODO
        BendPoints tremolo;

        //SET GET operations
        void setPause(bool pause) {_isPaused = pause;}
        bool getPause() const { return _isPaused; }

        void deleteNote(int string) {
            for (size_t i = 0; i < size(); ++i) {
                if (string == at(i)->getStringNumber()) {
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


        void setFret(std::uint8_t fret, int string) {
            if (size() == 0) {
                auto newNote = std::make_unique<Note>();
                newNote->setFret(fret);
                newNote->setStringNumber(string);
                newNote->setState(0);
                push_back(std::move(newNote));
                setPause(false);
                return;
            }

            for (size_t i = 0; i < size(); ++i) {
                if (at(i)->getStringNumber()==string) {
                    at(i)->setFret(fret);
                    return; //function done
                }
                if (at(i)->getStringNumber() > string) {
                    auto newNote = std::make_unique<Note>();
                    newNote->setFret(fret);
                    newNote->setStringNumber(string);
                    newNote->setState(0);
                    insertBefore(std::move(newNote),i);
                    return;
                }
            }

            int lastStringN = at(size()-1)->getStringNumber();
            if (lastStringN < string) {
                auto newNote = std::make_unique<Note>();
                newNote->setFret(fret);
                newNote->setStringNumber(string);
                newNote->setState(0);
                push_back(std::move(newNote));
                return;
            }
        }

        std::uint8_t getFret(int string) const {
            if (size() == 0)
                return 255;
            for (size_t i = 0; i < size(); ++i)
                if (at(i)->getStringNumber()==string) {
                    std::uint8_t fretValue = at(i)->getFret();
                    return fretValue;
                }
            return 255;
        }

        void setDuration(std::uint8_t dValue) { _duration = dValue; }
        void setDurationDetail(std::uint8_t dValue) {	_durationDetail = dValue; }
        void setDotted(std::uint8_t dottedValue) { _dotted = dottedValue; }

        std::uint8_t getDuration() const { return _duration;}
        std::uint8_t getDurationDetail () const { return _durationDetail; }
        std::uint8_t getDotted() const { return _dotted; }

        void setEffects(Effect eValue);
        ABitArray getEffects();

        //TREMOLO missing

        //боюс-боюс TODO
        void setChordDiagram(char* area) { memcpy(&_chordDiagram, area, sizeof(_chordDiagram)); }
        bool getChordDiagram(char* to) const { memcpy(to, &_chordDiagram, sizeof(_chordDiagram)); return true; }

        void setText(std::string& value) { _noticeText = value; }
        void getText(std::string& value) const { value = _noticeText; }

        void clone(Beat *from);
    };


}

#endif // BEAT_H
