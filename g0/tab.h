#ifndef TAB_H
#define TAB_H

#include "types.h"

#include <memory.h> //TODO remove
#include <unordered_map>
//TODO Tab, Track, Bar, Beat, Note - sepparate all the files

#include "abitarray.h"
#include "tabcommands.h" //move into g0

int updateDurationWithDetail(byte detail, int base);

class Tab;
class Track;
class Bar;
class Beat;
class Note;

//old version of bends used on early moments for
struct BendPointGPOld
{
    ul absolutePosition;
    ul heightPosition;
    byte vibratoFlag;
};

class BendPointsGPOld : public ChainContainer<BendPointGPOld, void>
{
protected:
    ul bendHeight;
    byte bendType;
public:

    byte getType() { return bendType; }
    void setType(byte newBendType) { bendType = newBendType; }

    ul getHeight() { return bendHeight; }
    void setHeight(ul newBendHeight) { bendHeight=newBendHeight;}

};

struct BendPoint
{
    byte vertical; //:4
    byte horizontal; //:5
    byte vFlag; //:2
};

class BendPoints : public ChainContainer<BendPoint, void>
{
protected:
    byte bendType;
public:
    byte getType() { return bendType; }
    void setType(byte newBendType) { bendType = newBendType; }

    void insertNewPoint(BendPoint bendPoint);
};



class Note 
{
	//class implements single note, a sound on one string
	public:

    void printToStream(std::ostream &stream);

	enum FretValues
	{
		emptyFret = 63,
		pauseFret = 62,
		ghostFret = 61,
		leegFret = 60
	};
        //check is upper replaced?

    enum NoteState
    {
        normalNote = 0,
        leegNote = 2,
        deadNote = 3,
        leegedNormal = 4,
        leegedLeeg = 6
    };


	Note():
    fret(emptyFret),volume(0),fingering(0),effectsA(0),effectsB(0),
          graceIsHere(false) {}
	virtual ~Note() {}

    Note &operator=(Note *anotherNote)
    {
        clone(anotherNote);
        return *this;
    }

	protected:
	//[BASE]
	byte fret; //[0-63]; some last values used for special coding {pause, empty, leege note... ghost note(x)}
	byte volume;//[0-64]]-1);  values less than 14 used for ppp pp p m f ff fff - before -is precents for 2 per step
	byte fingering; //store information about finger to play - //fingers {none, L1,L2,L3,L4,L5, R1,R2,R3,R4,R5 +(12)(13)(14)(15)(16) ) - pressure hand for another there
	///are pick effects
		
    byte noteState;//NoteState


	byte effectsA; // inEffects:[slide in up\down; grace; none] outEffects:[side out up\down; legato; bend; ghost; ] [let ring] [palm mute] [vibro]
	byte effectsB; // picking [tap slap pop up down stoks and non] [trill 0 2 4 8]
     
    //moved to set get      
   //short int *bendPoints;
   


	//to set sure  - if possible escape - then do it
	byte GpCompFing1;
	byte GpCompFing2;


    byte stringNumber;

    Note *prevNote;
    Note *nextNote;
	
	public:

    void setNext(Note *nextOne) { nextNote = nextOne; }
    void setPrev(Note *prevOne) { prevNote = prevOne; }

    Note *getNext() { return nextNote; }
    Note *getPrev() { return prevNote; }

    //For GP Compatibility left:
    //Grace note - 4bytes
    bool graceIsHere;
    byte graceNote[4];//compitability with guitar pro requires later migration

	BendPoints bend;
    //[EFFECTS]
    EffectsPack effPack;

	//SET GET operations
    void setStringNumber(byte num) {stringNumber = num;}
    byte getStringNumber() { return stringNumber; }

    void setFret(byte fValue) { fret = fValue; }
    byte getFret() { return fret; }


    int getMidiNote(int tune=0) //get tune from tracks hard chain
    {
        return fret + tune;
    }

    void setState(byte nState) { noteState = nState; }
    void signStateLeeged() { if (noteState<=1) noteState=4; if (noteState==2) noteState=6; }
    byte getState() { return noteState;}
	
	void setVolume(byte vValue) { volume = vValue; }
	byte getVolume() { return volume; }
	
	void setFingering(byte fValue) {fingering = fValue;}
	byte getFingering() { return fingering; }
	
