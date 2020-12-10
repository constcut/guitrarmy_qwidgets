#include "ptbfile.h"


#include "astreaming.h"

//temp action for bends

#include "g0/gtpfiles.h"

#include <cmath>


static AStreaming logger("ptbfile");

static char bufer[30000];

PtbFile::PtbFile()
{
}



bool PtbFile::loadString(AFile *file, std::string &strValue)
{


    int stringLen = 0;
    file->read(&stringLen,2);

    file->read(bufer,stringLen);
    bufer[stringLen] = 0;

    strValue = bufer;
    return true;
}

bool PtbFile::readString(AFile *file, std::string &strValue)
{
    //char bufer[20000]; //optimize

    int stringLen = 0;
    file->read(&stringLen,1);


    if (stringLen == 255)
    {
        stringLen = 0;
        file->read(&stringLen,2);
        LOG( <<"now becomes longer "<<stringLen);
        logger<<"lo";
    }


    file->read(bufer,stringLen);
    bufer[stringLen] = 0;

    strValue = bufer;
    return true;
}


int PtbFile::readCountItems(AFile *file)
{
    int countItems = 0;
    file->read(&countItems,2);

    if (countItems != 0)
    {
        int header = 0;
        file->read(&header,2);

        if (header == 0xffff)
        {
            short oneMore = 0;
            file->read(&oneMore,2);

            if (oneMore != 1)
                return -1;

            std::string some;
            loadString(file,some);

            LOG( << "Items string "<<some<<"; "<<countItems);
        }
    }

    return countItems;
}


bool PtbFile::loadFromFile(AFile *file, Tab *tab, bool skipVersion)
{
    //char bufer[8];

    if (skipVersion)
    {
        file->read(bufer,1);
    }
    else
    {
        file->read(bufer,4);
        bufer[4]=0;
    }

    short ver=0;
    file->read(&ver,2);

    //must be 4
    byte classif;
    file->read(&classif,1);

    LOG( << "Ptb file v="<<ver<<"; classification="<<classif);

    if (classif==0)
    {
        byte skipOne=0;
        file->read(&skipOne,1);

        std::string name;
        std::string interpret;
        readString(file,name);
        readString(file,interpret);


        byte releaseType = 0;
        file->read(&releaseType,1);

        LOG( << "N="<<name<<"; I="<<interpret<<"; rt="<<releaseType);

        if (releaseType == 0)
        {
            byte albumnType = 0;
            file->read(&albumnType,1);
            std::string albumn;
            readString(file,albumn);
            short year;
            file->read(&year,2);
            //readString(file,year);
            byte live=0;
            file->read(&live,1);

            LOG( <<"At="<<albumnType<<"; A="<<albumn<<"; y="<<year<<"; l="<<live);
        }
        else if (releaseType == 1)
        {
            std::string albumn;
            readString(file,albumn);
            byte live=0;
            file->read(&live,1);

            LOG( <<"A="<<albumn<<"; l="<<live);
        }
        else if (releaseType == 2)
        {
            std::string albumn;
            readString(file,albumn);

            short day=0,mon=0,year=0;
            file->read(&day,2);
            file->read(&mon,2);
            file->read(&year,2);

            LOG( <<"A="<<albumn<<" date " <<year<<"."<<mon<<"."<<day);
        }

        byte skipByte = 0;
        file->read(&skipByte,1);

        //logger <<"au ly flag "<<skipByte;

        if (skipByte == 0)
        {
            std::string author, liry;
            readString(file,author);
            readString(file,liry);

            LOG( << "Auth "<<author<<"; ly "<<liry<<"; skipb "<<skipByte);
        }

        std::string lastLines[7];

        for (int i = 0; i < 7; ++i)
        {
            readString(file,lastLines[i]);
            LOG( << "Last lines: "<<lastLines[i]);
        }

    }

    if (classif==1)
    {
        std::string name;
        //std::string interpret;
        readString(file,name);
        //loadString(file,interpret);
        std::string albumn;
        readString(file,albumn);

        short style = 0;
        byte level = 0;
        file->read(&style,2);
        file->read(&level,1);

        std::string lastLines[3];

        for (int i = 0; i < 3; ++i)
            readString(file,lastLines[i]);

    }

    //READ 2 tracks?
    ///=======================

    logger << "readInstruments data 1";


    readInstrumentsData(file,tab,0);

    logger << "readInstruments data 2";


    readInstrumentsData(file,tab,tab->len());
    //one more?

    return true;
}


