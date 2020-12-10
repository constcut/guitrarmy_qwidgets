#include "rec.h"

//later please incapsulate here:
#include "g0/fft.h"
#include "g0/waveanalys.h"

#include "g0/astreaming.h"
static AStreaming logger("rec");


Track *GWave::generateTrack()
{
    int fullLen = origin.size()/2; //16 bit only? yep
    short *rawData = (short*)(origin.data());

    int indataCursor = 0;
    //will fail all and everything now - not prepared
    Bar *newBar = generateBar(); //rawData,indataCursor);

    Track *track = new Track;
    //give birth std tunning etc

    while (newBar != 0)
    {
        track->add(newBar);
        newBar = generateBar();//rawData,indataCursor);
    }

    return track;
}

Bar *GWave::generateBar()//short *source, int &cursor)
{
    Bar *bar = new Bar;
    bar->flush();

    bar->setSignNum(32);
    bar->setSignDenum(4);

    for (int noteI = 0; noteI < notes.size(); ++noteI)
    {
        if (notes[noteI].freq < 1.0)
            continue; //skip emty ones (must be presetted for rhy analys)

        //1: finding beat length
        int noteBegin = notes[noteI].noteBegin;
        int noteEnd = notes[noteI].noteEnd;
        int noteLength = noteEnd-noteBegin;
        //2-32; 4-16; 8-8; 16-4th; 32-2th; 64-whole;
        ///findClosestsethm(); - good function for Rhy analyse

        //hot fix - aproximator
        if (noteLength > 32)
            noteLength += 16;
        else
        if (noteLength > 16)
            noteLength += 8;
        else
        if (noteLength > 8)
            noteLength += 4;
        else
        if (noteLength > 4)
            noteLength += 2;
        else
        if (noteLength > 2)
            noteLength += 1;
        //hot fix end


        byte durNoteValue=6;
        //First need to calculate the rest - not just throw it away
        while (noteLength >= 2)
        {
            --durNoteValue;
            noteLength /= 2;
        }

        if (durNoteValue > 6)
            durNoteValue = 0; // if too long then whole - yet.

        Beat *beat = new Beat();
        beat->setDuration(durNoteValue);
        beat->setDurationDetail(0); beat->setDotted(0);

        //calculate the fret
        FreqTable freqTab;
        double freq = notes[noteI].freq;
        int freqTabResult = freqTab.getScaledIndex(freq);

        //on normal guitar skip octave -1
        if (freqTabResult > 0)
        {
            freqTabResult -= 12; //big octave skip
            freqTabResult -= 4; //jump to Mi

            if (freqTabResult < 0)
                freqTabResult += 12; //one more hot fix on lower octave
        }
        else
            freqTabResult = 0;

        byte localFret = freqTabResult;

        //now add Note itself
        Note *newNote=new Note();
        newNote->setFret(localFret);
        newNote->setStringNumber(6); //string 6
        newNote->setState(0);

        beat->add(newNote);
        bar->add(beat);

        if (noteI != notes.size()-1)
        { //if this is not a last not - search for pause

            int pauseLen = notes[noteI+1].noteBegin-notes[noteI].noteEnd;
            byte pauseDur = 6;
            //hot fix - aproximator
            if (pauseLen > 32)
                pauseLen += 16;
            else
            if (pauseLen > 16)
                pauseLen += 8;
            else
            if (pauseLen > 8)
                pauseLen += 4;
            else
            if (pauseLen > 4)
                pauseLen += 2;
            else
            if (pauseLen > 2)
                pauseLen += 1;
            //hot fix end

            while (pauseLen >= 2)
            {
                --pauseDur;
                pauseLen/=2;
            }
            if (pauseDur > 6)
                pauseDur = 0;

            Beat *pauseBeat = new Beat();
            pauseBeat->setDuration(pauseDur);
            pauseBeat->setDurationDetail(0); pauseBeat->setDotted(0);
            bar->add(pauseBeat);
        }

    }

    return bar;
}


byte GWave::runRythmicAnalyse()
{
    int fullLen = origin.size()/2; //16 bit only? yep
    short *rawData = (short*)(origin.data());

    //transform rawdata to notes
    //rAn.findNotesPositions(energyLevels,energyTypes,&notes,0);
}

byte GWave::runMelodicAnalyse()
{
    short *rawData = (short*)(origin.data());

    mAn.setupFreq(rawData,125,&notes); //125 is bad bad
    //hide 125 under classs value
    //set up notes freqs
}