    void setEffect(byte eff);// { effectsA = eff; }
    EffectsPack getEffects();// { return effectsA; }
    void addEffects(EffectsPack &append) { effPack.mergeWith(append); }

	//effects need review
	//MISSING
	//INCLUDING	BEND
			
	void setGPCOMPFing1(byte fValue) { GpCompFing1 = fValue; }
	void setGPCOMPFing2(byte fValue) { GpCompFing2 = fValue; }
	byte getGPCOMPFing1() { return GpCompFing1;}
	byte getGPCOMPFing2() { return GpCompFing2;}
	
	//NO	CHECK	REMINDS	ABOUT	FUTURE	CHANGES
	void setGPCOMPGrace(size_t index, byte gValue) { graceNote[index] = gValue;}
	byte getGPCOMPGrace(size_t index) { return graceNote[index]; }

    void clone(Note *from);
	//pack function
};


//STARTING LARGE REFACTORING OVER THE TAB

class Beat : public ChainContainer<Note*, Bar>
{
		
public:
    Beat()
    {}

    virtual ~Beat()
    {
        for (ul i=0; i < len(); ++i)
                   delete at(i);
    }
    //usually size reserved

     void printToStream(std::ostream &stream);

	struct SingleChange
	{
		byte changeType;
		ul changeValue;
		byte changeCount;
	};

	//need inner functions for analtics of packing	
    class ChangesList : public ChainContainer<SingleChange, void>
	{
    public:
        //search functions
	};

	//used for compatibility with guitar pro
	struct GPChordDiagram
	{
		byte header;
		byte sharp;
		byte blank1;
		byte blank2;
		byte blank3;
		
		byte root;
		byte minMaj;
		byte steps9x;
		
		ul bass;
		ul deminush;
		
		byte add;
		char name[20];
		
		byte blank4;
		byte blank5;
		
		byte s5,s9,s11;
		
		ul baseFrets;
		ul frets[7];
		
		byte baresNum;
		byte fretBare[5];
		byte startBare[5];
		byte endBare[5];
		
		byte o1,o3,o5,o7,o9,o11,o13;
		
		byte blank6;
		
		byte fingering[7];
		byte showFing;
	} gpCompChordDiagram;


    struct ChangeTable
    {
       byte newInstr;
       byte newVolume;
       byte newPan;
       byte newChorus;
       byte newReverb;
       byte newPhaser; //6 total + 4
       byte newTremolo;
       ul newTempo; //OR IT IS INT?
       //10
       byte instrDur;
       byte volumeDur;
       byte panDur;
       byte chorusDur;
       byte reverbDur;
       byte phaserDur;
       byte tremoloDur;
       byte tempoDur;

       byte changesTo; //one track or all of them
       byte postFix;
    };

    EffectsPack effPack;

    Beat &operator=(Beat *another)
    {
        clone(another);
        return *this;
    }

protected:

    byte duration = 0; // 2 1 . 2 4 8 16 32 64 [8 values] - 3 bits
    byte durationDetail = 0; // none, dot, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 [4 bits] //one of 2-15 means empty(together with pause only!)
	//1 bit determines pause
    bool isPaused;

    byte dotted = 0; //0 1 2


    byte effects = 0; //tremolo precene - 1bit, (upstrings, downstrings+x) - 2 bits, bookmark - 1 bit, notice (text or elst) - 1 bit,
	//then 3 left for change - first bit of those 3 shows that there is change
	//+1 chord diagram
	//+1 reserved
	
	//moved to set get
	//short int *tremoloPoints;
			
	std::string bookmarkName;
	std::string noticeText; 

		  		  
	public:
		
	//pub are part of set and get
	ChangesList changes;
    BendPoints tremolo;



	//SET GET operations
    void setPause(bool pause) {isPaused = pause;}
    bool getPause() { return isPaused; }

    void deleteNote(int string)
    {
        for (ul i = 0; i < len(); ++i)
        {
            if (string == at(i)->getStringNumber())
            {
                remove(i);

                if (len() == 0)
                    setPause(true);

                return;
            }
        }
    }

    Note *getNote(int string)
    {
        for (ul i = 0; i < len(); ++i)

            if (at(i)->getStringNumber()==string)
                return at(i);

        return 0;
    }

