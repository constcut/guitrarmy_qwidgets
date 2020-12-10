#include "frequlate.h"

#include "g0/types.h"

Frequlate::Frequlate()
{
}

void octaveCalculations()
{
    double octave[]= {65.41, 69.93, 73.91, 77.78, 82.41,
                      87.31, 92.50, 98.0,  103.8, 110.0,
                      116.54, 123.48};

    double octave2[]= {55.0, 59.08, 61.50, 65.41, 69.93,
                                          73.91, 77.78, 82.41,
                                          87.31, 92.50, 98.0,  103.8};
    for (int i=0 ; i < 12; ++i)
    {
        octave[i] *= 64.0;
        octave2[i] *= 64.0;
    }


    double intervals[13] = {0};
    double intervals2[13] = {0};

    for (int i=0 ; i < 12; ++i)
    {
        if (i)
        {
            intervals[i]=100.0*(octave[i]/octave[i-1]-1.0);
            intervals2[i]=100.0*(octave2[i]/octave2[i-1]-1.0);
        }
        std::cout<<i<<" Maj: "<<intervals[i]<<"; Min: "<<intervals2[i]<<"; diff: "<<intervals[i]-intervals2[i]<<std::endl;
    }
}


//THIS function had to be optimized - refact
void findClosestRhythm(short absValue, byte &durGet, byte &detGet, byte &dotGet, int thatBPM)
{
   short pabsValue = absValue;

   short pabsNet[7] = {4000, 2000, 1000, 500, 250, 125, 62};

   short wideNet[13] = {500, 545, 571, 600, 615, 666, 727, 750, 800, 857, 888, 923,  1000};
   //                   l    ld11 h7   ld5  h13  h3   h11  ld    h5   hd7  h9   hd13    h

   // lo = 1; hi=0; then dot(easiear to write); then 4b trum;
   // 1 0 0;  1 1 11;  0 0 7;  1 1 5;  0 0 13;  0 0 3; 0 0 11; 0 0 5; 0 1 7;  0 0 9; 0 1 13; 0 0 0;

   byte widePrepare[13][3] = { {1,0,0}, {1,1,11}, {0,0,7}, {1,1,5}, {0,0,13}, {0,0,3}, {0,0,11}, {1,1,0},
                               {0,0,5}, {0,1,7}, {0,0,9}, {0,1,13}, {0,0,0}};

   //if (pabsValue > 4000)
   //attention + check all the values for whole note with dot

   if (thatBPM!=120)
   {
       short base = (240000/thatBPM)*2;;

       for (int i = 0; i < 7; ++i)
       {
           pabsNet[i] = base;
           base /= 2;
       }
   }

   short hiInd = 5; //high
   short loInd = 6; //low

  // log << "Find closest "<<pabsValue;

   for (short i = 0; i < 7; ++i)
   {
       if (pabsNet[i] == pabsValue)
       {
           durGet = i;
           detGet=dotGet=0;
           return;
       }

       if (pabsNet[i] < pabsValue)
       {
           loInd = i;
           hiInd = i-1;
           break;
       }
   }

   durGet = 0;

   short diffLo = pabsValue - pabsNet[loInd];
   short diffHi = pabsNet[hiInd]-pabsValue;

   if (diffHi > diffLo)
       durGet = loInd;
   else
       durGet = hiInd;

   dotGet = 0;
   detGet = 0;

   return;

  // log << "Lo "<<pabsNet[loInd]<<"; Hi "<<pabsNet[hiInd];

   //Lo 3 Hi 2
   short coefPabsValue = pabsValue;

   short reLo = loInd;

   if (reLo > 3)
       while (reLo > 3)
       {
           coefPabsValue *= 2;
           --reLo;
       }

   if (reLo < 3)
        while (reLo < 3)
        {
            coefPabsValue /= 2;
            ++reLo;
        }

  // log <<"coefPabsValue= "<<coefPabsValue<<"; coefLoInd= "<<reLo;

   short closestInd  = 0;
   short closestDiff = 10000;

   //there could be special optimization for the search
   for (short i = 0; i < 13; ++i)
   {
        if (wideNet[i] == coefPabsValue)
        {
            durGet = widePrepare[i][0] + hiInd;
            dotGet = widePrepare[i][1];
            detGet = widePrepare[i][2];
            return;
        }
        else
        {
            short diff = wideNet[i] > coefPabsValue ? wideNet[i] - coefPabsValue : coefPabsValue - wideNet[i];
            if (diff < closestDiff)
            {
                closestDiff = diff;
                closestInd = i;
            }
        }
   }

   //log <<"Closest difference= "<<closestDiff;

   durGet = widePrepare[closestInd][0] + hiInd;
   dotGet = widePrepare[closestInd][1];
   detGet = widePrepare[closestInd][2];
}

void findPauseList(short absValue)
{


    if (absValue % 125 == 0)
    {
        //could be set a simple
       // log << "Simple abs for pause list";
    }
    else
    {
        short lessQ = absValue % 1000;

        //log <<"Not Simple abs for pause list "<<lessQ;

        if (lessQ == 334)
        {

        }
        else if (lessQ == 667)
        {

        }
        //else ;
            //log <<"UnusualOne";
    }
}
