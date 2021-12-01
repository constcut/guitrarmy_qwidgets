#ifndef TABCOMMANDS_H
#define TABCOMMANDS_H

#include <cstdint>
#include <vector>
#include <memory>
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



enum class ReversableCommand { //Если будет конфликт имён убрать в клас хранитель
    Empty = 0,
    SwitchEffectNote,
    SwitchEffectBeat,
    SetFret,
    SetDuration,
    SetDurationDetail,
    SetDot,
    SetPause,
    DeleteNote,
    Pasty,
    Cut,
    Intrument,
    Pan,
    Volume,
    Drums,
    Bpm,
    InsertNewBar,
    ChangeNoteState,
    InsertNewPause,
    SetSign,
    InsertBeat,
    DeleteBar,
    DeleteRangeOfBars,
    DeleteRangeOfBeats
};


//      ReversableCommand::



class Note;
class Bar;
class Beat;

class SingleCommand //TODO rename reversable
{ //Или вместо варианта общий класс от которого наследоваться
    ReversableCommand commandType;
    std::uint8_t commandValue;
    std::uint8_t commandValue2;

    std::uint8_t track;
    std::uint8_t bar;
    std::uint8_t beat;
    std::uint8_t string;

public:

    //TODO хранить не в каждой каммоманде, а только в особых, хранить комманды как variant
    using NotesBuffer = std::vector<std::unique_ptr<Note>>;
    std::unique_ptr<NotesBuffer> storedNotes;

    using BeatsBuffer = std::vector<std::unique_ptr<Beat>>;
    std::unique_ptr<BeatsBuffer> storedBeats;

    using BarsBuffer = std::vector<std::unique_ptr<Bar>>;
    std::unique_ptr<BarsBuffer> storedBars;


    void requestStoredNotes(){
        storedNotes = std::make_unique<NotesBuffer>();
    }

    SingleCommand() : commandType(ReversableCommand::Empty),
        commandValue(0),track(0),bar(0),beat(0),string(0)
    {}

    SingleCommand(ReversableCommand newType, std::uint8_t newValue=0):
        commandType(newType), commandValue(newValue), track(0), bar(0), beat(0), string(0)
    {}

    void setType(ReversableCommand newType) { commandType = newType; }
    ReversableCommand getType() { return commandType; }

    void setValue(std::uint8_t newValue) { commandValue = newValue; }
    std::uint8_t getValue() { return commandValue; }

    void setValue2(std::uint8_t newValue) { commandValue2 = newValue; }
    std::uint8_t getValue2() { return commandValue2; }

    void setPosition(std::uint8_t trackN, std::uint8_t barN, std::uint8_t beatN, std::uint8_t stringN=255) {
        track = trackN;
        bar = barN;
        beat = beatN;
        string = stringN;
    }

    std::uint8_t getTrackNum() { return track; }
    std::uint8_t getBarNum() { return bar; }
    std::uint8_t getBeatNum() { return beat; }
    std::uint8_t getStringNum() { return string; }
};

#endif // TABCOMMANDS_H
