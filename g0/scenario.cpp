#include "g0/types.h"

#include <iostream>


#include "g0/midifile.h"
#include "g0/tab.h"
#include "g0/gtpfiles.h"
#include "g0/aconfig.h"

#include "g0/aexpimp.h"

#include <QMediaPlayer>

#include <fstream>
#include <QDebug>

#define log qDebug()

#ifdef WIN32
#include <windows.h>
#else

#endif

//#include <strstream>




//configuration connectors

extern bool gtpLog;
extern bool midiLog;
extern bool museLog;
extern bool tabLog;


//WINDOWS MIDI playback
/*
UINT playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName, UINT replay=0)
{
    UINT wDeviceID;
    DWORD dwReturn;
    MCI_OPEN_PARMS mciOpenParms;
    MCI_PLAY_PARMS mciPlayParms;
    MCI_STATUS_PARMS mciStatusParms;
    MCI_SEQ_SET_PARMS mciSeqSetParms;

    HWND hMainWnd=0;

    // Open the device by specifying the device and filename.
    // MCI will attempt to choose the MIDI mapper as the output port.

    std::wstring ws1;

    std::string fileMidi = "D:/TestFiles/new/gp5 test.mid";
    ws1 = L"D:/TestFiles/new/gp5 test.mid";

    //std::copy(ws1.begin(),ws1.end(),fileMidi.begin());

    mciOpenParms.lpstrDeviceType = L"sequencer";
    mciOpenParms.lpstrElementName = ws1.c_str();//; //lpszMIDIFileName;

    //mciOpenParms.


    if (dwReturn = mciSendCommand(NULL, MCI_OPEN,
        MCI_OPEN_TYPE | MCI_OPEN_ELEMENT ,
        (DWORD)(LPVOID) &mciOpenParms))
    {
        // Failed to open device. Don't close it; just return error.
        std::cout << "failed to open;"<<dwReturn;
        //return (dwReturn);
    }

    // The device opened successfully; get the device ID.
    wDeviceID = mciOpenParms.wDeviceID;

    if (replay)
    {
        wDeviceID = replay;

        mciPlayParms.dwCallback = (DWORD) hWndNotify;

        if (dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_WAIT,
            (DWORD)(LPVOID) &mciPlayParms))
        {
            mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, NULL);
            return (wDeviceID);
        }
    }

    // Check if the output port is the MIDI mapper.
    mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;
    if (dwReturn = mciSendCommand(wDeviceID, MCI_STATUS,
        MCI_STATUS_ITEM, (DWORD)(LPVOID) &mciStatusParms))
    {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        std::cout << "failed to check stat;"<<dwReturn;

        return (dwReturn);
    }

    // The output port is not the MIDI mapper.
    // Ask if the user wants to continue.
    if (LOWORD(mciStatusParms.dwReturn) != MIDI_MAPPER)
    {
        / *
        if (MessageBox(hMainWnd,
            "The MIDI mapper is not available. Continue?",
            "", MB_YESNO) == IDNO)
        {
            // User does not want to continue. Not an error;
            // just close the device and return.
            mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
            return (0L);
        }

    }

    // Begin playback. The window procedure function for the parent
    // window will be notified with an MM_MCINOTIFY message when
    // playback is complete. At this time, the window procedure closes
    // the device.

    / *
    mciPlayParms.dwCallback = (DWORD) hWndNotify;

    if (dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_WAIT, //MCI_WAIT
        (DWORD)(LPVOID) &mciPlayParms))
    {
        return (dwReturn);
    }

    mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, 0);

    return (wDeviceID);
}*/


void checks()
{
    std::cout << "Starting checks function [004]" <<std::endl;
   // bool typesFine = check_types();
    //AF ile afi le; //compile check           - no need for output
    MidiFile mfile;//compile check      - because if failed
    Tab tablature;
}


bool midiPrint(std::string fileName)
{
    std::ifstream ifile(fileName.c_str(), std::ifstream::binary);

    if (ifile.is_open())
        std::cout << "File opened" ;
    else
    {
        std::cout<<"Failed to open file" ;
        std::cout<<"Scenario done.";
        return false;
    }

    MidiFile midiFile;
    midiFile.readStream(ifile);
    std::cout << "Reading file finished";
    midiFile.printToStream(std::cout);
}

