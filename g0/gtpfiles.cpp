#include "gtpfiles.h"

#include <QDebug>
#include <fstream>

bool gtpLog = false;

////////////////////////////////////////////////////////////////////////////
char miniBufer[20480];




//Some of the logs disabled yet


std::string readString(std::ifstream &file, ul stringLen)
{
   //for current max
   //refact dynamic

   if (gtpLog)  qDebug() <<"Reaging string "<<stringLen<<" bytes";
   file.read((char*)miniBufer,stringLen);//changed attention
   miniBufer[stringLen] = 0;

   if (gtpLog)  qDebug() << "Readen string "<<miniBufer;

   std::string response = std::string(miniBufer);
   return response;
}

std::string readString(std::ifstream &file)
{
    ul stringLen = 0;
    std::string response = "";

    file.read((char*)&stringLen,4);

    if (stringLen > 0)
    {
        if (gtpLog)
            qDebug()<<"String len "<<stringLen;
        response = readString(file,stringLen); //attention and check old
    }
    else
    {
        if (gtpLog)  qDebug() << "String is empty!";
    }
    return response;
}

std::string readStringShiByte(std::ifstream &file)
{
    ul stringLen = 0;
    std::string response = "";

    file.read((char*)&stringLen,4);

    byte whatFo = 0; file.read((char*)&whatFo,1);

    if (stringLen == 0)
        stringLen = whatFo;
    else
    {
        --stringLen;
    }


    if (stringLen > 0)
    {
        if (gtpLog)
            qDebug()<<"String len "<<stringLen;
        response = readString(file,stringLen); //attention and check old
    }
    else
    {
        if (gtpLog)  qDebug() << "String is empty!";
    }
    return response;
}


void writeBendGTPOLD(std::ofstream *file, BendPointsGPOld *bend)
{
    //saved to be somewhere in the code
    byte bendType=bend->getType();
    file->write((const char*)&bendType,1);


    ul bendHeight = bend->getHeight();
    file->write((const char*)&bendHeight,4); //attention again

    ul pointsCount = bend->len();
    file->write((const char*)&pointsCount,4);

    for (ul pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        BendPointGPOld *point = &bend->getV(pointInd);

        ul absolutePosition = point->absolutePosition;
        ul verticalPosition = point->heightPosition;
        byte vibratoFlag = point->vibratoFlag;

        file->write((const char*)&absolutePosition,4);
        file->write((const char*)&verticalPosition,4);
        file->write((const char*)&vibratoFlag,1);
    }
}

void writeBendGTP(std::ofstream *file, BendPoints *bend)
{
    byte bendType=bend->getType();
    if (bendType>=1) bendType += 5;

    file->write((const char*)&bendType,1);

    //need to find a hieghest point - but this is only for output
    ///ul bendHeight = bend->getHeight();
    ///file->write((const char*)&bendHeight,4); //attention again

    ul pointsCount = bend->len();
    file->write((const char*)&pointsCount,4);

    for (ul pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        BendPoint *point = &bend->getV(pointInd);

        ul absolutePosition = point->horizontal;
        ul verticalPosition = point->vertical*25;
        byte vibratoFlag = point->vFlag;

        file->write((const char*)&absolutePosition,4);
        file->write((const char*)&verticalPosition,4);
        file->write((const char*)&vibratoFlag,1);
    }
}


void readBendGTP(std::ifstream *file, BendPoints *bend)
{
    byte bendType=0;
    file->read((char*)&bendType,1);

    ul bendHeight = 0;
    file->read((char*)&bendHeight,4); //attention again
    //25 quarter ; 75 - 3 quartes
    //50 half ; 100 - tone
    //... 300 - three tones
    ul pointsCount = 0;
    file->read((char*)&pointsCount,4);

    if (gtpLog)  qDebug()<< "Type "<<bendType<<"; Height "<<bendHeight<<"; N= "<<pointsCount;

    if (bendType >= 6)
        bend->setType(bendType-5); //temp action, yet not handled anyway
    else
        bend->setType(0);

    for (ul pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        ul absolutePosition = 0;
        ul verticalPosition = 0;
        byte vibratoFlag = 0;

        file->read((char*)&absolutePosition,4);
        file->read((char*)&verticalPosition,4);
        file->read((char*)&vibratoFlag,1);

        if (gtpLog)
            qDebug() << "Point# "<< pointInd << "; absPos="<<absolutePosition<<"; vertPos="
               <<verticalPosition<<"; vibrato- "<<vibratoFlag;


        BendPoint point;
        point.vertical = verticalPosition/25; //25 - 1/12 of 3 tones
        point.horizontal = absolutePosition;
        point.vFlag = vibratoFlag;
        bend->add(point);
    }

    if (gtpLog)  qDebug() << "Beng if (gtpLog)  logging finished with "<<(int)bend->len();

    //return bend;
}




void readChordDiagramGP3(std::ifstream &file)
{
       if (gtpLog)  qDebug() << "CHORD_";

    int header=0;
    file.read((char*)&header,1);

    if (header&0x01)
    {
        char skipper[40];
        file.read((char*)skipper,25);

        file.read((char*)skipper,1); //read byte ze

        file.read((char*)skipper,34);
        skipper[34] = 0;
        ///std::string chStr = std::string(skipper);
        int firstFret = 0;
        file.read((char*)&firstFret,4);

        for (int i = 0; i < 6; ++i)
        {
            int fret =0;
            file.read((char*)&fret,4);
        }

        file.read((char*)skipper,36);
    }
    else
    {
         std::string chStr = readStringShiByte(file);
         int firstFret = 0;
         file.read((char*)&firstFret,4);

         if (firstFret != 0)
             for (int i = 0; i < 6; ++i)
             {
                 int fret =0;
                 file.read((char*)&fret,4);
             }
    }
}


void readChordDiagramGP4(std::ifstream &file)
{
   if (gtpLog)  qDebug() << "CHORD";

   int header=0;
   file.read((char*)&header,1);


   char skipper[40];

    if (header&0x01)
    {

        file.read((char*)skipper,16);


        byte singleBy = 0;
        file.read((char*)&singleBy,1);

        file.read((char*)skipper,21);
        skipper[21] = 0;
        std::string chStr = std::string(skipper);

        if (gtpLog)
            qDebug() << "Ch text "<<chStr.c_str();

        int firstFret = 0;
        file.read((char*)&firstFret,4);//skli
        file.read((char*)&firstFret,4);

        for (int i = 0; i < 7; ++i)
        {
            int fret =0;
            file.read((char*)&fret,4);


        }

        file.read((char*)skipper,32);
    }
    else
    {
         //readStringShiByte(file);


         int fullLen = 0;
         file.read((char*)&fullLen,4);

         byte singleBy = 0;
         file.read((char*)&singleBy,1);

         file.read((char*)skipper,singleBy);
         skipper[singleBy] = 0;

         std::string chStr = std::string(skipper);

         if (gtpLog)
             qDebug() << "Chh text "<<chStr.c_str() ;

         int firstFret = 0;
         file.read((char*)&firstFret,4);

         if (firstFret != 0)
             for (int i = 0; i < 6; ++i)
             {
                 int fret =0;
                 file.read((char*)&fret,4);
             }
    }
}

void readChanges(std::ifstream &file, Beat *cursorBeat)
{

    Beat::ChangeTable changeStruct;// = {0};

    changeStruct.newTempo = 0;

    file.read((char*)&changeStruct.newInstr,1);  //1

    file.read((char*)&changeStruct.newVolume,1);
    file.read((char*)&changeStruct.newPan,1);
    file.read((char*)&changeStruct.newChorus,1);
    file.read((char*)&changeStruct.newReverb,1);
    file.read((char*)&changeStruct.newPhaser,1);
    file.read((char*)&changeStruct.newTremolo,1);
    file.read((char*)&changeStruct.newTempo,4); //8

    if (gtpLog)  qDebug() <<  "I "<<changeStruct.newInstr<<"; V "<<changeStruct.newVolume<<"; P "<<changeStruct.newPan<<
          "; C "<<changeStruct.newChorus<<"; R "<<changeStruct.newReverb<<"; Ph "<<changeStruct.newPhaser<<
          "; Tr "<<changeStruct.newTremolo<<"; T="<<changeStruct.newTempo;


    //NO INSTR IN DOCS
    if (changeStruct.newInstr != 255)
    {

        Beat::SingleChange instrCh;
        instrCh.changeCount = 0;
        instrCh.changeType = 1;
        instrCh.changeValue = changeStruct.newInstr;
        cursorBeat->changes.add(instrCh);

    }

    if (changeStruct.newVolume != 255)
    {
        file.read((char*)&changeStruct.volumeDur,1);

        Beat::SingleChange volCh;
        volCh.changeCount = 0;
        volCh.changeType = 2;
        volCh.changeValue = changeStruct.newVolume;
        cursorBeat->changes.add(volCh);
    }

    if (changeStruct.newPan != 255)
    {
        file.read((char*)&changeStruct.panDur,1);

        Beat::SingleChange panCh;
        panCh.changeCount = 0;
        panCh.changeType = 3;
        panCh.changeValue = changeStruct.newPan;
        cursorBeat->changes.add(panCh);
    }

    if (changeStruct.newChorus != 255)
    {
        file.read((char*)&changeStruct.chorusDur,1);

        Beat::SingleChange chorusCh;
        chorusCh.changeCount = 0;
        chorusCh.changeType = 4;
        cursorBeat->changes.add(chorusCh);
    }

    if (changeStruct.newReverb != 255)
    {
         file.read((char*)&changeStruct.reverbDur,1);

         Beat::SingleChange reverbCh;
         reverbCh.changeCount = 0;
         reverbCh.changeType = 5;
         cursorBeat->changes.add(reverbCh);
    }

    if (changeStruct.newPhaser != 255)
    {
        file.read((char*)&changeStruct.phaserDur,1);

        Beat::SingleChange phaserCh;
        phaserCh.changeCount = 0;
        phaserCh.changeType = 6;
        cursorBeat->changes.add(phaserCh);
    }

    if (changeStruct.newTremolo != 255)
    {
         file.read((char*)&changeStruct.tremoloDur,1);

         Beat::SingleChange tremoloCh;
         tremoloCh.changeCount = 0;
         tremoloCh.changeType = 7;
         cursorBeat->changes.add(tremoloCh);
    }

    //-1 for ul is hiegh but 10000 bpm insane

        if (changeStruct.newTempo < 10000) //some attention here
        {
             file.read((char*)&changeStruct.tempoDur,1);
             //set changes table inside
             Beat::SingleChange tempCh;
             tempCh.changeCount = 0;
             tempCh.changeType = 8;
             tempCh.changeValue = changeStruct.newTempo;

             cursorBeat->changes.add(tempCh);

        }


    cursorBeat->setEffects(28);
    cursorBeat->effPack.addPack(28,1,&(cursorBeat->changes));

    //refact
    file.read((char*)&changeStruct.changesTo,1); //not applied! attention


}


