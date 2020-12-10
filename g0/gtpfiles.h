#ifndef GTPFILES_H
#define GTPFILES_H

#include "g0/types.h"
#include "g0/tab.h"

void readBend(AFile *file, BendPoints *bend);
void writeBend(AFile *file, BendPoints *bend);

///HE LIVED ALONE!
///
/// //remind importers
class Gp4Import //: public AImport
{
    public:
    //from AFile to Tab
    //Gp4Import(AFile &from, Tab &to) : AImport(&from,&to,128) {} //not our types goes from 128

    bool import(AFile &file, Tab *tab, byte knownVersion=0);

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
    //from AFile to Tab
    //Gp4Import(AFile &from, Tab &to) : AImport(&from,&to,128) {} //not our types goes from 128

    bool exPort(AFile &file, Tab &tab);
};


//GP5

class Gp5Import //: public AImport
{
    public:
    bool import(AFile &file, Tab *tab, byte knownVersion=0);

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
    bool import(AFile &file, Tab *tab, byte knownVersion=0);
};

#endif // GTPFILES_H