void PtbFile::readInstrumentsData(AFile *file, Tab *tab, int shift)
{
    short skipShort = 0;

    logger<<"Reading instruments info";

    int trackInfoCount = readCountItems(file);

    if (trackInfoCount != 0)  LOG( << "Track items "<<trackInfoCount); //return back

    if (trackInfoCount > 10)
    {
        logger << "Attention with tracks";
    }

    std::vector<PtbTempo> tempMarkers;

    for (int i = 0; i < trackInfoCount; ++i)
    {
        readTrackInfo(file,tab);
        if (i < (trackInfoCount - 1))
        {
            file->read(&skipShort,2);
        }
    }

    int chordCount = readCountItems(file);

    for (int i = 0; i < chordCount; ++i)
    {
        readChord(file);
        if (i < (chordCount - 1))
        {
            file->read(&skipShort,2);
        }
    }

    int floatingTextCount = readCountItems(file);

    for (int i = 0; i < floatingTextCount; ++i)
    {
        readFloatingText(file);
        if (i < (floatingTextCount - 1))
        {
            file->read(&skipShort,2);
        }
    }

    int guitarInCount = readCountItems(file);

    for (int i = 0; i < guitarInCount; ++i)
    {
        readGuitarIn(file);
        if (i < (guitarInCount - 1))
        {
            file->read(&skipShort,2);
        }
    }

    int tempoMarkerCount = readCountItems(file);

    for (int i = 0; i < tempoMarkerCount; ++i)
    {
        readTempMarker(file,&tempMarkers);
        if (i < (tempoMarkerCount - 1))
        {
            file->read(&skipShort,2);
        }
    }

    int dynamicSectionCount = readCountItems(file);

    for (int i = 0; i < dynamicSectionCount; ++i)
    {
        readDynamic(file);
        if (i < (dynamicSectionCount - 1))
        {
            file->read(&skipShort,2);
        }
    }

    int sectionSymbolCount = readCountItems(file);

    for (int i = 0; i < sectionSymbolCount; ++i)
    {
        readSectionSymbol(file);
        if (i < (sectionSymbolCount - 1))
        {
            file->read(&skipShort,2);
        }
    }

    int sectionCount = readCountItems(file);


    for (int i = 0; i < sectionCount; ++i)
    {
        readSection(file,tab,shift,&tempMarkers,i);
        if (i < (sectionCount - 1))
        {
            file->read(&skipShort,2);
        }
    }

    if (tempMarkers.size())
        tab->setBPM(tempMarkers[0].bpm);
    else
        tab->setBPM(120);    //no error


}


void PtbFile::readTrackInfo(AFile *file, Tab *tab)
{
    byte number = 0;
    file->read(&number,1);

    std::string name;
    readString(file,name);

    byte instrument = 0;
    file->read(&instrument,1);

    byte volume = 0;
    file->read(&volume,1);

    byte panoram = 0;
    file->read(&panoram,1);

    LOG(<<"Track "<<number<<" "<<name<<" "<<instrument<<" "<<volume<<" "<<panoram);

    byte reverb = 0;
    file->read(&reverb,1);

    byte chorus = 0;
    file->read(&chorus,1);

    byte tremolo = 0;
    file->read(&tremolo,1);

    byte phaser = 0;
    file->read(&phaser,1);


    byte capo = 0;
    file->read(&capo,1);

    std::string tuneName;
    readString(file,tuneName);

    byte notation = 0;
    file->read(&notation,1);

    byte stringsCount = 0;
    file->read(&stringsCount,1);

    stringExtended tunesString;

    byte tunec[16];

    //fill track
    Track *track = new Track;
    track->setName(name);

    track->setVolume(volume/16);
    track->setPan(panoram/8);
    track->setInstrument(instrument);

    track->tuning.setStringsAmount(stringsCount);


    for (int i = 0; i < stringsCount; ++i)
    {
        byte stringTune = 0;
        file->read(&stringTune,1);

        tunesString<<stringTune<<" ";
        tunec[i]=stringTune;
        //6 is 0

       track->tuning.setTune(i,stringTune);
    }

    //MISING capo

    LOG(<<"Tunes are: "<<tunesString.c_str());

    tab->add(track);
}