    Note *getNoteInstance(int string)
    {
        /*
        for (ul i = 0; i < len(); ++i)

            if (getV(i).getStringNumber()==string)
                return getV(i);
        */

        Note *notePtr = getNote(string);
        if (notePtr)
        {
            Note *theClone = new Note();
            theClone->clone(notePtr);
            return theClone;
        }

        Note *emptyNote = new Note;
        emptyNote->setFret(255);
        return emptyNote;
    }

    void setFret(byte fret, int string)
    {

        if (len() == 0)
        {
            //paused
            Note *newNote = new Note();
            newNote->setFret(fret);
            newNote->setStringNumber(string);
            newNote->setState(0);

            //DEFAULT NOTE VALUES??
            add(newNote);

            setPause(false);
            return;
        }

        for (ul i = 0; i < len(); ++i)
        {
            if (at(i)->getStringNumber()==string)
            {
                at(i)->setFret(fret);
                return; //function done
            }

            if (at(i)->getStringNumber() > string)
            {
                Note *newNote=new Note();
                newNote->setFret(fret);
                newNote->setStringNumber(string);
                newNote->setState(0);


                insertBefore(newNote,i);
                return;
            }

        }


        int lastStringN = at(len()-1)->getStringNumber();
        if (lastStringN < string)
        {
            Note *newNote=new Note();
            newNote->setFret(fret);
            newNote->setStringNumber(string);
            newNote->setState(0);

            //DEFAULT NOTE VALUES??
            add(newNote);
            return;
        }
        //we got here - that means we need insert fret
    }


    byte getFret(int string)
    {
        if (len() == 0)
            return 255;

        for (ul i = 0; i < len(); ++i)
            if (at(i)->getStringNumber()==string)
            {
                byte fretValue = at(i)->getFret();
                return fretValue; //function done
            }

        return 255;
    }

	void setDuration(byte dValue) { duration = dValue; }	
	void setDurationDetail(byte dValue) {	durationDetail = dValue; }
    void setDotted(byte dottedValue) { dotted = dottedValue; }
	
	byte getDuration() { return duration;}
	byte getDurationDetail () { return durationDetail; }
    byte getDotted() { return dotted; }

    void setEffects(byte eValue);// { effects = eValue; }
    EffectsPack getEffects();// { return effects; }

	//TREMOLO missing
	
	//BOOKMARK - goes  for future
	
    void setGPCOMPChordDiagram(char *area) { memcpy(&gpCompChordDiagram,area,sizeof(gpCompChordDiagram)); }
    bool getGPCOMPChordDiagram(char *to) { memcpy(to,&gpCompChordDiagram,sizeof(gpCompChordDiagram)); return true; }

	void setGPCOMPText(std::string &value) { noticeText = value; }	
	void getGPCOMPText(std::string &value) { value = noticeText; }
	
    //pack function

    void clone(Beat *from);
};

class Bar : public ChainContainer<Beat*, Track>
{
	
public:
    Bar() {
        flush();
    }
    virtual ~Bar()
    {
        for (ul i=0; i < len(); ++i)
                   delete at(i);
    }

    void printToStream(std::ostream &stream);

    void flush()
    {
        signatureNum = signatureDenum = 0;
        repeat = repeatTimes = altRepeat = 0;
        markerColor = 0;
        completeStatus = 0;
    }

    Bar &operator=(Bar *another)
    {
        clone(another);
        return *this;
    }

    virtual void add(Beat *&val)
    {
        if (val)
        {
            val->setParent(this);
            ChainContainer<Beat*, Track>::add(val);
        }
    }

    virtual void insertBefore(Beat* &val, int index=0)
    {
        if (val)
        {
            val->setParent(this);
            ChainContainer<Beat*, Track>::insertBefore(val,index);
        }
    }


protected:

	byte signatureNum;
	byte signatureDenum;
	
	byte repeat;//reprease options
    byte repeatTimes;
    byte altRepeat;
	
	//GP comp - marker, tonality
	byte gpCompTonality;
	std::string markerText;
	ul markerColor; //white byte == 1 if empty

    byte completeStatus;
    short completeAbs;
    ul completeIndex;

	public:
	//SET GET operations
	
	void setSignNum(byte num) { signatureNum = num; }
	void setSignDenum(byte denum) { signatureDenum = denum; }
	
	byte getSignNum() { return signatureNum; }
	byte getSignDenum() { return signatureDenum; }

    //!completeStatus !!! that should go private and opt
    void countUsedSigns(byte &numGet, byte &denumGet);

