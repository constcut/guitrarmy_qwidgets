#ifndef TAB_H
#define TAB_H

#include "types.h"

#include <unordered_map>
#include <list>

#include "tab/commands.h"
#include "track.h"

#include <QDebug> //TODO удалить

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

    void setSignsTillEnd(size_t num, size_t denom);
    void changeTrackVolume(size_t newVol);
    void changeTrackName(std::string newName);
    void changeTrackInstrument(size_t val);
    void changeTrackPanoram(size_t val);
    void setMarker(std::string text);

    void gotoBar(size_t pos);
    void saveAs(std::string filename);
    void closeReprise(size_t count);


    void addMacro(TrackCommand command) {
        macroCommands.push_back(command);
    }
    void addMacro(StringCommand<TrackCommand> command) {
        macroCommands.push_back(command);
    }
    void addMacro(TwoIntCommand<TrackCommand> command) {
        macroCommands.push_back(command);
    }

    void playCommand(MacroCommand& command) {
        if (std::holds_alternative<TabCommand>(command)) {
            onTabCommand(std::get<TabCommand>(command)); //TODO флаг записи
        }
        else if (std::holds_alternative<TrackCommand>(command)) {
            at(currentTrack)->onTrackCommand(std::get<TrackCommand>(command));
        }
        else if (std::holds_alternative<IntCommand<TabCommand>>(command)) {
            auto paramCommand = std::get<IntCommand<TabCommand>>(command);
            if (intHandlers.count(paramCommand.type))
                (this->*intHandlers.at(paramCommand.type))(paramCommand.parameter);
        }
        else if (std::holds_alternative<TwoIntCommand<TabCommand>>(command)) {
            auto paramCommand = std::get<TwoIntCommand<TabCommand>>(command);
            if (twoIntHandlers.count(paramCommand.type))
                (this->*twoIntHandlers.at(paramCommand.type))(paramCommand.parameter1, paramCommand.parameter2);
        } else if (std::holds_alternative<StringCommand<TabCommand>>(command)) {
            auto paramCommand = std::get<StringCommand<TabCommand>>(command);
            if (stringHandlers.count(paramCommand.type))
                (this->*stringHandlers.at(paramCommand.type))(paramCommand.parameter);
        }
        else
            qDebug() << "unhandled";
    }

    const std::vector<MacroCommand>& getMacro() const { return macroCommands; }

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
        {TabCommand::Drums, &Tab::changeDrumsFlag},
        {TabCommand::NewTrack, &Tab::createNewTrack}};


    std::unordered_map<TabCommand, void (Tab::*)(size_t)> intHandlers =  {
        {TabCommand::Instument, &Tab::changeTrackInstrument},
        {TabCommand::GotoBar, &Tab::gotoBar},
        {TabCommand::CloseReprise, &Tab::closeReprise},
        {TabCommand::Panoram, &Tab::changeTrackPanoram},
        {TabCommand::Volume, &Tab::changeTrackVolume},
    };

    std::unordered_map<TabCommand, void (Tab::*)(size_t, size_t)> twoIntHandlers =  {
        {TabCommand::SetSignTillEnd, &Tab::setSignsTillEnd},
    };

    std::unordered_map<TabCommand, void (Tab::*)(std::string)> stringHandlers =  {
        {TabCommand::Name, &Tab::changeTrackName},
        {TabCommand::Name, &Tab::saveAs}
    };

    std::vector<MacroCommand> macroCommands;
};


//some usefull

int translateDenum(std::uint8_t den);

int translaeDuration(std::uint8_t dur);



#endif // TAB_H
