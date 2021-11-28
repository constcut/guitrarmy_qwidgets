#ifndef ABITARRAY_H
#define ABITARRAY_H

#include "types.h"

#include <map>

class Package //TODO review
{
protected:

    byte type;
    void *point;

public:

    Package();//:type(0),point(0){}


    void setType(byte newType);// { type = newType; }
    byte getType();// { return type; }

    void setPointer(void *newPoint);// { point = newPoint; }
    void *getPointer();// { return point; }
    bool createPointer();// { return false; } //creates new from type

    //here will appear also some way to pack data into turned on bits
    // a - universal packer - will spend time for nothing
    // b - short int as type of pack and structures to define values
    // and pointer to store it

    //tremolo + bend - BendPoints
    //grace note
    //trill picking
    //chord diagram
    //text + - marker

};

class ABitArray // Vector bool?
{
protected:
    size_t bits;

public:

    ABitArray();
    bool get(byte index);
    void set(byte index, bool value);

    size_t takeBits(){ return bits;}
    void putBits(size_t newBits) { bits = newBits; }

    bool inRange(byte lowIndex, byte highIndex);

    void logIt();

    bool empty();

    bool operator==(byte index);// { return get(index); }
    bool operator!=(byte index);//

    void flush();

};


class EffectsPack : public ABitArray
{
protected:

    std::map<byte,Package> packMap;

public:

    void addPack(byte index, byte type, void *point);
    void addPack(byte index, Package pack);
    Package* getPack(byte index);

    //operator +=
    void mergeWith(EffectsPack &addition);



};


class EffectsMap
{
protected:
    std::map<size_t,EffectsPack> mapOfEffects;

public:

    bool isThere(size_t index);
    EffectsPack getEffect(size_t index);
    void setEffect(size_t index, EffectsPack eff);

};


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






#endif // ABITARRAY_H
