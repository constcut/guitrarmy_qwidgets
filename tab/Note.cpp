#include "Note.hpp"

#include <iostream>

using namespace gtmy;


void Note::clone(Note *from)
{
    _fret = from->_fret;
    _volume = from->_volume;
    _noteState = from->_noteState;
    stringNumber = from->stringNumber;
    auto eff = from->getEffects();
    effPack.mergeWith(eff);
}

void Note::printToStream(std::ostream &stream) const
{
   stream << "Fret = " <<(int)this->_fret << std::endl;
}

const ABitArray& Note::getEffects() const {
    return effPack;
}

ABitArray& Note::getEffRef() {
   return effPack;
}

void Note::setEffect(Effect eff)
{
    if (eff == Effect::None)
        effPack.flush();
    else
        effPack.setEffectAt(eff,true);
}
