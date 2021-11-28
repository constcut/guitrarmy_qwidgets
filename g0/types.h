#ifndef TYPES_H
#define TYPES_H 13

#include "g0/buildheader.h"

#include <vector>
#include <string>

#include <list> 
#include <iostream>


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
    std::vector<Child*> sequence;
    ChainContainer<Child, Parent> *nextOne;
    ChainContainer<Child, Parent> *prevOne;
    Parent *parent;

    public:

    void setParent(Parent *newPa) { parent = newPa;}
    Parent *getParent() { return parent; }

    void setPrev(ChainContainer<Child, Parent> *prev)
    { prevOne=prev;}

    void setNext(ChainContainer<Child, Parent> *next)
    { nextOne=next;}

    ChainContainer<Child, Parent> * getPrev()
    { return prevOne;}

    ChainContainer<Child, Parent> * getNext()
    { return nextOne;}

    virtual ~ChainContainer() {}

    ChainContainer<Child, Parent>& operator=(ChainContainer<Child, Parent> &copy) {
        sequence.clear();
        sequence.insert(sequence.begin(),copy.sequence.begin(),copy.sequence.end());
        return *this;
    }

    ChainContainer<Child, Parent>& operator+=(ChainContainer<Child, Parent> &copy) {
        sequence.insert(sequence.begin(),copy.sequence.begin(),copy.sequence.end());
        return *this;
    }

    ChainContainer():nextOne(0),prevOne(0),parent(0){}

    ChainContainer(int predefinedSize):nextOne(0),prevOne(0),parent(0) {
        sequence.reserve(predefinedSize);
    }

    Child* operator[](size_t ind) {
        return sequence[ind];
    }

    virtual void push_back(Child* val) {
        sequence.push_back(val);
    }

    Child* &back() {
        return sequence.back();
    }

    void pop_back() {
        sequence.pop_back();
    }

    void change(int ind, Child* val) { //TODO remove?
        sequence[ind] = val;
    }

    Child* &at(int ind) {
       return sequence.at(ind);
    }

    size_t size() {
        return sequence.size();
    }

    void clear() {
        sequence.clear();
    }

    virtual void insertBefore(Child* val, int index=0) {
        sequence.insert(sequence.begin()+index, val);
    }

    void remove(int index) {
        sequence.erase(sequence.begin()+index);
    }

};


#endif // TYPES_H
