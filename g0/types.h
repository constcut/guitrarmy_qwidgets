#ifndef TYPES_H
#define TYPES_H 13

#include "g0/buildheader.h"

#include <vector>
#include <string>

#include <list> 
#include <iostream>


typedef unsigned int uint;
typedef unsigned char byte; //old definition
typedef unsigned long long ul;


//types checker

bool check_types();

int getTime();

bool testScenario();

void connectConfigs(void *ptr);

bool midiPrint(std::string fileName);

bool greatCheckScenarioCase(int scen, int from, int to, int v);
//typedef for local string & vector types?

//MASKS

#define MASK_MLEN 0x70
#define MASK_TIME_METRIC 0xF

#define MASK_OCTAVE  0x0f
#define MASK_KEY     0xf0
#define SHIFT_OCTAVE 0x0
#define SHIFT_KEY    0x4

#define MASK_HARMONY 0x0
#define MASK_EFFECTS 0x0

#define SHIFT_NOTE_STATE 0x0
#define MASK_NOTE_STATE 0x0
#define SHIFT_VOLUME 0x0
#define MASK_VOLUME 0x0


//Poly template: =====================================================================

template<typename T>
struct is_pointer { static const bool value = false; };

template<typename T>
struct is_pointer<T*> { static const bool value = true; };

template <typename CT> class Poly
{
    protected:
    std::vector<CT> sequence;

    Poly<CT> *nextOne;
    Poly<CT> *prevOne;

    void *parent;

    public:

    void setParent(void *newPa) { parent = newPa;}
    void *getParent() { return parent; }

    void setPrev(Poly<CT> *prev)
    { prevOne=prev;}

    void setNext(Poly<CT> *next)
    { nextOne=next;}

    Poly<CT> * getPrev()
    { return prevOne;}

    Poly<CT> * getNext()
    { return nextOne;}

    virtual ~Poly() {}

    Poly<CT>& operator=(Poly<CT> &copy)
    {
        sequence.clear();
        sequence.insert(sequence.begin(),copy.sequence.begin(),copy.sequence.end());
        return *this;
    }

    Poly<CT>& operator+=(Poly<CT> &copy)
    {
        //check for end
        //sequence.clear();
        sequence.insert(sequence.begin(),copy.sequence.begin(),copy.sequence.end());
        return *this;
    }

    Poly():nextOne(0),prevOne(0),parent(0)
    {
    }

    Poly(int predefinedSize):nextOne(0),prevOne(0),parent(0)
    {
        sequence.reserve(predefinedSize); // *=sizeof(CT); //?
    }

    CT& operator[](size_t ind)
    {
        //checks
        return sequence[ind];
    }

    virtual void add(CT &val)
    {

        Poly *is_poly = 0;



        if (is_pointer<CT>::value == true)
        {
           // is_poly = dynamic_cast<Poly*>(val);
        }
        else
        {
            is_poly = 0;
        }

        //dynamic_cast<Poly*>(val);
        if (is_poly)
            is_poly->setParent(this);

        sequence.push_back(val);
    }

    CT &back()
    {
        return sequence.back();
    }

    void popb()
    {
        sequence.pop_back();
    }

    void change(int ind, CT &val)
    {
        sequence[ind] = val;
    }

    CT &getV(int ind)
    {
       return sequence[ind];
    }

    size_t len()
    {
        return sequence.size();
    }

    void clear()
    {
        sequence.clear();
    }

    virtual void insertBefore(const CT &val, int index=0)
    {
        sequence.insert(sequence.begin()+index,val);
    }

    void remove(int index)
    {
        sequence.erase(sequence.begin()+index);
        //nothig said about parent
    }

};


/// NOW OVER THE NEW
///
/// also make PolyPtrChain to avoid 2 ptrs for each block
template <typename CT> class PolyPtr
{
    protected:
    std::vector<CT*> sequence;

    PolyPtr<CT> *nextOne;
    PolyPtr<CT> *prevOne;

    public:

    void setPrev(PolyPtr<CT> *prev)
    { prevOne=prev;}

    void setNext(PolyPtr<CT> *next)
    { nextOne=next;}

    PolyPtr<CT> * getPrev()
    { return prevOne;}

    PolyPtr<CT> * getNext()
    { return nextOne;}

    virtual ~PolyPtr()
    {
        //clean up
    }

    PolyPtr& operator=(PolyPtr &copy)
    {
        sequence.clear();
        sequence.insert(sequence.begin(),copy.sequence.begin(),copy.sequence.end());
        return *this;
    }

    PolyPtr& operator+=(PolyPtr &copy)
    {
        //check for end
        //sequence.clear();
        sequence.insert(sequence.begin(),copy.sequence.begin(),copy.sequence.end());
        return *this;
    }

    PolyPtr():nextOne(0),prevOne(0)
    {
    }

    PolyPtr(int predefinedSize):nextOne(0),prevOne(0)
    {
        sequence.reserve(predefinedSize); // *=sizeof(CT); //?
    }

    CT *operator[](size_t ind)
    {
        if (sequence.size() > ind)
            return sequence[ind];
        else
            return 0;
    }

    virtual void add(CT *val)
    {
        sequence.push_back(val);
    }

    CT *back()
    {
        return sequence.back();
    }

    void popb()
    {
        sequence.pop_back();
    }

    void change(int ind, CT *val)
    {
        sequence[ind] = val;
    }

    CT *getV(int ind)
    {
        //if (sequence.size() > ind)
            return sequence[ind];
        //else
            //return 0;
    }

    size_t len()
    {
        return sequence.size();
    }

    void clear()
    {
        sequence.clear();
    }

    void insertBefore(const CT *val, int index=0)
    {
        sequence.insert(sequence.begin()+index,val);
    }

    void remove(int index)
    {
        sequence.erase(sequence.begin()+index);
    }

};

#endif // TYPES_H
