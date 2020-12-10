#include "astreaming.h"
#include "g0/afile.h"

#include <QDateTime>
#include <QString>


AFile *AStreaming::logFile = 0;


void AStreaming::setLogFile(AFile *file)
{
    logFile = file;
}

AFile* AStreaming::getLogFile()
{
    return logFile;
}

//out of static

 AStreaming::AStreaming():file(0),pushToStd(true)
 {
     childLine = new AStreamingLine(this);
 }

 AStreaming::AStreaming(std::string logName, bool pushToStdOut):file(0),fileName(logName),pushToStd(pushToStdOut)
 {
     childLine = new AStreamingLine(this);
 }

 //NO console for a while
 void AStreaming::makeStamp()
 {

     QDateTime timeDate(QDateTime::currentDateTime());
     QString st = timeDate.toString(Qt::ISODate);
     std::string time = st.toStdString();
     time = time.substr(time.find("T")+1);

     //std::cout << st.toStdString().c_str();

     stamp.clear();

     //skip time for a while
     stamp << "[" << time << "]";
     stamp << "[" << fileName << "] ";

 }

void AStreaming::fin_last_line()
{

  //output stringEx
    if (pushToStd)
    std::cout <<std::endl<< stamp.c_str();

    if (logFile != 0)
    {
        logFile->write((void*)"\n",2);
        logFile->write((void*)stamp.c_str(),stamp.inside.size()); //BAD BAD

        logFile->flush();
    }

    if (file == 0)
    {
        //std::cout << "Console out "<<std::endl;
        //console output
        if (pushToStd)
        std::cout << stringEx.c_str();

        if (logFile != 0)
        {
            logFile->write((void*)stringEx.c_str(),stringEx.inside.size()); //BAD BAD

            logFile->flush();
        }
    }
    else
    {
        //std::cout << "File out "<<std::endl;
        //file output
        file->write((void*)stringEx.c_str(),stringEx.inside.size());

        file->flush();
    }
  //clean stringEx
    //stringEx = "";
    stringEx.clear();

}

void AStreaming::smoothPush(std::string stringValue, bool toFile)
{
  if (!toFile)
    stringEx.push_charStr((char*)stringValue.c_str());
  else
  {
      stringExtended pushed;
      pushed << stringValue;
      if (logFile)
      {
          logFile->write(pushed.inside.c_str(),pushed.inside.size());
      }
      std::cout << pushed.c_str(); //AFTERPUSH EFFECT
  }
}

void AStreaming::smoothPush(ul ulongValue, bool toFile)
{
    if (!toFile)
        stringEx.push_ul(ulongValue);
    else
    {
        stringExtended pushed;
        pushed << ulongValue;
        if (logFile)
        {
            logFile->write(pushed.inside.c_str(),pushed.inside.size());
        }
        std::cout << pushed.c_str(); //AFTERPUSH EFFECT
    }
}

//size_t skipped as unknown type yet .. maybe add later
void AStreaming::smoothPush(long longValue, bool toFile)
{
    if (!toFile)
     stringEx.push_long(longValue);
    else
    {
        stringExtended pushed;
        pushed << longValue;
        if (logFile)
        {
            logFile->write(pushed.inside.c_str(),pushed.inside.size());
        }
        std::cout << pushed.c_str(); //AFTERPUSH EFFECT
    }
}

void AStreaming::smoothPush(double doubleValue, bool toFile)
{
    if (!toFile)
        stringEx.push_double(doubleValue);
    else
    {
        stringExtended pushed;
        pushed << doubleValue;
        if (logFile)
        {
            logFile->write(pushed.inside.c_str(),pushed.inside.size());
        }
        std::cout << pushed.c_str(); //AFTERPUSH EFFECT
    }
}

void AStreaming::smoothPush(int intValue, bool toFile)
{
    if (!toFile)
        stringEx.push_int(intValue);
    else
    {
        stringExtended pushed;
        pushed << intValue;
        if (logFile)
        {
            logFile->write(pushed.inside.c_str(),pushed.inside.size());
        }
        std::cout << pushed.c_str(); //AFTERPUSH EFFECT
    }
}