    byte getCompleteStatus();
    double getCompleteAbs();
    ul   getCompleteIndex();
	
    void setRepeat(byte rValue, byte times=0)
    {
        if (rValue == 0) repeat = 0;
        else repeat |= rValue;
        if(times) repeatTimes=times;
    }
	byte getRepeat() { return repeat; }
    byte getRepeatTimes() { return repeatTimes; }

    void setAltRepeat(byte number) { altRepeat = number; }
    byte getAltRepeat() { return altRepeat; }
	
	void setGPCOMPTonality(byte tValue) { gpCompTonality = tValue; }
	byte getGPCOMPTonality() { return gpCompTonality; }
	
	void setGPCOMPMarker(std::string &text, ul color) { markerText = text; markerColor = color; }
	void getGPCOMPMarker(std::string &text, ul &color) { text = markerText; color = markerColor; } 
		
	//pack function
     void clone(Bar *from);
};


class GuitarTuning
{
	byte stringsAmount;
    byte tunes[10]; //as a maximum, mind later is it usefull to try set more or less by container?
    //set it to byte - in fact int would be 128 values of std midi - next could be used as quatones

	public:
	
	void setStringsAmount(byte amount) { stringsAmount = amount; }
	byte getStringsAmount() { return stringsAmount; }
    void setTune(byte index, byte value) { if (index <= 10) tunes[index] = value; } //(index >= 0) &&
    byte getTune(byte index) { if (index <= 10) return tunes[index]; return 0; }
};


class PolyBar : public ChainContainer<Bar*, Tab>
{
  public:
    PolyBar()
    {
    }
    virtual ~PolyBar()
    {
    }
};


class Track : public ChainContainer<Bar*, Tab>
{
public:

    Track():timeLoop(),pan(0),drums(false),status(0), _cursor(0),_cursorBeat(0),_stringCursor(0),
    _displayIndex(0),_lastSeen(0),_selectCursor(-1), _digitPress(-1) {
        GpCompInts[3]=24; //REFACT GCOMP
        _selectionBarFirst=-1;
        _selectionBarLast=-1;
        _selectionBeatFirst=-1;
        _selectionBeatLast=-1;
    } 

    virtual ~Track() {
        for (ul i=0; i < len(); ++i)
            delete at(i);
    }

    void printToStream(std::ostream &stream);

    ChainContainer<Bar*, Tab> timeLoop; //PolyBar //REFACT access
    std::vector<ul> timeLoopIndexStore;

    Track &operator=([[maybe_unused]]Track another)
    {
        //clone(another); //TODO
        return *this;
    }

    virtual void add(Bar*& val)
    {
        if (val){
            val->setParent(this);
            ChainContainer<Bar*, Tab>::add(val);
        }
    }

    virtual void insertBefore(Bar* &val, int index=0)
    {
        if (val)
        {
            val->setParent(this);
        ChainContainer<Bar*, Tab>::insertBefore(val,index);
        }
    }

protected:
	std::string name;
	ul instrument;
	ul color;
	byte pan; //or int??
	byte volume;
    bool drums;
	//?own temp bpm
    ul GpCompInts[4]; //GpComp - Port,Channel,ChannelE,Capo
    ul beatsAmount;
    byte status; //0 - none 1 - mute 2 - soloe

public:
	GuitarTuning tuning;

    ul connectBars();
    ul connectBeats();
    ul connectNotes(); //for let ring
    ul connectTimeLoop();
    void connectAll();

    void pushReprise(Bar *beginRepeat, Bar *endRepeat,
                     Bar *preTail, Bar *tailBegin, Bar *tailEnd, ul beginIndex, ul endIndex,
                     ul preTailIndex=0, ul tailBeginIndex=0, ul tailEndIndex=0);

	void setName( std::string &nValue) { name = nValue; }
    std::string getName() { return name; } //or return?
	
	void setInstrument(ul iValue) { instrument = iValue;}
	ul getInstrument() { return instrument; }
	
	void setColor(ul cValue) { color = cValue; }
	ul getColor() { return color; }
	
    void setPan(byte pValue) { pan = pValue; }
	byte getPan() { return pan; }
	
	void setVolume(byte vValue) { volume = vValue; }
	byte getVolume() { return volume; }
	
	
	void setGPCOMPInts(size_t index, ul value) { GpCompInts[index] = value; }
	ul getGPCOMPInts(size_t index) { return GpCompInts[index]; } //TODO get rid
	
