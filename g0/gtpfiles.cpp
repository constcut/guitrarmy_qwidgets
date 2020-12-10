#include "gtpfiles.h"


#include "g0/astreaming.h"
static AStreaming   logger("gtpfiles");

bool gtpLog = false;

////////////////////////////////////////////////////////////////////////////
char miniBufer[20480];


std::string readString(AFile &file, ul stringLen)
{
   //for current max
   //refact dynamic

   if (gtpLog)  LOG( <<"Reaging string "<<stringLen<<" bytes");
   file.read(miniBufer,stringLen);//changed attention
   miniBufer[stringLen] = 0;

   if (gtpLog)  LOG( << "Readen string "<<miniBufer);

   std::string response = std::string(miniBufer);
   return response;
}

std::string readString(AFile &file)
{
    ul stringLen = 0;
    std::string response = "";

    file.read(&stringLen,4);

    if (stringLen > 0)
    {
        if (gtpLog)
            LOG(<<"String len "<<stringLen);
        response = readString(file,stringLen); //attention and check old
    }
    else
    {
        if (gtpLog)  logger << "String is empty!";
    }
    return response;
}

std::string readStringShiByte(AFile &file)
{
    ul stringLen = 0;
    std::string response = "";

    file.read(&stringLen,4);

    byte whatFo = 0; file.read(&whatFo,1);

    if (stringLen == 0)
        stringLen = whatFo;
    else
    {
        --stringLen;
    }


    if (stringLen > 0)
    {
        if (gtpLog)
            LOG(<<"String len "<<stringLen);
        response = readString(file,stringLen); //attention and check old
    }
    else
    {
        if (gtpLog)  logger << "String is empty!";
    }
    return response;
}


void writeBendGTPOLD(AFile *file, BendPointsGPOld *bend)
{
    //saved to be somewhere in the code
    byte bendType=bend->getType();
    file->write(&bendType,1);

    ul bendHeight = bend->getHeight();
    file->write(&bendHeight,4); //attention again

    ul pointsCount = bend->len();
    file->write(&pointsCount,4);

    for (ul pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        BendPointGPOld *point = &bend->getV(pointInd);

        ul absolutePosition = point->absolutePosition;
        ul verticalPosition = point->heightPosition;
        byte vibratoFlag = point->vibratoFlag;

        file->write(&absolutePosition,4);
        file->write(&verticalPosition,4);
        file->write(&vibratoFlag,1);
    }
}

void writeBendGTP(AFile *file, BendPoints *bend)
{
    byte bendType=bend->getType();
    if (bendType>=1) bendType += 5;

    file->write(&bendType,1);

    //need to find a hieghest point - but this is only for output
    ///ul bendHeight = bend->getHeight();
    ///file->write(&bendHeight,4); //attention again

    ul pointsCount = bend->len();
    file->write(&pointsCount,4);

    for (ul pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        BendPoint *point = &bend->getV(pointInd);

        ul absolutePosition = point->horizontal;
        ul verticalPosition = point->vertical*25;
        byte vibratoFlag = point->vFlag;

        file->write(&absolutePosition,4);
        file->write(&verticalPosition,4);
        file->write(&vibratoFlag,1);
    }
}


void readBendGTP(AFile *file, BendPoints *bend)
{
    byte bendType=0;
    file->read(&bendType,1);

    ul bendHeight = 0;
    file->read(&bendHeight,4); //attention again
    //25 quarter ; 75 - 3 quartes
    //50 half ; 100 - tone
    //... 300 - three tones
    ul pointsCount = 0;
    file->read(&pointsCount,4);

    if (gtpLog)  LOG(<< "Type "<<bendType<<"; Height "<<bendHeight<<"; N= "<<pointsCount);

    if (bendType >= 6)
        bend->setType(bendType-5); //temp action, yet not handled anyway
    else
        bend->setType(0);

    for (ul pointInd=0; pointInd<pointsCount; ++pointInd)
    {
        ul absolutePosition = 0;
        ul verticalPosition = 0;
        byte vibratoFlag = 0;

        file->read(&absolutePosition,4);
        file->read(&verticalPosition,4);
        file->read(&vibratoFlag,1);

        if (gtpLog)
            LOG( << "Point# "<< pointInd << "; absPos="<<absolutePosition<<"; vertPos="
               <<verticalPosition<<"; vibrato- "<<vibratoFlag);


        BendPoint point;
        point.vertical = verticalPosition/25; //25 - 1/12 of 3 tones
        point.horizontal = absolutePosition;
        point.vFlag = vibratoFlag;
        bend->add(point);
    }

    if (gtpLog)  LOG( << "Beng if (gtpLog)  logging finished with "<<(int)bend->len());

    //return bend;
}




void readChordDiagramGP3(AFile &file)
{
       if (gtpLog)  logger << "CHORD_";

    int header=0;
    file.read(&header,1);

    if (header&0x01)
    {
        char skipper[40];
        file.read(skipper,25);

        file.read(skipper,1); //read byte ze

        file.read(skipper,34);
        skipper[34] = 0;
        ///std::string chStr = std::string(skipper);
        int firstFret = 0;
        file.read(&firstFret,4);

        for (int i = 0; i < 6; ++i)
        {
            int fret =0;
            file.read(&fret,4);
        }

        file.read(skipper,36);
    }
    else
    {
         std::string chStr = readStringShiByte(file);
         int firstFret = 0;
         file.read(&firstFret,4);

         if (firstFret != 0)
             for (int i = 0; i < 6; ++i)
             {
                 int fret =0;
                 file.read(&fret,4);
             }
    }
}


void readChordDiagramGP4(AFile &file)
{
   if (gtpLog)  logger << "CHORD";

   int header=0;
   file.read(&header,1);


   char skipper[40];

    if (header&0x01)
    {

        file.read(skipper,16);


        byte singleBy = 0;
        file.read(&singleBy,1);

        file.read(skipper,21);
        skipper[21] = 0;
        std::string chStr = std::string(skipper);

        if (gtpLog)
            LOG( << "Ch text "<<chStr.c_str());

        int firstFret = 0;
        file.read(&firstFret,4);//skli
        file.read(&firstFret,4);

        for (int i = 0; i < 7; ++i)
        {
            int fret =0;
            file.read(&fret,4);


        }

        file.read(skipper,32);
    }
    else
    {
         //readStringShiByte(file);


         int fullLen = 0;
         file.read(&fullLen,4);

         byte singleBy = 0;
         file.read(&singleBy,1);

         file.read(skipper,singleBy);
         skipper[singleBy] = 0;

         std::string chStr = std::string(skipper);

         if (gtpLog)
             LOG( << "Chh text "<<chStr.c_str() );

         int firstFret = 0;
         file.read(&firstFret,4);

         if (firstFret != 0)
             for (int i = 0; i < 6; ++i)
             {
                 int fret =0;
                 file.read(&fret,4);
             }
    }
}

void readChanges(AFile &file, Beat *cursorBeat)
{

    Beat::ChangeTable changeStruct;// = {0};

    changeStruct.newTempo = 0;

    file.read(&changeStruct.newInstr,1);  //1

    file.read(&changeStruct.newVolume,1);
    file.read(&changeStruct.newPan,1);
    file.read(&changeStruct.newChorus,1);
    file.read(&changeStruct.newReverb,1);
    file.read(&changeStruct.newPhaser,1);
    file.read(&changeStruct.newTremolo,1);
    file.read(&changeStruct.newTempo,4); //8

    if (gtpLog)  LOG( <<  "I "<<changeStruct.newInstr<<"; V "<<changeStruct.newVolume<<"; P "<<changeStruct.newPan<<
          "; C "<<changeStruct.newChorus<<"; R "<<changeStruct.newReverb<<"; Ph "<<changeStruct.newPhaser<<
          "; Tr "<<changeStruct.newTremolo<<"; T="<<changeStruct.newTempo);


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
        file.read(&changeStruct.volumeDur,1);

        Beat::SingleChange volCh;
        volCh.changeCount = 0;
        volCh.changeType = 2;
        volCh.changeValue = changeStruct.newVolume;
        cursorBeat->changes.add(volCh);
    }

    if (changeStruct.newPan != 255)
    {
        file.read(&changeStruct.panDur,1);

        Beat::SingleChange panCh;
        panCh.changeCount = 0;
        panCh.changeType = 3;
        panCh.changeValue = changeStruct.newPan;
        cursorBeat->changes.add(panCh);
    }

    if (changeStruct.newChorus != 255)
    {
        file.read(&changeStruct.chorusDur,1);

        Beat::SingleChange chorusCh;
        chorusCh.changeCount = 0;
        chorusCh.changeType = 4;
        cursorBeat->changes.add(chorusCh);
    }

    if (changeStruct.newReverb != 255)
    {
         file.read(&changeStruct.reverbDur,1);

         Beat::SingleChange reverbCh;
         reverbCh.changeCount = 0;
         reverbCh.changeType = 5;
         cursorBeat->changes.add(reverbCh);
    }

    if (changeStruct.newPhaser != 255)
    {
        file.read(&changeStruct.phaserDur,1);

        Beat::SingleChange phaserCh;
        phaserCh.changeCount = 0;
        phaserCh.changeType = 6;
        cursorBeat->changes.add(phaserCh);
    }

    if (changeStruct.newTremolo != 255)
    {
         file.read(&changeStruct.tremoloDur,1);

         Beat::SingleChange tremoloCh;
         tremoloCh.changeCount = 0;
         tremoloCh.changeType = 7;
         cursorBeat->changes.add(tremoloCh);
    }

    //-1 for ul is hiegh but 10000 bpm insane

        if (changeStruct.newTempo < 10000) //some attention here
        {
             file.read(&changeStruct.tempoDur,1);
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
    file.read(&changeStruct.changesTo,1); //not applied! attention


}