void readTrack(std::ifstream &file, Track *currentTrack, int gpVersion=4, int trackIndex=0, byte verInd=255)
{
    byte trackHeader = 0;
    file.read((char*)&trackHeader,1);


    if (trackHeader & 1)
    {
        currentTrack->setDrums(true);
        if (gtpLog)  qDebug() <<"This is drums track!";
    }

    if (gpVersion == 5)
    {
        //ONLY FOR FIRST
        if ((trackIndex==0)||(verInd==0))
        {
            ul byteZero = 0;
            //file.read((char*)&byteZero,4);

            file.read((char*)&byteZero,1);
        }
    }

    ul trackNameLen = 0;
    file.read((char*)&trackNameLen,1);

    if (gtpLog)  qDebug() <<"XLENTRACK " <<trackNameLen;

    char trackName[40];
    file.read((char*)trackName,40); //39? or 40?
    std::string trackNameStr(trackName);

    currentTrack->setName(trackNameStr);

    //trackName[trackNameLen] = 0;

    if (gtpLog)  qDebug() << " Track name '" << trackName << "' ; head = " <<(int)trackHeader;


    ul stringsAmount = 0;
    file.read((char*)&stringsAmount,4);

    if (gtpLog)  qDebug() <<"N strings " << (int) stringsAmount;


    ul tunes[7] = {0}; //TUNEC!!!
    for (ul ii = 0; ii < 7; ++ii)
    {
        file.read((char*)&tunes[ii],4);
    }

    currentTrack->tuning.setStringsAmount(stringsAmount);
    for (ul ii = 0; ii < stringsAmount; ++ii)
    {

        if (gtpLog)  qDebug() << "Tunes for "<<ii<<" is "<<tunes[ii];
        currentTrack->tuning.setTune(ii,tunes[ii]);
    }


    ul port=0, chan=0, chanE=0;
    file.read((char*)&port,4);
    file.read((char*)&chan,4);
    file.read((char*)&chanE,4);

    currentTrack->setGPCOMPInts(0,port);
    currentTrack->setGPCOMPInts(1,chan);
    currentTrack->setGPCOMPInts(2,chanE);

    if (gtpLog)  qDebug() << "Port " << port << "; chan " <<chan <<"; chanE "<<chanE;

    ul frets=0;
    file.read((char*)&frets,4);
    ul capo=0;
    file.read((char*)&capo,4);

    if (gtpLog)  qDebug() << "Frets " << frets <<"; capo " << capo;

    ul trackColor=0;
    file.read((char*)&trackColor,4);

    if (gtpLog)  qDebug() << "Color - "<<trackColor;

    currentTrack->setColor(trackColor);
    currentTrack->setGPCOMPInts(3,frets);
    currentTrack->setGPCOMPInts(4,capo);

    if (gpVersion==5)
    {
        char toSkip[256];

        if (verInd == 1)
            file.read((char*)toSkip,49);
        if (verInd == 0)
            file.read((char*)toSkip,44);

        ul intLen = 0;
        byte byteLen = 0;

        if (verInd == 1)
        {
            file.read((char*)&intLen,4);
            file.read((char*)&byteLen,1);
            file.read((char*)toSkip,intLen-1);
            file.read((char*)&intLen,4);
            file.read((char*)&byteLen,1);
            file.read((char*)toSkip,intLen-1);
        }

        //+after track?
    }
}

void readBeatEffects(std::ifstream &file, Beat *cursorBeat)
{
    byte beatEffectsHead1;
    byte beatEffectsHead2;

    file.read((char*)&beatEffectsHead1,1);
    file.read((char*)&beatEffectsHead2,1);

    //LARGE
    if (gtpLog)  qDebug() << "Beat effects flag present. H1=" << beatEffectsHead1 <<
           "; H2=" << beatEffectsHead2;

    //name as bools

    if (beatEffectsHead1 & 32)
    {   //tapping poping slaping

        byte tapPopSlap;
        file.read((char*)&tapPopSlap,1);

        if (gtpLog)  qDebug() << "TapPopSlap byte = "<<tapPopSlap;

        if (tapPopSlap)
        {
            byte beatEffSet = 29 + tapPopSlap;
            cursorBeat->setEffects(beatEffSet);
        }
    }

    if (beatEffectsHead1 & 0x10) //16
    {
        cursorBeat->setEffects(20); //would be fade in
    }

    if (beatEffectsHead2 & 4)
    {   //tremolo
       if (gtpLog)  qDebug() << " read bend tremolo";
       BendPoints *tremoloBend = new BendPoints;
       readBendGTP(&file,tremoloBend);
       cursorBeat->setEffects(19); //would be tremolo
       cursorBeat->effPack.addPack(19,2,tremoloBend);

    }

    //and dear rusedhsajhdkjsa

    if (beatEffectsHead1 & 64)
    {   //updown stroke
        byte upStroke, downStroke;
        file.read((char*)&upStroke,1);
        file.read((char*)&downStroke,1);
        if (gtpLog)  qDebug() << "Up Stroke =" << upStroke <<" Down Stroke="<<downStroke;

        if (upStroke)
        cursorBeat->setEffects(25); //upstroke

        if (downStroke)
            cursorBeat->setEffects(26);
        //NOTSET
    }

    if (beatEffectsHead2 & 2)
    {   //pick stoke
        byte pickStoke;
        file.read((char*)&pickStoke,1);
        if (gtpLog)  qDebug() << "Pick stoke ";

        if (pickStoke)
        {
            byte beatEffSet = 29 + pickStoke;
            cursorBeat->setEffects(beatEffSet);
            //useless features cover under anoter field not effects
        }
    }
}

void readNoteEffects(std::ifstream &file, Note *newNote, int gpVersion=4)
{
    if (gtpLog)  qDebug() <<"Bit 3 in header turned on";
    //NOT SET
    byte noteEffectsHead1, noteEffectsHead2;
    file.read((char*)&noteEffectsHead1,1);
    file.read((char*)&noteEffectsHead2,1);

    if (gtpLog)  qDebug() << "Note effects heads. H1=" <<noteEffectsHead1<<
            "; H2=" <<noteEffectsHead2;


    if (noteEffectsHead1&1)
    {//bend
        if (gtpLog)  qDebug() << "Bend found.";


        BendPoints *bend = new BendPoints;
        readBendGTP(&file,bend);

        if (gtpLog)
        qDebug()<< " Bend h "<<"; len "<<(int)bend->len()<<"; type"<<bend->getType();

        newNote->setEffect(17);//first common pattern
        newNote->effPack.addPack(17,2,bend); //type 2 is bend
    }


    if (noteEffectsHead1&16)
    {   //grace note
        if (gtpLog)  qDebug() << "Grace note follows";

        byte graceFret = 0;
        byte graceDynamic = 0;
        byte graceTransition = 0;
        byte graceDuration = 0;

        file.read((char*)&graceFret,1);
        file.read((char*)&graceDynamic,1);
        file.read((char*)&graceTransition,1);
        file.read((char*)&graceDuration,1);

        if (gtpLog)  qDebug()<<"Fret "<<graceFret<<" Dyn "<<graceDynamic<<" Trans "<<graceTransition<<" Dur "<<graceDuration;

        if (gpVersion==5)
        {
            byte flags =0;
            file.read((char*)&flags,1);
            if (gtpLog)  qDebug()<<"Gp5 grace flags "<<flags;
        }

        newNote->graceNote[0] = graceFret;
        newNote->graceNote[2] = graceTransition;
        newNote->graceNote[3] = graceDuration;

        newNote->graceIsHere = true; //temp way
        newNote->graceNote[1] = graceDynamic;

        newNote->setEffect(22); //grace note
    }

    if (noteEffectsHead2&1)
    {
        if (gtpLog)  qDebug() << "Staccato appear";

        newNote->setEffect(23); //staccato
    }

    if (noteEffectsHead2&2)
    {//palm muting

        if (gtpLog)  qDebug() << "Palm mute appear";

        newNote->setEffect(2);
    }

    if (noteEffectsHead1&2)
    {//legato
       newNote->setEffect(10);
       if (gtpLog)  qDebug() << "legatto turned on";
    }

    if (noteEffectsHead1&8)
    {//let ring
       newNote->setEffect(18);
       if (gtpLog)  qDebug() <<" Let ring turned on";
       if (gtpLog)  qDebug() <<" if (gtpLog)  log";
    }

    if (noteEffectsHead2&4)
    {//Tremolo picking : b
        byte tremoloPicking;
        file.read((char*)&tremoloPicking,1);
        if (gtpLog)  qDebug() << "Tremolo byte "<<tremoloPicking;
        newNote->setEffect(24); //tremolo picking
    }

    if (noteEffectsHead2&8)
    {//Slide : b
        byte slide;
        file.read((char*)&slide,1);
        if (gtpLog)  qDebug() << "Slide byte " <<slide;
        byte effect = 3;
        if (slide < 5)
        {
            effect += slide;
            //legatto slide here too
        }
        else
        {
            if (slide == 255)
                effect = 3+5;
            if (slide == 254)
                effect = 3+6;
        }
            newNote->setEffect(effect);
    }


    if (noteEffectsHead2&16)
    {//Harmonics : b
        byte harmonics;
        file.read((char*)&harmonics,1);
        if (gtpLog)  qDebug() << "Harmonics byte "<<harmonics;

        if (gpVersion==5)
        {
            if (harmonics==1)
            {
                //nautur
            }
            if (harmonics==2) //artif
            {
                byte skipIt;
                file.read((char*)&skipIt,1);
                file.read((char*)&skipIt,1);
                file.read((char*)&skipIt,1);
            }
            if (harmonics==3) //tapp
            {
                byte skipIt;
                file.read((char*)&skipIt,1);

            }
            if (harmonics==4) //pinch
            {

            }
            if (harmonics==5) //semi
            {

            } //attention - not sets

            byte effect = 11;
            newNote->setEffect(effect);

        }
        else
        {

            if (harmonics==15) harmonics=2;
            if (harmonics==17) harmonics=3;
            if (harmonics==22) harmonics=6;
            byte effect = 10 + harmonics;
            newNote->setEffect(effect);
        }
    }

    if (noteEffectsHead2&32)
    {//Trill : 2b
        byte trill1, trill2;
        file.read((char*)&trill1,1);
        file.read((char*)&trill2,1);
        if (gtpLog)  qDebug() << "Trill b1="<<trill1<<" trill b2="<<trill2;
    }\

    if (noteEffectsHead2&64)
    {
        //vibrato
        newNote->setEffect(1);//1 is vibrato
        if (gtpLog)  qDebug() << "Vibratto turned on";
    }
}

