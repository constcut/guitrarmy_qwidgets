#ifndef AEXPIMP_H
#define AEXPIMP_H

#include "types.h"

#include "g0/tab.h"
#include "g0/gmyfile.h"
#include "g0/gtpfiles.h"
#include "g0/ptbfile.h"

//RENAME	LATER	TO	APORT.h 
//like both parts of exPORT imPORT

//EXPORT
/*
class AExport
{
    //AMusic to MidiFile
    //Tab to GP4
    //Tab to GMYFile

protected:
	//crappy pointers definitions, that will die then
	void *origin;
	void *dest;
	
	byte type;
public:
	AExport(void * from, void * to, byte type) : origin(from), dest(to),type(type) {}

	virtual bool exPort() = 0;	
};

//IMPORT

class AImport
{
    //MidiFile to AMusic
    //GP4 to Tab
    //GMYFile to Tab

protected:
	//crappy pointers definitions, that will die then
	void *origin;
	void *dest;
	
	byte type;
public:	
	AImport(void * from, void * to, byte type) : origin(from), dest(to),type(type) {}

	//imPort
	virtual bool import() = 0;
};
//OLD MINDS - review them
/*/


//FIRST definitions

//class Gp4Import
//{
	//from AFile to Tab	
//};

class GTabLoader
{
public:
    bool open(std::string fileName);

    GTabLoader():type(255),tab(0) {}

    Tab *getTab() {return tab;} //some attention here posible leak if use mindless

protected:
    byte type; //0-10 for gmy
    //10 ptb
    //11 gp1 12 gp2 13 gp3
    //14 ptb5 16 gpx
    //17 tux
    //18 tab edit
    Tab *tab;
};

//Transmorfs for own?

#endif // AEXPIMP_H
