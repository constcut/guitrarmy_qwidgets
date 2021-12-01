#include "note.h"

#include <iostream>

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

ABitArray Note::getEffects()
{
    return effPack;
}

void Note::setEffect(Effect eff)
{
    if (eff == Effect::None)
        effPack.flush();
    else
        effPack.setEffectAt(eff,true);
}