void readNote(std::ifstream &file, Note *newNote, int gpVersion=4)
{
    //unused ul beatIndex, Bar *cursorBar,

    byte noteHeader;
    file.read((char*)&noteHeader,1);

    byte noteType=0;
    if (gtpLog)  qDebug() << "Note header "<<(int)noteHeader;

    newNote->setEffect(0); //flush first

    if (noteHeader & 0x20)
    {
        file.read((char*)&noteType,1);
        byte bby=0; //^IN DOCS WE HAVE SHORT INT HERE
        //file.read((char*)&bby,1);
        if (gtpLog)  qDebug() << "Note type = "<<(int)noteType<<" : "<<bby;

        //could be leag on 2
        //dead on 3 is ghost
        //normal leeg (dead?)
        //leeged normal, leeged leeg
        newNote->setState(noteType);

        //byte sNum=newNote->getStringNumber();

        if (noteType == 2)
        {
            //1:find last fret to set here
            //2:update status for last note

            /*

            Note *prevNote=0;
            if (gtpLog)  qDebug() << "Prev note for sNum="<<sNum;

            byte beatShiftBack = 1;
            byte wasInBlock = 0;
            bool notFoundInPrev=false;

                */

            /*
            if (beatIndex >= 1)
            {
                Beat *prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);

                pointx1:

                for (ul strInd = 0; strInd < prevBeat->len(); ++strInd)
                    {
                        Note *prevNoteSearch = &prevBeat->getV(strInd);
                        byte prevSNum = prevNoteSearch->getStringNumber();
                        byte fretPrevValue = prevNoteSearch->getFret();
                           if (gtpLog)  qDebug()<< strInd <<"PrevN sNum "<<prevSNum<<" "<<fretPrevValue;
                        if (sNum==prevSNum)
                        {
                            notFoundInPrev = false;
                            if (prevNote)
                                if (prevNote->getFret()==63)
                                    ;
                                       //prevNote->setFret(fretPrevValue);

                            prevNote = prevNoteSearch;
                            if (fretPrevValue==63)
                            {
                                //LZPlane..
                                if (beatIndex > beatShiftBack)
                                {
                                    ++beatShiftBack;
                                    prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);
                                    strInd=-1;
                                    ++wasInBlock;
                                    notFoundInPrev=true;
                                    continue;
                                }
                                else
                                    break;
                            }
                            break;
                        }
                    }

                if (notFoundInPrev)
                {
                    ++beatShiftBack;
                    prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);

                    if (beatIndex >= beatShiftBack)
                    goto pointx1;
                }
            }
            else
            {
                //protection from the fool of first leeg needed
                if (cursorBar)
                {
                     Bar *prevBar = cursorBar - 1;
                     Beat *prevBeat = &prevBar->getV(prevBar->len()-beatShiftBack);

                     pointx0:

                     for (ul strInd = 0; strInd < prevBeat->len(); ++strInd)
                         {
                             Note *prevNoteSearch = &prevBeat->getV(strInd);
                             byte prevSNum = prevNoteSearch->getStringNumber();
                             byte fretPrevValue = prevNoteSearch->getFret();
                             if (gtpLog)  qDebug() << strInd <<" PrevN sNum "<<prevSNum<<" "<<fretPrevValue;
                             if (sNum==prevSNum)
                             {
                                 notFoundInPrev = false;
                                 if (prevNote)
                                     if (prevNote->getFret()==63)
                                     {
                                           // prevNote->setFret(fretPrevValue);
                                     }

                                 prevNote = prevNoteSearch;
                                 if (fretPrevValue==63)
                                 {
                                     if (beatIndex > beatShiftBack)
                                     {
                                         ++beatShiftBack;
                                         prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);
                                         strInd=-1;
                                         ++wasInBlock;
                                         notFoundInPrev = true;
                                         continue;
                                     }
                                     else
                                         break;
                                 }
                                 break;
                             }
                         }

                     if (notFoundInPrev)
                     {
                         ++beatShiftBack;
                         prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);

                         if (beatIndex >= beatShiftBack)
                         goto pointx0;
                     }

                }

            }


            //SECTION 2 //set same effect

            if (prevNote)
            {
                byte prevFret = prevNote->getFret();
                if (gtpLog)  qDebug() << "Prev found "<<prevNote->getStringNumber()<<
                       " "<<prevFret ;


                prevNote->signStateLeeged();

                if (prevNote->getEffects() == 1)
                {
                    newNote->setEffect(1);
                    //and other effects not to break - refact attention
                }

                newNote->setFret(prevFret);


                if (gtpLog)  qDebug() << "After leeg sign state "<<prevNote->getState()<<" wib "<<wasInBlock;


                if (prevFret==63)
                if (gtpLog)  qDebug()<<"if (gtpLog)  log";
            }
            //*/
        }
    }

    //we push it down with a
    if (noteHeader & 1)
    {
        if (gpVersion==4)
        {
        //another duration
        if (gtpLog)  qDebug()  <<"Time independent ";
        byte t1,t2;
        file.read((char*)&t1,1);
        file.read((char*)&t2,1);
        if (gtpLog)  qDebug()<<"T: "<<t1<<";"<<t2;
        //attention?
        }
    }

    if (noteHeader & 16)
    {
        if (gtpLog)  qDebug() <<"Bit 4 in header turned on";
        byte bByte=0;
        file.read((char*)&bByte,1);
        if (gtpLog)  qDebug()<<"velocity byte(forte) "<<bByte;
        newNote->setVolume(bByte);
    }

    if (noteHeader & 32)
    {
        if (gtpLog)  qDebug() <<"Bit 5 in header turned on";
        byte bByte=0;
        file.read((char*)&bByte,1);
        if (gtpLog)  qDebug()<<"some byte fret "<<bByte;
        if (noteType != 2)
        {
            if (gtpLog)  qDebug()<<"not leeg setting prev fret";
            newNote->setFret(bByte);
        }
        else
            if (gtpLog)  qDebug() <<"leeg escape prev fret";
    }



    if (noteHeader & 2)
       if (gtpLog)  qDebug() <<"Bit 1 in header turned on"; //DOT NOTE //wow - where is it turned then?

    if (noteHeader & 4)
    {
        if (gtpLog)  qDebug() <<"Bit 2 in header turned on"; //GHOST NOTE
        //ghost not here
        newNote->setEffect(21); //ghost note
    }

    if (noteHeader & 64)
    {
        if (gtpLog)  qDebug() <<"Bit 6 in header turned on"; //ACCENTED
        newNote->setEffect(27); //there is no heavy accented note anymore (
        //in gp4

    }

    if (noteHeader & 128)
    {
        if (gtpLog)  qDebug() <<"Bit 7 in header turned on";

        byte bByte=0;
        byte bByte2=0;
        file.read((char*)&bByte,1);
        file.read((char*)&bByte2,1);

        if (gtpLog)  qDebug()<<"fingering byte "<<bByte<<":"<<bByte2;
    }

    if (gpVersion==5)
    {
        if (noteHeader & 2)
        {
            newNote->setEffect(27); //accented heavy
            //refact to use new value later
        }

        char toSkip[10];

        if (noteHeader & 1)
        {
            file.read((char*)toSkip,8);
        }

        file.read((char*)toSkip,1);
    }

    if (noteHeader & 8)
    {
       readNoteEffects(file,newNote,gpVersion);
    }
}

void readBeat(std::ifstream &file, Beat *cursorBeat)
{
    byte beatHeader = 0;
    file.read((char*)&beatHeader,1);

    bool dotted = beatHeader & 0x1;
    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;


    if (gtpLog)  qDebug() << "Beat header " << (int)beatHeader;

    cursorBeat->setPause(false);
    if (precStatus)
    {
        byte beatStatus;
        file.read((char*)&beatStatus,1);
        if (gtpLog)  qDebug() <<"Beat status "<<(int)beatStatus;
        if ((beatStatus == 2) || (beatStatus == 0))
         cursorBeat->setPause(true);
    }



    byte durationGP =0;
    file.read((char*)&durationGP,1);

    if (gtpLog)  qDebug() <<"Beat duration "<<(int)durationGP;

    byte duration=durationGP+2; //moved from -2 double to 1
    //x - double //0 - full //1 - half
    //2- forth(qua)  //3- eights  //4-16th
    //5-32th //6-64th

    //attention
    cursorBeat->setDuration(duration);
    cursorBeat->setDotted(dotted); //notify dotted
    //dotted will go now deeper

    if (precNTrump)
    {
        ul trumpletN = 0;
        file.read((char*)&trumpletN,4);
        if (gtpLog)  qDebug() <<"Beat tump "<<trumpletN;
        cursorBeat->setDurationDetail(trumpletN);
    }
    else
    {
       cursorBeat->setDurationDetail(0);
    }

    if (precChord)
    {
        readChordDiagramGP4(file);
    }

    if (precText)
    {
        if (gtpLog)  qDebug() << "Text";

        ul textLen = 0;
        file.read((char*)&textLen,4);

        byte byteLen = 0;
        file.read((char*)&byteLen,1);

        char textBufer[255];
        file.read((char*)textBufer,byteLen);

        //len+1
        textBufer[byteLen]=0;

        if (gtpLog)  qDebug() <<"TextLen "<<textLen<<" value "<<textBufer<<"; bL "<<byteLen;

        std::string foundText(textBufer);
        cursorBeat->setGPCOMPText(foundText);
    }

    if (precEffects)
    {
       readBeatEffects(file,cursorBeat);
    }

    if (precChanges)
    {
        if (gtpLog)  qDebug() << "Changes table found";
        readChanges(file,cursorBeat);
    }

}

//readGraceNote

ul readStringsFlag(std::ifstream &file, std::vector<int> &polyStrings)
{
    byte stringsFlag = 0;
    file.read((char*)&stringsFlag,1);

    ul totalCountStrings = 0;

    //attention here was from 7 to 0 now else
    if (stringsFlag&1) polyStrings.push_back(7);//++totalCountStrings;
    if (stringsFlag&2) polyStrings.push_back(6);//++totalCountStrings;
    if (stringsFlag&4) polyStrings.push_back(5);//++totalCountStrings;
    if (stringsFlag&8) polyStrings.push_back(4);//++totalCountStrings;
    if (stringsFlag&16) polyStrings.push_back(3);//++totalCountStrings;
    if (stringsFlag&32) polyStrings.push_back(2);//++totalCountStrings;
    if (stringsFlag&64) polyStrings.push_back(1);// ++totalCountStrings;
    if (stringsFlag&128) polyStrings.push_back(0);//checkcheck
    totalCountStrings  = polyStrings.size();

    if (gtpLog)  qDebug()<<"Strings flag "<<(int)stringsFlag
      <<"; total count "<<totalCountStrings;

    return totalCountStrings;
}


void readBar(std::ifstream &file, Tab *tab, ul tracksAmount, ul index)
{
    ul i = index;

    byte beatHeader = 0;
    file.read((char*)&beatHeader,1);

    byte precNum = beatHeader & 0x1;
    byte precDenum = beatHeader & 0x2;
    byte precBegRepeat = beatHeader & 0x4;
    byte precEndRepeat = beatHeader & 0x8;
    byte precNumAltEnding = beatHeader & 0x10;
    byte precMarker = beatHeader & 0x20;
    byte precTonality = beatHeader & 0x40;
    byte precDoubleBar = beatHeader & 0x80;

    if (gtpLog)  qDebug()<< i << " beat h= " << (int)beatHeader;
    if (gtpLog)  qDebug() << "[" << precNum << "][" << precDenum << "][" << precBegRepeat << "][" << precEndRepeat << "][" << precNumAltEnding <<
    "][" << precMarker << "][" << precTonality << "][" << precDoubleBar << "]";



    for (ul iTrack = 0; iTrack < tracksAmount; ++iTrack)
    {
        Bar *currentBar = tab->getV(iTrack)->getV(i);
        currentBar->setRepeat(0);
        currentBar->setAltRepeat(0);
    }

    if (precBegRepeat)
    {
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setRepeat(1); //ow shit - its broken
        }
    }

    if (precNum)
    {
        byte signNumeration = 0;
        file.read((char*)&signNumeration,1);
        if (gtpLog)  qDebug() << "Set num to " <<(int)signNumeration;

        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setSignNum(signNumeration);
        }
    }
    else
    {
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setSignNum(0);
        }
    }

    if (precDenum)
    {
        byte signDenumeration = 0;
        file.read((char*)&signDenumeration,1);
        if (gtpLog)  qDebug() << "Set denum to "	<<(int)signDenumeration;

        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setSignDenum(signDenumeration);
        }
    }
    else
    {
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setSignDenum(0);
        }
    }

    if (precEndRepeat)
    {
        byte repeatTimes = 0;
        file.read((char*)&repeatTimes,1);
        if (gtpLog)  qDebug() << "Repeat times " <<(int)repeatTimes;
        //i'm not sure, but repeat flag appear on next bar
        //maybe its bug or how it used to be on gtp

        //attention ! looks like 4 and 5 versions a different
        ++repeatTimes;

        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setRepeat(2,repeatTimes); //ow shit
        }
    }
    if (precNumAltEnding)
    {
        byte altEnding = 0;
        file.read((char*)&altEnding,1);
        if (gtpLog)  qDebug() << "AltEnding " << (int)altEnding;
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setAltRepeat(altEnding); //ow shit
        }
    }
    if (precMarker)
    {
        ul unknown = 0;
        file.read((char*)&unknown,4); //they say its a byte.. fuck them..

        byte markerSize;
        file.read((char*)&markerSize,1);

        char markerBufer[255];
        file.read((char*)markerBufer,markerSize);
        markerBufer[markerSize] = 0;

        ul markerColor;
        file.read((char*)&markerColor,4);

        if (gtpLog)  qDebug() << "Marker size "<<markerSize<<" buf "<<markerBufer;

        std::string markerBuferStr(markerBufer);
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setGPCOMPMarker(markerBuferStr,markerColor);
        }
    }
    if (precTonality)  //4?
    {
        byte tonality = 0;
        file.read((char*)&tonality,1); //skip 1!!
        file.read((char*)&tonality,1);
        if (gtpLog)  qDebug() << "Tonality " <<(int)tonality;
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setGPCOMPTonality(tonality);
        }
    }
}


