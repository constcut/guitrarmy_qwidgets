#ifndef TYPES_H
#define TYPES_H 13

#include "g0/buildheader.h"

#include <vector>
#include <string>

#include <list> 
//for variable strings

//WHILE DEBUGGING
#include <iostream>
//

typedef unsigned int uint;
typedef unsigned char byte; //old definition
//typedef short short int byte; //new definition
typedef unsigned long long ul;


//types checker

bool check_types();
/*{
    bool allisfine = true;
    allisfine &= (sizeof(long long) == 8);
    allisfine &= (sizeof(long) == 4);
    allisfine &= (sizeof(short) == 2);
    if (allisfine)
        std::cout << "Check Types fine" << std::endl;
    else
        std::cout << "Failed to check types" << std::endl;
    return allisfine;
} */

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

//YO
//yet Poly lays here




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



//ours strings abstraction

class stringExtended
{
    //dont forget about separators - now its useless but then could help
public:
    std::string inside;
    const char *c_str() { return inside.c_str(); }
    int size() { return inside.size(); }

    stringExtended& operator+=(const char *charStr);
    stringExtended& operator+=(char *charStr);
    stringExtended& operator+=(stringExtended &strEx);
    stringExtended& operator+=(ul &ulongValue);
    stringExtended& operator+=(long &longValue);
    stringExtended& operator+=(double &doubleValue);
    stringExtended& operator+=(std::string value);
    //stringExtended& operator+= em prime object?

    stringExtended& operator<<(const char *charStr);
    stringExtended& operator<<(char *charStr);
    stringExtended& operator<<(stringExtended &strEx);
    stringExtended& operator<<(ul &ulongValue);
    stringExtended& operator<<(long &longValue);
    stringExtended& operator<<(double &doubleValue);
    stringExtended& operator<<(int iValue);
    stringExtended& operator<<(std::string value);

    stringExtended& operator=(const char *charStr);
    stringExtended& operator=(char *charStr);
    stringExtended& operator=(stringExtended &strEx);
    stringExtended& operator=(ul &ulongValue);
    stringExtended& operator=(long &longValue);
    stringExtended& operator=(double &doubleValue);
    stringExtended& operator=(int intValue);
    stringExtended& operator=(std::string value);

    void push_constCharStr(const char *charStr);
    void push_charStr(char *charStr);
    void push_ul(ul&ulongValue);
    void push_long(long&longValue);
    void push_int(int&iValue);
    void push_double(double& doubleValue);
    void push_strEx(stringExtended&strEx);


    ul pop_ul() {return 0;}
    long pop_long() {return 0;}
    double pop_double() {return 0.0;}


    void clear() { inside.clear(); }
};

class TimeCosts
{
    //small timer for scenes time calculations - use for better way of
    //scene tester to check summ time and tab time
public:
    void start() {}
    void stop() {}
};

//typedef stringExtended sX;
// iX; ulX; lX; dX; bX;

/*
class Chain;

template <typename CT> class  ChainElement
{
  public:

  ChainElement(CT &source) { ptr = &source;}
  CT *get() { return ptr; }


  void setPrev(ChainElement *prevOne) { prev=prevOne; }
  void setChain(Chain *chPtr) { master = chPtr; }

  protected:

  ChainElement *prev;
  CT *ptr;
  Chain *master;

};

class Chain
{


};

*/



















#endif // TYPES_H
