#include "mainviews.h"

#include <QFileDialog>
#include <QMessageBox>

#include "g0/tab.h"
#include "g0/gtpfiles.h"
#include "g0/afile.h"
#include "g0/amusic.h"
#include "g0/midifile.h"

#include "g0/aconfig.h"
#include "g0/astreaming.h"
#include "g0/gmyfile.h"
#include "g0/aexpimp.h" //loader

#include "tabviews.h"





//#include <loguploader.h>
#include "qthelper.h"
//MUST NOT BE HERE
#include "mainwindow.h"

BendPoints* BendInput::ptrToBend = 0;
Note *BendInput::ptrToNote = 0;

Beat *ChangesInput::ptrToBeat = 0;

void BendInput::setPtrBend(void *ptr)
{
    ptrToBend = (BendPoints*)ptr;
}
void BendInput::setPtrNote(void *ptr)
{
    ptrToNote = (Note*)ptr;
}

void ChangesInput::setPtrBeat(Beat *beatPtr)
{
    ptrToBeat = beatPtr;
}


static AStreaming logger("mainviews");

void MainView::setTabLoaded(Tab *tab)
{
    changeCurrentView( tabsView );

    tabsView->setTab(tab);
    //getMaster()->changeChild(tabsView);
}

bool MainView::isPlaying()
{
    return tabsView->getPlaying();
}

void MainView::onclick(int x1, int y1)
{
    //tool bar now
    //if (pan->hit(x1,y1))
    //    pan->onclick(x1,y1); //

    //else
    if (currentView)
        currentView->onclick(x1,y1);
}

void MainView::ondblclick(int x1, int y1)
{
    //if (pan->hit(x1,y1))
       // pan->onclick(x1,y1);
    //else
    if (currentView)
        currentView->ondblclick(x1,y1);
}