bool Gp4Import::import(std::ifstream &file, Tab *tab, byte knownVersion)
{
    if (gtpLog)  qDebug() << "Starting GP4 import";


    if (knownVersion==0)
    {
        byte preVersion;///???
        file.read((char*)&preVersion,1);
        std::string formatVersion = readString(file,29);
        byte postVersion;///???
        file.read((char*)&postVersion,1);
    }

    std::string title,subtitle,interpret,albumn,author,copyright,tabAuthor,instructions;

    title = readString(file);
    std::cerr << "title " << title << std::endl;
    subtitle = readString(file);
    interpret = readString(file);
    albumn = readString(file);
    author = readString(file);
    copyright = readString(file);
    tabAuthor = readString(file);
    instructions = readString(file);

    //notice
    ul noticeLen = 0;
    file.read((char*)&noticeLen,4);
    if (gtpLog)  qDebug() << "Notice len is " << (int)noticeLen;

    if (noticeLen > 0)
    {
        if (gtpLog)  qDebug() << "Read notices ";
        for (ul i =0 ; i < noticeLen; ++i)
            std::string noticeOne = readString(file);
    }

    byte tripletFeel = 0;
    file.read((char*)&tripletFeel,1);

    int tripletFeelInt = (int)tripletFeel; //hate this - if (gtpLog)  log should fix it
    //refact
    if (gtpLog)  qDebug() << "Triplet feel = " << tripletFeelInt;

    ul lyTrack = 0;
    file.read((char*)&lyTrack,4);
    if (gtpLog)  qDebug() << "Lyrics track " <<(int)lyTrack;

    for (int i = 0; i < 5; ++i)
    {
        ul emWo = 0;
        file.read((char*)&emWo,4);
        std::string lyricsOne = readString(file);
    }

    ul bpm = 0;
    int signKey = 0;
    byte octave = 0;

    file.read((char*)&bpm,4);
    file.read((char*)&signKey,4);
    file.read((char*)&octave,1);

    tab->setBPM(bpm);

    if (gtpLog)  qDebug() <<"Bpm rate is " << bpm;
    if (gtpLog)  qDebug() <<"Sign Key = " <<signKey << " ; octave " <<octave ;

    //4 8 - 12
    char midiChannelsData[768];
    file.read((char*)midiChannelsData,768);

    //debug
    if (gtpLog)
        qDebug() << "Midi Channels data read. size of structure: "<<(int)sizeof(MidiChannelInfo)<<
           "; full size = "<<(int)(sizeof(MidiChannelInfo)*64)
               <<" and ul "<<(int)(sizeof(unsigned int));

    memcpy(tab->GpCompMidiChannels,midiChannelsData,768);


    ul beatsAmount = 0;
    ul tracksAmount = 0;

    file.read((char*)&beatsAmount,4);
    file.read((char*)&tracksAmount,4);

    if (gtpLog)  qDebug() << "Beats count " <<beatsAmount<<"; tracks count " <<tracksAmount;


    for (ul i = 0;i < tracksAmount; ++i)
    {
        Track *newTrack=new Track();

        for (ul j = 0; j < beatsAmount; ++j)
        {
            Bar *newBeatBar=new Bar(); //RESERVATION
            newTrack->add(newBeatBar);
        }

        tab->add(newTrack);
    }


    for (ul i = 0; i < beatsAmount; ++i)
        readBar(file,tab,tracksAmount,i);


    for (ul i = 0; i < tracksAmount; ++i)
    {
        Track *currentTrack = tab->getV(i);
        readTrack(file,currentTrack);
    }

    auto t1 =  tab->getV(0);
    Bar *cursorBar = t1->getV(0);

    Beat *cursorBeat = 0;

    if (cursorBar->len())
     cursorBeat =cursorBar->getV(0);

    if (gtpLog)  qDebug() <<"Begining beats amounts "<<beatsAmount;


    ul globalBeatsAmount = beatsAmount*tracksAmount;
    for (ul i = 0; i < globalBeatsAmount; ++i)
    {

        ul beatsInPair = 0;
        file.read((char*)&beatsInPair,4);

        if (gtpLog)  qDebug() <<i <<" Beats in pair " <<beatsInPair;

        //refact - over here was critical error its not usefull code
        if (beatsInPair > 1000)
        {
            if (i != 0)
            {
                if (gtpLog)  qDebug() << "DEBUG OUT";
                for (int iii = 0; iii < 10; ++iii)
                {
                    byte singleB;
                    file.read((char*)&singleB,1);
                    if (gtpLog)  qDebug() << "[" << iii << "] = " << singleB;
                }
                if (gtpLog)  qDebug() << "DEBUG OUT";
            }

            if (gtpLog)  qDebug() << "Seams to be critical error";
        }

        ul indexOfTrack = i % tracksAmount;
        Track *updatingTrack = tab->getV(indexOfTrack);
        ul indexOfBar = i / tracksAmount;
        Bar *updatingBar = updatingTrack->getV(indexOfBar);

        cursorBar = updatingBar;
        cursorBeat = 0;

        if (cursorBar->len())
            cursorBeat = cursorBar->getV(0);
        //++cursorBar;
        //cursorBeat = &cursorBar->getV(0);

        for (ul ind = 0; ind < beatsInPair; ++ind)
        {
            Beat *newOne = new Beat();
            cursorBar->add(newOne);
        }

        for (ul j = 0; j < beatsInPair; ++j)
        {
            cursorBeat = cursorBar->getV(j);
            //Reading beat main
            readBeat(file,cursorBeat);

            std::vector<int> polyStrings;
            ul totalCountStrings = readStringsFlag(file,polyStrings);


            for (ul noteInd = 0; noteInd < totalCountStrings; ++noteInd)
            {
                Note *newNote=new Note();

                byte sNum=polyStrings[totalCountStrings-noteInd-1];
                newNote->setStringNumber(sNum);

                //then note follows
                readNote(file,newNote);
                cursorBeat->add(newNote);
            }

            ++cursorBeat;

        }
    }

    if (gtpLog)  qDebug() << "Importing finished!";
    if (gtpLog)  qDebug() << "fine.";

    return true;
}

///////////////////////WRITE OPERATIONS////////////////////////////
/// \brief writeString
/// \param file
/// \param value
/// \param stringLen
///
//  ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ


void writeString(std::ofstream &file, std::string value, ul stringLen=0)
{
    if (stringLen==0)
        stringLen = value.length();

    file.write((const char*)&stringLen,4);
    file.write((const char*)value.c_str(),stringLen);
}


void writeTrack(std::ofstream &file, Track *currentTrack)
{
    //TEMPLATE
    byte trackHeader = 0;
    file.write((const char*)&trackHeader,1);

    ul trackNameLen = 0;
    file.write((const char*)&trackNameLen,1);

    char trackName[40];
    file.write((const char*)trackName,40);

    ul stringsAmount = 0;
    file.write((const char*)&stringsAmount,4);


    ul tunes[7] = {0}; //TUNEC!!!
    for (ul ii = 0; ii < 7; ++ii)
        file.write((const char*)&tunes[ii],4);


    ul port=0, chan=0, chanE=0;
    file.write((const char*)&port,4);
    file.write((const char*)&chan,4);
    file.write((const char*)&chanE,4);

    ul frets=0;
    file.write((const char*)&frets,4);
    ul capo=0;
    file.write((const char*)&capo,4);


    ul trackColor=0;
    file.write((const char*)&trackColor,4);

}

void writeBeatEffects(std::ofstream &file, Beat *cursorBeat)
{
    //TEMPLATE
    byte beatEffectsHead1; //prepare function!
    byte beatEffectsHead2;

    file.write((const char*)&beatEffectsHead1,1);
    file.write((const char*)&beatEffectsHead2,1);

    if (beatEffectsHead1 & 32)
    {   //tapping poping slaping
        byte tapPopSlap;
        file.write((const char*)&tapPopSlap,1);
    }

    if (beatEffectsHead2 & 4)
    {   //tremolo
       BendPoints tremoloBend;
       writeBendGTP(&file, &tremoloBend);
    }

    if (beatEffectsHead1 & 64)
    {   //updown stroke
        byte upStroke, downStroke;
        file.write((const char*)&upStroke,1);
        file.write((const char*)&downStroke,1);
    }

    if (beatEffectsHead2 & 2)
    {   //pick stoke
        byte pickStoke;
        file.write((const char*)&pickStoke,1);
    }
}

void writeBeat(std::ofstream &file, Beat *cursorBeat)
{   //template

    byte beatHeader = 0; //prepare function
    file.write((char*)&beatHeader,1);

    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;

    if (precStatus)
    {
        byte beatStatus;
        file.write((const char*)&beatStatus,1);
    }

    byte durationGP =0;
    file.write((const char*)&durationGP,1);

    if (precNTrump)
    {
        ul trumpletN = 0;
        file.write((const char*)&trumpletN,4);
    }

    if (precChord)
    {
        //readChordDiagram(file);
    }

    if (precText)
    {
        if (gtpLog)  qDebug() << "ATTENTION missing TEXT"; //!!!
    }

    if (precEffects)
    {
       writeBeatEffects(file,cursorBeat);
    }

    if (precChanges)
    {
        //readChanges(file,cursorBeat);
    }

}


void writeBar(std::ofstream &file, Bar *cursorBar)
{   //TEMPLATE

    byte beatHeader = 0; //PREPARE
    file.write((const char*)&beatHeader,1);

    byte precNum = beatHeader & 0x1;
    byte precDenum = beatHeader & 0x2;
    byte precEndRepeat = beatHeader & 0x8;
    byte precNumAltEnding = beatHeader & 0x10;
    byte precMarker = beatHeader & 0x20;
    byte precTonality = beatHeader & 0x40;

    if (precNum)
    {
        byte signNumeration = 0;
        file.write((const char*)&signNumeration,1);
    }

    if (precDenum)
    {
        byte signDenumeration = 0;
        file.write((const char*)&signDenumeration,1);
    }

    if (precEndRepeat)
    {
        byte repeatTimes = 0;
        file.write((const char*)&repeatTimes,1);
    }

    if (precNumAltEnding)
    {
        byte altEnding = 0;
        file.write((const char*)&altEnding,1);
    }

    if (precMarker)
    {
        byte markerSize;
        file.write((const char*)&markerSize,1);
        char markerBufer[255];
        file.write((const char*)markerBufer,markerSize);
        ul markerColor;
        file.write((const char*)&markerColor,4);
    }
    if (precTonality)  //4?
    {
        byte tonality = 0;
        file.write((const char*)&tonality,1); //skip 1!! ???
        file.write((const char*)&tonality,1);
    }
}


void writeNoteEffects(std::ofstream &file, Note *newNote)
{ //TEMPLATE

    byte noteEffectsHead1, noteEffectsHead2; //prepare
    file.write((const char*)&noteEffectsHead1,1);
    file.write((const char*)&noteEffectsHead2,1);

    if (noteEffectsHead1&1)
    {
        writeBendGTP(&file, &newNote->bend);
    }


    if (noteEffectsHead1&16)
    {   //grace note
        byte graceFret = 0;
        byte graceDynamic = 0;
        byte graceTransition = 0;
        byte graceDuration = 0;

        file.write((const char*)&graceFret,1);
        file.write((const char*)&graceDynamic,1);
        file.write((const char*)&graceTransition,1);
        file.write((const char*)&graceDuration,1);
    }

    if (noteEffectsHead2&4)
    {//Tremolo picking : b
        byte tremoloPicking;
        file.write((const char*)&tremoloPicking,1);
    }

    if (noteEffectsHead2&8)
    {//Slide : b
        byte slide;
        file.write((const char*)&slide,1);
    }


    if (noteEffectsHead2&16)
    {//Harmonics : b
        byte harmonics;
        file.write((const char*)&harmonics,1);
    }

    if (noteEffectsHead2&32)
    {//Trill : 2b
        byte trill1, trill2;
        file.write((const char*)&trill1,1);
        file.write((const char*)&trill2,1);
    }\
}