    void setDrums(bool newDrums) {
        drums = newDrums;
        if (drums) GpCompInts[3]=99; //refact
    }

    bool isDrums() { return drums; }

    byte getStatus() { return status; } //refact name
    void setStatus(byte newStat) { status = newStat; }

protected:
    size_t _cursor;
    size_t _cursorBeat;
    size_t _stringCursor;
    size_t _displayIndex;
    size_t _lastSeen;
    int _selectCursor; //TOSO to size_t
    int _selectionBarFirst;
    int _selectionBarLast;
    int _selectionBeatFirst;
    int _selectionBeatLast;
    int _digitPress;


public:
    size_t& cursor() { return _cursor; } //TODO после всего рефакторинга обязаны быть const
    size_t& cursorBeat() { return _cursorBeat; }
    size_t& stringCursor() { return _stringCursor; }
    size_t& displayIndex() { return _displayIndex; }
    size_t& lastSeen() { return _lastSeen; }
    int& selectCursor() { return _selectCursor;}
    int& selectBarFirst() { return _selectionBarFirst;}
    int& selectBarLast() { return _selectionBarLast;}
    int& selectBeatFirst() { return _selectionBeatFirst;}
    int& selectBeatLast() { return _selectionBeatLast;}
    int& digitPress() { return _digitPress; }


    //TODO command handler
    void switchEffect(int effIndex);
    void switchBeatEffect(int effIndex);
    void switchNoteState(byte changeState);
    void reverseCommand(SingleCommand &command);

    std::vector<SingleCommand> commandSequence;

    void onTrackCommand(TrackCommand command);

    void gotoTrackStart();

    void moveSelectionLeft();
    void moveSelectionRight();
    void insertBar();
    void moveToNextBar();
    void moveToPrevBar();
    void moveToPrevPage();
    void moveToNextPage();
    void moveToNextTrack();
    void moveToPrevTrack();
    void moveToStringUp();
    void moveToStringDown();
    void moveToPrevBeat();
    void moveToNextBeat();
    void setTrackPause();
    void deleteBar();
    void deleteSelectedBars();
    void deleteSelectedBeats();
    void deleteNote();
    void incDuration();
    void decDuration();
    void saveFromTrack();
    void newBar();
    void setDotOnBeat();
    void setTriolOnBeat();
    //void setBendOnNote(); //TODO + changes + chord

    void setTextOnBeat(std::string newText);
    void setBarSign(int num, int denom);
    void changeBarSigns(int num, int denom);

    void clipboardCopyBar(); //refact name
    void clipboarCopyBeat();
    void clipboardCopyBars();
    void clipboardCutBar();
    void clipboardPaste();
    void undoOnTrack();


