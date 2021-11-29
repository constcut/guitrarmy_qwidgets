#ifndef TYPES_H
#define TYPES_H 13

#include "g0/buildheader.h"

#include <vector>
#include <string>
#include <list> 
#include <iostream>
#include <memory>


int getTime();
bool testScenario();
void connectConfigs(void *ptr);
bool midiPrint(std::string fileName);
bool greatCheckScenarioCase(uint32_t scen, uint32_t from, uint32_t to, uint32_t v);


template<typename T>
struct is_pointer { static const bool value = false; };

template<typename T>
struct is_pointer<T*> { static const bool value = true; };


template <typename Child, typename Parent> class ChainContainer
{
protected:
    std::vector<std::unique_ptr<Child>> sequence;
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

    std::unique_ptr<Child>& operator[](size_t ind) {
        return sequence[ind];
    }

    virtual void push_back(std::unique_ptr<Child> val) {
        sequence.push_back(std::move(val));
    }

    Child* back() {
        return sequence.back().get();
    }

    void pop_back() {
        sequence.pop_back();
    }

    void change(int ind, std::unique_ptr<Child> val) { //TODO remove?
        sequence[ind] = std::move(val);
    }

    std::unique_ptr<Child>& at(int ind) {
       return sequence.at(ind);
    }

    size_t size() {
        return sequence.size();
    }

    void clear() {
        sequence.clear();
    }

    virtual void insertBefore(std::unique_ptr<Child> val, int index=0) {
        sequence.insert(sequence.begin()+index, std::move(val));
    }

    void remove(int index) {
        sequence.erase(sequence.begin()+index);
    }

};


#endif // TYPES_H
