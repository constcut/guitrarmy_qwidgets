#ifndef AMUSIC_H
#define AMUSIC_H

#include "types.h"
#include "abitarray.h"

class MidiFile;
class Tab;
//Rythm: =====================================================================

class Rythm : public Poly<ul>
{
    //first bit determines pause or signal
    //64,32,16,8,4,2,1,11 - 3 bits
    //next 4 bits used for
    //0 - normal 1 - dot. 2-14 trumplets. 15 - reserved
  protected:

    byte numeration;
    byte denumeration;
    byte overAllSize;

    void countSize();


  public:

    Rythm &operator=(Rythm another){ return *this; }

    virtual ~Rythm(){}
    Rythm() {}
    Rythm(size_t preSize): Poly<ul>(preSize){}

    enum MLen //as byte
    {
        whole,//mask for them
        half,
        quoter,
        eigth,
        sixt,
        thirty2,
        sixty4,
        doubled
    };

    enum TimeMetric
    {
        normal=1,
        dot=2,
        triplet=3,
        let4=4,
        let5=5,
        let6=6,
        let7=7,
        let8=8,
        let9=9,
        let10=10,
        let11=11,
        let12=12,
        let13=13
    };

    //index operations

    bool isPause(size_t ind);
    MLen getMLen(size_t ind);
    TimeMetric getTimeMetric(size_t ind);

    std::string to_s();

};

//Melody: =====================================================================

class Melody : public Poly<ul>
{
  protected:

  public:

    unsigned long instrument;
    byte volume;
    byte panoram;

    struct mnote
    {
        byte octave;
        byte key;
    };

    EffectsMap mapOfEffects;

    enum NoteState
    {
        normalNote=0,
        startingNote=1,
        continingNote=2,
        endingNote=3,
        deadNote=4
    };

    enum Volume //as int constants
    {
        vol_fff,
        vol_ff,
        vol_f,
        vol_m,
        vol_p,
        vol_pp,
        vol_ppp
        //etc
    };

    Melody &operator=(Melody another){ return *this; }

    virtual ~Melody(){}
    Melody() {}
    Melody(size_t preSize): Poly<ul>(preSize){}


    mnote getMNote(size_t ind);
    bool effectPresent(size_t ind);
    NoteState getNoteState(size_t ind);
    //effect getEffect(size_t ind);
    Volume getVolume(size_t ind);
    bool isHarmonic(size_t ind);

    std::string to_s();

};

//Poly covers ============================================


class PolyMelody : public Poly<Melody>
{
protected:
public:
  virtual ~PolyMelody(){}
  PolyMelody() {}
  PolyMelody(size_t preSize): Poly<Melody>(preSize){}

  std::string to_s();
};

class PolyRythm : public Poly<Rythm>
{
protected:
public:
  virtual ~PolyRythm(){}
  PolyRythm() {}
  PolyRythm(size_t preSize): Poly<Rythm>(preSize){}

  std::string to_s();
};


//AMusic: =====================================================================

class AMusic
{
protected:
    PolyRythm rSection;
    PolyMelody mSection;

    Poly<ul> hSection; //harmony section future calculations

    int bpm;

public:
    AMusic();
    //AMusic(const MidiFile &mFile);
    //AMusic(const Tab &tab);//or appl

    //operator [] for melody access
    //for Rythm just function //AMusicR where default operator - rythm

    Rythm  &gR(size_t ind) { return rSection[ind]; } //attention
    Melody &gM(size_t ind) { return mSection[ind]; } //not safe!
    ul gH(size_t ind) { return ind; } //not yet

    size_t lenR() { return rSection.len(); }
    size_t lenM() { return mSection.len(); }
    size_t lenH() { return 0; }
    void calcH() {} //note yet as upper one

    std::string to_s();

    MidiFile produceMidi(); //covered under midi now attention


    bool readFromTab(Tab *tab, ul barIndex=0);
    void test();

    void printToStream(std::ostream &stream);
    //void saveToStream();
    //void loadFromStream();

    //Accessors
    int getBPM() { return bpm; }
};

#endif // AMUSIC_H