    void LeegNote();
    void DeadNote();
    void Vibratto();
    void Slide();
    void Hammer();
    void LetRing();
    void PalmMute();
    void Harmonics();
    void TremoloPicking();
    void Trill();
    void Stokatto();
    void FadeIn(); //Todo fade out
    void Accent();
    void HeavyAccent();
    void UpStroke();
    void DownStroke();

private:
    //TODO внимательно проверить
    std::unordered_map<TrackCommand, void (Track::*)()> handlers =  {
        {TrackCommand::GotoStart, &Track::gotoTrackStart},
        {TrackCommand::SelectionExpandLeft, &Track::moveSelectionLeft},
        {TrackCommand::SelectionExpandRight, &Track::moveSelectionRight},
        {TrackCommand::InsertBar, &Track::insertBar},
        {TrackCommand::NextBar, &Track::moveToNextBar},
        {TrackCommand::PrevBar, &Track::moveToPrevBar},
        {TrackCommand::PrevPage, &Track::moveToPrevPage},
        {TrackCommand::NextTrack, &Track::moveToNextTrack},
        {TrackCommand::PrevTrack, &Track::moveToPrevTrack},
        {TrackCommand::StringDown, &Track::moveToStringDown},
        {TrackCommand::StringUp, &Track::moveToStringUp},
        {TrackCommand::PrevBeat, &Track::moveToPrevBeat},
        {TrackCommand::NextBeat, &Track::moveToNextBeat},
        {TrackCommand::SetPause, &Track::setTrackPause},
        {TrackCommand::DeleteBar, &Track::deleteBar},
        {TrackCommand::DeleteSelectedBars, &Track::deleteSelectedBars},
        {TrackCommand::DeleteSelectedBeats, &Track::deleteSelectedBeats},
        {TrackCommand::IncDuration, &Track::incDuration},
        {TrackCommand::DecDuration, &Track::decDuration},
        {TrackCommand::NewBar, &Track::newBar},
        {TrackCommand::SetDot, &Track::setDotOnBeat},
        {TrackCommand::SetTriole, &Track::setTriolOnBeat},
        {TrackCommand::Leeg, &Track::LeegNote},
        {TrackCommand::Dead, &Track::DeadNote},
        {TrackCommand::Vibrato, &Track::Vibratto},
        {TrackCommand::Slide, &Track::Slide},
        {TrackCommand::Hammer, &Track::Hammer},
        {TrackCommand::LetRing, &Track::LetRing},
        {TrackCommand::PalmMute, &Track::PalmMute},
        {TrackCommand::Harmonics, &Track::Harmonics},
        {TrackCommand::TremoloPickings, &Track::TremoloPicking},
        {TrackCommand::Trill, &Track::Trill},
        {TrackCommand::Stokatto, &Track::Stokatto},
        {TrackCommand::FadeIn, &Track::FadeIn},
        {TrackCommand::Accent, &Track::Accent},
        {TrackCommand::HeaveAccent, &Track::HeavyAccent},
        {TrackCommand::UpStroke, &Track::UpStroke},
        {TrackCommand::DownStroke, &Track::DownStroke},
        {TrackCommand::Cut, &Track::clipboardCutBar},
        {TrackCommand::Copy, &Track::clipboardCopyBar},
        {TrackCommand::CopyBeat, &Track::clipboarCopyBeat},
        {TrackCommand::CopyBars, &Track::clipboardCopyBars},
        {TrackCommand::Past, &Track::clipboardPaste},
        {TrackCommand::Undo, &Track::undoOnTrack},
        {TrackCommand::SaveFile, &Track::saveFromTrack}};
};

//here would lay Guitar Pro header comptibilator
struct MidiChannelInfo
{
    unsigned int instrument; //refact types

	byte volume;
	byte balance;
	byte chorus;
	byte reverb;

	byte phaser;
	byte tremolo;
	byte blank1;
	byte blank2;
};


struct VariableString
{
	byte lineType;
	std::string name;
	
	std::list<std::string> v;
	
	VariableString(std::string stringName, std::string stringValue) : lineType(0), name(stringName) { v.push_back(stringValue); }
	VariableString(std::string stringName, std::list<std::string> stringsValues) : lineType(1), name(stringName) { v = stringsValues; }
};

class VariableStrings : public ChainContainer<VariableString, void>
{
	//search options
};


struct TimeLineKnot
{
    byte type;
    int value;

    TimeLineKnot(byte newType, int newValue):type(newType),value(newValue){}
};


class Tab : public ChainContainer<Track*, void>
{
public:

    Tab() :isPlaying(false), displayTrack(0), currentTrack(0),
        currentBar(0), displayBar(0), lastOpenedTrack(0) {}

    std::vector<TimeLineKnot> timeLine;

    virtual ~Tab() {
        for (ul i=0; i < len(); ++i)
                   delete at(i);
    }

    void printToStream(std::ostream &stream);

    void createTimeLine(ul shiftTheCursor=0);
    byte getBPMStatusOnBar(ul barN);
    int getBpmOnBar(ul barN);

    void connectTracks(){
        for (ul i = 0; i < len(); ++i)
        at(i)->connectAll();
        createTimeLine();
    }

    void postGTP() {
        for (ul i = 0; i < len(); ++i) {
            ul port = at(i)->getGPCOMPInts(0);
            ul chan = at(i)->getGPCOMPInts(1);
            ul ind = (chan-1) + (port-1)*16;
            if (ind < 70) {
                int instr = GpCompMidiChannels[ind].instrument;
                byte pan = GpCompMidiChannels[ind].balance;
                byte vol = GpCompMidiChannels[ind].volume;
                Track *t=at(i);
                t->setInstrument(instr);
                t->setPan(pan);
                t->setVolume(vol);
            }
         }
    }

