#include "effects.h"
//.....................Effects.........................................

void EffectsPack::addPack(std::uint8_t index, std::uint8_t type, void *point)
{
    Package pack;
    pack.setPointer(point);
    pack.setType(type);
    packMap[index] = pack;
}

void EffectsPack::addPack(std::uint8_t index, Package pack)
{
    packMap[index] = pack;
}

Package *EffectsPack::getPack(std::uint8_t index)
{
    std::map<std::uint8_t,Package>::iterator itFind = packMap.find(index);
    if (itFind != packMap.end())
    {
       Package *pointer = &(itFind->second);
       return pointer;
    }
    return 0;
}

void EffectsPack::mergeWith(EffectsPack &addition)
{
    for (std::map<std::uint8_t,Package>::iterator itEff=addition.packMap.begin();
         itEff!=addition.packMap.end(); ++itEff)
    {
        //check for overwite attention
        packMap[itEff->first]=itEff->second;
    }

    bits |= addition.bits;
}

//....................EffectsMap....................................

bool EffectsMap::isThere(size_t index)
{
    std::map<size_t,EffectsPack>::iterator effIterator = mapOfEffects.find(index);
    if (effIterator == mapOfEffects.end())
        return false;
    return true;
}

EffectsPack EffectsMap::getEffect(size_t index)
{
    std::map<size_t,EffectsPack>::iterator effIterator = mapOfEffects.find(index);
    if (effIterator == mapOfEffects.end())
    {
        EffectsPack emptyPack;
        return emptyPack;
    }
    return effIterator->second;
}

void EffectsMap::setEffect(size_t index, EffectsPack eff)
{
    mapOfEffects[index] = eff;
}

/////////////////Bit array and Package/////////////////////////////////

ABitArray::ABitArray():bits(0)
{
}

bool ABitArray::get(std::uint8_t index)
{
    size_t maskValue = 1;
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

bool ABitArray::operator==(std::uint8_t index)
{
    return get(index);
}

bool ABitArray::operator!=(std::uint8_t index)
{
    return !get(index);
}

void ABitArray::flush()
{
    bits = 0;
}

void ABitArray::set(std::uint8_t index, bool value)
{
    size_t maskValue = 1;
    maskValue <<= (index-1); //-1

    if (value==true)
    { //turn on
        bits |= maskValue;
    }
    else
    { //turn off
        size_t antiMask = 0;
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

bool ABitArray::inRange(std::uint8_t lowIndex, std::uint8_t highIndex)
{
    bool wasThere = false;

    for (std::uint8_t index=lowIndex; index != (highIndex+1); ++index)
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


void Package::setType(std::uint8_t newType) {
    type = newType;
}

std::uint8_t Package::getType() {
    return type;
}

void Package::setPointer(void *newPoint) {
    point = newPoint;
}

void *Package::getPointer() { //TODO
    return point;
}

bool Package::createPointer() {
    return false;
} //creates new from type