void PtbFile::readChord(AFile *file)
{
    short skipper=0;

    file->read(&skipper,2); //chord key
    file->read(&skipper,1);
    file->read(&skipper,2); //modif
    file->read(&skipper,1);
    file->read(&skipper,1);

    int stringsCount = 0;
    file->read(&stringsCount,1);

    for (int i = 0; i < stringsCount; ++i)
        file->read(&skipper,1); //fret
}


void PtbFile::readFloatingText(AFile *file)
{
    std::string flText;

    readString(file,flText);

    int skipper=0;
    file->read(&skipper,4); //left, top, right, bottom
    file->read(&skipper,4);
    file->read(&skipper,4);
    file->read(&skipper,4);

    file->read(&skipper,1);

    //font
    std::string fontName;

    readString(file,fontName);

    file->read(&skipper,4); //point size
    file->read(&skipper,4);  //eight

    byte italic, underline, strikeout;
    file->read(&italic,1);
    file->read(&underline,1);
    file->read(&strikeout,1);

    int color = 0;
    file->read(&color,4);
}

void PtbFile::readGuitarIn(AFile *file)
{
    int section = 0;
    file->read(&section,2);
    int staff = 0;
    file->read(&staff,1);
    int position = 0;
    file->read(&position,1);

    byte skipper=0;
    file->read(&skipper,1);

    int info = 0;
    file->read(&info,1);

    //logger << "G In sec: "<<section<<" pos: "<<position<<" st: "<<staff<<" in: "<<info;
}

void PtbFile::readTempMarker(AFile *file, std::vector<PtbTempo> *ptbTemp)
{
    int section = 0;
    file->read(&section,2);
    int position = 0;
    file->read(&position,1);

    int temp = 0;
    file->read(&temp,2);

    int data = 0;
    file->read(&data,2);

    std::string description;
    readString(file,description);

    int tripletFeel = 0;
    //data & 0x01 - 8; 02 - 16;

    //tmp > 0 - set

    PtbTempo newTempo;
    newTempo.section = section;
    newTempo.position = position;
    newTempo.bpm = temp;

    ptbTemp->push_back(newTempo);
}


void PtbFile::readSectionSymbol(AFile *file)
{
    int section = 0;
    file->read(&section,2);
    int position = 0;
    file->read(&position,1);

    int data = 0;
    file->read(&data,4);
    //end number: data >> 16
}

void PtbFile::readDynamic(AFile *file)
{
    short skipper = 0;
    file->read(&skipper,2);
    file->read(&skipper,1);
    file->read(&skipper,1);
    file->read(&skipper,2);
    //rehsign byte string
}