void readTrack(AFile &file, Track *currentTrack, int gpVersion=4, int trackIndex=0, byte verInd=255)
{
    byte trackHeader = 0;
    file.read(&trackHeader,1);


    if (trackHeader & 1)
    {
        currentTrack->setDrums(true);
        if (gtpLog)  logger <<"This is drums track!";
    }

    if (gpVersion == 5)
    {
        //ONLY FOR FIRST
        if ((trackIndex==0)||(verInd==0))
        {
            ul byteZero = 0;
            //file.read(&byteZero,4);

            file.read(&byteZero,1);
        }
    }

    ul trackNameLen = 0;
    file.read(&trackNameLen,1);

    if (gtpLog)  LOG( <<"XLENTRACK " <<trackNameLen);

    char trackName[40];
    file.read(trackName,40); //39? or 40?
    std::string trackNameStr(trackName);

    currentTrack->setName(trackNameStr);

    //trackName[trackNameLen] = 0;

    if (gtpLog)  LOG( << " Track name '" << trackName << "' ; head = " <<(int)trackHeader);


    ul stringsAmount = 0;
    file.read(&stringsAmount,4);

    if (gtpLog)  LOG( <<"N strings " << (int) stringsAmount);


    ul tunes[7] = {0}; //TUNEC!!!
    for (ul ii = 0; ii < 7; ++ii)
    {
        file.read(&tunes[ii],4);
    }

    currentTrack->tuning.setStringsAmount(stringsAmount);
    for (ul ii = 0; ii < stringsAmount; ++ii)
    {

        if (gtpLog)  LOG( << "Tunes for "<<ii<<" is "<<tunes[ii]);
        currentTrack->tuning.setTune(ii,tunes[ii]);
    }


    ul port=0, chan=0, chanE=0;
    file.read(&port,4);
    file.read(&chan,4);
    file.read(&chanE,4);

    currentTrack->setGPCOMPInts(0,port);
    currentTrack->setGPCOMPInts(1,chan);
    currentTrack->setGPCOMPInts(2,chanE);

    if (gtpLog)  LOG( << "Port " << port << "; chan " <<chan <<"; chanE "<<chanE);

    ul frets=0;
    file.read(&frets,4);
    ul capo=0;
    file.read(&capo,4);

    if (gtpLog)  LOG( << "Frets " << frets <<"; capo " << capo);

    ul trackColor=0;
    file.read(&trackColor,4);

    if (gtpLog)  LOG( << "Color - "<<trackColor);

    currentTrack->setColor(trackColor);
    currentTrack->setGPCOMPInts(3,frets);
    currentTrack->setGPCOMPInts(4,capo);

    if (gpVersion==5)
    {
        char toSkip[256];

        if (verInd == 1)
            file.read(toSkip,49);
        if (verInd == 0)
            file.read(toSkip,44);

        ul intLen = 0;
        byte byteLen = 0;

        if (verInd == 1)
        {
            file.read(&intLen,4);
            file.read(&byteLen,1);
            file.read(toSkip,intLen-1);
            file.read(&intLen,4);
            file.read(&byteLen,1);
            file.read(toSkip,intLen-1);
        }

        //+after track?
    }
}

void readBeatEffects(AFile &file, Beat *cursorBeat)
{
    byte beatEffectsHead1;
    byte beatEffectsHead2;

    file.read(&beatEffectsHead1,1);
    file.read(&beatEffectsHead2,1);

    //LARGE
    if (gtpLog)  LOG( << "Beat effects flag present. H1=" << beatEffectsHead1 <<
           "; H2=" << beatEffectsHead2);

    //name as bools

    if (beatEffectsHead1 & 32)
    {   //tapping poping slaping

        byte tapPopSlap;
        file.read(&tapPopSlap,1);

        if (gtpLog)  LOG( << "TapPopSlap byte = "<<tapPopSlap);

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
       if (gtpLog)  logger << " read bend tremolo";
       BendPoints *tremoloBend = new BendPoints;
       readBendGTP(&file,tremoloBend);
       cursorBeat->setEffects(19); //would be tremolo
       cursorBeat->effPack.addPack(19,2,tremoloBend);

    }

    //and dear rusedhsajhdkjsa

    if (beatEffectsHead1 & 64)
    {   //updown stroke
        byte upStroke, downStroke;
        file.read(&upStroke,1);
        file.read(&downStroke,1);
        if (gtpLog)  LOG( << "Up Stroke =" << upStroke <<" Down Stroke="<<downStroke);

        if (upStroke)
        cursorBeat->setEffects(25); //upstroke

        if (downStroke)
            cursorBeat->setEffects(26);
        //NOTSET
    }

    if (beatEffectsHead2 & 2)
    {   //pick stoke
        byte pickStoke;
        file.read(&pickStoke,1);
        if (gtpLog)  logger << "Pick stoke ";

        if (pickStoke)
        {
            byte beatEffSet = 29 + pickStoke;
            cursorBeat->setEffects(beatEffSet);
            //useless features cover under anoter field not effects
        }
    }
}