void writeStringsFlag(std::ofstream &file, Beat *cursorBeat)
{ //TEMPLATE
} //UNDONE


void writeNote(std::ofstream &file, Note *newNote)
{ //TEMPLATE

    byte noteHeader; //prepare
    file.write((const char*)&noteHeader,1);

    if (noteHeader & 0x20)
    {
        byte noteType;
        file.write((const char*)&noteType,1);
    }

    if (noteHeader & 1)
    {
        if (gtpLog)  qDebug()  <<"Time independent ";
        byte t1,t2;
        file.write((const char*)&t1,1);
        file.write((const char*)&t2,1);
    }

    if (noteHeader & 16)
    {
        if (gtpLog)  qDebug() <<"Bit 4 in header turned on";
        byte bByte=0;
        file.write((const char*)&bByte,1);
    }

    if (noteHeader & 32)
    {
        if (gtpLog)  qDebug() <<"Bit 5 in header turned on";
        byte bByte=0;
        file.write((const char*)&bByte,1);
    }

    if (noteHeader & 128)
    {
        byte bByte=0;
        byte bByte2=0;
        file.write((const char*)&bByte,1);
        file.write((const char*)&bByte2,1);
    }

    if (noteHeader & 8)
    {
       writeNoteEffects(file,newNote);
    }
}





//END of WRITE OPERATIONS




//////////////////////FORMAT GTP5////////////////////////////////
//--////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void readChangesGP5(std::ifstream &file, Beat *cursorBeat, byte verInd)
{

    Beat::ChangeTable changeStruct;// = {0};

    changeStruct.newTempo = 0;

    file.read((char*)&changeStruct.newInstr,1);  //1

    char toSkip[60];

    file.read((char*)toSkip,16);


    file.read((char*)&changeStruct.newVolume,1);
    file.read((char*)&changeStruct.newPan,1);
    file.read((char*)&changeStruct.newChorus,1);
    file.read((char*)&changeStruct.newReverb,1);
    file.read((char*)&changeStruct.newPhaser,1);
    file.read((char*)&changeStruct.newTremolo,1);

    ul strLen  = 0;
    file.read((char*)&strLen,4);
    if (strLen > 1)
    {
        //byte readByte;
        //file.read((char*)&readByte,1);

        std::string tempoName = readString(file,strLen); //not -1
        if (gtpLog)
            qDebug()<<"tempo name "<<tempoName.c_str();
    }
    else
    {
        if (gtpLog) qDebug()<<"tempo name is empty";
        byte readByte;
        file.read((char*)&readByte,1);
    }


    file.read((char*)&changeStruct.newTempo,4); //8

    if (gtpLog)  qDebug() <<  "I "<<changeStruct.newInstr<<"; V "<<changeStruct.newVolume<<"; P "<<changeStruct.newPan<<
          "; C "<<changeStruct.newChorus<<"; R "<<changeStruct.newReverb<<"; Ph "<<changeStruct.newPhaser<<
          "; Tr "<<changeStruct.newTremolo<<"; T="<<changeStruct.newTempo;


    //NO INSTR IN DOCS
    if (changeStruct.newInstr != 255)
    {

        Beat::SingleChange instrCh;
        instrCh.changeCount = 0;
        instrCh.changeType = 1;
        instrCh.changeValue = changeStruct.newInstr;
        cursorBeat->changes.add(instrCh);

    }

    if (changeStruct.newVolume != 255)
    {
        file.read((char*)&changeStruct.volumeDur,1);

        Beat::SingleChange volCh;
        volCh.changeCount = 0;
        volCh.changeType = 2;
        volCh.changeValue = changeStruct.newVolume;
        cursorBeat->changes.add(volCh);
    }

    if (changeStruct.newPan != 255)
    {
        file.read((char*)&changeStruct.panDur,1);

        Beat::SingleChange panCh;
        panCh.changeCount = 0;
        panCh.changeType = 3;
        panCh.changeValue = changeStruct.newPan;
        cursorBeat->changes.add(panCh);
    }

    if (changeStruct.newChorus != 255)
    {
        file.read((char*)&changeStruct.chorusDur,1);

        Beat::SingleChange chorusCh;
        chorusCh.changeCount = 0;
        chorusCh.changeType = 4;
        cursorBeat->changes.add(chorusCh);
    }

    if (changeStruct.newReverb != 255)
    {
         file.read((char*)&changeStruct.reverbDur,1);

         Beat::SingleChange reverbCh;
         reverbCh.changeCount = 0;
         reverbCh.changeType = 5;
         cursorBeat->changes.add(reverbCh);
    }

    if (changeStruct.newPhaser != 255)
    {
        file.read((char*)&changeStruct.phaserDur,1);

        Beat::SingleChange phaserCh;
        phaserCh.changeCount = 0;
        phaserCh.changeType = 6;
        cursorBeat->changes.add(phaserCh);
    }

    if (changeStruct.newTremolo != 255)
    {
         file.read((char*)&changeStruct.tremoloDur,1);

         Beat::SingleChange tremoloCh;
         tremoloCh.changeCount = 0;
         tremoloCh.changeType = 7;
         cursorBeat->changes.add(tremoloCh);
    }

    //-1 for ul is hiegh but 10000 bpm insane

   if (changeStruct.newTempo < 100000) //some attention here
    {
         file.read((char*)&changeStruct.tempoDur,1);
         //set changes table inside
         Beat::SingleChange tempCh;
         tempCh.changeCount = 0;
         tempCh.changeType = 8;
         tempCh.changeValue = changeStruct.newTempo;

         cursorBeat->changes.add(tempCh);

         if (verInd==1)
         {
            byte someSkip;
            file.read((char*)&someSkip,1);

            if (someSkip)
            {
                qDebug()<<"Skip byte = "<<someSkip;
            }
         }

    }

    cursorBeat->setEffects(28);
    cursorBeat->effPack.addPack(28,1,&(cursorBeat->changes));

    //refact
    file.read((char*)&changeStruct.changesTo,1); //not applied! attention

    //if (1)
    {
        //char toSkip[2];
        //file.read((char*)toSkip,49); //or 44

        //ul intLen = 0;
        byte byteLen = 0;


        file.read((char*)&byteLen,1); //skipperd

        //file.read((char*)&intLen,4);

        if (verInd==1)
        {
            /* looks like 70 is missing 1byte
            for (int z=0; z<5; ++z)
            {
                byte readOne = 0;
                file.read((char*)&readOne,1);
                qDebug() <<"R n# "<<z<<" "<<readOne;
            }
            */

            //another function?


            std::string rS1 = readString(file);
            std::string rS2 = readString(file);

            if (gtpLog)
            {
                qDebug() <<"R1 "<<rS1.c_str()<<" : R1";
                qDebug() <<"R2 "<<rS2.c_str()<<" : R2";
            }

        }
    }
}

void readChordDiagramGP5(std::ifstream &file)
{
    //on version 4 it could be not always the same!!!
    //readChordDiagram(file);



    //return;
    char chordBufer[64];
    if (gtpLog)  qDebug() << "Chord";
    file.read((char*)chordBufer,17);


    //file.read((char*)chordBufer,21);
    //chordBufer[21]=0;

    char fByte = 0; file.read((char*)&fByte,1);
    std::string chStr = readString(file,21);

    if (gtpLog)
        qDebug() <<fByte<< " Ch str "<<chStr.c_str();


    file.read((char*)chordBufer,4);

    int firstFret = 0;
    file.read((char*)&firstFret,4); //first fret

    for (int i =0; i < 7; ++i)
    {
        file.read((char*)chordBufer,4);//string fret?
    }

    file.read((char*)chordBufer,32);

    //on version 4 it could be not always the same!!!
}


void readBeatGP5(std::ifstream &file, Beat *cursorBeat, byte verInd=255)
{
    byte beatHeader = 0;
    file.read((char*)&beatHeader,1);

    bool dotted = beatHeader & 0x1;
    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;


    if (gtpLog)  qDebug() << "Beat header " << (int)beatHeader;

    cursorBeat->setPause(false);
    if (precStatus)
    {
        byte beatStatus;
        file.read((char*)&beatStatus,1);
        if (gtpLog)  qDebug() <<"Beat status "<<(int)beatStatus;
        if ((beatStatus == 2) || (beatStatus == 0))
         cursorBeat->setPause(true);
    }



    byte durationGP =0;
    file.read((char*)&durationGP,1);

    if (gtpLog)  qDebug() <<"Beat duration "<<(int)durationGP;

    byte duration=durationGP+2; //moved from -2 double to 1
    //x - double //0 - full //1 - half
    //2- forth(qua)  //3- eights  //4-16th
    //5-32th //6-64th

    //attention
    cursorBeat->setDuration(duration);
    cursorBeat->setDotted(dotted); //notify dotted
    //dotted will go now deeper

    if (precNTrump)
    {
        ul trumpletN = 0;
        file.read((char*)&trumpletN,4);
        if (gtpLog)  qDebug() <<"Beat tump "<<trumpletN;
        cursorBeat->setDurationDetail(trumpletN);
    }
    else
    {
       cursorBeat->setDurationDetail(0);
    }

    if (precChord)
    {
        readChordDiagramGP5(file);
    }

    if (precText)
    {
        if (gtpLog)  qDebug() << "TEXT";

        ul textLen = 0;
        file.read((char*)&textLen,4);

        byte byteLen = 0;
        file.read((char*)&byteLen,1);

        char textBufer[255];
        file.read((char*)textBufer,byteLen);

        //len+1
        textBufer[byteLen]=0;

        if (gtpLog)  qDebug() <<"TextLen "<<textLen<<" value "<<textBufer<<"; bL "<<byteLen;

        std::string foundText(textBufer);
        cursorBeat->setGPCOMPText(foundText);
    }

    if (precEffects)
    {
       readBeatEffects(file,cursorBeat);
    }

    if (precChanges)
    {
        if (gtpLog)  qDebug() << "Changes table found";
        readChangesGP5(file,cursorBeat,verInd);
    }


}

