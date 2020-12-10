#include "abitarray.h"

//loging
#include "g0/astreaming.h"
static AStreaming logger("abitarray");



//.....................Effects.........................................

void EffectsPack::addPack(byte index, byte type, void *point)
{
    Package pack;
    pack.setPointer(point);
    pack.setType(type);
    packMap[index] = pack;
}

void EffectsPack::addPack(byte index, Package pack)
{
    packMap[index] = pack;
}

Package *EffectsPack::getPack(byte index)
{
    std::map<byte,Package>::iterator itFind = packMap.find(index);
    if (itFind != packMap.end())
    {
       Package *pointer = &(itFind->second);
       return pointer;
    }
    return 0;
}

void EffectsPack::mergeWith(EffectsPack &addition)
{
    for (std::map<byte,Package>::iterator itEff=addition.packMap.begin();
         itEff!=addition.packMap.end(); ++itEff)
    {
        //check for overwite attention
        packMap[itEff->first]=itEff->second;
    }

    bits |= addition.bits;
}

//....................EffectsMap....................................

bool EffectsMap::isThere(ul index)
{
    std::map<ul,EffectsPack>::iterator effIterator = mapOfEffects.find(index);
    if (effIterator == mapOfEffects.end())
        return false;
    return true;
}

EffectsPack EffectsMap::getEffect(ul index)
{
    std::map<ul,EffectsPack>::iterator effIterator = mapOfEffects.find(index);
    if (effIterator == mapOfEffects.end())
    {
        EffectsPack emptyPack;
        return emptyPack;
    }
    return effIterator->second;
}

void EffectsMap::setEffect(ul index, EffectsPack eff)
{
    mapOfEffects[index] = eff;
}

/////////////////Bit array and Package/////////////////////////////////

ABitArray::ABitArray():bits(0)
{
}

bool ABitArray::get(byte index)
{
    ul maskValue = 1;
    maskValue <<= (index-1); //-1

    if (index == 1)
    {
       logIt();
    //log << "Get index"<<index<<" mask "
    //<<maskValue<<" result "<<(bits & maskValue);
     //log << "log";
    }

    return bits & maskValue;
}

bool ABitArray::operator==(byte index)
{
    return get(index);
}

bool ABitArray::operator!=(byte index)
{
    return !get(index);
}

void ABitArray::flush()
{
    bits = 0;
}

void ABitArray::set(byte index, bool value)
{
    ul maskValue = 1;
    maskValue <<= (index-1); //-1

    if (value==true)
    { //turn on
        bits |= maskValue;
    }
    else
    { //turn off
        ul antiMask = 0;
        --antiMask;

        antiMask -= maskValue;

        bits &= antiMask;
    }

    //log << "Set "<<index<<" value "<<value<<" mask "<<maskValue<<" result "<<bits;
    //log << "log";

}

void ABitArray::logIt()
{
    //log<<"Output effects bit array "<<bits;
}

bool ABitArray::inRange(byte lowIndex, byte highIndex)
{
    bool wasThere = false;

    for (byte index=lowIndex; index != (highIndex+1); ++index)
    {
        wasThere |= get(index);
        //log<<"InRange "<<index<<" of " <<(highIndex)<<" was? "<<wasThere;
    }

    return wasThere;
}


bool ABitArray::empty()
{
    return bits == 0;
}


///////////PAck//////////////////////////////

Package::Package():type(0),point(0){}


void Package::setType(byte newType)
{
    type = newType;
}
byte Package::getType()
{
    return type;
}

void Package::setPointer(void *newPoint)
{
    point = newPoint;
}
void *Package::getPointer()
{
    return point;
}

bool Package::createPointer()
{
    return false;
} //creates new from type