void readNoteEffects(AFile &file, Note *newNote, int gpVersion=4)
{
    if (gtpLog)  logger <<"Bit 3 in header turned on";
    //NOT SET
    byte noteEffectsHead1, noteEffectsHead2;
    file.read(&noteEffectsHead1,1);
    file.read(&noteEffectsHead2,1);

    if (gtpLog)  LOG( << "Note effects heads. H1=" <<noteEffectsHead1<<
            "; H2=" <<noteEffectsHead2);


    if (noteEffectsHead1&1)
    {//bend
        if (gtpLog)  logger << "Bend found.";


        BendPoints *bend = new BendPoints;
        readBendGTP(&file,bend);

        if (gtpLog)
        LOG( <<(int)bend<< " Bend h "<<"; len "<<(int)bend->len()<<"; type"<<bend->getType());

        newNote->setEffect(17);//first common pattern
        newNote->effPack.addPack(17,2,bend); //type 2 is bend
    }


    if (noteEffectsHead1&16)
    {   //grace note
        if (gtpLog)  logger << "Grace note follows";

        byte graceFret = 0;
        byte graceDynamic = 0;
        byte graceTransition = 0;
        byte graceDuration = 0;

        file.read(&graceFret,1);
        file.read(&graceDynamic,1);
        file.read(&graceTransition,1);
        file.read(&graceDuration,1);

        if (gtpLog)  LOG(<<"Fret "<<graceFret<<" Dyn "<<graceDynamic<<" Trans "<<graceTransition<<" Dur "<<graceDuration);

        if (gpVersion==5)
        {
            byte flags =0;
            file.read(&flags,1);
            if (gtpLog)  logger<<"Gp5 grace flags "<<flags;
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
        if (gtpLog)  logger << "Staccato appear";

        newNote->setEffect(23); //staccato
    }

    if (noteEffectsHead2&2)
    {//palm muting

        if (gtpLog)  logger << "Palm mute appear";

        newNote->setEffect(2);
    }

    if (noteEffectsHead1&2)
    {//legato
       newNote->setEffect(10);
       if (gtpLog)  logger << "legatto turned on";
    }

    if (noteEffectsHead1&8)
    {//let ring
       newNote->setEffect(18);
       if (gtpLog)  logger <<" Let ring turned on";
       if (gtpLog)  logger <<" if (gtpLog)  log";
    }

    if (noteEffectsHead2&4)
    {//Tremolo picking : b
        byte tremoloPicking;
        file.read(&tremoloPicking,1);
        if (gtpLog)  logger << "Tremolo byte "<<tremoloPicking;
        newNote->setEffect(24); //tremolo picking
    }

    if (noteEffectsHead2&8)
    {//Slide : b
        byte slide;
        file.read(&slide,1);
        if (gtpLog)  logger << "Slide byte " <<slide;
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
        file.read(&harmonics,1);
        if (gtpLog)  logger << "Harmonics byte "<<harmonics;

        if (gpVersion==5)
        {
            if (harmonics==1)
            {
                //nautur
            }
            if (harmonics==2) //artif
            {
                byte skipIt;
                file.read(&skipIt,1);
                file.read(&skipIt,1);
                file.read(&skipIt,1);
            }
            if (harmonics==3) //tapp
            {
                byte skipIt;
                file.read(&skipIt,1);

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
        file.read(&trill1,1);
        file.read(&trill2,1);
        if (gtpLog)  logger << "Trill b1="<<trill1<<" trill b2="<<trill2;
    }\

    if (noteEffectsHead2&64)
    {
        //vibrato
        newNote->setEffect(1);//1 is vibrato
        if (gtpLog)  logger << "Vibratto turned on";
    }
}

void readNote(AFile &file, Note *newNote, int gpVersion=4)
{
    //unused ul beatIndex, Bar *cursorBar,

    byte noteHeader;
    file.read(&noteHeader,1);

    byte noteType=0;
    if (gtpLog)  LOG( << "Note header "<<(int)noteHeader);

    newNote->setEffect(0); //flush first

    if (noteHeader & 0x20)
    {
        file.read(&noteType,1);
        byte bby=0; //^IN DOCS WE HAVE SHORT INT HERE
        //file.read(&bby,1);
        if (gtpLog)  LOG( << "Note type = "<<(int)noteType<<" : "<<bby);

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
            if (gtpLog)  logger << "Prev note for sNum="<<sNum;

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
                           if (gtpLog)  logger<< strInd <<"PrevN sNum "<<prevSNum<<" "<<fretPrevValue;
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
                             if (gtpLog)  logger << strInd <<" PrevN sNum "<<prevSNum<<" "<<fretPrevValue;
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
                if (gtpLog)  logger << "Prev found "<<prevNote->getStringNumber()<<
                       " "<<prevFret ;


                prevNote->signStateLeeged();

                if (prevNote->getEffects() == 1)
                {
                    newNote->setEffect(1);
                    //and other effects not to break - refact attention
                }

                newNote->setFret(prevFret);


                if (gtpLog)  logger << "After leeg sign state "<<prevNote->getState()<<" wib "<<wasInBlock;


                if (prevFret==63)
                if (gtpLog)  logger<<"if (gtpLog)  log";
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
        if (gtpLog)  logger  <<"Time independent ";
        byte t1,t2;
        file.read(&t1,1);
        file.read(&t2,1);
        if (gtpLog)  LOG(<<"T: "<<t1<<";"<<t2);
        //attention?
        }
    }

    if (noteHeader & 16)
    {
        if (gtpLog)  logger <<"Bit 4 in header turned on";
        byte bByte=0;
        file.read(&bByte,1);
        if (gtpLog)  LOG(<<"velocity byte(forte) "<<bByte);
        newNote->setVolume(bByte);
    }

    if (noteHeader & 32)
    {
        if (gtpLog)  logger <<"Bit 5 in header turned on";
        byte bByte=0;
        file.read(&bByte,1);
        if (gtpLog)  LOG(<<"some byte fret "<<bByte);
        if (noteType != 2)
        {
            if (gtpLog)  logger<<"not leeg setting prev fret";
            newNote->setFret(bByte);
        }
        else
            if (gtpLog)  logger <<"leeg escape prev fret";
    }



    if (noteHeader & 2)
       if (gtpLog)  logger <<"Bit 1 in header turned on"; //DOT NOTE //wow - where is it turned then?

    if (noteHeader & 4)
    {
        if (gtpLog)  logger <<"Bit 2 in header turned on"; //GHOST NOTE
        //ghost not here
        newNote->setEffect(21); //ghost note
    }

    if (noteHeader & 64)
    {
        if (gtpLog)  logger <<"Bit 6 in header turned on"; //ACCENTED
        newNote->setEffect(27); //there is no heavy accented note anymore (
        //in gp4

    }

    if (noteHeader & 128)
    {
        if (gtpLog)  logger <<"Bit 7 in header turned on";

        byte bByte=0;
        byte bByte2=0;
        file.read(&bByte,1);
        file.read(&bByte2,1);

        if (gtpLog)  LOG(<<"fingering byte "<<bByte<<":"<<bByte2);
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
            file.read(toSkip,8);
        }

        file.read(toSkip,1);
    }

    if (noteHeader & 8)
    {
       readNoteEffects(file,newNote,gpVersion);
    }
}

void readBeat(AFile &file, Beat *cursorBeat)
{
    byte beatHeader = 0;
    file.read(&beatHeader,1);

    bool dotted = beatHeader & 0x1;
    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;


    if (gtpLog)  LOG( << "Beat header " << (int)beatHeader);

    cursorBeat->setPause(false);
    if (precStatus)
    {
        byte beatStatus;
        file.read(&beatStatus,1);
        if (gtpLog)  LOG( <<"Beat status "<<(int)beatStatus);
        if ((beatStatus == 2) || (beatStatus == 0))
         cursorBeat->setPause(true);
    }



    byte durationGP =0;
    file.read(&durationGP,1);

    if (gtpLog)  LOG( <<"Beat duration "<<(int)durationGP);

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
        file.read(&trumpletN,4);
        if (gtpLog)  LOG( <<"Beat tump "<<trumpletN);
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
        if (gtpLog)  logger << "Text";

        ul textLen = 0;
        file.read(&textLen,4);

        byte byteLen = 0;
        file.read(&byteLen,1);

        char textBufer[255];
        file.read(textBufer,byteLen);

        //len+1
        textBufer[byteLen]=0;

        if (gtpLog)  LOG( <<"TextLen "<<textLen<<" value "<<textBufer<<"; bL "<<byteLen);

        std::string foundText(textBufer);
        cursorBeat->setGPCOMPText(foundText);
    }

    if (precEffects)
    {
       readBeatEffects(file,cursorBeat);
    }

    if (precChanges)
    {
        if (gtpLog)  logger << "Changes table found";
        readChanges(file,cursorBeat);
    }

}

//readGraceNote

ul readStringsFlag(AFile &file, std::vector<int> &polyStrings)
{
    byte stringsFlag = 0;
    file.read(&stringsFlag,1);

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

    if (gtpLog)  LOG(<<"Strings flag "<<(int)stringsFlag
      <<"; total count "<<totalCountStrings);

    return totalCountStrings;
}


void readBar(AFile &file, Tab *tab, ul tracksAmount, ul index)
{
    ul i = index;

    byte beatHeader = 0;
    file.read(&beatHeader,1);

    byte precNum = beatHeader & 0x1;
    byte precDenum = beatHeader & 0x2;
    byte precBegRepeat = beatHeader & 0x4;
    byte precEndRepeat = beatHeader & 0x8;
    byte precNumAltEnding = beatHeader & 0x10;
    byte precMarker = beatHeader & 0x20;
    byte precTonality = beatHeader & 0x40;
    byte precDoubleBar = beatHeader & 0x80;

    if (gtpLog)  LOG(<< i << " beat h= " << (int)beatHeader);
    if (gtpLog)  LOG( << "[" << precNum << "][" << precDenum << "][" << precBegRepeat << "][" << precEndRepeat << "][" << precNumAltEnding <<
    "][" << precMarker << "][" << precTonality << "][" << precDoubleBar << "]");



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
        file.read(&signNumeration,1);
        if (gtpLog)  LOG( << "Set num to " <<(int)signNumeration);

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
        file.read(&signDenumeration,1);
        if (gtpLog)  LOG( << "Set denum to "	<<(int)signDenumeration);

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
        file.read(&repeatTimes,1);
        if (gtpLog)  LOG( << "Repeat times " <<(int)repeatTimes);
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
        file.read(&altEnding,1);
        if (gtpLog)  LOG( << "AltEnding " << (int)altEnding);
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setAltRepeat(altEnding); //ow shit
        }
    }
    if (precMarker)
    {
        ul unknown = 0;
        file.read(&unknown,4); //they say its a byte.. fuck them..

        byte markerSize;
        file.read(&markerSize,1);

        char markerBufer[255];
        file.read(markerBufer,markerSize);
        markerBufer[markerSize] = 0;

        ul markerColor;
        file.read(&markerColor,4);

        if (gtpLog)  LOG( << "Marker size "<<markerSize<<" buf "<<markerBufer);

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
        file.read(&tonality,1); //skip 1!!
        file.read(&tonality,1);
        if (gtpLog)  LOG( << "Tonality " <<(int)tonality);
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setGPCOMPTonality(tonality);
        }
    }
}


