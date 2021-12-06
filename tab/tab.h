#ifndef TAB_H
#define TAB_H

#include "types.h"

#include <unordered_map>
#include <list>
#include "tab/tabcommands.h"

#include "track.h"

int updateDurationWithDetail(std::uint8_t detail, int base);

class Track;


//here would lay Guitar Pro header comptibilator
struct MidiChannelInfo
{
    unsigned int instrument; //refact types

    std::uint8_t volume;
    std::uint8_t balance;
    std::uint8_t chorus;
    std::uint8_t reverb;

    std::uint8_t phaser;
    std::uint8_t tremolo;
    std::uint8_t blank1;
    std::uint8_t blank2;
};


struct VariableString
{
    std::uint8_t lineType;
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
    std::uint8_t type;
    int value;

    TimeLineKnot(std::uint8_t newType, int newValue):type(newType),value(newValue){}
};


class Tab : public ChainContainer<Track, void>
{
public:

    Tab() :isPlaying(false), displayTrack(0), currentTrack(0),
        currentBar(0), displayBar(0), lastOpenedTrack(0) {}

    std::vector<TimeLineKnot> timeLine;

    virtual ~Tab() = default;

    void printToStream(std::ostream &stream);

    void createTimeLine(size_t shiftTheCursor=0);
    std::uint8_t getBPMStatusOnBar(size_t barN);
    int getBpmOnBar(size_t barN);

    void connectTracks(){
        for (size_t i = 0; i < size(); ++i)
        at(i)->connectAll();
        createTimeLine();
    }

    void postGTP() {
        for (size_t i = 0; i < size(); ++i) {
            size_t port = at(i)->getGPCOMPInts(0);
            size_t chan = at(i)->getGPCOMPInts(1);
            size_t ind = (chan-1) + (port-1)*16;
            if (ind < 70) {
                int instr = GpCompMidiChannels[ind].instrument;
                std::uint8_t pan = GpCompMidiChannels[ind].balance;
                std::uint8_t vol = GpCompMidiChannels[ind].volume;
                Track *t = at(i).get();
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
    std::uint8_t GpCompSignKey;
    std::uint8_t GpCompSignOctave; //TODO get rid
    std::uint8_t GpCompTripletFeel;

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
    void createNewTrack();

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

int translateDenum(std::uint8_t den);

int translaeDuration(std::uint8_t dur);



#endif // TAB_H