void readBarGP5(std::ifstream &file, Tab *tab, ul tracksAmount, ul index)
{
    ul i = index;

    byte beatHeader = 0;
    file.read((char*)&beatHeader,1);

    byte precNum = beatHeader & 0x1;
    byte precDenum = beatHeader & 0x2;
    byte precBegRepeat = beatHeader & 0x4;
    byte precEndRepeat = beatHeader & 0x8;
    byte precNumAltEnding = beatHeader & 0x10;
    byte precMarker = beatHeader & 0x20;
    byte precTonality = beatHeader & 0x40;
    byte precDoubleBar = beatHeader & 0x80;

    if (gtpLog)  qDebug()<< i << " beat h= " << (int)beatHeader;
    if (gtpLog)  qDebug() << "[" << precNum << "][" << precDenum << "]["
        << precBegRepeat << "][" << precEndRepeat << "][" << precNumAltEnding <<
        "][" << precMarker << "][" << precTonality << "][" << precDoubleBar << "]";



    for (ul iTrack = 0; iTrack < tracksAmount; ++iTrack)
    {
        Bar *currentBar = tab->getV(iTrack)->getV(i);
        currentBar->setRepeat(0);
        currentBar->setAltRepeat(0);
    }

    if (precBegRepeat)
    {
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setRepeat(1); //ow shit - its broken
        }
    }

    if (precNum)
    {
        byte signNumeration = 0;
        file.read((char*)&signNumeration,1);
        if (gtpLog)  qDebug() << "Set num to " <<(int)signNumeration;

        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setSignNum(signNumeration);
        }
    }
    else
    {
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setSignNum(0);
        }
    }

    if (precDenum)
    {
        byte signDenumeration = 0;
        file.read((char*)&signDenumeration,1);
        if (gtpLog)  qDebug() << "Set denum to "	<<(int)signDenumeration;

        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setSignDenum(signDenumeration);
        }
    }
    else
    {
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setSignDenum(0);
        }
    }

    if (precEndRepeat)
    {
        byte repeatTimes = 0;
        file.read((char*)&repeatTimes,1);
        if (gtpLog)  qDebug() << "Repeat times " <<(int)repeatTimes;
        //i'm not sure, but repeat flag appear on next bar
        //maybe its bug or how it used to be on gtp
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setRepeat(2,repeatTimes); //ow shit
        }
    }

    if (precMarker)
    {
        ul unknown = 0;
        file.read((char*)&unknown,4); //they say its a byte.. fuck them..

        ul markerSize=0;
        file.read((char*)&markerSize,1);

        //file.read((char*)&markerSize,4);

        char markerBufer[255];
        file.read((char*)markerBufer,markerSize);
        markerBufer[markerSize] = 0;

        ul markerColor;
        file.read((char*)&markerColor,4);

        if (gtpLog)  qDebug() << "Marker size "<<markerSize<<" buf "<<markerBufer;

        std::string markerBuferStr(markerBufer);
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setGPCOMPMarker(markerBuferStr,markerColor);
        }
    }

    if (precNumAltEnding)
    {
        byte altEnding = 0;
        file.read((char*)&altEnding,1);
        if (gtpLog)  qDebug() << "AltEnding " << (int)altEnding;
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setAltRepeat(altEnding); //ow shit
        }
    }

    if (precTonality)  //4?
    {
        byte tonality = 0;
        file.read((char*)&tonality,1); //skip 1!!
        file.read((char*)&tonality,1);
        if (gtpLog)  qDebug() << "Tonality " <<(int)tonality;
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setGPCOMPTonality(tonality);
        }
    }

    char toSkip[5];
    if (precNum || precDenum)
    {
       file.read((char*)toSkip,4);
    }

    if (!precNumAltEnding)
    {
        file.read((char*)toSkip,1);
    }

    //not fromhere
    byte tripletFeel=0;
    file.read((char*)&tripletFeel,1);

   //byte skipByteOh = 0;
    //file.read((char*)&skipByteOh,1);

}


bool Gp5Import::import(std::ifstream &file, Tab *tab, byte knownVersion)
{
    if (gtpLog)  qDebug() << "Starting GP5 import";

    char placeToSkip[255];
    byte versionIndex = 255;

    if (knownVersion)
    {
        if (knownVersion == 5)
            versionIndex = 0;
        if (knownVersion == 51)
            versionIndex = 1;
    }
    else
    {
        byte preVersion;///???
        file.read((char*)&preVersion,1);
        std::string formatVersion = readString(file,29);
        byte postVersion;///???
        file.read((char*)&postVersion,1);


        std::string version0 = "FICHIER GUITAR PRO v5.00";
        std::string version1 = "FICHIER GUITAR PRO v5.10";

        std::string versionCut = formatVersion.substr(0,version0.size());



        if (versionCut == version0)
            versionIndex = 0;
        if (versionCut == version1)
            versionIndex = 1;
    }

    std::string title,subtitle,interpret,albumn,author,copyright,tabAuthor,instructions,words;

    title = readStringShiByte(file);
    subtitle = readStringShiByte(file);
    interpret = readStringShiByte(file);
    albumn = readStringShiByte(file);
    words  = readStringShiByte(file);
    author = readStringShiByte(file);
    copyright = readStringShiByte(file);
    tabAuthor = readStringShiByte(file);
    instructions = readStringShiByte(file);

    //notice
    ul noticeLen = 0;
    file.read((char*)&noticeLen,4);
    if (gtpLog)  qDebug() << "Notice len is " << (int)noticeLen;

    if (noticeLen > 0)
    {
        if (gtpLog)  qDebug() << "Read notices ";
        for (ul i =0 ; i < noticeLen; ++i)
        {
            /*
            ul intLen = 0;
            file.read((char*)&intLen,4);
            byte noteLen = 0;
            file.read((char*)&noteLen,1);
            qDebug() <<"Note strlens "<<noteLen<<" "<<intLen;
            */
            std::string noticeOne = readStringShiByte(file); //,noteLen
            qDebug()<<"Notice#"<<i<<" "<<noticeOne.c_str();
        }
    }

    //byte tripletFeel = 0; //not here in gp5
    //file.read((char*)&tripletFeel,1);


    ul lyTrack = 0;
    file.read((char*)&lyTrack,4);
    if (gtpLog)  qDebug() << "Lyrics track " <<(int)lyTrack ;

    for (int i = 0; i < 5; ++i)
    {
        ul emWo = 0;
        file.read((char*)&emWo,4);
        std::string lyricsOne = readString(file);
    }


    if (versionIndex == 1)
        file.read((char*)placeToSkip,49);
    else
        if (versionIndex == 0)
            file.read((char*)placeToSkip,30);

    for (int i = 0; i < 11; ++i)
    {
         ul pageInt = 0;
         file.read((char*)&pageInt,4);
         byte strLenIn = 0;
         file.read((char*)&strLenIn,1);

         if (gtpLog)
         qDebug()<<"Page int "<<pageInt<<" strLen "<<strLenIn;
         std::string whatIsThat = readString(file,strLenIn);
    }



    ul bpm = 0;
    int signKey = 0;
    byte octave = 0;

    file.read((char*)&bpm,4);


    if (versionIndex == 1)
    {
        byte skipByte=0;
        file.read((char*)&skipByte,1);
    }

    file.read((char*)&signKey,4);
    file.read((char*)&octave,1);

    tab->setBPM(bpm);

    if (gtpLog)  qDebug() <<"Bpm rate is " << bpm ;
    if (gtpLog)  qDebug() <<"Sign Key = " <<signKey << " ; octave " <<octave ;


    //4 8 - 12
    char midiChannelsData[768];
    file.read((char*)midiChannelsData,768);

    if (gtpLog)  qDebug() << "Midi Channels data read. size of structure: "<<(int)sizeof(MidiChannelInfo)<<
           "; full size = "<<(int)(sizeof(MidiChannelInfo)*64);

    memcpy(tab->GpCompMidiChannels,midiChannelsData,768);


    //Unknonwn skip
    file.read((char*)placeToSkip,42);


    ul beatsAmount = 0;
    ul tracksAmount = 0;

    file.read((char*)&beatsAmount,4);
    file.read((char*)&tracksAmount,4);

    if (gtpLog)  qDebug() << "Beats count " <<beatsAmount<<"; tracks count " <<tracksAmount;


    for (ul i = 0;i < tracksAmount; ++i)
    {
        Track *newTrack=new Track();

        for (ul j = 0; j < beatsAmount; ++j)
        {
            Bar *newBeatBar=new Bar(); //RESERVATION
            newTrack->add(newBeatBar);
        }
        tab->add(newTrack);
    }


    for (ul i = 0; i < beatsAmount; ++i)
    {
        byte skipOne=0;
        if (i > 0)
        {
            file.read((char*)&skipOne,1);
        }

        readBarGP5(file,tab,tracksAmount,i);

    }

    for (ul i = 0; i < tracksAmount; ++i)
    {
        Track *currentTrack = tab->getV(i);
        readTrack(file,currentTrack,5,i,versionIndex);//5 is the version
    }

    /*
    qDebug() <<"Tracks "<<(int)tab->len();
    qDebug() <<"-";
    qDebug()<<" in cur "<<(int)tab->getV(0)->len();
    qDebug() <<"+";
    */

    Bar *cursorBar = tab->getV(0)->getV(0);
    Beat *cursorBeat = 0; //cursorBar->getV(0);

    if (cursorBar)
    if (cursorBar->len())
     cursorBeat =cursorBar->getV(0);

    if (gtpLog)  qDebug() <<"Begining beats amounts "<<beatsAmount ;
    byte oneSkip = 0;

    if (versionIndex == 1)
        file.read((char*)&oneSkip,1);
    else
        if (versionIndex == 0)
        {
            file.read((char*)&oneSkip,1);
            file.read((char*)&oneSkip,1);
        }



    ul globalBeatsAmount = beatsAmount*tracksAmount;
    for (ul i = 0; i < globalBeatsAmount; ++i) //
    {
            //SOMWHERE OVER HERE IN GTP5 lays voices
            //for (int voices=0; voices <2; ++voices)
            {
            ul beatsInPair = 0;
            file.read((char*)&beatsInPair,4);

            if (gtpLog)  qDebug() <<i <<" Beats in pair " <<beatsInPair  ;

            //refact - over here was critical error its not usefull code
            if (beatsInPair > 1000)
            {
                if (i != 0)
                {
                    if (gtpLog)  qDebug() << "DEBUG OUT";
                    for (int iii = 0; iii < 10; ++iii)
                    {
                        byte singleB;
                        file.read((char*)&singleB,1);
                        if (gtpLog)  qDebug() << "[" << iii << "] = " << singleB;
                    }
                    if (gtpLog)  qDebug() << "DEBUG OUT";
                }

                if (gtpLog)  qDebug() << "Seams to be critical error";
            }

            ul indexOfTrack = i % tracksAmount;
            Track *updatingTrack = tab->getV(indexOfTrack);
            ul indexOfBar = i / tracksAmount;
            Bar *updatingBar = updatingTrack->getV(indexOfBar);

            cursorBar = updatingBar;
            cursorBeat = 0;

            if (cursorBar->len())
            cursorBeat = cursorBar->getV(0);
            //++cursorBar;
            //cursorBeat = &cursorBar->getV(0);

            for (ul ind = 0; ind < beatsInPair; ++ind)
            {
                Beat *newOne = new Beat();
                cursorBar->add(newOne);
            }

            for (ul j = 0; j < beatsInPair; ++j)
            {
                cursorBeat = cursorBar->getV(j);
                //Reading beat main
                readBeatGP5(file,cursorBeat,versionIndex);

                std::vector<int> polyStrings;
                ul totalCountStrings = readStringsFlag(file,polyStrings);


                //first here is fine
                for (ul noteInd = 0; noteInd < totalCountStrings; ++noteInd)
                {
                    Note *newNote=new Note();

                    byte sNum=polyStrings[totalCountStrings-noteInd-1];
                    newNote->setStringNumber(sNum);

                    //then note follows
                    readNote(file,newNote,5); //format 5
                    cursorBeat->add(newNote);
                }

                ++cursorBeat;

                //additions
                byte readByte=0;
                file.read((char*)&readByte,1);
                file.read((char*)&readByte,1);

                if (readByte & 0x8)
                {
                    byte oneMore;
                    file.read((char*)&oneMore,1);
                }
            }
        }
        //SOMWHERE OVER HERE IN GTP5 lays voices


        //OF FUCKS OF ATTENTION GROW HERE PLEASE(SORRY GREAT ONE)(YOU ARE THE ONLY who helps always)
        ///ITS A NIGHTMARE
        //God damn skip

      for (ul subvoice=0; subvoice < 1; ++subvoice)
      {
          ul preReader = 0;

          file.read((char*)&preReader,4);

          if (gtpLog)
            qDebug()<<" PRE-Reader "<<preReader;

          Beat readerBeat;
          //Bar readerBar;
          for (ul i = 0; i < preReader; ++i)
          {
              readBeatGP5(file,&readerBeat,versionIndex);

              std::vector<int> polyStrings;
              ul totalCountStrings = readStringsFlag(file,polyStrings);

              //first here is fine
              for (ul noteInd = 0; noteInd < totalCountStrings; ++noteInd)
              {
                  Note newNote;
                  byte sNum=polyStrings[totalCountStrings-noteInd-1];
                  newNote.setStringNumber(sNum);
                  readNote(file,&newNote,5); //format 5
                  //cursorBeat->add(newNote);
              }

              //additions
              byte readByte0,readByte=0;
              file.read((char*)&readByte0,1);
              file.read((char*)&readByte,1);

              if (readByte & 0x8)
              {
                  byte oneMore;
                  file.read((char*)&oneMore,1);
              }
          }

          byte skipperByte=0;
          file.read((char*)&skipperByte,1);
      }

      /*
      for (int i = 0; i < 11; ++i)
      {
          byte reader = 0;
          file.read((char*)&reader,1);

          if (gtpLog)
            qDebug()<<" Reader "<<reader;
      } //*/

      if (gtpLog)
        qDebug()<<"reaad";

    }

    if (gtpLog)  qDebug() << "Importing finished!";
    if (gtpLog)  qDebug() << "fine.";

    return true;
}

