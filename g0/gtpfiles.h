#ifndef GTPFILES_H
#define GTPFILES_H

#include "g0/types.h"
#include "g0/tab.h"

void readBend(std::ifstream *file, BendPoints *bend);
void writeBend(std::ofstream *file, BendPoints *bend);

///HE LIVED ALONE!
///
/// //remind importers
class Gp4Import //: public AImport
{
    public:
    //from A File to Tab
    //Gp4Import(A File &from, Tab &to) : AImport(&from,&to,128) {} //not our types goes from 128

    bool import(std::ifstream &file, Tab *tab, byte knownVersion=0);

    //readString
    //readBend

    //readChordDiagram
    //readBeatEffects
    //readNoteEffects
    //readMixTable
    //readGraceNote
    //readTrack
    //readBeat
    //readNote

};

class Gp4Export //: public AExport
{
    public:

    bool exPort(std::ofstream &file, Tab &tab);
};


//GP5

class Gp5Import //: public AImport
{
    public:
    bool import(std::ifstream &file, Tab *tab, byte knownVersion=0);

    //readString
    //readBend

    //readChordDiagram
    //readBeatEffects
    //readNoteEffects
    //readMixTable
    //readGraceNote
    //readTrack
    //readBeat
    //readNote

};


class Gp3Import
{

public:
    bool import(std::ifstream &file, Tab *tab, byte knownVersion=0);
};

#endif // GTPFILES_H
