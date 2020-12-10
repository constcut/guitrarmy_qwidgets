#ifndef ASTRING_H
#define ASTRING_H
//let legacy live here

#include "g0/tab.h"

#include "g0/astreaming.h"



class TextList
{
    char fields[16][128];
    int cursor;
    public:

    TextList();

    void setFields(int listNumber);
    void print();
    void createFromBar(Bar *bar);

    int addBeatToFields(Beat *beat);
};


class TextTrack
{
protected:
    std::vector<TextList> texts;

public:

    void createFromTrack(Track *currentTrack);
    void print();

};

#endif // ASTRING_H
