#include "Beat.hpp"
#include <iostream>


using namespace gtmy;


void BendPoints::insertNewPoint(BendPoint bendPoint)
{
    for (size_t i = 0; i < size()-1; ++i)
    {
        if (at(i).horizontal < bendPoint.horizontal)
            if (at(i+1).horizontal > bendPoint.horizontal)
            {
                insert(begin() + i + 1, bendPoint);
                return;
            }
        if (at(i).horizontal == bendPoint.horizontal)
        {
            at(i).vertical = bendPoint.vertical;
            return;
        }
    }
    push_back(bendPoint);
}


void Beat::clone(Beat *from)
{
   auto eff = from->getEffects();
   effPack.mergeWith(eff);
   _duration = from->_duration;
   _durationDetail = from->_durationDetail;
   _dotted = from->_dotted;
   _isPaused = from->_isPaused;
   _noticeText = from->_noticeText;

   _bookmarkName = from->_bookmarkName;

   for (size_t i = 0; i < from->size(); ++i)
   {
       Note *note = from->at(i).get();
       auto newNote = std::make_unique<Note>();
       newNote->clone(note);
       push_back(std::move(newNote));
   }
}


void Beat::printToStream(std::ostream &stream) const
{
    stream << "Outputing #"<<size()<<" Notes."<<std::endl;
    for (size_t ind = 0; ind < size(); ++ind)
            at(ind)->printToStream(stream);
}

ABitArray Beat::getEffects()
{
    return effPack;
}

void Beat::setEffects(Effect eValue)
{
    if (eValue == Effect::None)
         effPack.flush();
    else
        effPack.setEffectAt(eValue,true);
}
