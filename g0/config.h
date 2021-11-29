#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <vector>

#define CONF_PARAM(z) AConfig::getInstance()->values[ z ]

class QImage;

//qt dependent yet
class ImagePreloader
{
protected:
    std::map<std::string, QImage*> imageMap;
    bool inv;

public:
    ImagePreloader():inv(false){}

    void loadImage(std::string imageName);
    void loadImages();

    QImage *getImage(std::string imageName);

    void invertAll();

    void setInvert(bool toInvert) {inv=toInvert;}
};


class AConfig
{
public:
    bool *logs[10];
    std::string logsNames[10];
    int topIndex;

    //value names
    std::map<std::string,std::string> values;
    void addLine(std::string anotherLine);

    ImagePreloader imageLoader; //TODO sepparate so config works without qt

    double scaleCoef;

    double timeCoef;

public:
    AConfig();

    void connectLog(bool *ptrValue, int index=-1,std::string logName="unknown");

    void load(std::ifstream& file);
    void save(std::ofstream& file);

    void addValue(std::string name, std::string val);

    double getTimeCoef() { return timeCoef; }
    void setTimeCoef(double newTC) { timeCoef = newTC; }

    double getScaleCoef() { return scaleCoef; }
    void setScaleCoef(double scale) { scaleCoef = scale; }

    void printValues();
    void checkConfig();

    void cleanValues() { values.clear(); }

    static AConfig *inst;
    static void setInstance(AConfig *conf) { inst = conf; }
    static AConfig *getInstance() { return inst; }
};

class Skin
{
protected:
    //just cfg lines
    std::vector<std::string> configLines;
public:
    Skin() { }
    //
    virtual void init()=0;

    void setIntoConfig(AConfig *conf)
    {
        for (size_t i=0; i < configLines.size(); ++i)
            conf->addLine(configLines[i]);
    }
};

class DarkSkin : public Skin
{
public:
    DarkSkin(){init();}
    virtual void init()
    {
        configLines.push_back("colors.background = black");
        configLines.push_back("colors.curBar = white");
        configLines.push_back("colors.curBeat = gray");
        configLines.push_back("colors.curString = darkgray");
        configLines.push_back("colors.curTrack = darkgray");
        configLines.push_back("colors.default = AAAAAA");
        configLines.push_back("colors.exceed = darkred");
        configLines.push_back("colors.panBG = darkgray");
        configLines.push_back("invertImages = 1");
    }
};

class LightSkin : public Skin
{
public:
    LightSkin(){init();}
    virtual void init()
    {
        configLines.push_back("colors.background = white");
        configLines.push_back("colors.curBar = black");
        configLines.push_back("colors.curBeat = darkgray");
        configLines.push_back("colors.curString = gray");
        configLines.push_back("colors.curTrack = gray");
        configLines.push_back("colors.default = 888888");
        configLines.push_back("colors.exceed = darkred");
        configLines.push_back("colors.panBG = dark");
        configLines.push_back("invertImages = 0");
    }
};

class ClassicSkin : public Skin
{
public:
    ClassicSkin(){init();}
    virtual void init()
    {
        configLines.push_back("colors.background = 449966"); //TODO ? кажется сломались цвета
        configLines.push_back("colors.curBar = darkblue");
        configLines.push_back("colors.curBeat = yellow");
        configLines.push_back("colors.curString = 7799FF");
        configLines.push_back("colors.curTrack = blue");
        configLines.push_back("colors.default = black");
        configLines.push_back("colors.exceed = darkred");
        configLines.push_back("colors.panBG = gray");
        configLines.push_back("invertImages = 0");
    }
};

class ClassicInvertedSkin : public Skin
{
public:
    ClassicInvertedSkin(){init();}
    virtual void init()
    {
        configLines.push_back("colors.background = 449966");
        configLines.push_back("colors.curBar = darkblue");
        configLines.push_back("colors.curBeat = yellow");
        configLines.push_back("colors.curString = 7799FF");
        configLines.push_back("colors.curTrack = blue");
        configLines.push_back("colors.default = black");
        configLines.push_back("colors.exceed = darkred");
        configLines.push_back("colors.panBG = gray");
        configLines.push_back("invertImages = 1");
    }
};

// <application android:theme="@android:style/Theme.Holo"/>

//"@android:style/Theme.Material.Light"


#endif // CONFIG_H
