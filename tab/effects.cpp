#include "effects.h"
//.....................Effects.........................................

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

void ABitArray::mergeWith(ABitArray& addition) {
    bits |= addition.bits;
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