void MainView::keyevent(std::string press)
{
    if (press=="welcome")
    {
        changeCurrentView(welcome);
        return;
    }
    if (press=="bend_view")
    {
        changeCurrentView(bendInp);
        return;
    }
    if (press=="chord_view")
    {
        changeCurrentView(chordInp);
        return;
    }
    if (press=="change_view")
    {
        changeCurrentView(chanInp);
        return;
    }



        if (currentView)
            currentView->keyevent(press);

        if (press=="openPannel")
            if (currentView)
            {
                GPannel *viewPannel =0; //(GPannel*)currentView->getPannel();

                //no more
                if (viewPannel)
                {
                    if (viewPannel->isOpenned())
                        viewPannel->close();
                    else
                        viewPannel->preOpen();
                }
            }



        /* //code for debuggig
        if ((press == "q")||(press == "w")||(press =="e"))
        {
            return; //skip this commands for release
            std::string fileName = getTestsLocation();

            if (press == "q")
                fileName += "g4/3.52cut.gp4"; //3.4

            if (press == "w")
                fileName  +=  "g4/3.29cut_.gp4";

            if (press == "e")
                fileName += "g4/3.58cut.gp4";


            AFile file;
            file.open(fileName);

            Tab *forLoad=new Tab();
            Gp4Import importer;
            importer.import(file,*forLoad);


            forLoad->postGTP();
            forLoad->connectTracks();
            log << "file v 4 was opened"<<fileName.c_str();


            changeCurrentView(tabsView);

            tabsView->setTab(forLoad);   
            //getMaster()->changeChild(tabsView);
        }
        */

        if (press == "esc")
        {
            changeViewToLast();
        }

        if (press == "rec")
        {
            changeCurrentView(recordView);
        }

        if ((press == CONF_PARAM("TrackView.quickOpen"))||(press=="quickopen"))
        {

            if (tabsView->gotChanges()==false)
            {

                //load guitarmy file
                GmyFile gmyFile; //fkldsjfkldsj

                AFile file; //itfile);
                stringExtended gfileName;
                gfileName <<  getTestsLocation() <<"first.gmy";

                if (QFile::exists(gfileName.c_str()))
                {
                    file.open(gfileName.c_str());//first.gmy");

                    Tab *newTab = new Tab();
                    gmyFile.loadFromFile(&file,newTab);


                    newTab->connectTracks();

                    changeCurrentView(tabsView);
                    tabsView->setTab(newTab);
                    if (CONF_PARAM("skipTabView")=="1")
                    tabsView->keyevent("opentrack");
                }

            }
            //getMaster()->changeChild(tabsView);

            //otfile.close();
        }
        if (press == CONF_PARAM("Main.open"))
        {

            if (tabsView->gotChanges()==false)
            {
                QFileDialog *fd = new QFileDialog;


                fd->setStyleSheet("QScrollBar:horizontal {\
                                  border: 2px solid grey;\
                                  background: #32CC99;\
                                  height: 15px;\
                                  margin: 0px 20px 0 20px;\
                              }\
                              QLineEdit { height: 20px; \
                              }");

                fd->setViewMode(QFileDialog::List);


#ifdef __ANDROID_API__
                fd->setDirectory("/sdcard/");
                QScreen *screen = QApplication::screens().at(0);
                fd->setGeometry(0,0,screen->geometry().width(),screen->geometry().height());
#endif

                fd->setNameFilter("Tab files (*.g*);;All (*.*)");

                QString s;
                if (fd->exec())
                s = fd->selectedFiles().at(0);

                        //fd->getOpenFileName(0, QString::fromLocal8Bit("Открыть"),
                         //            getTestsLocation(),
                                  //   "Gx files (*.g*)");

                delete fd;

                clock_t afterT = getTime();

                GTabLoader tabLoader;
                if  (tabLoader.open(s.toStdString()))
                {
                    changeCurrentView(tabsView);
                    tabsView->setTab(tabLoader.getTab());
                    if (CONF_PARAM("skipTabView")=="1")
                    tabsView->keyevent("opentrack");

                    //getMaster()->changeChild(tabsView);

                    clock_t after2T = getTime();
                    clock_t diffT = after2T - afterT;

                    LOG( <<"Opening gtp "<<diffT);
                }
            }
        }



        if (press == "tests")
        {
            //getMaster()->changeChild(testsView);
             changeCurrentView(testsView);
        }

        if ((press == "config")) //sepparate to log view
        {
                //getMaster()->changeChild(configView);
                 changeCurrentView(configView);
        }

        if (press == "tap")
        {
            //tap Ry
             //getMaster()->changeChild(tapRyView);
             tapRyView->measureTime();
             changeCurrentView(tapRyView);
        }

        if (press == "pattern")
        {
            //pattern input
            //getMaster()->changeChild(patternInp);
             changeCurrentView(patternInp);
        }

        if (press == "tabview")
        {
             changeCurrentView(tabsView);
             //tabsView->keyevent("track");
        }

        if (press == "trackview")
        {
            changeCurrentView(tabsView->tracksView[tabsView->getLastOpenedTrack()]);
        }

        if (press == "info")
        {
            changeCurrentView(infView);
        }

        if (press == "morze")
        {
            changeCurrentView(morzeInp);
        }

        if (press == "newtab")
        {
            if (tabsView->gotChanges()==false)
            {
                //NEW tab

                Tab *newTab = new Tab();
                newTab->setBPM(120);

                //newTab->connectTracks();
                Track *track=new Track();
                std::string iName("NewInstrument");
                track->setName(iName);
                track->setInstrument(25);
                track->setVolume(15);
                track->setDrums(false);
                track->setPan(8); //center now
                track->tuning.setStringsAmount(6);

                track->tuning.setTune(0,64);
                track->tuning.setTune(1,59);
                track->tuning.setTune(2,55);
                track->tuning.setTune(3,50);
                track->tuning.setTune(4,45);
                track->tuning.setTune(5,40);


                Bar *bar=new Bar();
                bar->flush();
                bar->setSignDenum(4); bar->setSignNum(4);
                bar->setRepeat(0);


                for (int iB=0; iB <4; ++iB)
                {
                    Beat *beat=new Beat();
                    beat->setPause(true);
                    beat->setDotted(0);
                    beat->setDuration(3);
                    beat->setDurationDetail(0);

                    bar->add(beat);
                }

                track->add(bar);
                newTab->add(track);
                newTab->connectTracks();

                changeCurrentView(tabsView);
                tabsView->setTab(newTab);
                if (CONF_PARAM("skipTabView")=="1")
                tabsView->keyevent("opentrack");
                //getMaster()->changeChild(tabsView);

            }

        }
}

void MainView::draw(Painter *painter)
{
    int wi = getMaster()->getWidth();
    int hi = getMaster()->getHeight();

    //pan->setW(wi);
    //pan->resetButtons();
    //pan->draw(painter);

    if (currentView)
    {
        GPannel *viewPannel = 0; //(GPannel *)currentView->getPannel();

        if (viewPannel)
        {
            viewPannel->setH(hi);
            viewPannel->setW(wi);
            viewPannel->setY(hi-180);
            viewPannel->resetButtons();
        }

        currentView->setW(wi);
        currentView->setH(hi);

        if (currentView->getMaster() == 0)
            currentView->setMaster(getMaster());

        currentView->draw(painter);
    }

}



void MainView::setMaster(MasterView *mast)
{
    GView::setMaster(mast);
    if (testsView) testsView->setMaster(mast);
    if (tabsView) tabsView->setMaster(mast);
    if (configView) configView->setMaster(mast);
    if (tapRyView) tapRyView->setMaster(mast);
    if (patternInp) patternInp->setMaster(mast);
    //log <<"Main view set master called";
}

