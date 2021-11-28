#ifndef TABCOMMANDS_H
#define TABCOMMANDS_H

#include "g0/types.h"

//#include "g0/tab.h"

enum class TabCommand {
    SetSignTillEnd,
    SaveAs,
    Mute,
    Solo,
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    Drums,
    Instument,
    Panoram,
    Volume,
    Name,
    BPM,
    OpenTrack,
    NewTrack,
    DeleteTrack,
    PlayMidi,
    GenerateMidi,
    PauseMidi,
    AddMarker,
    OpenReprise,
    CloseReprise,
    GotoBar,
    Tune
};

enum class TrackCommand {
    PlayFromStart,
    GotoStart,
    SetSignForSelected,
    SelectionExpandLeft,
    SelectionExpandRight,
    InsertBar,
    NextBar,
    PrevBar,
    NextPage,
    PrevPage,
    NextTrack,
    PrevTrack,
    StringDown,
    StringUp,
    PrevBeat,
    NextBeat,
    SetPause,
    DeleteBar,
    DeleteSelectedBars,
    DeleteSelectedBeats,
    DeleteNote,
    IncDuration,
    DecDuration,
    PlayTrackMidi,
    SaveFile,
    SaveAsFromTrack,
    NewBar,
    SetDot,
    SetTriole,
    Leeg,
    Dead,
    Vibrato,
    Slide,
    Hammer,
    LetRing,
    PalmMute,
    Harmonics,
    TremoloPickings,
    Trill,
    Stokatto,
    FadeIn,
    Accent,
    HeaveAccent,
    Bend,
    Chord,
    Text,
    Changes,
    UpStroke,
    DownStroke,
    SetBarSign,
    Cut,
    Copy,
    CopyBeat,
    CopyBars,
    Past,
    Undo,
    //Next commands yet not implemented
    QuickOpen,
    Tapping,
    Slap,
    Pop,
    UpM,
    DownM,
    Figering
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

class Note;
class Bar;
class Beat;

class SingleCommand
{
    std::uint8_t commandType;

    std::uint8_t commandValue;
    std::uint8_t commandValue2;
    //position

    //yet set exceed - optimize to file format
    std::uint8_t track;
    int bar;
    int beat;
    std::uint8_t string; //note

public:

    //special store
    using NotesBuffer = std::vector<std::unique_ptr<Note>>;
    std::unique_ptr<NotesBuffer> storedNotes;

    std::unique_ptr<Beat> outerBeat;
    std::unique_ptr<Beat> outerBeatEnd;
    std::unique_ptr<Bar> outerBar;
    std::unique_ptr<Bar> outerBarEnd;

    //could be refacted
    Bar *startBar; //TODO найти разницу с outerBar \ END
    Bar *endBar;

    void requestStoredNotes(){
        storedNotes = std::make_unique<NotesBuffer>();
    }

    SingleCommand():commandType(0),commandValue(0),track(0),bar(0),beat(0),string(0),
        startBar(0),endBar(0)
    {}

    SingleCommand(std::uint8_t newType, std::uint8_t newValue=0):
        commandType(newType),commandValue(newValue),track(0),bar(0),beat(0),string(0),
        startBar(0),endBar(0)
    {
    }

    void setType(std::uint8_t newType) { commandType = newType; }
    std::uint8_t getType() { return commandType; }

    void setValue(std::uint8_t newValue) { commandValue = newValue; }
    std::uint8_t getValue() { return commandValue; }

    void setValue2(std::uint8_t newValue) { commandValue2 = newValue; }
    std::uint8_t getValue2() { return commandValue2; }


    void setPosition(std::uint8_t trackN, int barN, int beatN, std::uint8_t stringN=255)
    {
        track = trackN;
        bar = barN;
        beat = beatN;
        string = stringN;
    }

    std::uint8_t getTrackNum() { return track; }
    int getBarNum() { return bar; }
    int getBeatNum() { return beat; }
    std::uint8_t getStringNum() { return string; }

};

#endif // TABCOMMANDS_H