bool testMidi(std::string fileName, std::string outFileName)
{
    std::ifstream ifile(fileName.c_str(),std::ifstream::binary);

    if (ifile.is_open())
        log << "File opened" ;
    else
    {
        log <<"Failed to open file" ;
        log<<"Scenario done.";
        return false;
    }

    MidiFile midiFile;
    midiFile.readStream(ifile);
    log << "Reading file finished";
    midiFile.printToStream(std::cout);

    return true;
}

bool testGP3(std::string fileName, std::string outFileName, bool outputLog)
{
    std::ifstream itfile;
    itfile.open(fileName.c_str(),std::ifstream::binary);  //small file - runs ok!

    if (itfile.is_open())
        log << "File opened" ;
    else
    {
        log <<"Failed to open GP file" ;
        log<<"Scenario done.";
        return false;
    }

    Tab tab;
    Gp3Import importer; //(tabFile,tab);


    //swtich to different versions of importers 3\4\5

    importer.import(itfile,&tab);
    tab.connectTracks(); //new for chains refact

    std::cout << "All information about readin is in log"<<std::endl;

    return true;
}


bool testGP4(std::string fileName, std::string outFileName, bool outputLog)
{
    std::ifstream itfile;
    itfile.open(fileName.c_str(),std::ifstream::binary);  //small file - runs ok!

    if (itfile.is_open())
        log << "File opened" ;
    else
    {
        log <<"Failed to open GP file" ;
        log<<"Scenario done.";
        return false;
    }


    Tab tab;
    Gp4Import importer; //(tabFile,tab);
    //swtich to different versions of importers 3\4\5

    importer.import(itfile,&tab);
    tab.postGTP();
    tab.connectTracks(); //new for chains refact


    return true;
}

//refact later
bool testGP5(std::string fileName, std::string outFileName, bool outputLog)
{
    std::ifstream itfile;
    itfile.open(fileName.c_str(),std::ifstream::binary);  //small file - runs ok!

    if (itfile.is_open())
        log << "File opened" ;
    else
    {
        log <<"Failed to open GP file" ;
        log<<"Scenario done.";
        return false;
    }

    Tab tab;
    Gp5Import importer; //(tabFile,tab);


    //swtich to different versions of importers 3\4\5

    importer.import(itfile,&tab);
    tab.postGTP();
    tab.connectTracks(); //new for chains



    return true;
}

bool greatCheck()
{

    //phase 1
    int from = 1; //1
    int to = 109; //70

    int scen = 4;


    bool doTheLogs = false;

    for (int i = from; i <= to; ++i)
    {
        try
        {
            stringExtended newLine;
            newLine << scen << "." << i;

            std::string testLocation = getTestsLocation();

            std::string gp5File = testLocation + std::string("g5/") +std::string(newLine.c_str()) + std::string(".gp5");
            std::string gp4File = testLocation + std::string("g4/") +std::string(newLine.c_str()) + std::string(".gp4");
            std::string gp3File = testLocation + std::string("g3/") +std::string(newLine.c_str()) + std::string(".gp3");
            std::string outGp3 =  std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen3.mid");
            std::string outGp4 =  std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen4.mid");
            std::string outGp5 =  std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen5.mid");


            std::string midiFile = testLocation + std::string("m/") +std::string(newLine.c_str()) + std::string(".mid");
            std::string outMid = std::string("all_out/") + std::string(newLine.c_str()) + std::string("_amid.mid");

            log<<"TestFile "<<gp3File.c_str();

            //if ( testMidi(midiFile,outMid,log) == false ) return 0;

            //if ( testGP3 (gp3File,outGp3,log,doTheLogs)  == false ) return 0; //last true - no out

            //DONT FORGET
            //if ( testGP4 (gp4File,outGp4,log,doTheLogs)  == false ) return 0; //last true - no out
            if ( testGP5 (gp5File,outGp5,doTheLogs) == false ) return 0;

            std::cout << "done"<<std::endl;
        }
        catch(...)
        {
            std::cout <<"Exception";
        }
    }

    //2.1_midi 2.1_gen

    return true;
}

