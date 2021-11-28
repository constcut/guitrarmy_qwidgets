#include "beat.h"


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
   effPack.mergeWith(from->effPack);
   duration = from->duration;
   durationDetail = from->durationDetail;
   dotted = from->dotted;
   isPaused = from->isPaused;
   noticeText = from->noticeText;

   bookmarkName = from->bookmarkName;

   for (size_t i = 0; i < from->size(); ++i)
   {
       Note *note = from->at(i);
       Note *newNote=new Note();
       newNote->clone(note);
       push_back(newNote);
   }
}


void Beat::printToStream(std::ostream &stream)
{
    stream << "Outputing #"<<size()<<" Notes."<<std::endl;
    for (size_t ind = 0; ind < size(); ++ind)
            at(ind)->printToStream(stream);
}

EffectsPack Beat::getEffects()
{
    return effPack;
}

void Beat::setEffects(std::uint8_t eValue)
{
    if (eValue==0)
    {
         effPack.flush();
    }
        else
    effPack.set(eValue,true);
}
