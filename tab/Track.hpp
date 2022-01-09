#ifndef TRACK_H
#define TRACK_H

#include "Types.hpp"
#include <unordered_map>
#include "Commands.hpp" //move into g0
#include "Bar.hpp"


namespace gtmy {

    class Tab;

    class GuitarTuning {
        std::uint8_t stringsAmount;
        std::uint8_t tunes[10]; //TODO vector + at, to expeption and avoid check
        //set it to byte - in fact int would be 128 values of std midi - next could be used as quatones
    public:

        void setStringsAmount(std::uint8_t amount) { stringsAmount = amount; }
        std::uint8_t getStringsAmount() const { return stringsAmount; }

        void setTune(std::uint8_t index, std::uint8_t value) { if (index <= 10) tunes[index] = value; } //(index >= 0) &&
        std::uint8_t getTune(std::uint8_t index) const { if (index <= 10) return tunes[index]; return 0; }
    };


    class ChainedBars : public std::vector<Bar*> {
      public:
        ChainedBars() = default;
        virtual ~ChainedBars() = default;
    };

    enum class NoteStates {
        Leeg = 2,
        Dead = 3,
    };


    class Track : public ChainContainer<Bar, Tab>
    {
    public:

        Track():timeLoop(),_pan(0),_drums(false),_status(0), _cursor(0),_cursorBeat(0),_stringCursor(0),
        _displayIndex(0),_lastSeen(0),_selectCursor(-1), _digitPress(-1) {
            _midiInfo[3]=24; //REFACT GCOMP
            _selectionBarFirst=-1;
            _selectionBarLast=-1;
            _selectionBeatFirst=-1;
            _selectionBeatLast=-1;
        }

        virtual ~Track() = default;

        void printToStream(std::ostream &stream);

        std::vector<Bar*> timeLoop;
        std::vector<size_t> timeLoopIndexStore;

        Track &operator=([[maybe_unused]]Track another)
        {
            //clone(another); //TODO
            return *this;
        }

        virtual void push_back(std::unique_ptr<Bar> val)
        {
            if (val){
                val->setParent(this);
                ChainContainer<Bar, Tab>::push_back(std::move(val));
            }
        }

        virtual void insertBefore(std::unique_ptr<Bar> val, int index=0){
            if (val){
                val->setParent(this);
                ChainContainer<Bar, Tab>::insertBefore(std::move(val),index);
            }
        }

    private:

        std::string _name;
        size_t _instrument; //TODO enum
        size_t _color; //TODO другой тип
        std::uint8_t _pan;
        std::uint8_t _volume;
        bool _drums;

        size_t _midiInfo[4]; //Port,Channel,ChannelE,Capo TODO structure
        size_t _beatsAmount;
        std::uint8_t _status; //0 - none 1 - mute 2 - soloe

    public:
        GuitarTuning tuning;

        size_t connectBars();
        size_t connectBeats();
        size_t connectNotes(); //for let ring
        size_t connectTimeLoop();
        void connectAll();

        void pushReprise(Bar *beginRepeat, Bar *endRepeat,
                         Bar *preTail, Bar *tailBegin, Bar *tailEnd, size_t beginIndex, size_t endIndex,
                         size_t preTailIndex=0, size_t tailBeginIndex=0, size_t tailEndIndex=0);

        void setName( std::string &nValue) { _name = nValue; }
        std::string getName() const { return _name; }

        void setInstrument(size_t iValue) { _instrument = iValue;}
        size_t getInstrument() const { return _instrument; }

        void setColor(size_t cValue) { _color = cValue; }
        size_t getColor() const { return _color; }

        void setPan(std::uint8_t pValue) { _pan = pValue; }
        std::uint8_t getPan() const { return _pan; }

        void setVolume(std::uint8_t vValue) { _volume = vValue; }
        std::uint8_t getVolume() const { return _volume; }


        void setMidiInfo(size_t index, size_t value) { _midiInfo[index] = value; }
        size_t getMidiInfo(size_t index) { return _midiInfo[index]; } //TODO get rid

