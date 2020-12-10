#ifndef MIDIENGINE_H
#define MIDIENGINE_H

#include "g0/midifile.h"

#ifdef WIN32
#include "windows.h" //lib mm
#endif

class MidiEngine
{
protected:
#ifdef WIN32
         static HMIDIOUT winMidi;
         static UINT wTimerRes;
#endif

         static bool opened;

static MidiEngine *inst;


public:

    static void init();
    static void freeInitials();

#ifdef WIN32
static void sendSignalShortWin(DWORD signal);
#endif

    MidiEngine();
    ~MidiEngine();

    static void sendSignal(MidiSignal *signal);
    static void sendSignalShort(byte status, int byte1=-1, int byte2=-1);
    static void sendSignalLong(MidiSignal *signal);

    static void sendSignalShortDelay(int msdelay, byte status, int byte1=-1, int byte2=-1);

    static void setVolume(int vol);
    static int getVolume();

    static void startDefaultFile();
    static void stopDefaultFile();
    static void openDefaultFile();
    static void closeDefaultFile();

    static void playTrack(MidiTrack *track);

    //For better futute
    /*
    void playFile(MidiFile *file);
    void playFile(std::string fileName);

    ///controls
    //set get volume for midi sequencer(overal volume in guitamy)

    void play(); void stop(); void pause();

    //int scroll(int) ??

    /////////////////// Transform functions
    */

    static MidiTrack *uniteFileToTrack(MidiFile *midiFile);
};

#endif // MIDIENGINE_H
