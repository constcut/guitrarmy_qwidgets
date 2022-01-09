#include "ImagePreloader.hpp"

#include <QImage>
#include <QFile>
#include <QDebug>
#include <iostream>

#include "g0/Config.hpp"

void ImagePreloader::loadImage(std::string imageName)
{

   std::string fullImageName = "";


   //std::string(AConfig::getInstance().testsLocation)
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
       std::cerr << "Failed to open image file: " << imageName << std::endl;


   auto img = std::make_unique<QImage>(); //(fullImageName.c_str());

   img->load(&imageFile,0);

   if (img->height() < 72)
      *img = img->scaled(72,72);

   if (inv)
       if (img)
           img->invertPixels();

   imageMap[imageName] = std::move(img);
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


    for (size_t i = 0; i < 10; ++i)
    {
        std::string sX = std::to_string(i);
        loadImage(sX.c_str());
    }
}

QImage* ImagePreloader::getImage(std::string imageName)
{
    if (imageMap.count(imageName))
        return imageMap[imageName].get();
    return 0;
}

void ImagePreloader::invertAll()
{
    for (auto it = imageMap.begin(); it != imageMap.end(); ++it)
       it->second->invertPixels();
}