MainView::MainView():GView(0,0,800,480), currentView(0)
{
    slave=false;
    tabsView = new TabView();
    configView = new ConfigView();
    tapRyView = new TapRyView();
    testsView = new TestsView(this, tabsView);
    patternInp = new PatternInput();

    infView = new InfoView();
    morzeInp = new MorzeInput();
    recordView = new RecordView();

    welcome = new WelcomeView();

    bendInp= new BendInput();
    chanInp = new ChangesInput();
    chordInp = new ChordInput();

    /*
    chBut = new GCheckButton(500,200,30,30);
    labA = new GLabel(100,100,"open'o'","o");
    labB = new GLabel(100,150,"config 'c'","c");
    labC = new GLabel(100,200,"tests 't'","t");
    labD = new GLabel(100,250,"new 'n'","n");
    labE = new GLabel(100,300,"tap rhythm 'r'","r");
    labF = new GLabel(100,350,"pattern input 'i'","i");
    labVersion = new GLabel(100,400,getGuitarmyVersion());
    */

    //set up
    currentView  =  welcome; //testsView;

    pan = new GStickPannel(0,60,800);
    pan->setPressView(this);
    pan->setNoOpenButton();

}

ConfigView::ConfigView():GView()
{
    AConfig *confValues = new AConfig();
    connectConfigs(confValues);

    configPointer = confValues;

    stringExtended sP;
    sP<<confValues->logsNames[0]<<" "<<(int)*(confValues->logs[0])<<" press 1 to chng";
    labA = new GLabel(20,80+200-55,sP.c_str());

    sP.clear();
    sP<<confValues->logsNames[1]<<" "<<(int)*(confValues->logs[1])<<" press 2 to chng";

    labB = new GLabel(20,120+200-55,sP.c_str());

    sP.clear();
    sP<<confValues->logsNames[2]<<" "<<(int)*(confValues->logs[2])<<" press 3 to chng";

    labC = new GLabel(20,160+200-55,sP.c_str());

    sP.clear();
    sP<<confValues->logsNames[3]<<" "<<(int)*(confValues->logs[3])<<" press 4 to chng";

    labD = new GLabel(20,200+200-55,sP.c_str());


    //labScaleMinus = new GLabel(20,80-55,"zoomOut");

    //labScalePlus = new GLabel(20,150-55,"zoomIn");

}

void ConfigView::keyevent(std::string press)
{
    //if (press == "m")
        //getMaster()->resetToFirstChild();

    AConfig *confValues = this->configPointer;

    if (press == "1")
    {
      bool *x = this->configPointer->logs[0];
      bool inv = !(*x);
      *x = inv;

      stringExtended sP;
      sP<<confValues->logsNames[0]<<" "<<(int)*(confValues->logs[0])<<" press 1 to chng";

      labA->setText(sP.c_str());
    }
    if (press == "2")
    {
      bool *x = this->configPointer->logs[1];
      bool inv = !(*x);
      *x = inv;

      stringExtended sP;
      sP<<confValues->logsNames[1]<<" "
         <<(int)*(confValues->logs[1])<<" press 2 to chng";

      labB->setText(sP.c_str());
    }
    if (press == "3")
    {
      bool *x = this->configPointer->logs[2];
      bool inv = !(*x);
      *x = inv;

      stringExtended sP;
      sP<<confValues->logsNames[2]<<" "
         <<(int)*(confValues->logs[2])<<" press 3 to chng";

      labC->setText(sP.c_str());
    }
    if (press == "4")
    {
      bool *x = this->configPointer->logs[3];
      bool inv = !(*x);
      *x = inv;

      stringExtended sP;
      sP<<confValues->logsNames[3]<<" "
         <<(int)*(confValues->logs[3])<<" press 4 to chng";

      labD->setText(sP.c_str());
    }
}

void ConfigView::onclick(int x1, int y1)
{
    //if (labA->hit(x1,y1)) keyevent("1");
    //if (labB->hit(x1,y1)) keyevent("2");
    //if (labC->hit(x1,y1)) keyevent("3");
    //if (labD->hit(x1,y1)) keyevent("4");

    /*
    if (labScaleMinus->hit(x1,y1))
    {
        double currentScale = AConfig::getInstance()->getScaleCoef();

        if (currentScale >= 0.5)
        {
            currentScale -= 0.25;
            AConfig::getInstance()->setScaleCoef(currentScale);
        }
    }

    if (labScalePlus->hit(x1,y1))
    {
        double currentScale = AConfig::getInstance()->getScaleCoef();

        if (currentScale <= 4.0)
        {
            currentScale += 0.25;
            AConfig::getInstance()->setScaleCoef(currentScale);
        }
    }
    */
}


