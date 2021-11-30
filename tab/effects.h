#ifndef EFFECTS_H
#define EFFECTS_H

#include "types.h"

#include <map>

enum class Effect {
    None = 0,
    Vibrato = 1,
    PalmMute = 2,
    Hammer = 3, //TODO it kind of slide
    Slide = 4,
    LegatoSlide = 5,
    SlideDownV1 = 6,
    SlideUpV1 = 7,
    SlideDownV2 = 8,
    SlideUpV2 = 9,
    Legato = 10,
    Harmonics = 11,
    HarmonicsV2 = 12,
    HarmonicsV3 = 13,
    HarmonicsV4 = 14,
    HarmonicsV5 = 15,
    HarmonicsV6 = 16,
    Bend = 17,
    LetRing = 18,
    Tremolo = 19,
    FadeIn = 20,
    GhostNote = 21,
    GraceNote = 22,
    Stokatto = 23,
    TremoloPick = 24,
    UpStroke = 25,
    DownStroke = 26,
    HeavyAccented = 27,
    Changes = 28,
    Tap = 29,
    Slap = 30,
    Pop = 31,
    Picking1 = 32,
    Picking2 = 33,
    Picking3 = 34,
    Picking4 = 35
};

 //     Effect::

class ABitArray
{
protected:
    uint64_t bits; //TODO vector<bool> resize 32

public:

    ABitArray();
    bool getEffectAt(Effect id);
    void setEffectAt(Effect id, bool value);

    size_t takeBits(){ return bits;}
    void putBits(size_t newBits) { bits = newBits; }

    bool inRange(Effect lowId, Effect highId);

    void logIt();

    bool empty();

    bool operator==(Effect id);// { return get(index); }
    bool operator!=(Effect id);//

    void flush();

    void mergeWith(ABitArray& addition);

};





////////////////////Pack reference - used only for GMY format///////////////////////////

    //pack guide:
    //fret - 6 bit; volume - 6 bit; fingering - 4 bit; = 2bytes
    //...on another mind 3 bits could be used as flags - effects,fingering+else
    //or volume could be packed in smaller distance [0-8] or [0-16]
    //pack effects into one more byte, and if bend is present, then:

   // semisemisemi tone (1/8) semisemi tone (1/4) semi tone (1/2) tone (1) double tone (2) - 5 bits for presence of each
   //duration of point - (double, single, half, 4, 8, 16, 32, 64) - 8 bits
   //+ 1 bit flag of last point
   //+ 1 bit flag of not finishing bend
   //+ 1 bit flag of continued bend

    //So in normal mode minimal note will have 3 bytes; another mode - extra packing
    //0-31 fret with secial coding (limits amount of frets - but always more then 24)
    //8 values for volume - forte piano stype - 3 bits

    //next byte is really hard but helps compression -
    //if hierest bit is == 1 then effects follow
    //if hierest bit it == 0 then
                        // if next bit == 1 then we have note of same volume - and fret stored inside;
                        //next note comes in same way
    //another secret 0-31 for fret and 3 bits for [effects precence1 bit] 2 reserved - volume as duration will store in Beat
    //change precence - in Beat will show that duration changin, or volume is changing - for extra high packing
    //NO	MO	MINDS	ANOUT	HIGH	PACK. main ideas stored here now






#endif // EFFECTS_H
