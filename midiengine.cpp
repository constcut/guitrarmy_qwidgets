#include "midiengine.h"

#include "g0/astreaming.h"

#include <QMediaPlayer>



#define min_DEF(a,b) a>b ? b:a;
#define max_DEF(a,b) a>b ? a:b;

MidiEngine *MidiEngine::inst=0;

#ifdef WIN32
         HMIDIOUT MidiEngine::winMidi;
         UINT MidiEngine::wTimerRes;
#endif


         bool MidiEngine::opened=false;

static AStreaming logger("midi-eng");


//Old functions


#ifndef WIN32
QMediaPlayer *midiPlayer=0;

#endif


void MidiEngine::startDefaultFile()
{
    if (opened)
        freeInitials();

    //if (lastDev)
       // mciSendCommand(lastDev, MCI_CLOSE, 0, NULL); //close it

    //lastDev =  playMIDIFile(0,0,lastDev);
#ifdef WIN32
     mciSendStringA("play gMIDI",0,0,0);
#else
    //midiPlayer.play();

    if (midiPlayer)
    midiPlayer->play();


#endif

}

void MidiEngine::stopDefaultFile()
{
    if (opened)
        freeInitials();

#ifdef WIN32
   mciSendStringA("stop gMIDI",0,0,0);
#else
   if (midiPlayer)
   midiPlayer->stop();
#endif

   /*
    *   mciSendString("PAUSE MP3","",0,0)
        mciSendString("RESUME MP3","",0,0) */
}

void MidiEngine::openDefaultFile()
{
    if (opened)
        freeInitials();

    stringExtended command;
#ifdef WIN32
    mciSendStringA("close gMIDI",0,0,0);
#else
    //midiPlayer.setMedia(QUrl::fromLocalFile("/sdcard/p/tests/midiOutput.mid"));

    if (midiPlayer==0)
        midiPlayer = new QMediaPlayer();

    command <<getTestsLocation()<<"midiOutput.mid";
    QString playerPath = command.c_str(); // "/sdcard/p/tests/midiOutput.mid";
    midiPlayer->setMedia(QUrl::fromLocalFile(playerPath));
    return;

#endif
    //char locationD[] = "C:\\Qt\\Qt5.3.1\\Tools\\QtCreator\\bin\\build-g1UI-Desktop_Qt_5_4_1_MinGW_32bit-Debug\\debug\\tests\\";
    command<<"open \""<<getInvertedLocation()<<"midiOutput.mid\" type sequencer alias gMIDI";

 #ifdef WIN32
    mciSendStringA(command.c_str(),0,0,0);
#endif
}

void MidiEngine::closeDefaultFile()
{
    if (opened)
        freeInitials();

#ifdef WIN32
mciSendStringA("close gMIDI",0,0,0);
#endif
}


void MidiEngine::init()
{
#ifdef WIN32
     unsigned int err = midiOutOpen(&winMidi, 0, 0, 0, CALLBACK_NULL);
       if (err != MMSYSERR_NOERROR)
       {
          LOG(<<"error opening default MIDI device: "<<(int)err);
       }
       else
           logger<<"successfully opened default MIDI device";


#define TimerResolution 1

       TIMECAPS tc;


       if (timeGetDevCaps(&tc,sizeof(TIMECAPS)) != TIMERR_NOERROR)
       {
           //can't continue
           logger << "MM timer critical error";
       }

       UINT timeRes = max_DEF(tc.wPeriodMin,TimerResolution);
       wTimerRes = min_DEF(timeRes ,tc.wPeriodMax);

       //wTimerRes = TimerResolution;
       timeBeginPeriod(wTimerRes);


#endif
       opened = true;
}

void MidiEngine::freeInitials()
{
#ifdef WIN32
    midiOutClose(winMidi);
    //logger<<"closed default MIDI device";
#endif
    opened = false;
}

MidiEngine::MidiEngine()
{
    if (inst!=0) return;
    inst = this; //last one
    init();
}

MidiEngine::~MidiEngine()
{
    freeInitials();
}

void MidiEngine::playTrack(MidiTrack *track)
{
    if (opened==false)
        init();

    for (int i = 0; i < track->len(); ++i)
    {
        MidiSignal *sig = track->getV(i);

        ul absValue = 10; //sig->time.getValue()/
        //ul waitTime = absValue

        //need right time

        if (sig->byte0!=0xff)
            sendSignalShortDelay(absValue,
                        sig->byte0,sig->param1,
                            sig->param2);
    }
}

