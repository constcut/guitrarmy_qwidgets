#ifndef TAB_H
#define TAB_H

#include "types.h"

#include <memory.h> //TODO remove
#include <unordered_map>
//TODO Tab, Track, Bar, Beat, Note - sepparate all the files

#include "abitarray.h"
#include "tabcommands.h" //move into g0

#include "track.h"

int updateDurationWithDetail(byte detail, int base);

class Track;


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