/////////////////////   GP 3  /////////////////////////////////

void readChangesGP3(std::ifstream &file, Beat *cursorBeat)
{

    //gtpLog = true; //autolog!

    Beat::ChangeTable changeStruct;// = {0};

    changeStruct.newTempo = 0;

    file.read((char*)&changeStruct.newInstr,1);  //1

    file.read((char*)&changeStruct.newVolume,1);
    file.read((char*)&changeStruct.newPan,1);
    file.read((char*)&changeStruct.newChorus,1);
    file.read((char*)&changeStruct.newReverb,1);
    file.read((char*)&changeStruct.newPhaser,1);
    file.read((char*)&changeStruct.newTremolo,1);
    file.read((char*)&changeStruct.newTempo,4); //8

    if (gtpLog)  qDebug() <<  "I "<<changeStruct.newInstr<<"; V "<<changeStruct.newVolume<<"; P "<<changeStruct.newPan<<
          "; C "<<changeStruct.newChorus<<"; R "<<changeStruct.newReverb<<"; Ph "<<changeStruct.newPhaser<<
          "; Tr "<<changeStruct.newTremolo<<"; T="<<changeStruct.newTempo;


    //NO INSTR IN DOCS
    if (changeStruct.newInstr != 255)
    {
        //file.read((char*)&changeStruct.instrDur,1);

        Beat::SingleChange instrCh;
        instrCh.changeCount = 0;
        instrCh.changeType = 1;
        instrCh.changeValue = changeStruct.newInstr;
        cursorBeat->changes.add(instrCh);

    }

    if (changeStruct.newVolume != 255)
    {
        file.read((char*)&changeStruct.volumeDur,1);

        Beat::SingleChange volCh;
        volCh.changeCount = 0;
        volCh.changeType = 2;
        volCh.changeValue = changeStruct.newVolume;
        cursorBeat->changes.add(volCh);
    }

    if (changeStruct.newPan != 255)
    {
        file.read((char*)&changeStruct.panDur,1);

        Beat::SingleChange panCh;
        panCh.changeCount = 0;
        panCh.changeType = 3;
        panCh.changeValue = changeStruct.newPan;
        cursorBeat->changes.add(panCh);
    }

    if (changeStruct.newChorus != 255)
    {
        file.read((char*)&changeStruct.chorusDur,1);

        Beat::SingleChange chorusCh;
        chorusCh.changeCount = 0;
        chorusCh.changeType = 4;
        cursorBeat->changes.add(chorusCh);
    }

    if (changeStruct.newReverb != 255)
    {
         file.read((char*)&changeStruct.reverbDur,1);

         Beat::SingleChange reverbCh;
         reverbCh.changeCount = 0;
         reverbCh.changeType = 5;
         cursorBeat->changes.add(reverbCh);
    }

    if (changeStruct.newPhaser != 255)
    {
        file.read((char*)&changeStruct.phaserDur,1);

        Beat::SingleChange phaserCh;
        phaserCh.changeCount = 0;
        phaserCh.changeType = 6;
        cursorBeat->changes.add(phaserCh);
    }

    if (changeStruct.newTremolo != 255)
    {
         file.read((char*)&changeStruct.tremoloDur,1);

         Beat::SingleChange tremoloCh;
         tremoloCh.changeCount = 0;
         tremoloCh.changeType = 7;
         cursorBeat->changes.add(tremoloCh);
    }

    //-1 for ul is hiegh but 10000 bpm insane

        if (changeStruct.newTempo < 10000) //some attention here
        {
             file.read((char*)&changeStruct.tempoDur,1);
             //set changes table inside
             Beat::SingleChange tempCh;
             tempCh.changeCount = 0;
             tempCh.changeType = 8;
             tempCh.changeValue = changeStruct.newTempo;

             cursorBeat->changes.add(tempCh);

        }


    cursorBeat->setEffects(28);
    cursorBeat->effPack.addPack(28,1,&(cursorBeat->changes));

}

void readBeatEffectsGP3(std::ifstream &file, Beat *cursorBeat)
{
    byte beatEffectsHead;
    file.read((char*)&beatEffectsHead,1);

    if (gtpLog)  qDebug() << "Beat effects flag present. H1=" << beatEffectsHead;

    if ((beatEffectsHead&1)||(beatEffectsHead&2))
    {
        //vibrato
        cursorBeat->setEffects(2);
    }

    if (beatEffectsHead&10)
    {
        //fade in
        cursorBeat->setEffects(20); //would be fade in
    }

    if (beatEffectsHead & 32)
    {   //tapping poping slaping

        byte tapPopSlap;
        file.read((char*)&tapPopSlap,1);

        if (gtpLog)  qDebug() << "TapPopSlap byte = "<<tapPopSlap;

        if (tapPopSlap)
        {
            byte beatEffSet = 29 + tapPopSlap;
            cursorBeat->setEffects(beatEffSet);
            int skipInt;
            file.read((char*)&skipInt,4);
        }
        else
        {
            if (gtpLog)  qDebug() << " read bend tremolo";
            BendPoints tremoloBend;
            int tremoloValue = 0;
            file.read((char*)&tremoloValue,4);
            //readBend(file,tremoloBend);
            cursorBeat->setEffects(19); //would be tremolo
        }
    }

    if (beatEffectsHead & 64)
    {   //updown stroke
        byte upStroke, downStroke;
        file.read((char*)&upStroke,1);
        file.read((char*)&downStroke,1);
        if (gtpLog)  qDebug() << "Up Stroke =" << upStroke <<" Down Stroke="<<downStroke;

        if (upStroke)
        cursorBeat->setEffects(25); //upstroke

        if (downStroke)
            cursorBeat->setEffects(26);

        if (gtpLog)  qDebug() << "if (gtpLog)  log";
        //NOTSET
    }

    if (beatEffectsHead & 4)
    {
        //harm natur
        cursorBeat->setEffects(11);
        /*
         TGEffectHarmonic harmonic = getFactory().newEffectHarmonic();
            harmonic.setType(TGEffectHarmonic.TYPE_NATURAL);
            effect.setHarmonic(harmonic);
         */
    }

    if (beatEffectsHead & 4)
    {
        //harm artif
        cursorBeat->setEffects(12);
        /*
           TGEffectHarmonic harmonic = getFactory().newEffectHarmonic();
            harmonic.setType(TGEffectHarmonic.TYPE_ARTIFICIAL);
            harmonic.setData(0);
            effect.setHarmonic(harmonic);
         */
    }
}

void readNoteEffectsGP3(std::ifstream &file, Note *newNote)
{
    byte noteEffectsHead;
    file.read((char*)&noteEffectsHead,1);

    if (gtpLog)  qDebug() << "Note effects heads. H1=" <<noteEffectsHead;


    if (noteEffectsHead&1)
    {//bend
        if (gtpLog)  qDebug() << "Bend found.";
        readBendGTP(&file,&(newNote->bend));
        newNote->setEffect(17);//first common pattern
    }


    if (noteEffectsHead&16)
    {   //grace note
        if (gtpLog)  qDebug() << "Grace note follows";

        byte graceFret = 0;
        byte graceDynamic = 0;
        byte graceTransition = 0;
        byte graceDuration = 0;

        file.read((char*)&graceFret,1);
        file.read((char*)&graceDynamic,1);
        file.read((char*)&graceTransition,1);
        file.read((char*)&graceDuration,1);

        if (gtpLog)  qDebug()<<"Fret "<<graceFret<<" Dyn "
            <<graceDynamic<<" Trans "<<graceTransition<<" Dur "<<graceDuration;

        newNote->graceNote[0] = graceFret;
        newNote->graceNote[1] = graceDynamic;
        newNote->graceNote[2] = graceTransition;
        newNote->graceNote[3] = graceDuration;

        newNote->graceIsHere = true; //temp way

        newNote->setEffect(22); //grace note
    }


    if (noteEffectsHead&2)
    {//legato
       newNote->setEffect(10);
       if (gtpLog)  qDebug() << "legatto turned on";
    }

    if (noteEffectsHead&4)
    {//Slide : b

        if (gtpLog)  qDebug() << "Slide ";
        byte effect = 3;
        newNote->setEffect(effect);
    }

    if (noteEffectsHead&8)
    {//let ring
       newNote->setEffect(18);
       if (gtpLog)  qDebug() <<" Let ring turned on";
       if (gtpLog)  qDebug() <<" if (gtpLog)  log";
    }

}



void readBeatGP3(std::ifstream &file, Beat *cursorBeat)
{
    byte beatHeader = 0;
    file.read((char*)&beatHeader,1);

    bool dotted = beatHeader & 0x1;
    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;


    if (gtpLog)  qDebug() << "Beat header " << (int)beatHeader;

    cursorBeat->setPause(false);
    if (precStatus)
    {
        byte beatStatus;
        file.read((char*)&beatStatus,1);
        if (gtpLog)  qDebug() <<"Beat status "<<(int)beatStatus;
        if ((beatStatus == 2) || (beatStatus == 0))
         cursorBeat->setPause(true);
    }



    byte durationGP =0;
    file.read((char*)&durationGP,1);

    if (gtpLog)  qDebug() <<"Beat duration "<<(int)durationGP;

    byte duration=durationGP+2; //moved from -2 double to 1
    //x - double //0 - full //1 - half
    //2- forth(qua)  //3- eights  //4-16th
    //5-32th //6-64th

    //attention
    cursorBeat->setDuration(duration);
    cursorBeat->setDotted(dotted); //notify dotted
    //dotted will go now deeper

    if (precNTrump)
    {
        ul trumpletN = 0;
        file.read((char*)&trumpletN,4);
        if (gtpLog)  qDebug() <<"Beat tump "<<trumpletN;
        cursorBeat->setDurationDetail(trumpletN);
    }
    else
    {
       cursorBeat->setDurationDetail(0);
    }

    if (precChord)
    {
        readChordDiagramGP3(file);
    }

    if (precText)
    {
        if (gtpLog)  qDebug() << "TEXT";

        ul textLen = 0;
        file.read((char*)&textLen,4);

        byte byteLen = 0;
        file.read((char*)&byteLen,1);

        char textBufer[255];
        file.read((char*)textBufer,byteLen);

        //len+1
        textBufer[byteLen]=0;

        if (gtpLog)  qDebug() <<"TextLen "<<textLen<<" value "<<textBufer<<"; bL "<<byteLen;

        std::string foundText(textBufer);
        cursorBeat->setGPCOMPText(foundText);
    }

    if (precEffects)
    {
       readBeatEffectsGP3(file,cursorBeat);
    }

    if (precChanges)
    {
        if (gtpLog)  qDebug() << "Changes table found";
        readChangesGP3(file,cursorBeat);
    }

}