void MidiEngine::sendSignal(MidiSignal *signal)
{
    if (opened==false)
        init();

    //splits in two
    if (signal->byte0==0xff)
    {
        sendSignalLong(signal);
    }
    else
    {
        //always sets all?? check in file save
        sendSignalShort(signal->byte0,signal->param1,signal->param2);
    }
}

void MidiEngine::sendSignalShort(byte status, int byte1, int byte2)
{
    if (opened==false)
        init();

#ifdef WIN32

    DWORD signal = status;

    if (byte1!=-1)
        signal += byte1<<8;
    if (byte2!=-1)
        signal +=byte2<<16;

    MMRESULT fResult = midiOutShortMsg(winMidi,signal);

#endif
}

void MidiEngine::sendSignalLong(MidiSignal *signal)
{
    if (opened==false)
        init();

#ifdef WIN32

    MIDIHDR mH;
    //midiOutLongMsg(winMidi,mH,sizeof(mH));
    //its midi bufer

#endif
}


#ifdef WIN32

void Midi_Callback_Win(UINT uId, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
    LOG( << "Midi cb begin "<<(ul)dwUser);
    MidiEngine::sendSignalShortWin(dwUser);
    LOG( << "Midi cb end "<<(ul)dwUser);
}


void MidiEngine::sendSignalShortWin(DWORD signal)
{
    if (opened==false)
        init();

    MMRESULT fResult = midiOutShortMsg(winMidi,signal);
}

#endif

void MidiEngine::sendSignalShortDelay( int msdelay, byte status, int byte1, int byte2)
{
    if (opened==false)
        init();

#ifdef WIN32

    DWORD signal = status;

    if (byte1!=-1)
        signal += byte1<<8;
    if (byte2!=-1)
        signal +=byte2<<16;


    LOG(<<"Pushing signal "<<(ul)signal<<"for ms delay "<<msdelay);
    LOG(<<"Sinal parts "<<byte1<<" "<<byte2<<"; "<<status);

    if (timeSetEvent(msdelay, wTimerRes, Midi_Callback_Win,signal,TIME_ONESHOT) ==  0)
    {
        logger <<"failed to start mmtimer";
    }
    else
        logger << "Timer was set for midi event";


#endif
}

void MidiEngine::setVolume(int vol)
{
    if (opened==false)
        init();

#ifdef WIN32
    DWORD volToSet = vol;
    midiOutSetVolume(winMidi,&volToSet);
#endif
}

int MidiEngine::getVolume()
{
    if (opened==false)
        init();

    int response = 0;
#ifdef WIN32
    DWORD vol = 0;
    midiOutGetVolume(winMidi,&vol);
    response=vol;
#endif
    return response;
}

bool midiAbsSortFunction(MidiSignal *a, MidiSignal *b)
{
    ul timeA = a->absValue; //GET ABS!!
    ul timeB = b->absValue;

    return timeA>timeB;
}

MidiTrack *MidiEngine::uniteFileToTrack(MidiFile *midiFile)
{
    //1 makes all signals global counters apended ul(eats mem but helps alot)
    //2 put all together
    //3 sort by global
    //4 trace - group by same time

    std::vector<MidiSignal*> allSignals;

    for (int trackI = 0; trackI < midiFile->len(); ++trackI)
    {
        MidiTrack *track = midiFile->getV(trackI);
        ul absTimeShift =0;
        for (int sigI = 0; sigI < track->len(); ++sigI)
        {
            MidiSignal *sig = track->getV(sigI);

            ul signalTimeShift = sig->time.getValue();
            absTimeShift += signalTimeShift;

            sig->absValue = absTimeShift;

            allSignals.push_back(sig);
        }
    }

    std::sort(allSignals.begin(),allSignals.end(),midiAbsSortFunction);

    //repair local position from abs

    MidiTrack *result = new MidiTrack();

    ul lastGlobalAbs = 0;
    for (int sigI = allSignals.size()-1; sigI >= 0; --sigI)
    {
            MidiSignal *sig = allSignals[sigI];


            ul currentAbs = sig->absValue;
            ul shift = currentAbs - lastGlobalAbs;


            MidiSignal *signalCopy = new MidiSignal(sig->byte0,
                                                    sig->param1,
                                                    sig->param2,shift);

            signalCopy->absValue = sig->absValue;

            if (sig->byte0==0xff)
                signalCopy->metaStore = sig->metaStore; //attention


            lastGlobalAbs = currentAbs;

            result->add(signalCopy);

    }

    LOG( << "Produced midi track with "<<(int)result->len()<<" elements");

    return result;
}

