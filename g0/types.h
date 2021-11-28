#ifndef TYPES_H
#define TYPES_H 13

#include "g0/buildheader.h"

#include <vector>
#include <string>

#include <list> 
#include <iostream>


typedef unsigned int uint;
typedef unsigned char byte; //old definition
typedef unsigned long long ul; //TODO заменить во всем проекте на стандартные типы


//types checker //TODO Вынести куда-то
bool check_types();
int getTime();
bool testScenario();
void connectConfigs(void *ptr);
bool midiPrint(std::string fileName);
bool greatCheckScenarioCase(int scen, int from, int to, int v);



template<typename T>
struct is_pointer { static const bool value = false; };

template<typename T>
struct is_pointer<T*> { static const bool value = true; };


template <typename Child, typename Parent> class ChainContainer
{
protected:
    std::vector<Child> sequence; //Эксперименты и замеры TODO
    ChainContainer<Child, Parent> *nextOne;
    ChainContainer<Child, Parent> *prevOne;
    void *parent; //TODO ChainContainer too

    public:

    void setParent(void *newPa) { parent = newPa;}
    void *getParent() { return parent; }

    void setPrev(ChainContainer<Child, Parent> *prev)
    { prevOne=prev;}

    void setNext(ChainContainer<Child, Parent> *next)
    { nextOne=next;}

    ChainContainer<Child, Parent> * getPrev()
    { return prevOne;}

    ChainContainer<Child, Parent> * getNext()
    { return nextOne;}

    virtual ~ChainContainer() {}

    ChainContainer<Child, Parent>& operator=(ChainContainer<Child, Parent> &copy)
    {
        sequence.clear();
        sequence.insert(sequence.begin(),copy.sequence.begin(),copy.sequence.end());
        return *this;
    }

    ChainContainer<Child, Parent>& operator+=(ChainContainer<Child, Parent> &copy)
    {
        //check for end
        //sequence.clear();
        sequence.insert(sequence.begin(),copy.sequence.begin(),copy.sequence.end());
        return *this;
    }

    ChainContainer():nextOne(0),prevOne(0),parent(0)
    {
    }

    ChainContainer(int predefinedSize):nextOne(0),prevOne(0),parent(0) {
        sequence.reserve(predefinedSize); // *=sizeof(CT); //?
    }

    Child& operator[](size_t ind)
    {
        //checks
        return sequence[ind];
    }

    virtual void add(Child &val)
    {
        ChainContainer *is_poly = 0;
        if (is_pointer<Child>::value == true){
           // is_poly = dynamic_cast<ChainContainer*>(val);
        }
        else
            is_poly = 0;
        //dynamic_cast<ChainContainer*>(val);
        if (is_poly)
            is_poly->setParent(this);
        sequence.push_back(val);
    }

    Child &back() {
        return sequence.back();
    }

    void popb() {
        sequence.pop_back();
    }

    void change(int ind, Child &val) {
        sequence[ind] = val;
    }

    Child &getV(int ind) {
       return sequence[ind];
    }

    size_t len() {
        return sequence.size();
    }

    void clear() {
        sequence.clear();
    }

    virtual void insertBefore(const Child &val, int index=0) {
        sequence.insert(sequence.begin()+index,val);
    }

    void remove(int index) {
        sequence.erase(sequence.begin()+index);
        //nothig said about parent
    }

};


#endif // TYPES_H