bool Gp4Import::import(AFile &file, Tab *tab, byte knownVersion)
{
    if (gtpLog)  logger << "Starting GP4 import";


    if (knownVersion==0)
    {
        byte preVersion;///???
        file.read(&preVersion,1);
        std::string formatVersion = readString(file,29);
        byte postVersion;///???
        file.read(&postVersion,1);
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
    file.read(&noticeLen,4);
    if (gtpLog)  LOG( << "Notice len is " << (int)noticeLen );

    if (noticeLen > 0)
    {
        if (gtpLog)  logger << "Read notices ";
        for (ul i =0 ; i < noticeLen; ++i)
            std::string noticeOne = readString(file);
    }

    byte tripletFeel = 0;
    file.read(&tripletFeel,1);

    int tripletFeelInt = (int)tripletFeel; //hate this - if (gtpLog)  log should fix it
    //refact
    if (gtpLog)  LOG( << "Triplet feel = " << tripletFeelInt );

    ul lyTrack = 0;
    file.read(&lyTrack,4);
    if (gtpLog)  LOG( << "Lyrics track " <<(int)lyTrack );

    for (int i = 0; i < 5; ++i)
    {
        ul emWo = 0;
        file.read(&emWo,4);
        std::string lyricsOne = readString(file);
    }

    ul bpm = 0;
    int signKey = 0;
    byte octave = 0;

    file.read(&bpm,4);
    file.read(&signKey,4);
    file.read(&octave,1);

    tab->setBPM(bpm);

    if (gtpLog)  LOG( <<"Bpm rate is " << bpm );
    if (gtpLog)  LOG( <<"Sign Key = " <<signKey << " ; octave " <<octave );

    //4 8 - 12
    char midiChannelsData[768];
    file.read(midiChannelsData,768);

    //debug
    if (gtpLog)
        LOG( << "Midi Channels data read. size of structure: "<<(int)sizeof(MidiChannelInfo)<<
           "; full size = "<<(int)(sizeof(MidiChannelInfo)*64)
               <<" and ul "<<(int)(sizeof(unsigned int)) );

    memcpy(tab->GpCompMidiChannels,midiChannelsData,768);


    ul beatsAmount = 0;
    ul tracksAmount = 0;

    file.read(&beatsAmount,4);
    file.read(&tracksAmount,4);

    if (gtpLog)  LOG( << "Beats count " <<beatsAmount<<"; tracks count " <<tracksAmount );


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

    Bar *cursorBar = tab->getV(0)->getV(0);

    Beat *cursorBeat = 0;

    if (cursorBar->len())
     cursorBeat =cursorBar->getV(0);

    if (gtpLog)  LOG( <<"Begining beats amounts "<<beatsAmount );


    ul globalBeatsAmount = beatsAmount*tracksAmount;
    for (ul i = 0; i < globalBeatsAmount; ++i)
    {

        ul beatsInPair = 0;
        file.read(&beatsInPair,4);

        if (gtpLog)  LOG( <<i <<" Beats in pair " <<beatsInPair );

        //refact - over here was critical error its not usefull code
        if (beatsInPair > 1000)
        {
            if (i != 0)
            {
                if (gtpLog)  logger << "DEBUG OUT";
                for (int iii = 0; iii < 10; ++iii)
                {
                    byte singleB;
                    file.read(&singleB,1);
                    if (gtpLog)  LOG( << "[" << iii << "] = " << singleB);
                }
                if (gtpLog)  logger << "DEBUG OUT";
            }

            if (gtpLog)  logger << "Seams to be critical error";
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

    if (gtpLog)  logger << "Importing finished!";
    if (gtpLog)  logger << "fine.";

    return true;
}

///////////////////////WRITE OPERATIONS////////////////////////////
/// \brief writeString
/// \param file
/// \param value
/// \param stringLen
///
//  ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ


/* void writeString(AFile &file, std::string value, ul stringLen=0)
{
    if (stringLen==0)
        stringLen = value.length();

    file.write(&stringLen,4);
    file.write(value.c_str(),stringLen);
}


void writeTrack(AFile &file, Track *currentTrack)
{
    //TEMPLATE
    byte trackHeader = 0;
    file.write(&trackHeader,1);

    ul trackNameLen = 0;
    file.write(&trackNameLen,1);

    char trackName[40];
    file.write(trackName,40);

    ul stringsAmount = 0;
    file.write(&stringsAmount,4);


    ul tunes[7] = {0}; //TUNEC!!!
    for (ul ii = 0; ii < 7; ++ii)
        file.write(&tunes[ii],4);


    ul port=0, chan=0, chanE=0;
    file.write(&port,4);
    file.write(&chan,4);
    file.write(&chanE,4);

    ul frets=0;
    file.write(&frets,4);
    ul capo=0;
    file.write(&capo,4);


    ul trackColor=0;
    file.write(&trackColor,4);

}

void writeBeatEffects(AFile &file, Beat *cursorBeat)
{
    //TEMPLATE
    byte beatEffectsHead1; //prepare function!
    byte beatEffectsHead2;

    file.write(&beatEffectsHead1,1);
    file.write(&beatEffectsHead2,1);

    if (beatEffectsHead1 & 32)
    {   //tapping poping slaping
        byte tapPopSlap;
        file.write(&tapPopSlap,1);
    }

    if (beatEffectsHead2 & 4)
    {   //tremolo
       BendPoints tremoloBend;
       writeBend(file,tremoloBend);
    }

    if (beatEffectsHead1 & 64)
    {   //updown stroke
        byte upStroke, downStroke;
        file.write(&upStroke,1);
        file.write(&downStroke,1);
    }

    if (beatEffectsHead2 & 2)
    {   //pick stoke
        byte pickStoke;
        file.write(&pickStoke,1);
    }
}

void writeBeat(AFile &file, Beat *cursorBeat)
{   //template

    byte beatHeader = 0; //prepare function
    file.read(&beatHeader,1);

    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;

    if (precStatus)
    {
        byte beatStatus;
        file.write(&beatStatus,1);
    }

    byte durationGP =0;
    file.write(&durationGP,1);

    if (precNTrump)
    {
        ul trumpletN = 0;
        file.write(&trumpletN,4);
    }

    if (precChord)
    {
        //readChordDiagram(file);
    }

    if (precText)
    {
        if (gtpLog)  logger << "ATTENTION missing TEXT"; //!!!
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


void writeBar(AFile &file, Bar *cursorBar)
{   //TEMPLATE

    byte beatHeader = 0; //PREPARE
    file.write(&beatHeader,1);

    byte precNum = beatHeader & 0x1;
    byte precDenum = beatHeader & 0x2;
    byte precEndRepeat = beatHeader & 0x8;
    byte precNumAltEnding = beatHeader & 0x10;
    byte precMarker = beatHeader & 0x20;
    byte precTonality = beatHeader & 0x40;

    if (precNum)
    {
        byte signNumeration = 0;
        file.write(&signNumeration,1);
    }

    if (precDenum)
    {
        byte signDenumeration = 0;
        file.write(&signDenumeration,1);
    }

    if (precEndRepeat)
    {
        byte repeatTimes = 0;
        file.write(&repeatTimes,1);
    }

    if (precNumAltEnding)
    {
        byte altEnding = 0;
        file.write(&altEnding,1);
    }

    if (precMarker)
    {
        byte markerSize;
        file.write(&markerSize,1);
        char markerBufer[255];
        file.write(markerBufer,markerSize);
        ul markerColor;
        file.write(&markerColor,4);
    }
    if (precTonality)  //4?
    {
        byte tonality = 0;
        file.write(&tonality,1); //skip 1!! ???
        file.write(&tonality,1);
    }
}


void writeNoteEffects(AFile &file, Note *newNote)
{ //TEMPLATE

    byte noteEffectsHead1, noteEffectsHead2; //prepare
    file.write(&noteEffectsHead1,1);
    file.write(&noteEffectsHead2,1);

    if (noteEffectsHead1&1)
    {
        writeBend(file,newNote->bend);
    }


    if (noteEffectsHead1&16)
    {   //grace note
        byte graceFret = 0;
        byte graceDynamic = 0;
        byte graceTransition = 0;
        byte graceDuration = 0;

        file.write(&graceFret,1);
        file.write(&graceDynamic,1);
        file.write(&graceTransition,1);
        file.write(&graceDuration,1);
    }

    if (noteEffectsHead2&4)
    {//Tremolo picking : b
        byte tremoloPicking;
        file.write(&tremoloPicking,1);
    }

    if (noteEffectsHead2&8)
    {//Slide : b
        byte slide;
        file.write(&slide,1);
    }


    if (noteEffectsHead2&16)
    {//Harmonics : b
        byte harmonics;
        file.write(&harmonics,1);
    }

    if (noteEffectsHead2&32)
    {//Trill : 2b
        byte trill1, trill2;
        file.write(&trill1,1);
        file.write(&trill2,1);
    }\
}

void writeStringsFlag(AFile &file, Beat *cursorBeat)
{ //TEMPLATE
} //UNDONE


void writeNote(AFile &file, Note *newNote)
{ //TEMPLATE

    byte noteHeader; //prepare
    file.write(&noteHeader,1);

    if (noteHeader & 0x20)
    {
        byte noteType;
        file.write(&noteType,1);
    }

    if (noteHeader & 1)
    {
        if (gtpLog)  log  <<"Time independent ";
        byte t1,t2;
        file.write(&t1,1);
        file.write(&t2,1);
    }

    if (noteHeader & 16)
    {
        if (gtpLog)  logger <<"Bit 4 in header turned on";
        byte bByte=0;
        file.write(&bByte,1);
    }

    if (noteHeader & 32)
    {
        if (gtpLog)  logger <<"Bit 5 in header turned on";
        byte bByte=0;
        file.write(&bByte,1);
    }

    if (noteHeader & 128)
    {
        byte bByte=0;
        byte bByte2=0;
        file.write(&bByte,1);
        file.write(&bByte2,1);
    }

    if (noteHeader & 8)
    {
       writeNoteEffects(file,newNote);
    }
}


bool Gp4Export::exPort(AFile &file, Tab &tab)
{
    file.opened();

    tab.getV(0);

    if (gtpLog)  logger << "Starting GP4 export";

    byte preVersion; //=len?
    std::string formatVersion = ""; //deadcode?
    byte postVersion; //==0?

    file.write(&preVersion,1);
    file.write(formatVersion.c_str(),29);
    file.write(&postVersion,1);

    //writing text information
    std::string title,subtitle,interpret,albumn,author,copyright,tabAuthor,instructions;

    writeString(file,title);
    writeString(file,subtitle);
    writeString(file,interpret);
    writeString(file,albumn);
    writeString(file,author);
    writeString(file,copyright);
    writeString(file,tabAuthor);
    writeString(file,instructions);

    //notices
    ul noticeLen = 0;
    file.write(&noticeLen,4);

    if (noticeLen > 0)
    {
        if (gtpLog)  logger << "Write notices";
        std::string anotherNotice;
        for (ul i=0; i < noticeLen; ++i)
            writeString(file,anotherNotice);
    }

    byte tripletFeel = 0;
    file.write(&tripletFeel,1);

    //lyrics
    ul lyTrack = 0;
    file.write(&lyTrack,4);

    for (int i = 0; i < 5; ++i)
    {
        ul emWo = 0; //what does this mean? refact
        file.write(&emWo,4);
        std::string lyricsOne;
        writeString(file,lyricsOne);
    }

    //other values
    ul bpm = 0;
    int signKey = 0;
    byte octave = 0;

    file.write(&bpm,4);
    file.write(&signKey,4);
    file.write(&octave,1);

    //midi channels data
    char midiChannelsData[768];
    file.write(midiChannelsData,768); // 64*each

    //anounts
    ul beatsAmount = 0;
    ul tracksAmount = 0;

    file.write(&beatsAmount,4);
    file.write(&tracksAmount,4);

    for (ul i = 0; i < beatsAmount; ++i)
    {
        //prepare header
        byte beatHeader = 0;
        file.write(&beatHeader,1);

        //WRITE BEAT - as subfunction after read beat would be done
    }


    for (ul i = 0; i < tracksAmount; ++i)
    {
        //should not use old silly - subfunctions go on
    }

    ul globalBeatsAmount = beatsAmount*tracksAmount;
    for (ul i = 0; i < globalBeatsAmount; ++i)
    {
        ul beatsInPair = 0;
        file.write(&beatsInPair,4);

        ul indexOfTrack = i % tracksAmount;
        ul indexOfBar = i / tracksAmount;


        for (ul j = 0; j < beatsInPair; ++j)
        {
            //go on beats subfunction that covers all
            //and also beats effects
            //and also note function
            //that covers notes effects
        }

    }

    if (gtpLog)  logger << "Exporting finish";

    return true;
}

*/

//END of WRITE OPERATIONS

//////////////////////FORMAT GTP5////////////////////////////////
//--////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void readChangesGP5(AFile &file, Beat *cursorBeat, byte verInd)
{

    Beat::ChangeTable changeStruct;// = {0};

    changeStruct.newTempo = 0;

    file.read(&changeStruct.newInstr,1);  //1

    char toSkip[60];

    file.read(toSkip,16);


    file.read(&changeStruct.newVolume,1);
    file.read(&changeStruct.newPan,1);
    file.read(&changeStruct.newChorus,1);
    file.read(&changeStruct.newReverb,1);
    file.read(&changeStruct.newPhaser,1);
    file.read(&changeStruct.newTremolo,1);

    ul strLen  = 0;
    file.read(&strLen,4);
    if (strLen > 1)
    {
        //byte readByte;
        //file.read(&readByte,1);

        std::string tempoName = readString(file,strLen); //not -1
        if (gtpLog)
            LOG(<<"tempo name "<<tempoName.c_str());
    }
    else
    {
        if (gtpLog) logger<<"tempo name is empty";
        byte readByte;
        file.read(&readByte,1);
    }


    file.read(&changeStruct.newTempo,4); //8

    if (gtpLog)  LOG( <<  "I "<<changeStruct.newInstr<<"; V "<<changeStruct.newVolume<<"; P "<<changeStruct.newPan<<
          "; C "<<changeStruct.newChorus<<"; R "<<changeStruct.newReverb<<"; Ph "<<changeStruct.newPhaser<<
          "; Tr "<<changeStruct.newTremolo<<"; T="<<changeStruct.newTempo);


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
        file.read(&changeStruct.volumeDur,1);

        Beat::SingleChange volCh;
        volCh.changeCount = 0;
        volCh.changeType = 2;
        volCh.changeValue = changeStruct.newVolume;
        cursorBeat->changes.add(volCh);
    }

    if (changeStruct.newPan != 255)
    {
        file.read(&changeStruct.panDur,1);

        Beat::SingleChange panCh;
        panCh.changeCount = 0;
        panCh.changeType = 3;
        panCh.changeValue = changeStruct.newPan;
        cursorBeat->changes.add(panCh);
    }

    if (changeStruct.newChorus != 255)
    {
        file.read(&changeStruct.chorusDur,1);

        Beat::SingleChange chorusCh;
        chorusCh.changeCount = 0;
        chorusCh.changeType = 4;
        cursorBeat->changes.add(chorusCh);
    }

    if (changeStruct.newReverb != 255)
    {
         file.read(&changeStruct.reverbDur,1);

         Beat::SingleChange reverbCh;
         reverbCh.changeCount = 0;
         reverbCh.changeType = 5;
         cursorBeat->changes.add(reverbCh);
    }

    if (changeStruct.newPhaser != 255)
    {
        file.read(&changeStruct.phaserDur,1);

        Beat::SingleChange phaserCh;
        phaserCh.changeCount = 0;
        phaserCh.changeType = 6;
        cursorBeat->changes.add(phaserCh);
    }

    if (changeStruct.newTremolo != 255)
    {
         file.read(&changeStruct.tremoloDur,1);

         Beat::SingleChange tremoloCh;
         tremoloCh.changeCount = 0;
         tremoloCh.changeType = 7;
         cursorBeat->changes.add(tremoloCh);
    }

    //-1 for ul is hiegh but 10000 bpm insane

   if (changeStruct.newTempo < 100000) //some attention here
    {
         file.read(&changeStruct.tempoDur,1);
         //set changes table inside
         Beat::SingleChange tempCh;
         tempCh.changeCount = 0;
         tempCh.changeType = 8;
         tempCh.changeValue = changeStruct.newTempo;

         cursorBeat->changes.add(tempCh);

         if (verInd==1)
         {
            byte someSkip;
            file.read(&someSkip,1);

            if (someSkip)
            {
                LOG(<<"Skip byte = "<<someSkip);
            }
         }

    }

    cursorBeat->setEffects(28);
    cursorBeat->effPack.addPack(28,1,&(cursorBeat->changes));

    //refact
    file.read(&changeStruct.changesTo,1); //not applied! attention

    //if (1)
    {
        //char toSkip[2];
        //file.read(toSkip,49); //or 44

        //ul intLen = 0;
        byte byteLen = 0;


        file.read(&byteLen,1); //skipperd

        //file.read(&intLen,4);

        if (verInd==1)
        {
            /* looks like 70 is missing 1byte
            for (int z=0; z<5; ++z)
            {
                byte readOne = 0;
                file.read(&readOne,1);
                logger <<"R n# "<<z<<" "<<readOne;
            }
            */

            //another function?


            std::string rS1 = readString(file);
            std::string rS2 = readString(file);

            if (gtpLog)
            {
                LOG( <<"R1 "<<rS1.c_str()<<" : R1");
                LOG( <<"R2 "<<rS2.c_str()<<" : R2");
            }

        }
    }
}

void readChordDiagramGP5(AFile &file)
{
    //on version 4 it could be not always the same!!!
    //readChordDiagram(file);



    //return;
    char chordBufer[64];
    if (gtpLog)  logger << "Chord";
    file.read(chordBufer,17);


    //file.read(chordBufer,21);
    //chordBufer[21]=0;

    char fByte = 0; file.read(&fByte,1);
    std::string chStr = readString(file,21);

    if (gtpLog)
        LOG( <<fByte<< " Ch str "<<chStr.c_str());


    file.read(chordBufer,4);

    int firstFret = 0;
    file.read(&firstFret,4); //first fret

    for (int i =0; i < 7; ++i)
    {
        file.read(chordBufer,4);//string fret?
    }

    file.read(chordBufer,32);

    //on version 4 it could be not always the same!!!
}


void readBeatGP5(AFile &file, Beat *cursorBeat, byte verInd=255)
{
    byte beatHeader = 0;
    file.read(&beatHeader,1);

    bool dotted = beatHeader & 0x1;
    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;


    if (gtpLog)  LOG( << "Beat header " << (int)beatHeader);

    cursorBeat->setPause(false);
    if (precStatus)
    {
        byte beatStatus;
        file.read(&beatStatus,1);
        if (gtpLog)  LOG( <<"Beat status "<<(int)beatStatus);
        if ((beatStatus == 2) || (beatStatus == 0))
         cursorBeat->setPause(true);
    }



    byte durationGP =0;
    file.read(&durationGP,1);

    if (gtpLog)  LOG( <<"Beat duration "<<(int)durationGP);

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
        file.read(&trumpletN,4);
        if (gtpLog)  LOG( <<"Beat tump "<<trumpletN);
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
        if (gtpLog)  logger << "TEXT";

        ul textLen = 0;
        file.read(&textLen,4);

        byte byteLen = 0;
        file.read(&byteLen,1);

        char textBufer[255];
        file.read(textBufer,byteLen);

        //len+1
        textBufer[byteLen]=0;

        if (gtpLog)  LOG( <<"TextLen "<<textLen<<" value "<<textBufer<<"; bL "<<byteLen);

        std::string foundText(textBufer);
        cursorBeat->setGPCOMPText(foundText);
    }

    if (precEffects)
    {
       readBeatEffects(file,cursorBeat);
    }

    if (precChanges)
    {
        if (gtpLog)  logger << "Changes table found";
        readChangesGP5(file,cursorBeat,verInd);
    }


}

void readBarGP5(AFile &file, Tab *tab, ul tracksAmount, ul index)
{
    ul i = index;

    byte beatHeader = 0;
    file.read(&beatHeader,1);

    byte precNum = beatHeader & 0x1;
    byte precDenum = beatHeader & 0x2;
    byte precBegRepeat = beatHeader & 0x4;
    byte precEndRepeat = beatHeader & 0x8;
    byte precNumAltEnding = beatHeader & 0x10;
    byte precMarker = beatHeader & 0x20;
    byte precTonality = beatHeader & 0x40;
    byte precDoubleBar = beatHeader & 0x80;

    if (gtpLog)  LOG(<< i << " beat h= " << (int)beatHeader);
    if (gtpLog)  LOG( << "[" << precNum << "][" << precDenum << "][" << precBegRepeat << "][" << precEndRepeat << "][" << precNumAltEnding <<
    "][" << precMarker << "][" << precTonality << "][" << precDoubleBar << "]");



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
        file.read(&signNumeration,1);
        if (gtpLog)  LOG( << "Set num to " <<(int)signNumeration );

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
        file.read(&signDenumeration,1);
        if (gtpLog)  LOG( << "Set denum to "	<<(int)signDenumeration);

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
        file.read(&repeatTimes,1);
        if (gtpLog)  LOG( << "Repeat times " <<(int)repeatTimes);
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
        file.read(&unknown,4); //they say its a byte.. fuck them..

        ul markerSize=0;
        file.read(&markerSize,1);

        //file.read(&markerSize,4);

        char markerBufer[255];
        file.read(markerBufer,markerSize);
        markerBufer[markerSize] = 0;

        ul markerColor;
        file.read(&markerColor,4);

        if (gtpLog)  LOG( << "Marker size "<<markerSize<<" buf "<<markerBufer);

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
        file.read(&altEnding,1);
        if (gtpLog)  LOG( << "AltEnding " << (int)altEnding);
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setAltRepeat(altEnding); //ow shit
        }
    }

    if (precTonality)  //4?
    {
        byte tonality = 0;
        file.read(&tonality,1); //skip 1!!
        file.read(&tonality,1);
        if (gtpLog)  LOG( << "Tonality " <<(int)tonality);
        for (ul iTrack = 0;iTrack < tracksAmount; ++iTrack)
        {
            Bar *currentBar = tab->getV(iTrack)->getV(i);
            currentBar->setGPCOMPTonality(tonality);
        }
    }

    char toSkip[5];
    if (precNum || precDenum)
    {
       file.read(toSkip,4);
    }

    if (!precNumAltEnding)
    {
        file.read(toSkip,1);
    }

    //not fromhere
    byte tripletFeel=0;
    file.read(&tripletFeel,1);

   //byte skipByteOh = 0;
    //file.read(&skipByteOh,1);

}


