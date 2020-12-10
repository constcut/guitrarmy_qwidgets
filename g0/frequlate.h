#ifndef FREQULATE_H
#define FREQULATE_H


#include "g0/types.h"

class Frequlate
{
public:
    Frequlate();
};

void octaveCalculations();

void findClosestRhythm(short absValue, byte &durGet, byte &detGet, byte &dotGet, int thatBPM);

//class used for the frequency calculator - ang grouperations

/*
Ñ 	65.41			1
    69.30
D 	73.91			1,1299495
    77.78
E 	82.41			1,2598990 1,1150047
F 	87.31 			1,3348111 1,0594588 1,1813015
    92.50
G	98.00			1,4982418
    103.80
A 	110.00
    116.54
B	123.48
Ñ 	130.82
    138.59
D	147.83
    155.56
E	164.81
F	174.62
    185.00
G	196.00
    207.00
A	220.00
*/

#endif // FREQULATE_H
