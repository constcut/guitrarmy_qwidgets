#include "midiengine.h"
#include "g0/config.h"

#include <QMediaPlayer>

#include <sstream>

#include <QDebug>

//TODO


#define min_DEF(a,b) a>b ? b:a;
#define max_DEF(a,b) a>b ? a:b;

MidiEngine *MidiEngine::inst=0;

#ifdef WIN32
         HMIDIOUT MidiEngine::winMidi;
         UINT MidiEngine::wTimerRes;
#endif


         bool MidiEngine::opened=false;



//Old functions


#ifndef WIN32
QMediaPlayer midiPlayer;

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

    //if (midiPlayer)
    midiPlayer.play();


#endif

}

void MidiEngine::stopDefaultFile()
{
    if (opened)
        freeInitials();

#ifdef WIN32
   mciSendStringA("stop gMIDI",0,0,0);
#else
   //if (midiPlayer)
   midiPlayer.stop();
#endif

   /*
    *   mciSendString("PAUSE MP3","",0,0)
        mciSendString("RESUME MP3","",0,0) */
}

void MidiEngine::openDefaultFile()
{
    if (opened)
        freeInitials();

    
#ifdef WIN32
    mciSendStringA("close gMIDI",0,0,0);
#else
    //midiPlayer.setMedia(QUrl::fromLocalFile("/sdcard/p/tests/midiOutput.mid"));

    //if (midiPlayer==nullptr)
        //midiPlayer = std::make_unique<QMediaPlayer>();

    std::string command = std::string(AConfig::getInstance().globals.testsLocation) + "midiOutput.mid";
    QString playerPath = command.c_str(); // "/sdcard/p/tests/midiOutput.mid";
    midiPlayer.setMedia(QUrl::fromLocalFile(playerPath));
    return;

#endif
    //char locationD[] = "C:\\Qt\\Qt5.3.1\\Tools\\QtCreator\\bin\\build-g1UI-Desktop_Qt_5_4_1_MinGW_32bit-Debug\\debug\\tests\\";

 #ifdef WIN32
 std::stringstream command;
     command<<"open \""<<AConfig::getInstance().globals.invertedLocation<<"midiOutput.mid\" type sequencer alias gMIDI";

    mciSendStringA(command.str().c_str(),0,0,0);
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
          qDebug()<<"error opening default MIDI device: "<<(int)err;
       }
       else
           qDebug()<<"successfully opened default MIDI device";


#define TimerResolution 1

       TIMECAPS tc;


       if (timeGetDevCaps(&tc,sizeof(TIMECAPS)) != TIMERR_NOERROR)
       {
           //can't continue
           qDebug() << "MM timer critical error";
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
    //qDebug()<<"closed default MIDI device";
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

    for (size_t i = 0; i < track->size(); ++i)
    {
        auto& sig = track->at(i);

        size_t absoluteTime = 10; //sig->time.getValue()/
        //ul waitTime = absoluteTime

        //need right time

        if (sig->byte0!=0xff)
            sendSignalShortDelay(absoluteTime,
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

void MidiEngine::sendSignalShort(std::uint8_t status, int byte1, int byte2)
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
    qDebug() << "Midi cb begin "<<dwUser;
    MidiEngine::sendSignalShortWin(dwUser);
    qDebug() << "Midi cb end "<<dwUser;
}


void MidiEngine::sendSignalShortWin(DWORD signal)
{
    if (opened==false)
        init();

    MMRESULT fResult = midiOutShortMsg(winMidi,signal);
}

#endif

void MidiEngine::sendSignalShortDelay( int msdelay, std::uint8_t status, int byte1, int byte2)
{
    if (opened==false)
        init();

#ifdef WIN32

    DWORD signal = status;

    if (byte1!=-1)
        signal += byte1<<8;
    if (byte2!=-1)
        signal +=byte2<<16;


    qDebug()<<"Pushing signal "<<signal<<"for ms delay "<<msdelay;
    qDebug()<<"Sinal parts "<<byte1<<" "<<byte2<<"; "<<status;

    if (timeSetEvent(msdelay, wTimerRes, Midi_Callback_Win,signal,TIME_ONESHOT) ==  0)
    {
        qDebug() <<"failed to start mmtimer";
    }
    else
        qDebug() << "Timer was set for midi event";


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
    size_t timeA = a->absoluteTime; //GET ABS!!
    size_t timeB = b->absoluteTime;

    return timeA>timeB;
}

std::unique_ptr<MidiTrack> MidiEngine::uniteFileToTrack(MidiFile *midiFile)
{
    //1 makes all signals global counters apended ul(eats mem but helps alot)
    //2 put all together
    //3 sort by global
    //4 trace - group by same time

    std::vector<MidiSignal*> allSignals;

    for (size_t trackI = 0; trackI < midiFile->size(); ++trackI)
    {
        auto& track = midiFile->at(trackI);
        size_t absTimeShift =0;
        for (size_t sigI = 0; sigI < track->size(); ++sigI)
        {
            auto& sig = track->at(sigI);

            size_t signalTimeShift = sig->timeStamp.getValue();
            absTimeShift += signalTimeShift;

            sig->absoluteTime = absTimeShift;

            allSignals.push_back(sig.get());
        }
    }

    std::sort(allSignals.begin(),allSignals.end(),midiAbsSortFunction);

    //repair local position from abs

    auto result = std::make_unique<MidiTrack>();

    size_t lastGlobalAbs = 0;
    for (int sigI = allSignals.size()-1; sigI >= 0; --sigI)
    {
            MidiSignal *sig = allSignals[sigI];
            size_t currentAbs = sig->absoluteTime;
            size_t shift = currentAbs - lastGlobalAbs;
            auto signalCopy = std::make_unique<MidiSignal>(sig->byte0,
                                                    sig->param1,
                                                    sig->param2,shift);

            signalCopy->absoluteTime = sig->absoluteTime;
            if (sig->byte0==0xff) {
                signalCopy->metaBufer = sig->metaBufer;
                signalCopy->metaLen = sig->metaLen;
             }//attention

            lastGlobalAbs = currentAbs;
            result->push_back(std::move(signalCopy));
    }
    qDebug() << "Produced midi track with "<<(int)result->size()<<" elements";
    return result;
}

