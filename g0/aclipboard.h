#ifndef ACLIPBOARD_H
#define ACLIPBOARD_H

//first one
//index clipboard
class AClipboard
{
protected:

    //first just use ints, then can review it on refactoring
    int type;
    int n1,m1,k1;
    int n2,m2,k2;

    void *ptr;

    static AClipboard *currentClip;

public:
    AClipboard():
        type(-1),
        n1(-1),m1(-1),k1(-1),
        n2(-1),m2(-1),k2(-1),
        ptr(0)
    {
    }

    static AClipboard* current() { return currentClip; }
    static void setCurrentClip(AClipboard* newClip) {currentClip=newClip;}


    void setType(int newType) { type = newType; }
    //types 0 index bar
    //      1 index beat
    //      2 index bars
    //      3 index beats

    //      4 ptr bar
    //

    int getType() { return type; }

    void setPtr(void *newPtr) {ptr=newPtr;}
    void *getPtr(){return ptr;}

    void setBeginIndexes(int track, int bar, int beat=-1)
    {
        n1=track;
        m1=bar;
        if (beat!=-1) k1=beat;
    }

    void setEndIndexes(int track, int bar, int beat=-1)
    {
        n2=track;
        m2=bar;
        if (beat!=-1) k2=beat;
    }

    int getTrackIndex() { return n1; }
    int getBarIndex() { return m1; }
    int getBeatIndex() { return k1; }


    int getSecondBarI() { return m2; }
    int getSecondBeatI() { return k2; }
    int getSecondTrackI() { return n1; }
};



#endif // ACLIPBOARD_H
