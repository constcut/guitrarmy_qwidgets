#ifndef BAR_H
#define BAR_H

#include "types.h"
#include "beat.h"


class Track;

class Bar : public ChainContainer<Beat*, Track> {

public:
    Bar() {
        flush();
    }
    virtual ~Bar() {
        for (ul i=0; i < len(); ++i)
            delete at(i);
    }

    void printToStream(std::ostream &stream);

    void flush()
    {
        signatureNum = signatureDenum = 0;
        repeat = repeatTimes = altRepeat = 0;
        markerColor = 0;
        completeStatus = 0;
    }

    Bar &operator=(Bar *another)
    {
        clone(another);
        return *this;
    }

    virtual void add(Beat *&val)
    {
        if (val)
        {
            val->setParent(this);
            ChainContainer<Beat*, Track>::add(val);
        }
    }

    virtual void insertBefore(Beat* &val, int index=0)
    {
        if (val)
        {
            val->setParent(this);
            ChainContainer<Beat*, Track>::insertBefore(val,index);
        }
    }


protected:

    byte signatureNum;
    byte signatureDenum;

    byte repeat;//reprease options
    byte repeatTimes;
    byte altRepeat;

    //GP comp - marker, tonality
    byte gpCompTonality;
    std::string markerText;
    ul markerColor; //white byte == 1 if empty

    byte completeStatus;
    short completeAbs;
    ul completeIndex;

    public:
    //SET GET operations

    void setSignNum(byte num) { signatureNum = num; }
    void setSignDenum(byte denum) { signatureDenum = denum; }

    byte getSignNum() { return signatureNum; }
    byte getSignDenum() { return signatureDenum; }

    //!completeStatus !!! that should go private and opt
    void countUsedSigns(byte &numGet, byte &denumGet);

    byte getCompleteStatus();
    double getCompleteAbs();
    ul   getCompleteIndex();

    void setRepeat(byte rValue, byte times=0)
    {
        if (rValue == 0) repeat = 0;
        else repeat |= rValue;
        if(times) repeatTimes=times;
    }
    byte getRepeat() { return repeat; }
    byte getRepeatTimes() { return repeatTimes; }

    void setAltRepeat(byte number) { altRepeat = number; }
    byte getAltRepeat() { return altRepeat; }

    void setGPCOMPTonality(byte tValue) { gpCompTonality = tValue; }
    byte getGPCOMPTonality() { return gpCompTonality; }

    void setGPCOMPMarker(std::string &text, ul color) { markerText = text; markerColor = color; }
    void getGPCOMPMarker(std::string &text, ul &color) { text = markerText; color = markerColor; }

    //pack function
     void clone(Bar *from);
};


#endif // BAR_H