bool Gp5Import::import(AFile &file, Tab *tab, byte knownVersion)
{
    if (gtpLog)  logger << "Starting GP5 import";

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
        file.read(&preVersion,1);
        std::string formatVersion = readString(file,29);
        byte postVersion;///???
        file.read(&postVersion,1);


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
    file.read(&noticeLen,4);
    if (gtpLog)  LOG( << "Notice len is " << (int)noticeLen);

    if (noticeLen > 0)
    {
        if (gtpLog)  logger << "Read notices ";
        for (ul i =0 ; i < noticeLen; ++i)
        {
            /*
            ul intLen = 0;
            file.read(&intLen,4);
            byte noteLen = 0;
            file.read(&noteLen,1);
            logger <<"Note strlens "<<noteLen<<" "<<intLen;
            */
            std::string noticeOne = readStringShiByte(file); //,noteLen
            LOG(<<"Notice#"<<i<<" "<<noticeOne.c_str());
        }
    }

    //byte tripletFeel = 0; //not here in gp5
    //file.read(&tripletFeel,1);


    ul lyTrack = 0;
    file.read(&lyTrack,4);
    if (gtpLog)  LOG( << "Lyrics track " <<(int)lyTrack) ;

    for (int i = 0; i < 5; ++i)
    {
        ul emWo = 0;
        file.read(&emWo,4);
        std::string lyricsOne = readString(file);
    }


    if (versionIndex == 1)
        file.read(placeToSkip,49);
    else
        if (versionIndex == 0)
            file.read(placeToSkip,30);

    for (int i = 0; i < 11; ++i)
    {
         ul pageInt = 0;
         file.read(&pageInt,4);
         byte strLenIn = 0;
         file.read(&strLenIn,1);

         if (gtpLog)
         LOG(<<"Page int "<<pageInt<<" strLen "<<strLenIn);
         std::string whatIsThat = readString(file,strLenIn);
    }



    ul bpm = 0;
    int signKey = 0;
    byte octave = 0;

    file.read(&bpm,4);


    if (versionIndex == 1)
    {
        byte skipByte=0;
        file.read(&skipByte,1);
    }

    file.read(&signKey,4);
    file.read(&octave,1);

    tab->setBPM(bpm);

    if (gtpLog)  LOG( <<"Bpm rate is " << bpm );
    if (gtpLog)  LOG( <<"Sign Key = " <<signKey << " ; octave " <<octave) ;


    //4 8 - 12
    char midiChannelsData[768];
    file.read(midiChannelsData,768);

    if (gtpLog)  LOG( << "Midi Channels data read. size of structure: "<<(int)sizeof(MidiChannelInfo)<<
           "; full size = "<<(int)(sizeof(MidiChannelInfo)*64));

    memcpy(tab->GpCompMidiChannels,midiChannelsData,768);


    //Unknonwn skip
    file.read(placeToSkip,42);


    ul beatsAmount = 0;
    ul tracksAmount = 0;

    file.read(&beatsAmount,4);
    file.read(&tracksAmount,4);

    if (gtpLog)  LOG( << "Beats count " <<beatsAmount<<"; tracks count " <<tracksAmount);


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
            file.read(&skipOne,1);
        }

        readBarGP5(file,tab,tracksAmount,i);

    }

    for (ul i = 0; i < tracksAmount; ++i)
    {
        Track *currentTrack = tab->getV(i);
        readTrack(file,currentTrack,5,i,versionIndex);//5 is the version
    }

    /*
    logger <<"Tracks "<<(int)tab->len();
    logger <<"-";
    logger<<" in cur "<<(int)tab->getV(0)->len();
    logger <<"+";
    */

    Bar *cursorBar = tab->getV(0)->getV(0);
    Beat *cursorBeat = 0; //cursorBar->getV(0);

    if (cursorBar)
    if (cursorBar->len())
     cursorBeat =cursorBar->getV(0);

    if (gtpLog)  LOG( <<"Begining beats amounts "<<beatsAmount );
    byte oneSkip = 0;

    if (versionIndex == 1)
        file.read(&oneSkip,1);
    else
        if (versionIndex == 0)
        {
            file.read(&oneSkip,1);
            file.read(&oneSkip,1);
        }



    ul globalBeatsAmount = beatsAmount*tracksAmount;
    for (ul i = 0; i < globalBeatsAmount; ++i) //
    {
            //SOMWHERE OVER HERE IN GTP5 lays voices
            //for (int voices=0; voices <2; ++voices)
            {
            ul beatsInPair = 0;
            file.read(&beatsInPair,4);

            if (gtpLog)  LOG( <<i <<" Beats in pair " <<beatsInPair ) ;

            //refact - over here was critical error its not usefull code
            if (beatsInPair > 1000)
            {
                if (i != 0)
                {
                    if (gtpLog)  logger << "DEBUG OUT";
                    for (int iii = 0; iii < 10; ++iii)
                    {
                        byte singleB;
                        file.read(&singleB,1);
                        if (gtpLog)  LOG( << "[" << iii << "] = " << singleB);
                    }
                    if (gtpLog)  logger << "DEBUG OUT";
                }

                if (gtpLog)  logger << "Seams to be critical error";
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
                file.read(&readByte,1);
                file.read(&readByte,1);

                if (readByte & 0x8)
                {
                    byte oneMore;
                    file.read(&oneMore,1);
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

          file.read(&preReader,4);

          if (gtpLog)
            LOG(<<" PRE-Reader "<<preReader);

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
              file.read(&readByte0,1);
              file.read(&readByte,1);

              if (readByte & 0x8)
              {
                  byte oneMore;
                  file.read(&oneMore,1);
              }
          }

          byte skipperByte=0;
          file.read(&skipperByte,1);
      }

      /*
      for (int i = 0; i < 11; ++i)
      {
          byte reader = 0;
          file.read(&reader,1);

          if (gtpLog)
            logger<<" Reader "<<reader;
      } //*/

      if (gtpLog)
        logger<<"reaad";

    }

    if (gtpLog)  logger << "Importing finished!";
    if (gtpLog)  logger << "fine.";

    return true;
}