        void setDrums(bool newDrums) {
            _drums = newDrums;
            if (_drums) _midiInfo[3]=99;
        }

        bool isDrums() { return _drums; }

        std::uint8_t getStatus() { return _status; }
        void setStatus(std::uint8_t newStat) { _status = newStat; }

    protected:
        size_t _cursor;
        size_t _cursorBeat;
        size_t _stringCursor;
        size_t _displayIndex;
        size_t _lastSeen;
        int _selectCursor; //TOSO to size_t
        int _selectionBarFirst;
        int _selectionBarLast;
        int _selectionBeatFirst;
        int _selectionBeatLast;
        int _digitPress;


    public:
        size_t& cursor() { return _cursor; } //TODO после всего рефакторинга обязаны быть const (нужны для комманд)
        size_t& cursorBeat() { return _cursorBeat; }
        size_t& stringCursor() { return _stringCursor; }
        size_t& displayIndex() { return _displayIndex; }
        size_t& lastSeen() { return _lastSeen; }
        int& selectCursor() { return _selectCursor;}
        int& selectBarFirst() { return _selectionBarFirst;}
        int& selectBarLast() { return _selectionBarLast;}
        int& selectBeatFirst() { return _selectionBeatFirst;}
        int& selectBeatLast() { return _selectionBeatLast;}
        int& digitPress() { return _digitPress; }


        //TODO command handler
        void switchEffect(Effect effect);
        void switchBeatEffect(Effect effect);
        void switchNoteState(NoteStates changeState);
        void reverseCommand(ReversableCommand command);

        std::vector<ReversableCommand> commandSequence;

        void onTrackCommand(TrackCommand command);

        void gotoTrackStart();

        void moveSelectionLeft();
        void moveSelectionRight();
        void insertBar();
        void insertNewPause();
        void moveToNextBar();
        void moveToPrevBar();
        void moveToPrevPage();
        void moveToNextPage();
        void moveToNextTrack();
        void moveToPrevTrack();
        void moveToStringUp();
        void moveToStringDown();
        void moveToPrevBeat();
        void moveToNextBeat();
        void setTrackPause();
        void deleteBar();
        void deleteSelectedBars();
        void deleteSelectedBeats();
        void deleteNote();
        void incDuration();
        void decDuration();
        void saveFromTrack();
        void newBar();
        void setDotOnBeat();
        void setTriolOnBeat();
        //void setBendOnNote(); //TODO + changes + chord

        void setTextOnBeat(std::string newText);
        void setBarSign(size_t num, size_t denom);
        void changeBarSigns(size_t num, size_t denom);

        void clipboardCopyBar(); //refact name
        void clipboarCopyBeat();
        void clipboardCopyBars();
        void clipboardCutBar();
        void clipboardPaste();
        void undoOnTrack();


        void LeegNote();
        void DeadNote();
        void Vibratto();
        void Slide();
        void Hammer();
        void LetRing();
        void PalmMute();
        void Harmonics();
        void TremoloPicking();
        void Trill();
        void Stokatto();
        void FadeIn(); //Todo fade out
        void Accent();
        void HeavyAccent();
        void UpStroke();
        void DownStroke();


        void playCommand(MacroCommand& command) {
            //TODO Возможно тут же обрабатывать и единичную комманду?
            if (std::holds_alternative<TwoIntCommand<TrackCommand>>(command)) {
                auto paramCommand = std::get<TwoIntCommand<TrackCommand>>(command);
                if (twoIntHandlers.count(paramCommand.type))
                    (this->*twoIntHandlers.at(paramCommand.type))(paramCommand.parameter1, paramCommand.parameter2);
            } else if (std::holds_alternative<StringCommand<TrackCommand>>(command)) {
                auto paramCommand = std::get<StringCommand<TrackCommand>>(command);
                if (stringHandlers.count(paramCommand.type))
                    (this->*stringHandlers.at(paramCommand.type))(paramCommand.parameter);
            }
        }


