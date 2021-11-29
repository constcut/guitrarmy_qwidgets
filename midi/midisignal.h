#ifndef MIDISIGNAL_H
#define MIDISIGNAL_H

#include <list>
#include <vector>

#include <deque>
#include <fstream>



class NBytesInt : public std::deque<std::uint8_t> {

public:
    NBytesInt(){}
    NBytesInt(std::uint32_t source);

    std::uint32_t readStream(std::ifstream& f);
    std::uint32_t writeStream(std::ofstream& f);

    std::uint32_t getValue();
};



class MidiSignal
{
public:

    std::uint8_t getEventType() ;
    std::uint8_t getChannel();
    bool isMetaEvent();

    MidiSignal();
    MidiSignal(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2=0, std::uint32_t timeShift=0);

    std::uint32_t calculateSize(bool skipSomeMessages=false);

    bool canSkipThat(); ///TODO doublecheck

    std::uint32_t readStream(std::ifstream& f);
    std::uint32_t writeStream(std::ofstream& f, bool skipSomeMessages=false); //?Todo review name skip

    std::string nameEvent(std::int8_t eventNumber);
    std::string nameController(std::uint8_t controllerNumber);

    double getSecondsLength(double bpm=120.0);

    const std::vector<std::uint8_t>& getMetaInfo() { return metaBufer; }

public: //TODO :(
    NBytesInt timeStamp;

    std::uint8_t byte0; //type + channel //TODO rename typeAndChannel
    std::uint8_t param1, param2; //parameters //TODO rename paramter1, parameter2

public: //TODO cover with push functions

    double absoluteTime; //bad code detected

    NBytesInt metaLen;
    std::vector<std::uint8_t> metaBufer;
};

#endif // MIDISIGNAL_H
