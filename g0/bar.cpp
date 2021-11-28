#include "bar.h"

#include <QDebug>

bool barLog = false;


int translateDenum(byte den) //TODO static?
{
    switch (den)
    {
       case 1: return 4000;
       case 2: return 2000;
       case 4: return 1000;
       case 8: return 500;
       case 16: return 250;
       case 32: return 125;
    }

    return 0;
}

int translaeDuration(byte dur)
{
    switch (dur)
    {
       case 0: return 4000;
       case 1: return 2000;
       case 2: return 1000;
       case 3: return 500;
       case 4: return 250;
       case 5: return 125;
    }

    return 0;
}

int updateDurationWithDetail(byte detail, int base)
{
    int result = base;

    switch (detail)
    {
       case 3:
       case 6:
       case 12:
           result = base*2.0/3.0;
        break;

       case 5:
       case 10:
           result = base*4.0/5.0;
        break;

       case 7:
           result = base*4.0/7.0;
        break;

       case 9:
           result = base*8.0/9.0;
        break;

       case 11:
           result = base*8.0/11.0;
        break;

       case 13:
           result = base*8.0/13.0;
        break;
    }

    return result;
}



void Bar::clone(Bar *from) {
   flush();

   signatureDenum = from->signatureDenum;
   signatureNum = from->signatureNum;

   for (ul i = 0; i < from->len(); ++i)
   {
       Beat *beat = from->at(i);
       Beat *newBeat=new Beat();

       newBeat->clone(beat);
       add(newBeat);
   }
}

ul Bar::getCompleteIndex()
{
   return completeIndex;
}


double Bar::getCompleteAbs()
{
    return completeAbs;
}


//REPLACE with name CALCULATE COMPLETE status
//and add just getCompleteStatus

byte Bar::getCompleteStatus()
{
    byte completeStatus = 255; //calculation failed

    byte thatNum = getSignNum();
    byte thatDen = getSignDenum();

    int barAbs = translateDenum(thatDen)*thatNum;

    int usedAbs = 0;
    int lastAbs = 0;

    completeIndex = 0;

    ul barSize = len();

    for (ul i = 0; i < barSize; ++i)
    {
       byte beatDur = at(i)->getDuration();
       byte durDetail = at(i)->getDurationDetail();
       byte isDotted = at(i)->getDotted();

       int localAbs = translaeDuration(beatDur);

       if (durDetail)
           localAbs = updateDurationWithDetail(durDetail,localAbs);

       if (isDotted&1)
       {
           //first only one dot
           localAbs *= 3;
           localAbs /= 2;
       }

       usedAbs += localAbs;

       //qDebug() << "On "<<i<<" used = "<<usedAbs;

       if ((i+1) != barSize) //without case of last
       if (usedAbs == barAbs)
       {
           completeStatus=2; //exceed
           completeIndex=i;
           completeAbs=localAbs;
           return completeStatus;
       }

       if (usedAbs > barAbs)
       {
           completeStatus=2; //exceed
           completeIndex=i;

           int exceedAbs = usedAbs - barAbs;

           if (barLog)
           qDebug() << "EXCEED abs "<<exceedAbs <<" localAbs "<<localAbs<<" Ind "<<completeIndex;

           short newNoteAbs = localAbs - exceedAbs;

           if (barLog)
           qDebug() << "New note "<<newNoteAbs;

           if (newNoteAbs > 10)
               lastAbs = newNoteAbs;
           else
           {
              --completeIndex; //to small for new note
               qDebug() <<"SHIFTBACK";
           }

           completeAbs = lastAbs;

           break;
       }

       lastAbs = localAbs;

    }


   if (usedAbs < barAbs)
   {
       completeIndex=0; //just for logs
       completeStatus=1; //incomplete
       lastAbs=barAbs-usedAbs;

       completeAbs = lastAbs;

       if (lastAbs < 50)
           completeStatus = 0;
   }

   if (usedAbs == barAbs)
   {
       completeStatus = 0;
   }

   //qDebug() << "Bar abs len "<< barAbs<<"; used "<<usedAbs;
   //qDebug() <<"Complete status="<<completeStatus<<"; lastAbs="<<lastAbs<<"; cInd="<<completeIndex;

    return completeStatus;
}



////////////Count bar operations ///////////////////////////////////

void Bar::countUsedSigns(byte &numGet, byte &denumGet)
{
    ul num = 0;
    ul denum = 32;

    denum *= 3;//temp action for triplets (for n trumplets better multipleyer must)
    denum *= 2;

    for (ul beatInd = 0; beatInd < len(); ++beatInd)
    {
        ul duration = at(beatInd)->getDuration();
        ul detail = at(beatInd)->getDurationDetail();

        ul addition = 0;

        switch (duration)
        { //remember 8 is 8
            case 5: addition=1; break;
            case 4: addition=2; break;
            case 3: addition=4; break;
            case 2: addition=8; break;
            case 1: addition=16; break;
            case 0: addition=32; break;
            default:
            if (barLog)  qDebug()<<"Shit in duration "<<duration;
        }

        addition *=2; //dots on 32
        addition *= 3; //triplets

        if (detail==3)
        {
            addition *= 2;
            addition /= 3;
        }

        bool byteDote = at(beatInd)->getDotted();
        if (byteDote==1)
        {
            addition += addition/2;
        }

        //recalculations for addition
         num+=addition;
        if (barLog)  qDebug()<<"Addition is "<<addition<<"; det= "<<detail
             <<" dur= "<<duration<<" full "<<num<<"; dot "<<byteDote;
    }

    //decre

    while ( (num%2==0)&&(denum%2==0) )
    {
        num /= 2;
        denum /= 2;
    }

    while ( (num%3==0)&&(denum%3==0) )
    {
        num /= 3;
        denum /= 3;
    }

    numGet = num;
    denumGet = denum;
}


void Bar::printToStream(std::ostream &stream)
{
    stream << "Outputing #"<<len()<<" Beats."<<std::endl;
    for (ul ind = 0; ind < len(); ++ind)
            at(ind)->printToStream(stream);
}