void ConfigView::draw(Painter *painter)
{
    //labA->draw(painter);
    //labB->draw(painter);
    //labC->draw(painter);
    //labD->draw(painter);

    //labScaleMinus->draw(painter);
    //labScalePlus->draw(painter);

    int i = 0;

    int limitY = getMaster()->getHeight();

    int curX = 200;
    int curY = 75;

    /*
    for (std::map<std::string,std::string>::iterator itV=AConfig::getInstance()->values.begin();
         itV!=AConfig::getInstance()->values.end(); ++itV, ++i)
    {
            stringExtended line;
            line << itV->first.c_str()<<"="<<itV->second.c_str();
            curY = (i+1)*15 + 60;

            if (curY>=(limitY-50))
            {
                curX += 250;
                i=0;
                curY = (i+1)*15 + 60;
                if (curX>700) break;
            }

            painter->drawText(curX,curY,line.c_str());
    }
    */
}



///tests buttons

void TestsView::fastTestAll()
{
    clock_t now = getTime();
    bottom.setText("starting full tests");


    //NORMAL CASE TEST ALL
    /*
    for (size_t i = 0; i < buttons.size(); ++i)
    {
        stringExtended sX;
        sX << ":/own_tests/"<<buttons[i].getText().c_str()<<".gp4";

        QFile qFile; //(sX.c_str());
        qFile.setFileName(sX.c_str());

        if (qFile.open(QIODevice::ReadOnly) == false)
            std::cout << "Failed to open";

        AFile file(&qFile);

        Tab *forLoad=new Tab();
        Gp4Import importer;
        importer.import(file,forLoad);

        forLoad->postGTP();
        forLoad->connectTracks();
        logger << "file v 4 was opened"<<sX.c_str();

        MidiFile midiCheck;
        midiCheck.fromTab(forLoad);
        AMusic amCheck;
        amCheck.readFromTab(forLoad);
        MidiFile amMidiCheck;
        amMidiCheck.generateFromAMusic(amCheck); //to ptr later

        logger<<"full scen done";
        delete forLoad;
    }
    */


    for (int i = 1217; i < 9570; ++i)//5077 142 error
    {
        int curTest = i;

        stringExtended sX;
        sX << getTestsLocation() << "ptb/"<<curTest;

        Tab *forLoad=0;

        GTabLoader loader;
        if (loader.open(sX.c_str()) == false)
        {
            LOG( << "Failed to open test "<<curTest);
            continue;
           //return;
        }

        LOG( << "Done test "<<i);

        forLoad = loader.getTab();
        //forLoad->setBPM(120);

        //delete forLoad;



        MainView *mainView = (MainView*)getMaster()->getFirstChild();
        mainView->changeCurrentView(tabsView);
        tabsView->setTab(forLoad);
        if (CONF_PARAM("skipTabView")=="1")
        tabsView->keyevent("opentrack");




    }


    clock_t after = getTime();
    clock_t diff = after-now;

    stringExtended mesure;
    mesure <<"Time spent  - "<<diff<<" ms";;
    bottom.setText(mesure.c_str());
}


void TestsView::openTestNumber(int num)
{
    if (tabsView->gotChanges()==false)
    {

         stringExtended sX;
         //sX << getTestsLocation()<<"g4/"<<buttons[i].getText().c_str()<<".gp4";

         sX << ":/own_tests/"<<buttons[num].getText().c_str()<<".gp4";

         LOG( << "Opening test "<<buttons[num].getText().c_str());

         QFile qFile; //(sX.c_str());
         qFile.setFileName(sX.c_str());

         if (qFile.open(QIODevice::ReadOnly) == false)
             std::cout << "Failed to open";

         AFile file(&qFile);


         //AFile file;
         //file.open(sX.c_str());


         Tab *forLoad=new Tab();
         Gp4Import importer;
         importer.import(file,forLoad);

         forLoad->postGTP();
         forLoad->connectTracks();
         LOG( << "file v 4 was opened"<<sX.c_str());

        MainView *mainView = (MainView*)getMaster()->getFirstChild();
        mainView->changeCurrentView(tabsView);
        tabsView->setTab(forLoad);
        if (CONF_PARAM("skipTabView")=="1")
        tabsView->keyevent("opentrack");
    }

}

void TestsView::onclick(int x1, int y1)
{
    for (size_t i = 0; i < buttons.size(); ++i)
        if (buttons[i].hit(x1,y1))
        {
            openTestNumber(i);
            return;
        }

    if (stopPlaylist->hit(x1,y1))
    {
        getMaster()->cleanPlayList();
    }

    if (playlistButton->hit(x1,y1))
    {
        getMaster()->pushForceKey(playlistButton->getPressSyn());
    }

    if (playlist2Button->hit(x1,y1))
    {
        getMaster()->pushForceKey(playlist2Button->getPressSyn());
    }
}

//MORZE

