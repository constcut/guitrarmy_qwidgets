#include "Effects.hpp"
//.....................Effects.........................................


using namespace gtmy;

ABitArray::ABitArray():bits(0)
{
}

bool ABitArray::getEffectAt(Effect id)
{
    uint8_t index = static_cast<uint8_t>(id);

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

bool ABitArray::operator==(Effect id)
{
    return getEffectAt(id);
}

bool ABitArray::operator!=(Effect id)
{
    return !getEffectAt(id);
}

void ABitArray::flush()
{
    bits = 0;
}

void ABitArray::setEffectAt(Effect id, bool value)
{
    uint8_t index = static_cast<uint8_t>(id);
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

void ABitArray::mergeWith(ABitArray& addition) {
    bits |= addition.bits;
}

void ABitArray::logIt()
{
    //log<<"Output effects bit array "<<bits;
}

bool ABitArray::inRange(Effect lowId, Effect highId)
{
    bool wasThere = false;

    uint8_t lowIndex = static_cast<uint8_t>(lowId);
    uint8_t highIndex = static_cast<uint8_t>(highId);

    for (std::uint8_t index=lowIndex; index != (highIndex+1); ++index)
    {
        Effect id = static_cast<Effect>(index);
        wasThere |= getEffectAt(id);
        //log<<"InRange "<<index<<" of " <<(highIndex)<<" was? "<<wasThere;
    }

    return wasThere;
}


bool ABitArray::empty()
{
    return bits == 0;
}