/////////////////////   GP 3  /////////////////////////////////

void readChangesGP3(AFile &file, Beat *cursorBeat)
{

    //gtpLog = true; //autolog!

    Beat::ChangeTable changeStruct;// = {0};

    changeStruct.newTempo = 0;

    file.read(&changeStruct.newInstr,1);  //1

    file.read(&changeStruct.newVolume,1);
    file.read(&changeStruct.newPan,1);
    file.read(&changeStruct.newChorus,1);
    file.read(&changeStruct.newReverb,1);
    file.read(&changeStruct.newPhaser,1);
    file.read(&changeStruct.newTremolo,1);
    file.read(&changeStruct.newTempo,4); //8

    if (gtpLog)  LOG( <<  "I "<<changeStruct.newInstr<<"; V "<<changeStruct.newVolume<<"; P "<<changeStruct.newPan<<
          "; C "<<changeStruct.newChorus<<"; R "<<changeStruct.newReverb<<"; Ph "<<changeStruct.newPhaser<<
          "; Tr "<<changeStruct.newTremolo<<"; T="<<changeStruct.newTempo);


    //NO INSTR IN DOCS
    if (changeStruct.newInstr != 255)
    {
        //file.read(&changeStruct.instrDur,1);

        Beat::SingleChange instrCh;
        instrCh.changeCount = 0;
        instrCh.changeType = 1;
        instrCh.changeValue = changeStruct.newInstr;
        cursorBeat->changes.add(instrCh);

    }

    if (changeStruct.newVolume != 255)
    {
        file.read(&changeStruct.volumeDur,1);

        Beat::SingleChange volCh;
        volCh.changeCount = 0;
        volCh.changeType = 2;
        volCh.changeValue = changeStruct.newVolume;
        cursorBeat->changes.add(volCh);
    }

    if (changeStruct.newPan != 255)
    {
        file.read(&changeStruct.panDur,1);

        Beat::SingleChange panCh;
        panCh.changeCount = 0;
        panCh.changeType = 3;
        panCh.changeValue = changeStruct.newPan;
        cursorBeat->changes.add(panCh);
    }

    if (changeStruct.newChorus != 255)
    {
        file.read(&changeStruct.chorusDur,1);

        Beat::SingleChange chorusCh;
        chorusCh.changeCount = 0;
        chorusCh.changeType = 4;
        cursorBeat->changes.add(chorusCh);
    }

    if (changeStruct.newReverb != 255)
    {
         file.read(&changeStruct.reverbDur,1);

         Beat::SingleChange reverbCh;
         reverbCh.changeCount = 0;
         reverbCh.changeType = 5;
         cursorBeat->changes.add(reverbCh);
    }

    if (changeStruct.newPhaser != 255)
    {
        file.read(&changeStruct.phaserDur,1);

        Beat::SingleChange phaserCh;
        phaserCh.changeCount = 0;
        phaserCh.changeType = 6;
        cursorBeat->changes.add(phaserCh);
    }

    if (changeStruct.newTremolo != 255)
    {
         file.read(&changeStruct.tremoloDur,1);

         Beat::SingleChange tremoloCh;
         tremoloCh.changeCount = 0;
         tremoloCh.changeType = 7;
         cursorBeat->changes.add(tremoloCh);
    }

    //-1 for ul is hiegh but 10000 bpm insane

        if (changeStruct.newTempo < 10000) //some attention here
        {
             file.read(&changeStruct.tempoDur,1);
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

void readBeatEffectsGP3(AFile &file, Beat *cursorBeat)
{
    byte beatEffectsHead;
    file.read(&beatEffectsHead,1);

    if (gtpLog)  LOG( << "Beat effects flag present. H1=" << beatEffectsHead);

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
        file.read(&tapPopSlap,1);

        if (gtpLog)  LOG( << "TapPopSlap byte = "<<tapPopSlap );

        if (tapPopSlap)
        {
            byte beatEffSet = 29 + tapPopSlap;
            cursorBeat->setEffects(beatEffSet);
            int skipInt;
            file.read(&skipInt,4);
        }
        else
        {
            if (gtpLog)  logger << " read bend tremolo";
            BendPoints tremoloBend;
            int tremoloValue = 0;
            file.read(&tremoloValue,4);
            //readBend(file,tremoloBend);
            cursorBeat->setEffects(19); //would be tremolo
        }
    }

    if (beatEffectsHead & 64)
    {   //updown stroke
        byte upStroke, downStroke;
        file.read(&upStroke,1);
        file.read(&downStroke,1);
        if (gtpLog)  LOG( << "Up Stroke =" << upStroke <<" Down Stroke="<<downStroke );

        if (upStroke)
        cursorBeat->setEffects(25); //upstroke

        if (downStroke)
            cursorBeat->setEffects(26);

        if (gtpLog)  LOG( << "if (gtpLog)  log");
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

void readNoteEffectsGP3(AFile &file, Note *newNote)
{
    byte noteEffectsHead;
    file.read(&noteEffectsHead,1);

    if (gtpLog)  LOG( << "Note effects heads. H1=" <<noteEffectsHead);


    if (noteEffectsHead&1)
    {//bend
        if (gtpLog)  logger << "Bend found.";
        readBendGTP(&file,&(newNote->bend));
        newNote->setEffect(17);//first common pattern
    }


    if (noteEffectsHead&16)
    {   //grace note
        if (gtpLog)  logger << "Grace note follows";

        byte graceFret = 0;
        byte graceDynamic = 0;
        byte graceTransition = 0;
        byte graceDuration = 0;

        file.read(&graceFret,1);
        file.read(&graceDynamic,1);
        file.read(&graceTransition,1);
        file.read(&graceDuration,1);

        if (gtpLog)  LOG(<<"Fret "<<graceFret<<" Dyn "<<graceDynamic<<" Trans "<<graceTransition<<" Dur "<<graceDuration);

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
       if (gtpLog)  logger << "legatto turned on";
    }

    if (noteEffectsHead&4)
    {//Slide : b

        if (gtpLog)  logger << "Slide ";
        byte effect = 3;
        newNote->setEffect(effect);
    }

    if (noteEffectsHead&8)
    {//let ring
       newNote->setEffect(18);
       if (gtpLog)  logger <<" Let ring turned on";
       if (gtpLog)  logger <<" if (gtpLog)  log";
    }

}



void readBeatGP3(AFile &file, Beat *cursorBeat)
{
    byte beatHeader = 0;
    file.read(&beatHeader,1);

    bool dotted = beatHeader & 0x1;
    bool precChord = beatHeader & 0x2;
    bool precText = beatHeader & 0x4;
    bool precEffects = beatHeader & 0x8;
    bool precChanges = beatHeader & 0x10;
    bool precNTrump = beatHeader & 0x20;
    bool precStatus = beatHeader & 0x40;


    if (gtpLog)  LOG( << "Beat header " << (int)beatHeader);

    cursorBeat->setPause(false);
    if (precStatus)
    {
        byte beatStatus;
        file.read(&beatStatus,1);
        if (gtpLog)  LOG( <<"Beat status "<<(int)beatStatus);
        if ((beatStatus == 2) || (beatStatus == 0))
         cursorBeat->setPause(true);
    }



    byte durationGP =0;
    file.read(&durationGP,1);

    if (gtpLog)  LOG( <<"Beat duration "<<(int)durationGP);

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
        file.read(&trumpletN,4);
        if (gtpLog)  LOG( <<"Beat tump "<<trumpletN);
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
        if (gtpLog)  logger << "TEXT";

        ul textLen = 0;
        file.read(&textLen,4);

        byte byteLen = 0;
        file.read(&byteLen,1);

        char textBufer[255];
        file.read(textBufer,byteLen);

        //len+1
        textBufer[byteLen]=0;

        if (gtpLog)  LOG( <<"TextLen "<<textLen<<" value "<<textBufer<<"; bL "<<byteLen);

        std::string foundText(textBufer);
        cursorBeat->setGPCOMPText(foundText);
    }

    if (precEffects)
    {
       readBeatEffectsGP3(file,cursorBeat);
    }

    if (precChanges)
    {
        if (gtpLog)  logger << "Changes table found";
        readChangesGP3(file,cursorBeat);
    }

}

void readNoteGP3(AFile &file, Note *newNote, ul beatIndex, Bar *cursorBar)
{
    byte noteHeader;
    file.read(&noteHeader,1);

    byte noteType=0;
    if (gtpLog)  LOG( << "Note header "<<(int)noteHeader);

    newNote->setEffect(0); //flush first

    if (noteHeader & 0x20)
    {
        file.read(&noteType,1);
        byte bby=0; //^IN DOCS WE HAVE SHORT INT HERE
        //file.read(&bby,1);
        if (gtpLog)  LOG( << "Note type = "<<(int)noteType<<" : "<<bby);

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
            if (gtpLog)  logger << "Prev note for sNum="<<sNum;

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
                           if (gtpLog)  logger<< strInd <<"PrevN sNum "<<prevSNum<<" "<<fretPrevValue;
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
                             if (gtpLog)  logger << strInd <<" PrevN sNum "<<prevSNum<<" "<<fretPrevValue;
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
                if (gtpLog)  logger << "Prev found "<<prevNote->getStringNumber()<<
                       " "<<prevFret ;


                prevNote->signStateLeeged();

                if (prevNote->getEffects() == 1)
                {
                    newNote->setEffect(1);
                    //and other effects not to break - refact attention
                }

                newNote->setFret(prevFret);


                if (gtpLog)  logger << "After leeg sign state "<<prevNote->getState()<<" wib "<<wasInBlock;


                if (prevFret==63)
                if (gtpLog)  logger<<"if (gtpLog)  log";
            }
        } */
    }

    //we push it down with a
    if (noteHeader & 1)
    {

        //another duration
        if (gtpLog)  logger  <<"Time independent ";
        byte t1,t2;
        file.read(&t1,1);
        file.read(&t2,1);
        if (gtpLog)  LOG(<<"T: "<<t1<<";"<<t2);
        //attention?

    }

    if (noteHeader & 16)
    {
        if (gtpLog)  logger <<"Bit 4 in header turned on";
        byte bByte=0;
        file.read(&bByte,1);
        if (gtpLog)  LOG(<<"velocity byte(forte) "<<bByte);
        newNote->setVolume(bByte);
    }

    if (noteHeader & 32)
    {
        if (gtpLog)  logger <<"Bit 5 in header turned on";
        byte bByte=0;
        file.read(&bByte,1);
        if (gtpLog)  LOG(<<"some byte fret "<<bByte);
        if (noteType != 2)
        {
            if (gtpLog)  logger<<"not leeg setting prev fret";
            newNote->setFret(bByte);
        }
        else
            if (gtpLog)  logger <<"leeg escape prev fret";
    }



    if (noteHeader & 2)
       if (gtpLog)  logger <<"Bit 1 in header turned on"; //DOT NOTE //wow - where is it turned then?

    if (noteHeader & 4)
    {
        if (gtpLog)  logger <<"Bit 2 in header turned on"; //GHOST NOTE
        //ghost not here
        newNote->setEffect(21); //ghost note
    }

    if (noteHeader & 64)
    {
        if (gtpLog)  logger <<"Bit 6 in header turned on"; //ACCENTED
        newNote->setEffect(27); //there is no heavy accented note anymore (
        //in gp4

    }

    if (noteHeader & 128)
    {
        if (gtpLog)  logger <<"Bit 7 in header turned on";

        byte bByte=0;
        byte bByte2=0;
        file.read(&bByte,1);
        file.read(&bByte2,1);

        if (gtpLog)  LOG(<<"fingering byte "<<bByte<<":"<<bByte2);
    }

    if (noteHeader & 8)
    {
       readNoteEffectsGP3(file,newNote);
    }
}

