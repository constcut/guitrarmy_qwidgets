#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <vector>

#define CONF_PARAM(z) AConfig::getInst().values[ z ]



namespace gtmy {


    void setTestLocation(std::string newTL);
    void initGlobals();


    class AConfig
    {
    public:
        bool *logs[10];
        std::string logsNames[10];
        int topIndex;

        std::unordered_map<std::string,std::string> values;
        void addLine(std::string anotherLine);

        double scaleCoef;
        double timeCoef; //TODO setters getters

        std::string testsLocation;
        std::string invertedLocation;
        std::string theUserId;
        bool isMobile;
        struct ScreenSize
        {
            int width;
            int height;
            int dpi; //not set yet
            double scale;

        } screenSize;
        std::string platform;

    public:

        void connectLog(bool *ptrValue, int index=-1,std::string logName="unknown");

        void load(std::ifstream& file);
        void save(std::ofstream& file) const;

        void addValue(std::string name, std::string val);

        double getTimeCoef() const { return timeCoef; }
        void setTimeCoef(double newTC) { timeCoef = newTC; }

        double getScaleCoef() const { return scaleCoef; }
        void setScaleCoef(double scale) { scaleCoef = scale; }

        void printValues() const;
        void checkConfig();

        void cleanValues() { values.clear(); }


    public:
            static AConfig& getInst() {
                static AConfig instance;
                return instance;
            }

    private:
            AConfig(): topIndex(-1),scaleCoef(1.0),timeCoef(1) {}
            AConfig(const AConfig& root) = delete;
            AConfig& operator=(const AConfig&) = delete;

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

        void setIntoConfig(AConfig& conf)
        {
            for (size_t i=0; i < configLines.size(); ++i)
                conf.addLine(configLines[i]);
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
            configLines.push_back("colors.background = green");
            configLines.push_back("colors.curBar = darkblue");
            configLines.push_back("colors.curBeat = yellow");
            configLines.push_back("colors.curString = yellow");
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


}


#endif // CONFIG_H
