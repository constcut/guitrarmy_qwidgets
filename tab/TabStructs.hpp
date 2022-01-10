#ifndef TABSTRUCTS_HPP
#define TABSTRUCTS_HPP

#include "Chain.hpp"
#include <list>

namespace gtmy {

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
}




#endif // TABSTRUCTS_HPP