bool GWave::loadFile(std::string fileName, std::vector<int> *params)
{
    //refactoring begins here!
    QFile fileHandler;
    fileHandler.setFileName(fileName.c_str());
    if (fileHandler.open(QIODevice::ReadOnly) == false) return;

    origin = fileHandler.readAll();

    fileHandler.close();

    //parse params given in order
    int zoomValue = 10;
    int zoomCoef = zoomValue; //oups
    int bpmValue = 120;
    int eLev1Value = 500;
    int eLev2Value = 850;
    int eLev3Value = 500;

    int wavePosition = 0; //shift from the begin

    if (params)
    {
        if (params->size()>=1)
            zoomValue = params->at(0);
        if (params->size()>=2)
            bpmValue = params->at(1);
        if (params->size()>=3)
            eLev1Value = params->at(2);
        if (params->size()>=4)
            eLev2Value = params->at(3);
        if (params->size()>=5)
            eLev3Value = params->at(4);
       // if (params->size()>=6)
           // wavePosition = params->at(5);
    }

    //From ORIGINAL RECIPE ....

    short *rawData = (short*)origin.data();
    int waveLimit = origin.size()/2;


    int localPosition = 0; //part of crap

    energyTypes.clear();
    energyLevels.clear();

    double floatBPM = bpmValue;
    int bpmDependentWindow = 8000.0/(floatBPM/60.0)/32.0;

    //energy levels + types allocation
    int amountOfEnergyLevels = waveLimit/bpmDependentWindow;

    for (int i = wavePosition+1; i < waveLimit; )
    {
        int fullSumm = 0;

        for (int coefIndex = 0; coefIndex < bpmDependentWindow; ++coefIndex)
        {
            fullSumm += abs(rawData[i-1]-rawData[i]);
            ++i;
            if (i >= waveLimit) break;
        }

        fullSumm /= bpmDependentWindow;
        energyLevels.push_back(fullSumm);
        ++localPosition;
    }


    energyTypes.insert(energyTypes.begin(),energyLevels.size(),0);
    for (int i = 1; i < localPosition-1; ++i)
    {
        energyTypes[i]=0;
        if (energyLevels[i]<energyLevels[i+1])
            energyTypes[i]=2;
        if (energyLevels[i]>energyLevels[i+1])
            energyTypes[i]=1;
        if ((energyLevels[i] > energyLevels[i+1])&&(energyLevels[i] > energyLevels[i-1]))
        {
            energyTypes[i]=3; //not yet absolute
        }
        if ((energyLevels[i] < energyLevels[i+1])&&(energyLevels[i] < energyLevels[i-1]))
        {
            energyTypes[i]=4;// not yet /absolute
        }
    }

    //NOW MIXED TOGETHER MEL AND RHY RECOGNIZERS
    notes.clear();

    rAn.findNotesPositions(&energyLevels,&energyTypes,&notes,params);
    mAn.setupFreq(rawData,bpmDependentWindow,&notes);

    LOG( << "Found "<<(int)notes.size()<<" notes ");

}


void BaseMel::setupFreq(short *source, int bpmWindow, std::vector<RecognizedNote> *notes)
{
    //Melodic recognyzer
    for (int i = 0; i < notes->size(); ++i)
    {
        int noteEnd = (*notes)[i].noteEnd;
        int notePeak = (*notes)[i].notePeak;
        int noteBegin = (*notes)[i].noteBegin;

        if (noteEnd-notePeak >= 9)//constant must be recalculated
        {
            FFT fft(1024); //2048
            std::vector<Peak> *peaks=0;

             //memory leak here
            fft.transform(&(source[(notePeak+1)*bpmWindow])); //notePeak was
            fft.countAmplitude();

            fft.findPeaks(7.8125); // /2.0
            peaks=fft.getPeaks();

            stringExtended sX;
            double peaksSumm = 0.0;
            for (int i = 0; i < peaks->size(); ++i)
            {
                Peak peak= peaks->operator [](i);
                sX<<"PB "<<peak.getFreq()<<":"<<peak.getPosition()<<":"<<peak.getAmplitude()*10<<"; ";
                //getFreq
                peaksSumm += peak.getAmplitude();
            }
            //logger << "#" <<sX.c_str(); // peaklog
            //logger << "Peaks amp "<<peaksSumm;

            LocalFreqTable localFreq;
            localFreq.addPeaks(peaks);
            localFreq.sortPeaks();
            //old way of vote
            //localFreq.recountHarmonicalUnion();
            //localFreq.voteNow();
            localFreq.voteNewWay();

            std::vector<LocalFreqTable::LFTvote> *votes = localFreq.getRezultVotes();
            stringExtended sX3;
            for (int i = 0; i < votes->size(); ++i)
            {
                  sX3 << (*votes)[i].rFreq<<" "<<(*votes)[i].value<<";";
            }
            //logger <<"!!"<<sX3.c_str(); //peaklog

            sX.clear();
            peaks = localFreq.getPeaks();
            for (int i = 0; i < peaks->size(); ++i)
            {
                Peak peak= peaks->operator [](i);
                sX<<"PA "<<peak.getFreq()<<":"<<peak.getAmplitude()*10<<"; ";
                //getFreq
            }
            //logger << "#" <<sX.c_str(); // peak log


            double *ampSrc = fft.getAmplitude();

            stringExtended sX2;
            for (int i = 0; i < 128; ++i)
            {
                sX2<<(int)(ampSrc[i]*100)<<" ;";
            }
            //logger << "#" << sX2.c_str();

            stringExtended freqStr;
            freqStr<<(*votes)[0].rFreq;

            (*notes)[i].freq = (*votes)[0].rFreq;

        }
        else
        {
            //short rythmic note
        }

    }
}

