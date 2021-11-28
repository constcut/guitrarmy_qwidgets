#ifndef TRACK_H
#define TRACK_H

#include "types.h"

#include <memory.h> //TODO remove
#include <unordered_map>
#include "abitarray.h"
#include "tabcommands.h" //move into g0

#include "bar.h"


class Tab;
class Track;
class Bar;
class Beat;
class Note;



class GuitarTuning
{
    byte stringsAmount;
    byte tunes[10]; //as a maximum, mind later is it usefull to try set more or less by container?
    //set it to byte - in fact int would be 128 values of std midi - next could be used as quatones

    public:

    void setStringsAmount(byte amount) { stringsAmount = amount; }
    byte getStringsAmount() { return stringsAmount; }
    void setTune(byte index, byte value) { if (index <= 10) tunes[index] = value; } //(index >= 0) &&
    byte getTune(byte index) { if (index <= 10) return tunes[index]; return 0; }
};


class PolyBar : public ChainContainer<Bar*, Tab>
{
  public:
    PolyBar()
    {
    }
    virtual ~PolyBar()
    {
    }
};


class Track : public ChainContainer<Bar*, Tab>
{
public:

    Track():timeLoop(),pan(0),drums(false),status(0), _cursor(0),_cursorBeat(0),_stringCursor(0),
    _displayIndex(0),_lastSeen(0),_selectCursor(-1), _digitPress(-1) {
        GpCompInts[3]=24; //REFACT GCOMP
        _selectionBarFirst=-1;
        _selectionBarLast=-1;
        _selectionBeatFirst=-1;
        _selectionBeatLast=-1;
    }

    virtual ~Track() {
        for (ul i=0; i < len(); ++i)
            delete at(i);
    }

    void printToStream(std::ostream &stream);

    ChainContainer<Bar*, Tab> timeLoop; //PolyBar //REFACT access
    std::vector<ul> timeLoopIndexStore;

    Track &operator=([[maybe_unused]]Track another)
    {
        //clone(another); //TODO
        return *this;
    }

    virtual void add(Bar*& val)
    {
        if (val){
            val->setParent(this);
            ChainContainer<Bar*, Tab>::add(val);
        }
    }

    virtual void insertBefore(Bar* &val, int index=0)
    {
        if (val)
        {
            val->setParent(this);
        ChainContainer<Bar*, Tab>::insertBefore(val,index);
        }
    }

protected:
    std::string name;
    ul instrument;
    ul color;
    byte pan; //or int??
    byte volume;
    bool drums;
    //?own temp bpm
    ul GpCompInts[4]; //GpComp - Port,Channel,ChannelE,Capo
    ul beatsAmount;
    byte status; //0 - none 1 - mute 2 - soloe

public:
    GuitarTuning tuning;

    ul connectBars();
    ul connectBeats();
    ul connectNotes(); //for let ring
    ul connectTimeLoop();
    void connectAll();

    void pushReprise(Bar *beginRepeat, Bar *endRepeat,
                     Bar *preTail, Bar *tailBegin, Bar *tailEnd, ul beginIndex, ul endIndex,
                     ul preTailIndex=0, ul tailBeginIndex=0, ul tailEndIndex=0);

    void setName( std::string &nValue) { name = nValue; }
    std::string getName() { return name; } //or return?

    void setInstrument(ul iValue) { instrument = iValue;}
    ul getInstrument() { return instrument; }

    void setColor(ul cValue) { color = cValue; }
    ul getColor() { return color; }

    void setPan(byte pValue) { pan = pValue; }
    byte getPan() { return pan; }

    void setVolume(byte vValue) { volume = vValue; }
    byte getVolume() { return volume; }


    void setGPCOMPInts(size_t index, ul value) { GpCompInts[index] = value; }
    ul getGPCOMPInts(size_t index) { return GpCompInts[index]; } //TODO get rid

    void setDrums(bool newDrums) {
        drums = newDrums;
        if (drums) GpCompInts[3]=99; //refact
    }

    bool isDrums() { return drums; }

    byte getStatus() { return status; } //refact name
    void setStatus(byte newStat) { status = newStat; }

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
    size_t& cursor() { return _cursor; } //TODO после всего рефакторинга обязаны быть const
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
    void switchEffect(int effIndex);
    void switchBeatEffect(int effIndex);
    void switchNoteState(byte changeState);
    void reverseCommand(SingleCommand &command);

    std::vector<SingleCommand> commandSequence;

    void onTrackCommand(TrackCommand command);

    void gotoTrackStart();

    void moveSelectionLeft();
    void moveSelectionRight();
    void insertBar();
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
    void setBarSign(int num, int denom);
    void changeBarSigns(int num, int denom);

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

private:
    //TODO внимательно проверить
    std::unordered_map<TrackCommand, void (Track::*)()> handlers =  {
        {TrackCommand::GotoStart, &Track::gotoTrackStart},
        {TrackCommand::SelectionExpandLeft, &Track::moveSelectionLeft},
        {TrackCommand::SelectionExpandRight, &Track::moveSelectionRight},
        {TrackCommand::InsertBar, &Track::insertBar},
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
        {TrackCommand::Past, &Track::clipboardPaste},
        {TrackCommand::Undo, &Track::undoOnTrack},
        {TrackCommand::SaveFile, &Track::saveFromTrack}};
};

#endif // TRACK_H
