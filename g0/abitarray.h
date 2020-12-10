#ifndef ABITARRAY_H
#define ABITARRAY_H

#include "types.h"

#include <map>

class Package
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

class ABitArray
{
protected:
    ul bits;

public:

    ABitArray();
    bool get(byte index);
    void set(byte index, bool value);

    ul takeBits(){ return bits;}
    void putBits(ul newBits) { bits = newBits; }

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
    std::map<ul,EffectsPack> mapOfEffects;

public:

    bool isThere(ul index);
    EffectsPack getEffect(ul index);
    void setEffect(ul index, EffectsPack eff);

};



#endif // ABITARRAY_H