void AStreaming::flush()
{
    if (stringEx.inside.empty()==false)
    {
        fin_last_line();

    }
}

/////////////////Operators//////////////////////////

AStreamingLine &AStreaming::operator<<(std::string stringValue)
{
    makeStamp();//new line
    stringEx.clear();
    stringEx <<stringValue;
    //fin_last_line();

    AStreamingLine bastard(this);
    return bastard; //return *childLine;
}

AStreamingLine &AStreaming::operator<<(ul value)
{
    makeStamp();//new line
    stringEx.clear();
    stringEx << value;
    //fin_last_line();
    AStreamingLine bastard(this);
    return bastard; //return *childLine;
}

AStreamingLine &AStreaming::operator<<(long value)
{
    makeStamp();//new line
    stringEx.clear();
    stringEx << value;
    //fin_last_line();
    AStreamingLine bastard(this);
    return bastard; //return *childLine;
}

AStreamingLine &AStreaming::operator<<(double value)
{
    makeStamp();//new line
    stringEx.clear();
    stringEx << value;
    //fin_last_line();
    AStreamingLine bastard(this);
    return bastard; //return *childLine;
}

AStreamingLine &AStreaming::operator<<(byte value)
{
    makeStamp();//new line
    stringEx.clear();
    stringEx << value;
    //fin_last_line();
    AStreamingLine bastard(this);
    return bastard; //return *childLine;
}

AStreamingLine &AStreaming::operator<<(int value)
{
    makeStamp();//new line
    stringEx.clear();
    stringEx << value;
    //fin_last_line();
    AStreamingLine bastard(this);
    return bastard; //return *childLine;
}

////////////////CHILD/////////////////////////

//This is a first child - but it could have others, yet they all cycled on first child

//parentLine = 0 for root element

AStreaming &AStreamingLine::operator<<(std::string stringValue)
{
    if (father)
       father->smoothPush(stringValue,true);
    else
        stepFather->operator <<(stringValue); //true - to file
 //child = new AStreamingLine(this);
 //AStreamingLine child2(this);
 //child = &child2;
    //return child2;
    return *father;//*this;
}

//итак возвращаться значения должны с увеличением значений, но это значит что каждый вложенный
//должен создать себе подобного,

AStreaming &AStreamingLine::operator<<(ul value)
{
    if (father)
       father->smoothPush(value,true);
    else
        stepFather->operator <<(value);
 //child = new AStreamingLine(this);

// AStreamingLine child2(this);
 //child = &child2;
    //return child2; //same as down *this //and child*
  return *father;//*this;
}

AStreaming &AStreamingLine::operator<<(long value)
{
    if (father)
       father->smoothPush(value,true);
    else
        stepFather->operator <<(value);
 //child = new AStreamingLine(this);
 return *father;//*this;
}

AStreaming &AStreamingLine::operator<<(int value)
{
    if (father)
       father->smoothPush(value,true);
    else
        stepFather->operator <<(value);
 //child = new AStreamingLine(this);
return *father;//*this;
}

AStreaming &AStreamingLine::operator<<(double value)
{
    if (father)
       father->smoothPush(value,true);
    else
        stepFather->operator <<(value);
 //child = new AStreamingLine(this);
 return *father;//*this;
}

AStreaming &AStreamingLine::operator<<(byte value)
{
 int byte = value;
 if (father)
    father->smoothPush(byte,true);
 else
     stepFather->operator <<(value);

return *father;//*this;
}



AStreamingLine::~AStreamingLine()
{
    //notify
    //return;
    //std::cout <<"destructor~"<<std::endl;

    /*
    if (father)
        std::cout << "Father line" <<std::endl;
    else
        std::cout <<"Step line" <<std::endl;

    */

    //if (child)
    {
       //delete child; //child would care of themselfs

        if (stepFather)
        {
            if (stepFather->father)
            {
                father->fin_last_line();
            }
        }

        if (father)
        {
            father->fin_last_line();
        }
    }

}