void PtbFile::readSection(AFile *file, Tab *tab, int shift, std::vector<PtbTempo> *tempMarker, int sectionNum)
{
    int skipper = 0;

    file->read(&skipper,4); //left t r b
    file->read(&skipper,4);
    file->read(&skipper,4);
    file->read(&skipper,4);

    int lastBarData = 0;
    file->read(&lastBarData,1);

    file->read(&skipper,4); //by single not named

    std::vector <PtbPreBar> barsPosition;
    readBarLine(file,&barsPosition);

    int dirCount = readCountItems(file);
    for (int i = 0; i < dirCount; ++i)
    {
        readDirection(file);
        if (i < (dirCount -1))
            file->read(&skipper,2);
    }

    int chordTextCount = readCountItems(file);
    for (int i = 0; i < chordTextCount; ++i)
    {
        readChordText(file);
        if (i < (chordTextCount -1))
            file->read(&skipper,2);
    }

    int rhySla = readCountItems(file);
    for (int i = 0; i < rhySla; ++i)
    {
        readRhythmSlash(file);
        if (i < (rhySla -1))
            file->read(&skipper,2);
    }

    int staffCount = readCountItems(file);

    //So here we read notes -- but only then we read bars

    std::vector < std::vector < PtbPreBeat > > beatPosition;

    for (int i = 0; i < staffCount; ++i)
    {
        std::vector  < PtbPreBeat > anBP;
        beatPosition.push_back(anBP);

        readStaff(file, &beatPosition[i]);
        if (i < (staffCount -1))
            file->read(&skipper,2);
    }

    int musicBarCount = readCountItems(file);

    //if (musicBarCount) logger << "Bar items "<<musicBarCount; //return back

    //OK here is a trouble?



    for (int i = 0; i < musicBarCount; ++i)
    {
        readBarLine(file,&barsPosition); //special?
        if (i < (musicBarCount -1))
            file->read(&skipper,2);
    }


    parseBarsBeats(beatPosition, barsPosition, tab, shift, tempMarker, sectionNum);
    ///New bar it says
    ///
    static int addBars=0;
    ++addBars;

    ///logger << "Addition bars "<<addBars;

  //RepeatClose (((lastBarData >>> 5) == 4) //?
    //?(lastBarData - 128):0);
}


void PtbFile::parseBarsBeats(std::vector < std::vector < PtbPreBeat > > &beatsPos,
                    std::vector < PtbPreBar > &barsPos,
                    Tab *tab, int shift, std::vector<PtbTempo> *tempMarkers, int sectionNum)
{
    //INVESTIGATION ABOUT RESTS
    /*
    logger << "Tracks "<<(int)beatsPos.size();
    logger << "Bars "<<(int)barsPos.size();
    logger << "z";
    */

    //don't know how to set on right possition yet

    for (int i = 0; i < beatsPos.size(); ++i)
    {
        //tab->getV(i); //each of tracks that appeared

        int inBeatInd = 0;

        if (i+shift >= tab->len())
        {
            LOG( << "Issue #1 in PTB found "<<(i+shift)<<" for "<<(int)tab->len());
            continue;
        }

        for (int j = 0; j < barsPos.size(); ++j)
        {
            Bar *newBar = new Bar;
            newBar->flush();

            newBar->setSignNum(barsPos[j].num);
            newBar->setSignDenum(barsPos[j].den);


            if (barsPos[j].repBeg)
                    newBar->setRepeat(1);
            if (barsPos[j].repEnd)
                    newBar->setRepeat(2,barsPos[j].repEnd);
            ///repeat

            tab->getV(i+shift)->add(newBar);

            int upperBorder = 0;

            if (j == barsPos.size()-1)
               upperBorder = 255;
            else
                upperBorder = barsPos[j+1].position;

            if (beatsPos[i].size())
            {
                while (beatsPos[i][inBeatInd].position < upperBorder)
                {

                    for (int tempI = 1; tempI < tempMarkers->size(); ++tempI)
                    {
                        if (tempMarkers->at(tempI).section == sectionNum)
                            if (beatsPos[i][inBeatInd].position == tempMarkers->at(tempI).position)
                                if (tempMarkers->at(tempI).bpm)
                                {

                                    Beat *beat = beatsPos[i][inBeatInd].val;
                                    beat->effPack.set(28,true);

                                    Beat::SingleChange tempChange;
                                    tempChange.changeCount = 0;
                                    tempChange.changeType = 8;
                                    tempChange.changeValue = tempMarkers->at(tempI).bpm;
                                    beat->changes.add(tempChange);
                                    beat->effPack.addPack(28,1,&beat->changes); //error or not?
                                }
                    }



                    newBar->add(beatsPos[i][inBeatInd].val);
                    ++inBeatInd;

                    if (inBeatInd >= beatsPos[i].size())
                        break;
                }
            }
            else
            {
                Beat *newBeat = new Beat;
                newBeat->setDotted(0);
                newBeat->setDuration(3);
                newBeat->setDurationDetail(0);
                newBeat->setPause(true);

                newBar->add(newBeat);
            }

        }
    }
    for (int j = 0; j < tab->len() - shift - beatsPos.size(); ++j)
    {
        for (int k = 0; k < barsPos.size(); ++k)
        {
            Bar *newBar = new Bar;
            newBar->flush();

            newBar->setSignNum(barsPos[k].num);
            newBar->setSignDenum(barsPos[k].den);

            tab->getV(j+shift+beatsPos.size())->add(newBar);

            if (barsPos[k].repBeg)
                    newBar->setRepeat(1);
            if (barsPos[k].repEnd)
                    newBar->setRepeat(2,barsPos[k].repEnd);

            Beat *newBeat = new Beat;
            newBeat->setDotted(0);
            newBeat->setDuration(3);
            newBeat->setDurationDetail(0);
            newBeat->setPause(true);

            newBar->add(newBeat);
        }
    }
}