//ABOUT QT PLATFOR DEP
void InfoView::onclick(int x1, int y1)
{
    if (labelB->hit(x1,y1))
        QMessageBox::aboutQt(0,"About Qt");

    if (labelA->hit(x1,y1))
    {

    }

    if (help->hit(x1,y1))
    {
        getMaster()->showHelp();

        help->setVisible(false);
        logShow->setVisible(false);

        sendCrash->setVisible(false);

        labelA->setVisible(false);
        labelA2->setVisible(false);
        icons->setVisible(false);

        labelB->setVisible(false);
        labelC->setVisible(false);
        labelD->setVisible(false);
        labelE->setVisible(false);
        return;
    }

    if (logShow->hit(x1,y1))
    {


        QString crashName = getTestsLocation() + QString("crashs.glog");
        if (QFile::exists(crashName))
        {
            QFile logData;
            logData.setFileName(crashName);
            logData.open(QIODevice::ReadOnly);
            QByteArray allLogData = logData.readAll();

            stringExtended lines[7];
            lines[0] << "File exists";
            lines[1] << "Full size " << allLogData.size();
            lines[2] << "Could be sent in ~ "<< allLogData.size()/1000<<" iterations";

            labelA->setText(lines[0].c_str());
            labelA2->setText(lines[1].c_str());
            icons->setText(lines[2].c_str());

            labelB->setText("");
            labelC->setText("");
            labelD->setText("");
            labelE->setText("");

            logData.close();
        }
        else
        {
            stringExtended lines[7];
            lines[0] << "File doesn't exists - no crashes saved";
            labelA->setText(lines[0].c_str());
        }


       return;
    }

    if (sendCrash->hit(x1,y1))
    {
        stringExtended requestLine;

        std::string sendData = "crashlog";

        QString crashName = getTestsLocation() + QString("crashs.glog");
        if (QFile::exists(crashName))
        {
            QFile logData;
            logData.setFileName(crashName);
            logData.open(QIODevice::ReadOnly);
            QByteArray allLogData = logData.readAll();

            QByteArray newSequence;

            if (allLogData.size() > 1000)
            {
                //cycle
                // 1:cut part
                // 2:send part
                //left last unsent
            }
            else
            {
                newSequence = alphaCut(allLogData);
                sendData = newSequence.toStdString();

            }
            logData.close();
        }

        std::string userIdStd = getUserId();
        QDateTime timeDate(QDateTime::currentDateTime());
        QString st = timeDate.toString(Qt::ISODate);
        st.replace(":","");
        std::string time = st.toStdString();


        requestLine<< "http://guitarmy.in/glogs/crash.php?log="
                    <<sendData<<"&date="<<time<<"&user="<<userIdStd;


        HttpFileLoader *logUp= new HttpFileLoader;
        logUp->request(QUrl(requestLine.c_str()));



         std::cout << requestLine.c_str() << std::endl;
         std::cout << "String full size "<<requestLine.size()<<std::endl;

        return;
    }

    if (help->isVisible()==false)
    {
        help->setVisible(true);
    }

    if (logShow->isVisible()==false)
    {
        logShow->setVisible(true);
    }

    if (sendCrash->isVisible()==false)
    {
        sendCrash->setVisible(true);
    }

}

//Bend View

void BendInput::draw(Painter *painter)
{
    if (ptrToBend)
        top->setText("Bend is set!");
    else
        top->setText("Bend not set!");

    top->draw(painter);
    okButton->draw(painter);
    delButton->draw(painter);

    if (ptrToBend)
    {
        painter->drawRect(20,80,480,240);

        for (int i = 0; i < 12; ++i)
            painter->drawLine(20,80+i*20,500,80+i*20);

        for (int i = 0; i < 60; ++i)
            painter->drawLine(20+i*8,80,20+i*8,320);

        //8 px for horizon step, 20 for vertical
        int prevX = 0; int prevY = 0;

        painter->changeColor("red");
        for (ul i = 0; i < ptrToBend->len(); ++i)
        {
            byte vertical = ptrToBend->getV(i).vertical;
            byte horizontal = ptrToBend->getV(i).horizontal;
            //logger << i << ") v= "<<vertical<<"; hor = "<<horizontal;

            int localY = vertical*-20; localY += 80+240;
            int localX = horizontal*8; localX += 20; //second op - shift in rect

            if (i) //skip first
                painter->drawLine(prevX,prevY,localX,localY);

            prevX = localX; prevY = localY;
            painter->drawEllipse(localX-3,localY-3,6,6,"red");
        }
        //logger << "Bend outpited";
    }
}

void BendInput::keyevent(std::string press)
{
    //reaction on combo - setting default bends
    ///getComboBoxValue(index);
    if (press.substr(0,4)=="com:")
    {
        std::string rest = press.substr(4);
        size_t separator = rest.find(":");
        std::string combo = rest.substr(0,separator);
        std::string item = rest.substr(separator+1);

        int itemNum = atoi(item.c_str());

        int bendType=-1;
        int bendHeight=-1;

        if (combo=="0")
        {
            bendType = itemNum;
            bendHeight = 12-getMaster()->getComboBoxValue(1);
        }
        if (combo=="1")
        {
            bendType = getMaster()->getComboBoxValue(0);
            bendHeight = 12-itemNum;
        }

        if (bendType!=-1 && bendHeight!=-1)
        {
            LOG( << "Bend c type "<<bendType<<"; h "<<bendHeight);
            fillBend(bendType,bendHeight);
        }
        logger << "comBend";
    }

    //also must have back button that will set bend value into note (instaed of just escape it)
}