    Tab &operator=([[maybe_unused]]Tab another) {
        //lone(another); //TODO
        return *this;
    }


protected:	
    int bpmTemp;
	//version control flag
	std::string origin; //glink - short link determines were from file came (from guitarmy network) az09AZ style	
	byte GpCompSignKey;
    byte GpCompSignOctave; //TODO get rid
	byte GpCompTripletFeel;

public:
    VariableStrings variableInforation;
    MidiChannelInfo GpCompMidiChannels[64]; //TODO refactor
    int getBPM() { return bpmTemp; }
    void setBPM(int newBPM) { bpmTemp = newBPM; }
	
protected: //Move from TabView
    bool isPlaying;
    size_t displayTrack;
    size_t currentTrack;
    size_t currentBar;
    size_t displayBar;
    int lastOpenedTrack;

public:
    bool playing() {
        return isPlaying;
    }
    void setPlaying(bool v) {
        isPlaying = v;
    }
    size_t& getDisplayTrack() {
        return displayTrack;
    }
    size_t& getCurrentTrack() {
        return currentTrack;
    }
    size_t& getCurrentBar() {
        return currentBar;
    }
    size_t& getDisplayBar() {
        return displayBar;
    }
    void onTabCommand(TabCommand command);

    int& getLastOpenedTrack() {
        return lastOpenedTrack;
    }

public: //later cover under midlayer TabCommandsHandler
    Track* createNewTrack(); 

    void muteTrack(); //current
    void soloTrack();
    void changeDrumsFlag();
    void midiPause();
    void openReprise();
    void closeReprise();

    void moveCursorInTrackRight();
    void moveCursorInTrackLeft();
    void moveCursorOfTrackUp();
    void moveCursorOfTrackDown();

    void deleteTrack();

    void setSignsTillEnd(int num, int denom);
    void changeTrackVolume(int newVol);
    void changeTrackName(std::string newName);
    void changeTrackInstrument(int val);
    void changeTrackPanoram(int val);
    void setMarker(std::string text);

    void gotoBar(size_t pos);
    void saveAs(std::string filename);
    void closeReprise(size_t count);


private:
    std::unordered_map<TabCommand, void (Tab::*)()> handlers =  {
        {TabCommand::Mute, &Tab::muteTrack},
        {TabCommand::Solo, &Tab::soloTrack},
        {TabCommand::MoveRight, &Tab::moveCursorInTrackRight},
        {TabCommand::MoveLeft, &Tab::moveCursorInTrackLeft},
        {TabCommand::MoveUp, &Tab::moveCursorOfTrackUp},
        {TabCommand::MoveDown, &Tab::moveCursorOfTrackDown},
        {TabCommand::PauseMidi, &Tab::midiPause},
        {TabCommand::OpenReprise, &Tab::openReprise},
        {TabCommand::Drums, &Tab::changeDrumsFlag}};
};


//some usefull

int translateDenum(byte den);

int translaeDuration(byte dur);


////////////////////Pack reference - used only for GMY format///////////////////////////

	//pack guide:
	//fret - 6 bit; volume - 6 bit; fingering - 4 bit; = 2bytes
	//...on another mind 3 bits could be used as flags - effects,fingering+else
	//or volume could be packed in smaller distance [0-8] or [0-16]
    //pack effects into one more byte, and if bend is present, then:
   
   // semisemisemi tone (1/8) semisemi tone (1/4) semi tone (1/2) tone (1) double tone (2) - 5 bits for presence of each
   //duration of point - (double, single, half, 4, 8, 16, 32, 64) - 8 bits
   //+ 1 bit flag of last point
   //+ 1 bit flag of not finishing bend
   //+ 1 bit flag of continued bend

	//So in normal mode minimal note will have 3 bytes; another mode - extra packing
	//0-31 fret with secial coding (limits amount of frets - but always more then 24)
	//8 values for volume - forte piano stype - 3 bits 
	
	//next byte is really hard but helps compression - 
	//if hierest bit is == 1 then effects follow
	//if hierest bit it == 0 then 
						// if next bit == 1 then we have note of same volume - and fret stored inside;
						//next note comes in same way
	//another secret 0-31 for fret and 3 bits for [effects precence1 bit] 2 reserved - volume as duration will store in Beat
	//change precence - in Beat will show that duration changin, or volume is changing - for extra high packing
	//NO	MO	MINDS	ANOUT	HIGH	PACK. main ideas stored here now




#endif // TAB_H