void PtbFile::readDirection(AFile *file)
{
    byte position = 0;
    file->read(&position,1);
    byte symbCount = 0;
    file->read(&symbCount,1);

    for (int i = 0; i < symbCount; ++i)
    {
        int data = 0;
        file->read(&data,2);
    }
}

void PtbFile::readChordText(AFile *file)
{
    int skipper = 0;
    file->read(&skipper,1);
    file->read(&skipper,2);
    file->read(&skipper,1);
    file->read(&skipper,2);
    file->read(&skipper,1);
}

void PtbFile::readRhythmSlash(AFile *file)
{
    int skipper = 0;
    file->read(&skipper,1);
    file->read(&skipper,1);
    file->read(&skipper,4);
}


void PtbFile::readBarLine(AFile *file, std::vector<PtbPreBar> *barsPos)
{
    byte position = 0;
    byte type = 0;



    file->read(&position,1);
    file->read(&type,1);

    //RepeatStart(((type >>> 5) == 3));
    //RepeatEnd((((type >>> 5) == 4)?(type - 128):0));

    //Key Signature
    byte keySign = 0;
    file->read(&keySign,1);

    //time sgnature
    int data = 0;
    file->read(&data,4);

    int sigNum=((((data >> 24) - ((data >> 24) % 8)) / 8) + 1);
    int sigDen=pow(2,(data >> 24) % 8);

    if ((sigNum != 4) || (sigDen != 4))
    {
        //logger << "ISSUE"; //sorry) testing
    }

    byte measurePulses = 0;
    file->read(&measurePulses,1);

    //setNum(((((data >> 24) - ((data >> 24) % 8)) / 8) + 1));
    //setDen((int)Math.pow(2,(data >> 24) % 8));

    //Rehersal sign
    byte hmm = 0;
    file->read(&hmm,1);
    std::string rehSign;
    readString(file,rehSign);


    if (barsPos)
    {
        PtbPreBar barPosition;
        barPosition.position = position;
        barPosition.den = sigDen;
        barPosition.num = sigNum;
        barPosition.repBeg = (type >> 5) == 3; //there was >>>
        barPosition.repEnd = (((type >> 5) == 4)?(type - 128):0);

        barsPos->push_back(barPosition);
    }

    static int barsCount = 0;

    ++barsCount;


    //logger<<"Bar# "<<position<<"; t="<<type<<"; keyS="<<keySign
    //     <<" Num ="<<sigNum<<"; Den ="<<sigDen
    //     <<"; mp="<<measurePulses<<"; from total "<<barsCount; //reh

}

void PtbFile::readStaff(AFile *file, std::vector<PtbPreBeat> *beatsPos)
{
    short skipper=0;
    file->read(&skipper,1);
    file->read(&skipper,1);
    file->read(&skipper,1);
    file->read(&skipper,1);
    file->read(&skipper,1);



    for (int voice = 0; voice < 2; ++voice)
    {
        int items = readCountItems(file);

        //if (items != 0) logger<<"Beat items "<<items; //return back

        for (int i = 0; i < items; ++i)
        {
            readPosition(file,beatsPos);
            if (i < (items-1))
            {
                file->read(&skipper,2);
            }
        }
    }
}