void BendInput::onclick(int x1, int y1)
{
    if (okButton->hit(x1,y1))
    {
         if (ptrToNote)
         {
             if (ptrToBend)
             {
                ptrToNote->effPack.set(17,true);
                ptrToNote->effPack.addPack(17,2,ptrToBend);
             }
             getMaster()->pushForceKey("esc"); //go prev
             //better have another function atention
         }
    }
    if (delButton->hit(x1,y1))
    {
        if (ptrToNote)
        {
            bool whatIsThere = ptrToNote->effPack.get(17);
            LOG( << "Deleting bend that is "<<(int)whatIsThere);

            ptrToNote->effPack.addPack(17,2,0);
            ptrToNote->effPack.set(17,false);
            //delete ptrToBend;
            ptrToBend = 0;
        }
    }
    //shoud react on press and calculate
    ///vertical and horizontal
    //to print it in the top value

    int localX = x1-20;
    int localY = y1-80; localY = 240 - localY;

    LOG( << "Locals "<<localX<<"; "<<localY);

    //ATTENTION please note that there are yen no chanes

    if ((localX < 480) & (localY < 240))
    {
        byte vertical = localY/20;
        byte horizontal = localX/8;
        LOG( << "hit "<<vertical<<"; "<<horizontal);

        if (ptrToBend)
        {
            for (ul i = 0; i < ptrToBend->len(); ++i)
            {
                int pointV= ptrToBend->getV(i).vertical;
                int pointH= ptrToBend->getV(i).horizontal;

                //diff + - 1 ?

                if ((pointV==vertical) && (pointH==horizontal))
                {
                    logger << "point really hitten";
                    ptrToBend->remove(i);
                    return;
                }
            }
            BendPoint newPoint;
            newPoint.vertical = vertical;
            newPoint.horizontal = horizontal;
            newPoint.vFlag = 0;
            ptrToBend->insertNewPoint(newPoint);
        }
    }


    //1: does in hit the inner rect?
    //2: shifts from the border - devide them - vert + hor we have
}

void BendInput::fillBend(int type, int height)
{
    if (ptrToBend==0)
        ptrToBend = new BendPoints; //simple escape then could be reason of leak
    //first only type==0 (normal bend)
    if (type==0)
    {
        ptrToBend->clear();
        BendPoint p0; p0.vertical = 0; p0.horizontal = 0; p0.vFlag=0;
        BendPoint p1; p1.vertical = height; p1.horizontal = 15; p1.vFlag=0;
        BendPoint p2; p2.vertical = height; p2.horizontal = 60; p2.vFlag=0;
        ptrToBend->add(p0); ptrToBend->add(p1); ptrToBend->add(p2);
    }
    if (type==1)
    {
        ptrToBend->clear();
        BendPoint p0; p0.vertical = 0; p0.horizontal = 0; p0.vFlag=0;
        BendPoint p1; p1.vertical = height; p1.horizontal = 10; p1.vFlag=0;
        BendPoint p2; p2.vertical = height; p2.horizontal = 20; p2.vFlag=0;
        BendPoint p3; p3.vertical = 0; p3.horizontal = 30; p3.vFlag=0;
        BendPoint p4; p4.vertical = 0; p4.horizontal = 60; p4.vFlag=0;
        ptrToBend->add(p0); ptrToBend->add(p1); ptrToBend->add(p2); ptrToBend->add(p3); ptrToBend->add(p4);
    }
    if (type==2)
    {
        ptrToBend->clear();
        BendPoint p0; p0.vertical = 0; p0.horizontal = 0; p0.vFlag=0;
        BendPoint p1; p1.vertical = height; p1.horizontal = 10; p1.vFlag=0;
        BendPoint p2; p2.vertical = height; p2.horizontal = 20; p2.vFlag=0;
        BendPoint p3; p3.vertical = 0; p3.horizontal = 30; p3.vFlag=0;
        BendPoint p4; p4.vertical = 0; p4.horizontal = 40; p4.vFlag=0;
        BendPoint p5; p5.vertical = height; p5.horizontal = 50; p5.vFlag=0;
        BendPoint p6; p6.vertical = height; p6.horizontal = 60; p6.vFlag=0;
        ptrToBend->add(p0); ptrToBend->add(p1); ptrToBend->add(p2); ptrToBend->add(p3);
        ptrToBend->add(p4); ptrToBend->add(p5); ptrToBend->add(p6);
    }
    if (type==3)
    {
        ptrToBend->clear();
        BendPoint p0; p0.vertical = height; p0.horizontal = 0; p0.vFlag=0;
        BendPoint p1; p1.vertical = height; p1.horizontal = 60; p1.vFlag=0;
        ptrToBend->add(p0); ptrToBend->add(p1);
    }
    if (type==4)
    {
        ptrToBend->clear();
        BendPoint p0; p0.vertical = height; p0.horizontal = 0; p0.vFlag=0;
        BendPoint p1; p1.vertical = height; p1.horizontal = 15; p1.vFlag=0;
        BendPoint p2; p2.vertical = 0; p2.horizontal = 30; p2.vFlag=0;
        BendPoint p3; p3.vertical = 0; p3.horizontal = 60; p3.vFlag=0;
        ptrToBend->add(p0); ptrToBend->add(p1); ptrToBend->add(p2);  ptrToBend->add(p3);
    }
}

