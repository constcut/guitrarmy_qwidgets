#ifndef TABCLIPBOARD_H
#define TABCLIPBOARD_H


class Bar;


enum class ClipboardType {
    NotSet = -1,
    SingleBarCopy = 0,
    SingleBeatCopy = 1,
    BarsCopy = 2,
    BeatsCopy = 3,
    BarPointer = 4,
};


class AClipboard
{
protected:

    ClipboardType type;

    int track1 = -1, bar1 = -1, beat1 = -1;
    int track2 = -1, bar2 = -1, beat2 = -1;

    Bar* ptr;

    static AClipboard *currentClip; //TODO multiple (stack)

public:

    AClipboard():
        type(ClipboardType::NotSet), ptr(nullptr)
    {}

    static AClipboard* current() { return currentClip; }
    static void setCurrentClip(AClipboard* newClip) { currentClip=newClip; }


    void setClipboardType(ClipboardType newType) {
        type = newType;
        if (type == ClipboardType::NotSet)
            flush();
    }

    void flush() {
        track1 = bar1 = beat1 = -1;
        track2 = bar2 = beat2 = -1;
    }

    ClipboardType getClipboardType() { return type; }

    void setPtr(Bar *newPtr) { ptr=newPtr; }
    Bar* getPtr() { return ptr; }

    void setBeginIndexes(int track, int bar, int beat=-1)
    {
        track1 = track;
        bar1 = bar;
        if (beat != -1)
            beat1 = beat;
    }

    void setEndIndexes(int track, int bar, int beat=-1)
    {
        track2 = track;
        bar2 = bar;
        if (beat != -1)
            beat2 = beat;
    }

    int getTrackIndex() { return track1; }
    int getBarIndex() { return bar1; }
    int getBeatIndex() { return beat1; }


    int getSecondBarI() { return track2; }
    int getSecondBeatI() { return bar2; }
    int getSecondTrackI() { return beat2; }
};



#endif // TABCLIPBOARD_H