/////////////////////////////////////////////////////////////


bool Gp3Import::import(AFile &file, Tab *tab, byte knownVersion)
{
    if (gtpLog)  logger << "Starting GP3 import";

    if (knownVersion==0)
    {
        byte preVersion;
        file.read(&preVersion,1);
        std::string formatVersion = readString(file,29);
        byte postVersion;
        file.read(&postVersion,1);
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
    file.read(&noticeLen,4);
    if (gtpLog)  LOG( << "Notice len is " << (int)noticeLen);

    if (noticeLen > 0)
    {
        if (gtpLog)  logger << "Read notices ";
        for (ul i =0 ; i < noticeLen; ++i)
            std::string noticeOne = readString(file);
    }

    byte tripletFeel = 0;
    file.read(&tripletFeel,1);

    int tripletFeelInt = (int)tripletFeel; //hate this - if (gtpLog)  log should fix it
    if (gtpLog)  LOG( << "Triplet feel = " << tripletFeelInt) ;



    ul bpm = 0;
    int signKey = 0;


    file.read(&bpm,4);
    file.read(&signKey,4);


    tab->setBPM(bpm);

    if (gtpLog)  LOG( <<"Bpm rate is " << bpm );
    if (gtpLog)  LOG( <<"Sign Key = " <<signKey ); // << " ; octave " <<octave ;

    //4 8 - 12
    char midiChannelsData[768];
    file.read(midiChannelsData,768);

    if (gtpLog)  LOG( << "Midi Channels data read. size of structure: "<<(int)sizeof(MidiChannelInfo)<<
           "; full size = "<<(int)(sizeof(MidiChannelInfo)*64 ));

    memcpy(tab->GpCompMidiChannels,midiChannelsData,768);


    ul beatsAmount = 0;
    ul tracksAmount = 0;

    file.read(&beatsAmount,4);
    file.read(&tracksAmount,4);

    if (gtpLog)  LOG( << "Beats count " <<beatsAmount<<"; tracks count " <<tracksAmount );


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


    if (gtpLog)  LOG( <<"Begining beats amounts "<<beatsAmount);


    ul globalBeatsAmount = beatsAmount*tracksAmount;
    for (ul i = 0; i < globalBeatsAmount; ++i)
    {

        ul beatsInPair = 0;
        file.read(&beatsInPair,4);

        if (gtpLog)  LOG( <<i <<" Beats in pair " <<beatsInPair );

        //refact - over here was critical error its not usefull code
        if (beatsInPair > 1000)
        {
            if (i != 0)
            {
                if (gtpLog)  logger << "DEBUG OUT";
                for (int iii = 0; iii < 10; ++iii)
                {
                    byte singleB;
                    file.read(&singleB,1);
                    if (gtpLog)  LOG( << "[" << iii << "] = " << singleB);
                }
                if (gtpLog)  logger << "DEBUG OUT";
            }

            if (gtpLog)  logger << "Seams to be critical error";
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

    if (gtpLog)  logger << "Importing finished!";
    if (gtpLog)  logger << "fine.";

    return true;
}