bool greatCheckScenarioCase(int scen, int from, int to, int v)
{
    bool doTheLogs = false;
    for (int i = from; i <= to; ++i)
    {
        try
        {
            stringExtended newLine;
            newLine << scen << "." << i;

            std::string testLocation = getTestsLocation();
            std::string gp5File = testLocation + std::string("g5/") +std::string(newLine.c_str()) + std::string(".gp5");
            std::string gp4File = testLocation + std::string("g4/") +std::string(newLine.c_str()) + std::string(".gp4");
            std::string gp3File = testLocation + std::string("g4/") +std::string(newLine.c_str()) + std::string(".gp4");


            std::string outGp5 =  std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen5.mid");
            std::string outGp4 =  std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen4.mid");
            std::string outGp4plus = testLocation +  std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen4p.mid");
            std::string outGp3 =  std::string("all_out/") + std::string(newLine.c_str()) + std::string("_gen3.mid");


            std::string midiFile = testLocation + std::string("m/") +std::string(newLine.c_str()) + std::string(".mid");
            std::string outMid = std::string("all_out/") + std::string(newLine.c_str()) + std::string("_amid.mid");

            log<<"TestFile "<<newLine.c_str();

            if (v==5)
            if ( testGP5 (gp5File,outGp5,doTheLogs) == false )
                log <<"Test gp5 "<<newLine.c_str()<<" failed";

            if (v==4)
            {
                if ( testGP4 (gp4File,outGp4,doTheLogs) == false )
                  log <<"Test gp4 "<<newLine.c_str()<<" failed";

                GTabLoader loader;
                //HERE IS LEAK
                if (loader.open(gp4File) == false)
                    log <<"G4+ failed file "<<newLine.c_str();
                MidiFile mid;
                mid.fromTab(loader.getTab());
                std::ofstream outMidiFile(outGp4plus);
                if (outMidiFile.is_open() == false)
                    log <<"failed to open for output "<<outGp4plus.c_str();
                mid.writeStream(outMidiFile);
                outMidiFile.close();

            }

            if (v==3)
            if ( testGP3 (gp3File,outGp3,doTheLogs) == false )
                log <<"Test gp3 "<<newLine.c_str()<<" failed";

            std::cout << "test file done"<<std::endl;
        }
        catch(...)
        {
            std::cout <<"Exception";
        }
    }

    //2.1_midi 2.1_gen

    return true;
}

void connectConfigs(void *ptr)
{
    AConfig *config = (AConfig*)ptr;
    config->connectLog(&tabLog,0,"tab");
    //config->connectLog(&museLog,1,"amusic");
    config->connectLog(&midiLog,2,"midi");
    config->connectLog(&gtpLog,3,"gtp");
}


bool testScenario()
{ 
    /*
    std::string logName; //LOG file name
#ifdef WIN32
    logName="D:/TestFiles/new/scene.txt";
#else
    logName="scene.txt"; //android
#endif

    checks(); //checks first

    std::ofstream lofile;
    lofile.open(logName.c_str());


    if (lofile.is_open())
      std::cout << "Log file opened."<<std::endl;
    else
    {
      std::cout << "Failed to open log out file :("<<std::endl;
      return false;
    }

    A F ile logFile(lofile);
    A S treaming::setLogFile(&logFile);*/
    //Log operation finished

    /////////////////////////////////////////////////////////////////
    //------------------------------------------------------------//
    /*/TEST FILE NAME
    std::string scenarioN = "3.2";
                             //4.103
    bool tuxMidi = true;
    bool useMidi =  false;
    //TEST OPTIONS FINISHED
    bool waitAfterMidi = useMidi;
    /////*////////////////////////

    //Full line check
    //UPDATE: greatCheck(Scen,start,end); greatestCheck - with all the ranges
    greatCheck();
    return false;


    //ALT WAY to open)
    /*
    std::string testLocation = getTestsLocation();
    std::string midiTestFile = testLocation + std::string("m/") + scenarioN + std::string(".mid");
    if (tuxMidi)
        midiTestFile = testLocation + std::string("m/") + scenarioN + std::string("_tux.mid");
    std::string gp4TestFile = testLocation + std::string("g4/") + scenarioN + std::string(".gp4");
    std::string gp5 TestFile = testLocation + std::string("g5/") + scenarioN + std::string(".gp5");

    if (useMidi)
    if ( testMidi(midiTestFile,"out.mid",log) == false ) return 0;

    if ( waitAfterMidi ) std::cin.get();

    //if ( testGP4 (gp4TestFile,"out2.mid",log, true)  == false ) return 0;

    if (!none.empty())
     testGP5 (none,"gp5 test.mid",log, true);
    else
    if ( testGP5 (gp5 TestFile,"gp5 test.mid",log, true)  == false ) return 0;

    //------------------------------------------------------------//


    //playMIDIFile(0,LPSTR("D:/TestFiles/new/gp5 test.mid"));
    //std::cout << "Press enter to exit";
    //std::cin.get();

    ///////////////////////////////////////////////////////////////

    return 0; //return a.exec();
    */
}