void BaseRhy::findNotesPositions(std::vector<int> *energyLevels, std::vector<int> *energyTypes, std::vector<RecognizedNote> *notes, std::vector<int> *params)
{
    int bpmValue = 120;
    int eLev1Value = 500;
    int eLev2Value = 850;
    int eLev3Value = 500;

    int wavePosition = 0; //shift from the begin

    if (params)
    {
        if (params->size()>=2)
            bpmValue = params->at(1);
        if (params->size()>=3)
            eLev1Value = params->at(2);
        if (params->size()>=4)
            eLev2Value = params->at(3);
        if (params->size()>=5)
            eLev3Value = params->at(4);
    }

    //Rhythmic recogizer
    for (int i = 0; i < energyLevels->size()-1; ++i)
    {
        if ((energyTypes->at(i) == 4) || (energyTypes->at(i+1) == 2))//blue now or yellow is next
        if (energyLevels->at(i) < eLev1Value) //the constant
        {
            int noteBegin = i;
            ++i;
            if (energyTypes->at(i)==2 || energyTypes->at(i)==3)
            {
                while (energyTypes->at(i)==2) //while yello
                {
                    ++i; //skip
                    if (i >= energyLevels->size()) break;
                }
                if (i >= energyLevels->size()) break;

                if (energyTypes->at(i)==3) //green finally
                    if (energyLevels->at(i) > eLev2Value)//the constant
                    {
                        //and lound
                        int notePeak = i; //could save peak value


                        int noteEnd = -1;
                        while (i < energyLevels->size()-1)
                        {
                            ++i;
                            if (energyTypes->at(i)==4)
                            if (energyLevels->at(i) < eLev3Value)
                            {
                                noteEnd = i;
                                break;
                            }
                        }

                        if (noteEnd != -1)
                        {

                            RecognizedNote anotherNote;
                            anotherNote.noteBegin = noteBegin;
                            anotherNote.notePeak = notePeak;
                            anotherNote.noteEnd = noteEnd;
                            anotherNote.freq = -1.0; // rhythmic value
                            notes->push_back(anotherNote);

                        }
                        --i; //to not skip not end
                    }
            }
            //else //false alarm
        }
    }
}

//------------------------------------------------
/*
 *
 *
 */ // / ---------------------------------------------
 int ScaledWave::makeFrom(GWave *wav, double scaleCoef)
 {
     coef = scaleCoef;
     waveData.clear();

     double cycleLen = 8000.0/coef; //to global settings

     short *rawData = (short*)(wav->origin.data());

     int globalIndex = 0;
     int samplesAmount = wav->origin.size()/2;
     while (globalIndex<samplesAmount)
     {
         int minV = 65535;
         int maxV = -65535;
         for (double inScaleInd=0.0; inScaleInd< cycleLen; inScaleInd+=1.0)
         {

            if (rawData[globalIndex] > maxV)
                maxV = rawData[globalIndex];

            if (rawData[globalIndex] < minV)
                minV = rawData[globalIndex];

            ++globalIndex;
            if (globalIndex>=samplesAmount) break;
         }
         waveData.push_back(minV);
         waveData.push_back(maxV);
     }


 }
//=====================================================



