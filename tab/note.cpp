#include "note.h"

void Note::clone(Note *from)
{
    fret = from->fret;
    volume = from->volume;
    noteState = from->noteState;
    stringNumber = from->stringNumber;
    effPack.mergeWith(from->effPack);
}

void Note::printToStream(std::ostream &stream)
{
   stream << "Fret = " <<(int)this->fret << std::endl;
}

EffectsPack Note::getEffects()
{
    return effPack;
}

void Note::setEffect(std::uint8_t eff)
{
    if (eff==0)
    {
        effPack.flush();
    }
        else
    effPack.set(eff,true);
}