    private:
        //TODO внимательно проверить
        std::unordered_map<TrackCommand, void (Track::*)()> handlers =  {
            {TrackCommand::GotoStart, &Track::gotoTrackStart},
            {TrackCommand::SelectionExpandLeft, &Track::moveSelectionLeft},
            {TrackCommand::SelectionExpandRight, &Track::moveSelectionRight},
            {TrackCommand::InsertBar, &Track::insertBar},
            //{TrackCommand::InsertNewPause, &Track::insertNewPause}, //TODO
            {TrackCommand::NextBar, &Track::moveToNextBar},
            {TrackCommand::PrevBar, &Track::moveToPrevBar},
            {TrackCommand::PrevPage, &Track::moveToPrevPage},
            {TrackCommand::NextTrack, &Track::moveToNextTrack},
            {TrackCommand::PrevTrack, &Track::moveToPrevTrack},
            {TrackCommand::StringDown, &Track::moveToStringDown},
            {TrackCommand::StringUp, &Track::moveToStringUp},
            {TrackCommand::PrevBeat, &Track::moveToPrevBeat},
            {TrackCommand::NextBeat, &Track::moveToNextBeat},
            {TrackCommand::SetPause, &Track::setTrackPause},
            {TrackCommand::DeleteBar, &Track::deleteBar},
            {TrackCommand::DeleteSelectedBars, &Track::deleteSelectedBars},
            {TrackCommand::DeleteSelectedBeats, &Track::deleteSelectedBeats},
            {TrackCommand::IncDuration, &Track::incDuration},
            {TrackCommand::DecDuration, &Track::decDuration},
            {TrackCommand::NewBar, &Track::newBar},
            {TrackCommand::SetDot, &Track::setDotOnBeat},
            {TrackCommand::SetTriole, &Track::setTriolOnBeat},
            {TrackCommand::Leeg, &Track::LeegNote},
            {TrackCommand::Dead, &Track::DeadNote},
            {TrackCommand::Vibrato, &Track::Vibratto},
            {TrackCommand::Slide, &Track::Slide},
            {TrackCommand::Hammer, &Track::Hammer},
            {TrackCommand::LetRing, &Track::LetRing},
            {TrackCommand::PalmMute, &Track::PalmMute},
            {TrackCommand::Harmonics, &Track::Harmonics},
            {TrackCommand::TremoloPickings, &Track::TremoloPicking},
            {TrackCommand::Trill, &Track::Trill},
            {TrackCommand::Stokatto, &Track::Stokatto},
            {TrackCommand::FadeIn, &Track::FadeIn},
            {TrackCommand::Accent, &Track::Accent},
            {TrackCommand::HeaveAccent, &Track::HeavyAccent},
            {TrackCommand::UpStroke, &Track::UpStroke},
            {TrackCommand::DownStroke, &Track::DownStroke},
            {TrackCommand::Cut, &Track::clipboardCutBar},
            {TrackCommand::Copy, &Track::clipboardCopyBar},
            {TrackCommand::CopyBeat, &Track::clipboarCopyBeat},
            {TrackCommand::CopyBars, &Track::clipboardCopyBars},
            {TrackCommand::Paste, &Track::clipboardPaste},
            {TrackCommand::Undo, &Track::undoOnTrack},
            {TrackCommand::SaveFile, &Track::saveFromTrack}};


        std::unordered_map<TrackCommand, void (Tab::*)(size_t)> intHandlers =  {
        };

        std::unordered_map<TrackCommand, void (Track::*)(size_t, size_t)> twoIntHandlers =  {
            {TrackCommand::SetSignForSelected , &Track::changeBarSigns},
            {TrackCommand::SetBarSign , &Track::setBarSign}
        };

        std::unordered_map<TrackCommand, void (Track::*)(std::string)> stringHandlers =  {
            {TrackCommand::Text, &Track::setTextOnBeat},
        };

    };

}

#endif // TRACK_H
