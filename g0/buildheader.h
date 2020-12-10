#ifndef BUILDHEADER
#define BUILDHEADER

#define GBUILD_QT 1

#include <string>

const char *getTestsLocation();
const char *getInvertedLocation();
void setTestLocation(std::string newTL);

#define VERSION_MACROS "Guitarmy b-version 0.4.9"

const char *getGuitarmyVersion();

const char *getUserId();
void setUserId(std::string value);

void initGlobals();

struct GlobalVariables
{
    std::string testsLocation;
    std::string invertedLocation; //for win only

    std::string theUserId;

    bool isMobile; //true - mobile - else desktop

    struct ScreenSize
    {
        int width;
        int height;
        int dpi; //not set yet

        double scale; //move here from configuration

    } screenSize;

    std::string platform;

};

extern GlobalVariables globals;

#endif // BUILDHEADER

