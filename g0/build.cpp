#include "buildheader.h"

#include <QTime>
#include <QElapsedTimer>

#include <iostream>

const char GVERSION[] = VERSION_MACROS;

GlobalVariables globals;

QElapsedTimer timer;

int getTime()
{
    //QTime time;

    static bool firstRun = true;
    if (firstRun)
    {
        timer.start();
        firstRun=false;
    }

    int result = timer.elapsed();
    if (result < 0) result *= -1;

    return result;
}


void setUserId(std::string value)
{
    globals.theUserId = value;
}

const char* getUserId()
{
    return globals.theUserId.c_str();
}

const char *getGuitarmyVersion()
{
    return GVERSION;
}

void setTestLocation(std::string newTL)
{
    std::string invertedLocation="";

    std::cout << "setCurrent location to "<<newTL.c_str()<<std::endl;
    globals.testsLocation = newTL;

    //invert
    invertedLocation.clear();

    for (size_t i = 0; i < newTL.length(); ++i)
        if (newTL[i]=='/')
            invertedLocation.push_back('\\');
        else
            invertedLocation.push_back(newTL[i]);

    globals.invertedLocation = invertedLocation;
}

const char *getTestsLocation() { return globals.testsLocation.c_str(); }

const char *getInvertedLocation() { return globals.invertedLocation.c_str(); }

//===============================================================

void initGlobals()
{
    //cover first set here from main function

    globals.invertedLocation = "";
    globals.testsLocation = "";
    globals.theUserId = "";

    globals.screenSize.height=0;
    globals.screenSize.width=0;
    globals.screenSize.dpi=0;

    //ALSO move some parts of config here
    //scale
    //time coef

    globals.isMobile = false; //desktop actually is default

#ifdef __ANDROID_API__
    globals.isMobile = true;
#endif
    //iOs + Win Phone definitions

    globals.platform = "other";

#ifdef linux
    globals.platform = "linux";  //not very sure
#endif
#ifdef WIN32
   globals.platform = "windows";
#endif
#ifdef __ANDROID_API__
   globals.platform = "android";
#endif

}



