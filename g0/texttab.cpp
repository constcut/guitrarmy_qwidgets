#include "texttab.h"


////////////////TEXT BAR/////////////////////////////////

TextList::TextList()
{
    setFields(1);
} //init

void TextList::setFields(int listNumber)
{
    cursor = 10;
    for (ul i=0; i<16; ++i)
    for (ul j=0; j<128;++j)
    fields[i][j]=' ';
    fields[0][0]='x';
    for (ul i=0; i<6; ++i)
    fields[i+1][0]=48+i+1;
    for (ul i=0;i < 7; ++i)
    {
    fields[i][1]=fields[i][2]
    =fields[i][4]='!';
    fields[i][3]=fields[i][77]='#';
    fields[i][79]=0;
    fields[i][78]=fields[i][76]='.';
    //ields[i][0]=i+1;
    }
    fields[6][77]=fields[0][3]=listNumber+48; //9 lists only first then to other
}

void TextList::print()
{
    std::cout << "**********"<<std::endl << "**********"<<std::endl;
    for (ul i=0; i < 7; ++i)
    {
        //fields[i][60] = 0;
        std::cout << fields[i]<<std::endl;
    }
    std::cout << "**********"<<std::endl;
}

void TextList::createFromBar(Bar *bar)
{
    Bar *cBar=bar;
    Beat *cBeat=cBar->getV(0);
    for (ul i = 0; i < cBar->len(); ++i)
    {
        //log << "Adding beat #"<<i;
        if (addBeatToFields(cBeat)==0)
        {
        //create new, connect with old, push beat to it, replace old ptr with new
        }
        ++cBeat;
    }
}

int TextList::addBeatToFields(Beat *beat)
{
    int stringsAmount = beat->len();

    fields[0][cursor] = 48+beat->getDuration();

    //log << "N "<<stringsAmount << " strings there." <<" Beat mark"<<fields[0][cursor];
    //for each string

    for (int i = 0; i < stringsAmount; ++i)
    {
        //fields[cursor][stringnumber]

        Note *note =beat->getV(i);
        int stringnumber = 7-note->getStringNumber();//fucking too much?
        //fucking gtp format((

        int fret = note->getFret();
        fields[stringnumber][cursor]=((fret-fret%10)/10)+48;
        fields[stringnumber][cursor+1]=fret%10+48;
                cursor += 3;

    }
    //if note present push to textfields
    //shift if needed createNew
    return 0;
}

//////////////////////TEXT TRACK////////////////////////


void TextTrack::createFromTrack(Track *currentTrack)
{
    ul trackLen = currentTrack->len();

    for (ul barIndex = 0; barIndex < trackLen; ++barIndex)
    {
        TextList newTextList;
        Bar *currentBar = currentTrack->getV(barIndex);
        newTextList.createFromBar(currentBar);
        texts.push_back(newTextList);
    }
}

void TextTrack::print()
{
    for (size_t i = 0; i < texts.size(); ++i)
    {
        texts[i].print();
    }
}

/////////////////////TEXT BAR////////////////////////////