//CHANGES

void ChangesInput::draw(Painter *painter)
{
    top->draw(painter);

    if (ptrToBeat)
    {
        painter->drawText(70,30,"instruments change:");
        painter->drawText(70,80,"bpm change:");
        painter->drawText(70,130,"volume change:");
        painter->drawText(70,180,"pan change:");


        top->setText("beat ptr was set");
        if (ptrToBeat->effPack.get(28)==true)
        {
            Package *changePack = ptrToBeat->effPack.getPack(28);
            if (changePack)
            {
                Beat::ChangesList *changes = changePack->getPointer();
                if (changes)
                {
                    getMaster()->setComboBox(5,"changecombo4",550,70,50,40,0);
                    getMaster()->setComboBox(2,"changecombo4",550,20,50,40,0);
                    getMaster()->setComboBox(8,"changecombo4",550,120,50,40,0);
                    getMaster()->setComboBox(11,"changecombo4",550,170,50,40,0);


                    for (ul i = 0; i < changes->len(); ++i)
                    {
                        byte changeType = changes->getV(i).changeType;
                        ul changeValue = changes->getV(i).changeValue;
                        byte changeCount = changes->getV(i).changeCount;

                        stringExtended sX;
                        sX<<"Type "<<changeType<<"; Value "<<changeValue<<"; count "<<changeCount;
                        painter->drawText(50,250+25*i,sX.c_str());

                        if (changeType==8)
                        {
                            getMaster()->setComboBox(3,"bpm",220,70,150,40,changeValue);
                            getMaster()->setComboBox(4,"changecombo3",390,70,150,40,-1);
                            getMaster()->setComboBox(5,"changecombo4",550,70,50,40,1);
                        }

                        if (changeType==1)
                        {
                            getMaster()->setComboBox(0,"instruments",220,20,150,40,changeValue);
                            getMaster()->setComboBox(1,"changecombo3",390,20,150,40,-1);
                            getMaster()->setComboBox(2,"changecombo4",550,20,50,40,1);
                        }

                        if (changeType==2)
                        {
                            getMaster()->setComboBox(6,"volume",220,120,150,40,changeValue);
                            getMaster()->setComboBox(7,"changecombo3",390,120,150,40,-1);
                            getMaster()->setComboBox(8,"changecombo4",550,120,50,40,1);
                        }
                        if (changeType==3)
                        {
                            getMaster()->setComboBox(9,"pan",220,170,150,40,changeValue);
                            getMaster()->setComboBox(10,"changecombo3",390,170,150,40,-1);
                            getMaster()->setComboBox(11,"changecombo4",550,170,50,40,1);
                        }
                    }

                   // getMaster()->SetButton(12,"back", 600, 20, 70,  30, "esc"); //refact

                }
            }
        }
    }
    else
        top->setText("beat ptr was not set");
}


void ChangesInput::turnOffChange(std::string combo)
{
    Package *changePack = ptrToBeat->effPack.getPack(28);
    Beat::ChangesList *changes = 0;
    if (changePack)
        changes = changePack->getPointer();

    byte awaitType = 255;

    if (combo=="2")
        awaitType = 1;
    if (combo=="5")
        awaitType = 8;
    if (combo=="8")
        awaitType = 2;
    if (combo=="11")
        awaitType = 3;

    if (changes)
        for (ul i = 0; i < changes->len(); ++i)
        {
            if (changes->getV(i).changeType == awaitType)
            {
                changes->remove(i);
                if (changes->len() == 0)
                {
                    ptrToBeat->effPack.set(28,false);
                }
                return;
            }
        }
}

