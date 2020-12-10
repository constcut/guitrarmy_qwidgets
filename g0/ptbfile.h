#ifndef PTBFILE_H
#define PTBFILE_H

#include "g0/afile.h"
#include "g0/tab.h"

struct PtbPreBeat
{
    byte position;
    Beat *val;
};

struct PtbPreBar
{
    byte position;

    byte num;
    byte den;

    byte repBeg;
    byte repEnd;
};

struct PtbTempo
{
    int section;
    byte position;
    int bpm;
};

class PtbFile
{
public:
    PtbFile();

    //bool saveToFile(AFile *file, Tab *tab);
    bool loadFromFile(AFile *file, Tab *tab, bool skipVersion=false);

protected:

    bool loadString(AFile *file, std::string &strValue);
    //bool saveString(AFile *file, std::string &strValue);

    bool readString(AFile *file, std::string &strValue);

    int readCountItems(AFile *file);

    void readInstrumentsData(AFile *file, Tab *tab, int shift);

    void readTrackInfo(AFile *file, Tab *tab);
    void readChord(AFile *file);

    void readFloatingText(AFile *file);
    void readGuitarIn(AFile *file);
    void readTempMarker(AFile *file, std::vector<PtbTempo> *ptbTemp);

    void readSectionSymbol(AFile *file);
    void readSection(AFile *file, Tab *tab, int shift, std::vector<PtbTempo> *tempMarker, int sectionNum);
    void readDynamic(AFile *file);

    void readDirection(AFile *file);
    void readBarLine(AFile *file, std::vector< PtbPreBar > *barsPos);
    void readChordText(AFile *file);
    void readRhythmSlash(AFile *file);
    void readStaff(AFile *file, std::vector < PtbPreBeat > *beatsPos);

    void readPosition(AFile *file, std::vector < PtbPreBeat > *beatsPos);
    void readNote(AFile *file, Beat *beat);


    void parseBarsBeats(std::vector<std::vector<PtbPreBeat> > &beatsPos,
                        std::vector<PtbPreBar> &barsPos,
                        Tab *tab, int shift, std::vector<PtbTempo> *tempMarkers, int sectionNum);
};


#endif // PTBFILE_H
