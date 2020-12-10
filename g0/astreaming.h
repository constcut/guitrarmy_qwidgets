#ifndef ASTREAMING_H
#define ASTREAMING_H

#include "g0/types.h"

#include "g0/afile.h"

#include <ctime>

//debugging
#include <iostream>


//class for streaming << >> operations
//for hadling files logs strings

//it saves in abstraction inputed values
//2long long short char string that covers under container

//when container reaches size of ZZZ
//it sends signal to putout stream

//putout could be could by itself


class AStreamingLine;
class AStreaming;

class AStreaming
{
public:
//make possible to print here as to stdin out)
//or print to stream add print to log functions

protected:
    AStreamingLine *childLine;
    stringExtended stringEx;
    stringExtended stamp;
    AFile *file;

    std::string fileName; //for log file
    time_t logStamp;

    bool pushToStd;

    static AFile *logFile;
public:

    static void setLogFile(AFile *file);
    static AFile* getLogFile();

    void makeStamp();
    ~AStreaming() { /*fin_last_line();*/ }
    AStreaming();//{childLine = new AStreamingLine(this);}
    AStreaming(std::string logName, bool pushToStdOut=true);

    AStreamingLine &operator<<(std::string stringValue);
    AStreamingLine &operator<<(ul value);
    AStreamingLine &operator<<(long value);
    AStreamingLine &operator<<(double value);
    AStreamingLine &operator<<(byte value);
    AStreamingLine &operator<<(int value);

    /*AStreamingLine &operator<<(size_t value)
    {   //trick
        int intValue = value;
        return this->operator <<(intValue);
    }*/


    void smoothPush(std::string stringValue, bool toFile=false);
    void smoothPush(ul ulongValue, bool toFile=false);
    void smoothPush(long longValue, bool toFile=false);
    void smoothPush(int intValue, bool toFile=false);
    void smoothPush(byte byteValue, bool toFile=false);
    void smoothPush(double doubleValue, bool toFile=false);

    //void smoothPush(int val) { smoothPush((long)val); }

    void fin_last_line();

    void flush();

    void setFile(AFile &file);
    void setFile(AFile *file);

};

class AStreamingLine
{
protected:
    AStreaming *father;
    AStreamingLine *stepFather;

    AStreamingLine *child;
public:
    AStreamingLine(AStreaming *pa):father(pa),stepFather(0),child(0)
    {//no more?
        //std::cout <<"faLine"<<std::endl;
        //end line before output - out short solution
       // pa->fin_last_line(); //pushing from bufer to stream
    }

    AStreamingLine(AStreamingLine *pa):father(0),stepFather(pa),child(0)
    {

    }

    ~AStreamingLine();


    AStreaming &operator<<(std::string stringValue);
    AStreaming &operator<<(ul value);
    AStreaming &operator<<(long value);
    AStreaming &operator<<(int value);
    AStreaming &operator<<(double value);
    AStreaming &operator<<(byte value);
};


//Log class

//based on astreaming - object
//Log(std::string)

//AStreaming log("tab.h") - writes always to log.txt (if not set another file as default)
//[time][file name] body;
//so -> should add "set stamp"

#define LOG(STREAM) {stringExtended sX; sX STREAM; logger << sX.c_str(); }




#endif // ASTREAMING_H