void readNoteGP3(std::ifstream &file, Note *newNote, ul beatIndex, Bar *cursorBar)
{
    byte noteHeader;
    file.read((char*)&noteHeader,1);

    byte noteType=0;
    if (gtpLog)  qDebug() << "Note header "<<(int)noteHeader;

    newNote->setEffect(0); //flush first

    if (noteHeader & 0x20)
    {
        file.read((char*)&noteType,1);
        byte bby=0; //^IN DOCS WE HAVE SHORT INT HERE
        //file.read((char*)&bby,1);
        if (gtpLog)  qDebug() << "Note type = "<<(int)noteType<<" : "<<bby;

        //could be leag on 2
        //dead on 3 is ghost
        //normal leeg (dead?)
        //leeged normal, leeged leeg
        newNote->setState(noteType);

        /*
        byte sNum=newNote->getStringNumber();

        if (noteType == 2)
        {
            //1:find last fret to set here
            //2:update status for last note

            Note *prevNote=0;
            if (gtpLog)  qDebug() << "Prev note for sNum="<<sNum;

            byte beatShiftBack = 1;
            byte wasInBlock = 0;
            bool notFoundInPrev=false;

            if (beatIndex >= 1)
            {
                Beat *prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);

                pointx1:

                for (ul strInd = 0; strInd < prevBeat->len(); ++strInd)
                    {
                        Note *prevNoteSearch = &prevBeat->getV(strInd);
                        byte prevSNum = prevNoteSearch->getStringNumber();
                        byte fretPrevValue = prevNoteSearch->getFret();
                           if (gtpLog)  qDebug()<< strInd <<"PrevN sNum "<<prevSNum<<" "<<fretPrevValue;
                        if (sNum==prevSNum)
                        {
                            notFoundInPrev = false;


                            //if (prevNote)
                                //if (prevNote->getFret()==63)
                                    //;
                                       //prevNote->setFret(fretPrevValue);

                            prevNote = prevNoteSearch;
                            if (fretPrevValue==63)
                            {
                                //LZPlane..
                                if (beatIndex > beatShiftBack)
                                {
                                    ++beatShiftBack;
                                    prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);
                                    strInd=-1;
                                    ++wasInBlock;
                                    notFoundInPrev=true;
                                    continue;
                                }
                                else
                                    break;
                            }
                            break;
                        }
                    }

                if (notFoundInPrev)
                {
                    ++beatShiftBack;
                    prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);

                    if (beatIndex >= beatShiftBack)
                    goto pointx1;
                }
            }
            else
            {
                //protection from the fool of first leeg needed
                if (cursorBar)
                {
                     Bar *prevBar = cursorBar - 1;
                     Beat *prevBeat = &prevBar->getV(prevBar->len()-beatShiftBack);

                     pointx0:

                     for (ul strInd = 0; strInd < prevBeat->len(); ++strInd)
                         {
                             Note *prevNoteSearch = &prevBeat->getV(strInd);
                             byte prevSNum = prevNoteSearch->getStringNumber();
                             byte fretPrevValue = prevNoteSearch->getFret();
                             if (gtpLog)  qDebug() << strInd <<" PrevN sNum "<<prevSNum<<" "<<fretPrevValue;
                             if (sNum==prevSNum)
                             {
                                 notFoundInPrev = false;
                                 if (prevNote)
                                     if (prevNote->getFret()==63)
                                     {
                                           // prevNote->setFret(fretPrevValue);
                                     }

                                 prevNote = prevNoteSearch;
                                 if (fretPrevValue==63)
                                 {
                                     if (beatIndex > beatShiftBack)
                                     {
                                         ++beatShiftBack;
                                         prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);
                                         strInd=-1;
                                         ++wasInBlock;
                                         notFoundInPrev = true;
                                         continue;
                                     }
                                     else
                                         break;
                                 }
                                 break;
                             }
                         }

                     if (notFoundInPrev)
                     {
                         ++beatShiftBack;
                         prevBeat = &cursorBar->getV(beatIndex-beatShiftBack);

                         if (beatIndex >= beatShiftBack)
                         goto pointx0;
                     }

                }

            }
            if (prevNote)
            {
                byte prevFret = prevNote->getFret();
                if (gtpLog)  qDebug() << "Prev found "<<prevNote->getStringNumber()<<
                       " "<<prevFret ;


                prevNote->signStateLeeged();

                if (prevNote->getEffects() == 1)
                {
                    newNote->setEffect(1);
                    //and other effects not to break - refact attention
                }

                newNote->setFret(prevFret);


                if (gtpLog)  qDebug() << "After leeg sign state "<<prevNote->getState()<<" wib "<<wasInBlock;


                if (prevFret==63)
                if (gtpLog)  qDebug()<<"if (gtpLog)  log";
            }
        } */
    }

    //we push it down with a
    if (noteHeader & 1)
    {

        //another duration
        if (gtpLog)  qDebug()  <<"Time independent ";
        byte t1,t2;
        file.read((char*)&t1,1);
        file.read((char*)&t2,1);
        if (gtpLog)  qDebug()<<"T: "<<t1<<";"<<t2;
        //attention?

    }

    if (noteHeader & 16)
    {
        if (gtpLog)  qDebug() <<"Bit 4 in header turned on";
        byte bByte=0;
        file.read((char*)&bByte,1);
        if (gtpLog)  qDebug()<<"velocity byte(forte) "<<bByte;
        newNote->setVolume(bByte);
    }

    if (noteHeader & 32)
    {
        if (gtpLog)  qDebug() <<"Bit 5 in header turned on";
        byte bByte=0;
        file.read((char*)&bByte,1);
        if (gtpLog)  qDebug()<<"some byte fret "<<bByte;
        if (noteType != 2)
        {
            if (gtpLog)  qDebug()<<"not leeg setting prev fret";
            newNote->setFret(bByte);
        }
        else
            if (gtpLog)  qDebug() <<"leeg escape prev fret";
    }



    if (noteHeader & 2)
       if (gtpLog)  qDebug() <<"Bit 1 in header turned on"; //DOT NOTE //wow - where is it turned then?

    if (noteHeader & 4)
    {
        if (gtpLog)  qDebug() <<"Bit 2 in header turned on"; //GHOST NOTE
        //ghost not here
        newNote->setEffect(21); //ghost note
    }

    if (noteHeader & 64)
    {
        if (gtpLog)  qDebug() <<"Bit 6 in header turned on"; //ACCENTED
        newNote->setEffect(27); //there is no heavy accented note anymore (
        //in gp4

    }

    if (noteHeader & 128)
    {
        if (gtpLog)  qDebug() <<"Bit 7 in header turned on";

        byte bByte=0;
        byte bByte2=0;
        file.read((char*)&bByte,1);
        file.read((char*)&bByte2,1);

        if (gtpLog)  qDebug()<<"fingering byte "<<bByte<<":"<<bByte2;
    }

    if (noteHeader & 8)
    {
       readNoteEffectsGP3(file,newNote);
    }
}

/////////////////////////////////////////////////////////////


bool Gp3Import::import(std::ifstream &file, Tab *tab, byte knownVersion)
{
    if (gtpLog)  qDebug() << "Starting GP3 import";

    if (knownVersion==0)
    {
        byte preVersion;
        file.read((char*)&preVersion,1);
        std::string formatVersion = readString(file,29);
        byte postVersion;
        file.read((char*)&postVersion,1);
    }

    std::string title,subtitle,interpret,albumn,author,copyright,tabAuthor,instructions;

    title = readString(file);
    subtitle = readString(file);
    interpret = readString(file);
    albumn = readString(file);
    author = readString(file);
    copyright = readString(file);
    tabAuthor = readString(file);
    instructions = readString(file);

    //notice
    ul noticeLen = 0;
    file.read((char*)&noticeLen,4);
    if (gtpLog)  qDebug() << "Notice len is " << (int)noticeLen;

    if (noticeLen > 0)
    {
        if (gtpLog)  qDebug() << "Read notices ";
        for (ul i =0 ; i < noticeLen; ++i)
            std::string noticeOne = readString(file);
    }

    byte tripletFeel = 0;
    file.read((char*)&tripletFeel,1);

    int tripletFeelInt = (int)tripletFeel; //hate this - if (gtpLog)  log should fix it
    if (gtpLog)  qDebug() << "Triplet feel = " << tripletFeelInt ;



    ul bpm = 0;
    int signKey = 0;


    file.read((char*)&bpm,4);
    file.read((char*)&signKey,4);


    tab->setBPM(bpm);

    if (gtpLog)  qDebug() <<"Bpm rate is " << bpm ;
    if (gtpLog)  qDebug() <<"Sign Key = " <<signKey ; // << " ; octave " <<octave ;

    //4 8 - 12
    char midiChannelsData[768];
    file.read((char*)midiChannelsData,768);

    if (gtpLog)  qDebug() << "Midi Channels data read. size of structure: "<<(int)sizeof(MidiChannelInfo)<<
           "; full size = "<<(int)(sizeof(MidiChannelInfo)*64 );

    memcpy(tab->GpCompMidiChannels,midiChannelsData,768);


    ul beatsAmount = 0;
    ul tracksAmount = 0;

    file.read((char*)&beatsAmount,4);
    file.read((char*)&tracksAmount,4);

    if (gtpLog)  qDebug() << "Beats count " <<beatsAmount<<"; tracks count " <<tracksAmount ;


    for (ul i = 0;i < tracksAmount; ++i)
    {
        Track *newTrack = new Track();

        for (ul j = 0; j < beatsAmount; ++j)
        {
            Bar *newBeatBar=new Bar(); //RESERVATION
            newTrack->add(newBeatBar);
        }
        tab->add(newTrack);
    }


    for (ul i = 0; i < beatsAmount; ++i)
        readBar(file,tab,tracksAmount,i);


    for (ul i = 0; i < tracksAmount; ++i)
    {
        Track *currentTrack = tab->getV(i);
        readTrack(file,currentTrack);
    }

    Bar *cursorBar = tab->getV(0)->getV(0);
    Beat *cursorBeat = 0;

    if (cursorBar)
    if (cursorBar->len())
     cursorBeat =cursorBar->getV(0);


    if (gtpLog)  qDebug() <<"Begining beats amounts "<<beatsAmount;


    ul globalBeatsAmount = beatsAmount*tracksAmount;
    for (ul i = 0; i < globalBeatsAmount; ++i)
    {

        ul beatsInPair = 0;
        file.read((char*)&beatsInPair,4);

        if (gtpLog)  qDebug() <<i <<" Beats in pair " <<beatsInPair;

        //refact - over here was critical error its not usefull code
        if (beatsInPair > 1000)
        {
            if (i != 0)
            {
                if (gtpLog)  qDebug() << "DEBUG OUT";
                for (int iii = 0; iii < 10; ++iii)
                {
                    byte singleB;
                    file.read((char*)&singleB,1);
                    if (gtpLog)  qDebug() << "[" << iii << "] = " << singleB;
                }
                if (gtpLog)  qDebug() << "DEBUG OUT";
            }

            if (gtpLog)  qDebug() << "Seams to be critical error";
        }

        ul indexOfTrack = i % tracksAmount;
        Track *updatingTrack = tab->getV(indexOfTrack);
        ul indexOfBar = i / tracksAmount;
        Bar *updatingBar = updatingTrack->getV(indexOfBar);

        cursorBar = updatingBar;

        if (cursorBar)
        {
            if (cursorBar->len())
            cursorBeat = cursorBar->getV(0);
            else
                cursorBeat = 0;
        }
        //++cursorBar;
        //cursorBeat = &cursorBar->getV(0);

        for (ul ind = 0; ind < beatsInPair; ++ind)
        {
            Beat *newOne = new Beat();
            cursorBar->add(newOne);
        }

        for (ul j = 0; j < beatsInPair; ++j)
        {
            cursorBeat = cursorBar->getV(j);
            //Reading beat main
            readBeatGP3(file,cursorBeat);
            //!!! THIS IS SHIT had to be CHANGED

            std::vector<int> polyStrings;
            ul totalCountStrings = readStringsFlag(file,polyStrings);


            for (ul noteInd = 0; noteInd < totalCountStrings; ++noteInd)
            {
                Note *newNote= new Note();

                byte sNum=polyStrings[totalCountStrings-noteInd-1];
                newNote->setStringNumber(sNum);

                //then note follows
                readNoteGP3(file,newNote,j,cursorBar);
                //!!! THIS IS SHIT had to be CHANGED
                cursorBeat->add(newNote);
            }

            ++cursorBeat;

        }
    }

    if (gtpLog)  qDebug() << "Importing finished!";
    if (gtpLog)  qDebug() << "fine.";

    return true;
}