void ChangesInput::turnOnChange(std::string combo)
{
    Package *changePack = ptrToBeat->effPack.getPack(28);
    Beat::ChangesList *changes = 0;
    if (changePack)
        changes = changePack->getPointer();

    byte awaitType = 255;
    ul changeValue = 0;
    byte changeAfter = 0;

    if (ptrToBeat->effPack.get(28)==false)
        ptrToBeat->effPack.set(28,true);

    if (combo=="2")
    {
        awaitType = 1;
        changeValue =getMaster()->getComboBoxValue(0);//1;
        changeAfter =getMaster()->getComboBoxValue(1);//2
    }
    if (combo=="5")
    {
        awaitType = 8;
        changeValue =getMaster()->getComboBoxValue(3);//3;
        changeAfter =getMaster()->getComboBoxValue(4);//4
    }
    if (combo=="8")
    {
        awaitType = 2;
        changeValue =getMaster()->getComboBoxValue(6);//6;
        changeAfter =getMaster()->getComboBoxValue(7);//7
    }
    if (combo=="11")
    {
        awaitType = 3;
        changeValue =getMaster()->getComboBoxValue(9);//9;
        changeAfter =getMaster()->getComboBoxValue(10);//10
    }

    bool notFound = true;
    if (changes==0)
    {
        ptrToBeat->effPack.set(28,true);
        ptrToBeat->effPack.addPack(28,1,&ptrToBeat->changes);
        changes = &ptrToBeat->changes;
    }
    else
    {
        for (ul i = 0; i < changes->len(); ++i)
        {
            if (changes->getV(i).changeType == awaitType)
            {
                changes->getV(i).changeType = awaitType;
                changes->getV(i).changeValue = changeValue;
                changes->getV(i).changeCount = changeAfter;
                return;
            }
        }
    }

    if (notFound)
    {
        Beat::SingleChange newChange;
        newChange.changeType = awaitType;
        newChange.changeValue = changeValue;
        newChange.changeCount = changeAfter;
        changes->add(newChange);
    }

}

void ChangesInput::changeMainValue(int combo, int newValue)
{
    Package *changePack = ptrToBeat->effPack.getPack(28);
    Beat::ChangesList *changes = 0;
    if (changePack)
        changes = changePack->getPointer();

    byte awaitType = 255;

    if (combo==0)
        awaitType = 1;
    if (combo==3)
        awaitType = 8;
    if (combo==6)
        awaitType = 2;
    if (combo==9)
        awaitType = 3;

    if (changes)
        for (ul i = 0; i < changes->len(); ++i)
        {
            if (changes->getV(i).changeType == awaitType)
            {
                changes->getV(i).changeValue = newValue;
                return;
            }
        }
}

void ChangesInput::changeSubValue(int combo, int newValue)
{
    Package *changePack = ptrToBeat->effPack.getPack(28);
    Beat::ChangesList *changes = 0;
    if (changePack)
        changes = changePack->getPointer();

    byte awaitType = 255;

    if (combo==0)
        awaitType = 1;
    if (combo==3)
        awaitType = 8;
    if (combo==6)
        awaitType = 2;
    if (combo==9)
        awaitType = 3;

    if (changes)
        for (ul i = 0; i < changes->len(); ++i)
        {
            if (changes->getV(i).changeType == awaitType)
            {
                changes->getV(i).changeCount = newValue;
                return;
            }
        }
}

void ChangesInput::keyevent(std::string press)
{

    if (press.substr(0,4)=="com:")
    {
        std::string rest = press.substr(4);
        size_t separator = rest.find(":");
        std::string combo = rest.substr(0,separator);
        std::string item = rest.substr(separator+1);

        int itemNum = atoi(item.c_str());



        //subfunctions - delete change type
        //edit change type

        if (combo=="2")
        {
            if (itemNum!=0)
                turnOnChange(combo);
            else //of
                turnOffChange(combo);
        }
        if (combo=="5")
        {
            if (itemNum!=0)
                turnOnChange(combo);
            else //of
                turnOffChange(combo);
        }
        if (combo=="8")
        {
            if (itemNum!=0)
                turnOnChange(combo);
            else //of
                turnOffChange(combo);
        }
        if (combo=="11")
        {
            if (itemNum!=0)
                turnOnChange(combo);
            else //of
                turnOffChange(combo);
        }

        if (combo=="0")
            changeMainValue(0,itemNum);
        if (combo=="3")
            changeMainValue(3,itemNum);
        if (combo=="6")
            changeMainValue(6,itemNum);
        if (combo=="9")
            changeMainValue(9,itemNum);

        if (combo=="2")
            changeSubValue(0,itemNum);
        if (combo=="5")
            changeSubValue(3,itemNum);
        if (combo=="8")
            changeSubValue(6,itemNum);
        if (combo=="11")
            changeSubValue(9,itemNum);

        /*
        if (combo=="0")
        {
            if (itemNum==0)
            {
               getMaster()->renewComboParams(1,"instruments"); //value
            }
            if (itemNum==1)
            {
                //prev input to set better?
                getMaster()->renewComboParams(1,"bpm");
            }
            if (itemNum==2)
            {
                getMaster()->renewComboParams(1,"volume"); //value
            }
            if (itemNum==3)
            {
                getMaster()->renewComboParams(1,"pan");
            }
        }
        */
    }
}

