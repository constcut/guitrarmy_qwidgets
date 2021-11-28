#include "beat.h"

void Beat::clone(Beat *from)
{
   effPack.mergeWith(from->effPack);
   duration = from->duration;
   durationDetail = from->durationDetail;
   dotted = from->dotted;
   isPaused = from->isPaused;
   noticeText = from->noticeText;

   bookmarkName = from->bookmarkName;

   for (ul i = 0; i < from->len(); ++i)
   {
       Note *note = from->at(i);
       Note *newNote=new Note();
       newNote->clone(note);
       add(newNote);
   }
}


void Beat::printToStream(std::ostream &stream)
{
    stream << "Outputing #"<<len()<<" Notes."<<std::endl;
    for (ul ind = 0; ind < len(); ++ind)
            at(ind)->printToStream(stream);
}

EffectsPack Beat::getEffects()
{
    return effPack;
}

void Beat::setEffects(byte eValue)
{
    if (eValue==0)
    {
         effPack.flush();
    }
        else
    effPack.set(eValue,true);
}
