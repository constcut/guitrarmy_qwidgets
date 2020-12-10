#include "aconfig.h"

#include <QImage>

#include <g0/astreaming.h>

static AStreaming logger("config");

AConfig *AConfig::inst;

AConfig::AConfig():topIndex(-1),scaleCoef(1.0),timeCoef(1)
{
}

void AConfig::connectLog(bool *ptrValue, int index,std::string logName)
{
    if (index==-1)
        index = topIndex+1;

    logs[index] = ptrValue;
    logsNames[index] = logName;

    if (index>topIndex)
        topIndex=index;
}



//log Name 1/0
//v Name 1290391
void AConfig::load(AFile &file)
{
    std::string lastLine = "firstOne";

    int count = 0;
    while (!lastLine.empty())
    {
        ++count;
        std::string line = file.readLine();

        if (line != lastLine)
        {
            addLine(line);
            lastLine = line;
        }
        else
            break;
        //hotfix
        if (line.substr(0,4)=="zend")
            break;
    }

    std::cout <<"Total count "<<count;


}

std::string repairString(std::string str)
{
    std::string response = str;
    //first 0
    while (response[0]==' ')
        response.erase(0,1);



    if (response[0]=='"')
    {
        int clIndex = 0;
        for (int i = 1; i < response.size(); ++i)
            if (response[i]=='"')
            {
                clIndex = i;
                break;
            }

        if (clIndex)
        response = response.substr(1,clIndex-1);
    }


    return response;
}

void AConfig::addLine(std::string anotherLine)
{
            //std::cout <<anotherLine.c_str()<<std::endl;
            size_t eqSign = anotherLine.find("=");

            if (eqSign != std::string::npos)
            {
                std::string left = anotherLine.substr(0,eqSign-1);
                std::string right = anotherLine.substr(eqSign+1);
                right = repairString(right);
                values[left]=right;
                std::cout <<left.c_str()<<"="<<right.c_str()<<std::endl;
            }
}

//should connect some values to be used
//first
void AConfig::save(AFile &file)
{
     //file.opened();
     for (std::map<std::string,std::string>::iterator it = values.begin();
          it!= values.end(); ++it)
     {
         stringExtended curLine;
         curLine << (*it).first.c_str() <<" = "<<(*it).second.c_str()<<"\n";
         file.write(curLine.c_str(),curLine.size());
     }
     file.close();
}

void AConfig::printValues()
{
    LOG(<<"Configuration parameters");
    for (std::map<std::string,std::string>::iterator it = values.begin();
         it!= values.end(); ++it)
    {
        LOG( << (*it).first.c_str() <<" = "<<(*it).second.c_str());
    }
    LOG(<<"Config printed");
}

 void AConfig::addValue(std::string name, std::string val)
 {
    std::map<std::string,std::string>::iterator it;
    it = values.find(name);
    if (it == values.end())
    {
        std::pair<std::string,std::string> newLine;
        newLine.first = name;
        newLine.second = val;
        values.insert(newLine);
       // std::map<std::string,std::string>:
    }

 }