void PtbFile::readPosition(AFile *file, std::vector<PtbPreBeat> *beatsPos)
{
    byte position = 0;
    byte beaming = 0;

    file->read(&position,1);
    file->read(&beaming,1);

    beaming = ((beaming - 128 < 0)?beaming:beaming - 128);

    Beat *newBeat = 0;

    if (beatsPos)
    {
        newBeat = new Beat;
        PtbPreBeat preBeat;
        preBeat.position = position;
        preBeat.val = newBeat;

        beatsPos->push_back(preBeat);
    }

    //logger <<"InPos "<<position<<"; "<<beaming;

    //
    int skipper = 0;
    file->read(&skipper,1);

    byte data1 = 0;
    file->read(&data1,1);

    file->read(&skipper,1);

    byte data3 = 0;
    file->read(&data3,1);

    byte dur = 0;
    file->read(&dur,1);

    int multiBarRest = 1;
    byte complexCount = 0;
    file->read(&complexCount,1);

    for (int i = 0; i < complexCount; ++i)
    {
        int counts = 0;
        file->read(&counts,2);

        file->read(&skipper,1);

        byte type = 0;
        file->read(&type,1);

        if (type & 0x08)
            multiBarRest = counts;
    }


    int notesCount = readCountItems(file);

    //if (notesCount)
        //logger << "Notes amount : "<<notesCount;

    if (notesCount > 8)
    {
        logger << "Something got wrong in notes";
    }


    if (newBeat)
    {
        newBeat->setDotted(data1 & 0x03); //01 02

        byte fineDur;
        if (dur==1)
            fineDur = 0; //enumerate
        if (dur==2)
            fineDur = 1;
        if (dur==4)
            fineDur = 2;
        if (dur==8)
            fineDur = 3;
        if (dur==16)
            fineDur = 4;
        if (dur==32)
            fineDur = 5;
        if (dur==64)
            fineDur = 6;

        newBeat->setDuration(fineDur);
        newBeat->setPause(false);

        //data1 & 0x20 upstroke, data1 & 0x40 - down
        //grace data3 & 0x01
    }

    for (int i = 0; i < notesCount; ++i)
    {
        readNote(file,newBeat);
        if (i < (notesCount-1))
            file->read(&skipper,2);
    }



    /*
    beat.setMultiBarRest((itemCount == 0)?multiBarRest:1);
    beat.setVibrato(((data1 & 0x08) != 0) || ((data1 & 0x10) != 0));
    beat.setGrace((data3 & 0x01) != 0);

        // Set the duration
    beat.setDuration(durationValue);
    beat.setDotted((data1 & 0x01) != 0);
    beat.setDoubleDotted((data1 & 0x02) != 0);
    beat.setArpeggioUp((data1 & 0x20) != 0);
    beat.setArpeggioDown((data1 & 0x40) != 0);
    beat.setEnters(((beaming - (beaming % 8)) / 8) + 1);
    beat.setTimes((beaming % 8) + 1);
    */

}

void PtbFile::readNote(AFile *file, Beat *beat)
{
    byte position = 0;
    file->read(&position,1);
    int simpleData = 0;
    file->read(&simpleData,2);
    int symbolCount = 0;
    file->read(&symbolCount,1);

    int skipper = 0;
    for (int i = 0; i < symbolCount; ++i)
    {
        file->read(&skipper,1);
        file->read(&skipper,1);

        byte data3=0,data4=0;

        file->read(&data3,1);
        file->read(&data4,1);
        //note.setBend((data4 == 101)?((data3 / 16) + 1):0);
        //note.setSlide((data4 == 100));
    }

    if (beat)
    {
        Note *newNote = new Note;
        newNote->setFret(position & 0x1f);
        newNote->setStringNumber(((position & 0xe0) >> 5) + 1);

        beat->add(newNote);
        //dead simpleData & 0x02 ; tied  simpleData & 0x01
    }


}