void AConfig::checkConfig()
{
    addValue("TabView.genAMusic","g");
    addValue("TabView.genMidi","h");
    addValue("TrackView.nextBeat",">>>");
    addValue("TrackView.prevBeat","<<<");
    addValue("TrackView.nextBar","x");
    addValue("TrackView.prevBar","z");
    addValue("TrackView.stringUp","^^^");
    addValue("TrackView.stringDown","vvv");
    addValue("TrackView.playAMusic","spc");
    addValue("TrackView.playMidi","ent");

    addValue("TrackView.setPause","p");
    addValue("TrackView.deleteNote","del");
    addValue("TrackView.increaceDuration","q");
    addValue("TrackView.decreaceDuration","w");
    addValue("TrackView.save","s");
    addValue("TrackView.quickOpen","y");
    addValue("Main.open","open");
    addValue("colors.background","white");
    addValue("colors.panBG","gray");
    addValue("colors.curBar","black");
    addValue("colors.curBeat","gray");
    addValue("colors.curString","8899DD"); //hmm
    addValue("colors.default","gray");
    addValue("colors.exceed","darkred");
    addValue("colors.curTrack","blue");
    addValue("colors.selection","darkgreen");

    addValue("colors.combo.background","gray");
    addValue("colors.combo.pressed","darkgray");
    addValue("colors.combo.focus","white");
    addValue("colors.combo.hover","darkblue");

    addValue("colors.button.background","gray");
    addValue("colors.button.pressed","darkgray");
    addValue("colors.button.focus","white");
    addValue("colors.button.hover","darkblue");

    addValue("colors.button.bgspecial","darkgray");




    addValue("effects.vibrato","vib");
    addValue("effects.slide","sli");
    addValue("effects.hammer","ham");
    addValue("effects.letring","l.r.");
    addValue("effects.palmmute","p.m.");
    addValue("effects.harmonics","harm");
    addValue("effects.trill","trill");
    addValue("effects.tremolo","trem");
    addValue("effects.stokatto","stok");
    addValue("effects.fadein","f In");
    addValue("effects.accent","acc");
    addValue("TrackView.alwaysShowBarSign","0");

    addValue("gestures","classic");

    addValue("TrackView.largeNotes","1");

    addValue("fullscreen","1");

    //bottom
    addValue("toolBar","0");   //top bottom right left 0
    addValue("mainMenu","0");    //0 or else
    addValue("pannels","oneline");//tabbed docked classic(cutten yet) oneline

    addValue("invertImages","0");

    addValue("statusBar","1");
    addValue("metronome","0");

    addValue("crashOnPlayHotFix","1"); //to remember the leak
    //if not found sepparae buttons

    addValue("logs.tab","0");
    addValue("logs.midi","0");
    addValue("logs.abstractmusic","0");
    addValue("logs.ownformat","0");
    addValue("logs.othersformats","0");

    addValue("images","1");

    addValue ("crashAutoSend","0");
    addValue ("addRootCrashes","1");
    addValue ("sdcardLogDebug","0");//debug

    addValue("iconsSet","1");
    addValue("turnPinchZoomOn","0");

    addValue("tapAndHoldCopy","0");

    addValue("skipTabView","1");

    addValue("showStatusViewHint","0");

    addValue("upsideDownNotes","1");
    addValue("showNotesView","0");

    addValue("mergeMidiTracks","0");

    addValue("zend","ofconfig");

    addValue("onloadBaseCheck","1");
}


 void ImagePreloader::loadImage(std::string imageName)
 {

    std::string fullImageName = "";


    //std::string(getTestsLocation())
           // + std::string("Icons/") + imageName +std::string(".png");

    std::string iconsSet;
     if (CONF_PARAM("iconsSet")=="1")
         iconsSet = ":/icons/";
     else
         iconsSet = ":/icons2/";

    fullImageName = iconsSet + imageName +std::string(".png");

    QFile imageFile;
    imageFile.setFileName(fullImageName.c_str());

    if (imageFile.open(QIODevice::ReadOnly) == false)
        std::cout << "Failed to open";


    QImage *img = new QImage(); //(fullImageName.c_str());

    img->load(&imageFile,0);

    if (img->height() < 72)
    {
       QImage *oldImage = img; //leak
       *img = img->scaled(72,72);
    }

    if (inv)
        if (img)
            img->invertPixels();

    imageMap[imageName] = img;
 }

 void ImagePreloader::loadImages()
 {
     loadImage("prevBeat"); loadImage("nextBeat"); loadImage("upString"); loadImage("downString");
     loadImage("prevBar"); loadImage("nextBar"); loadImage("play"); loadImage("save"); loadImage("open");

     loadImage("openPannel"); loadImage("info"); loadImage("morze");

     //loadImage("mHello"); loadImage("mHello2"); loadImage("mHello3");

     loadImage("config"); loadImage("tap"); loadImage("record");
     loadImage("new"); loadImage("pattern"); loadImage("tests");

     loadImage("tab"); loadImage("backview");

     loadImage("bg"); loadImage("qp"); loadImage("qm");
     loadImage("p"); loadImage("del"); loadImage("ins");
     loadImage("."); loadImage("-3-"); loadImage("leeg");
     loadImage("x");

     //NOW about TabView

     loadImage("goToN"); loadImage("V"); loadImage("^");
     loadImage("44"); loadImage("alt"); loadImage("bpm");
     loadImage("deleteTrack"); loadImage("newTrack");
     loadImage("drums"); loadImage("mute");
     loadImage("instr"); loadImage("marker");
     loadImage("name"); loadImage("solo");
     loadImage("next"); loadImage("prev");
     loadImage("pan"); loadImage("repBegin"); loadImage("repEnd");
     loadImage("track"); loadImage("tune");
     loadImage("volume");

     //Effects

     loadImage("acc"); loadImage("bend"); loadImage("changes");
     loadImage("chord"); loadImage("downm"); loadImage("downstroke");
     loadImage("fadeIn"); loadImage("fing"); loadImage("hacc");
     loadImage("harm"); loadImage("lr"); loadImage("pm");
     loadImage("pop"); loadImage("slap"); loadImage("tapp");
     loadImage("sli"); loadImage("stok");
     loadImage("trem"); loadImage("txt"); loadImage("upm");
     loadImage("upstroke"); loadImage("vib");

     //Clipboard
     loadImage("clip"); loadImage("copy");
     loadImage("copyBeat"); loadImage("copyBars");
     loadImage("cut"); loadImage("paste");
     loadImage("clip1");
     loadImage("clip2");
     loadImage("clip3");

     //Few missing
     loadImage("back"); loadImage("eff");
     loadImage("nextTrack"); loadImage("prevTrack");
     loadImage("newBar"); loadImage("undo");
     loadImage("prevPage"); loadImage("nextPage");
     loadImage("zoomIn"); loadImage("zoomOut");

     loadImage("trill"); loadImage("ham");


     for (int i = 0; i < 10; ++i)
     {
         stringExtended sX;
         sX<<i; loadImage(sX.c_str());
     }
 }

 void *ImagePreloader::getImage(std::string imageName)
 {
     if (imageMap.find(imageName)!=imageMap.end())
     {
         return imageMap[imageName];
     }
     return 0;
 }

void ImagePreloader::invertAll()
{
     for (std::map<std::string,void *>::iterator it = imageMap.begin();
          it != imageMap.end(); ++it)
     {
        QImage *img = (QImage*)it->second;
        img->invertPixels();
     }
}

