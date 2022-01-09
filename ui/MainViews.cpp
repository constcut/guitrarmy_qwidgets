#include "MainViews.hpp"

#include <QFileDialog>
#include <QMessageBox>

#include "tab/Tab.hpp"
#include "tab/GtpFiles.hpp"
#include "midi/MidiFile.hpp"

#include "g0/Config.hpp"
#include "tab/GmyFile.hpp"
#include "tab/TabLoader.hpp" //loader

#include "TabViews.hpp"

#include <fstream>


#include <QDebug>


#include "MainWindow.hpp"


using namespace gtmy;


void gtmy::changeColor(const std::string& color, QPainter* src);
void gtmy::drawEllipse(QColor c, QPainter *painter, int x, int y, int w, int h);


BendPoints* BendInput::ptrToBend = 0;
Note *BendInput::ptrToNote = 0;

Beat *ChangesInput::ptrToBeat = 0;

void BendInput::setPtrBend(BendPoints* ptr)
{
    ptrToBend = ptr;
}
void BendInput::setPtrNote(Note *ptr)
{
    ptrToNote = ptr;
}

void ChangesInput::setPtrBeat(Beat *beatPtr)
{
    ptrToBeat = beatPtr;
}



bool MainView::isPlaying()
{
    return tabsView->getPlaying();
}

void MainView::onclick(int x1, int y1)
{
    if (currentView)
        currentView->onclick(x1,y1);
}

void MainView::ondblclick(int x1, int y1)
{
    if (currentView)
        currentView->ondblclick(x1,y1);
}

void MainView::onTabCommand(TabCommand command) {
    if (currentView)
        currentView->onTabCommand(command);
}

void MainView::onTrackCommand(TrackCommand command) {
    if (currentView)
        currentView->onTrackCommand(command);
}

void MainView::keyevent(std::string press)
{
    if (press=="welcome")
    {
        changeCurrentView(welcome.get());
        return;
    }
    if (press=="bend_view")
    {
        changeCurrentView(bendInp.get());
        return;
    }
    if (press=="chord_view")
    {
        changeCurrentView(chordInp.get());
        return;
    }
    if (press=="change_view")
    {
        changeCurrentView(chanInp.get());
        return;
    }



        if (currentView)
            currentView->keyevent(press);

        if (press == "esc")
            changeViewToLast();

        if ((press == CONF_PARAM("TrackView.quickOpen"))||(press=="quickopen")) {

            if (tabsView->gotChanges()==false) {
                GmyFile gmyFile; 
                std::string gfileName =  std::string(AConfig::getInst().testsLocation)  + "first.gmy";
                if (QFile::exists(gfileName.c_str()))
                {
                    std::ifstream file(gfileName.c_str(), std::ios::binary);

                    auto newTab = std::make_unique<Tab>();
                    gmyFile.loadFromFile(file,newTab.get());

                    newTab->connectTracks();

                    changeCurrentView(tabsView.get());
                    tabsView->setTab(std::move(newTab));
                    if (CONF_PARAM("skipTabView")=="1")
                    tabsView->onTabCommand(TabCommand::OpenTrack);
                }

            }
        }
        if (press == CONF_PARAM("Main.open")) {

            if (tabsView->gotChanges()==false) {
                auto fd = std::make_unique<QFileDialog>();


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

                        //fd->getOpenFileName(0, QString::fromLocal8Bit("�������"),
                         //            AConfig::getInstance().testsLocation,
                                  //   "Gx files (*.g*)");

                clock_t afterT = getTime();

                GTabLoader tabLoader;
                if  (tabLoader.open(s.toStdString()))
                {
                    changeCurrentView(tabsView.get());
                    tabsView->setTab(std::move(tabLoader.getTab()));
                    if (CONF_PARAM("skipTabView")=="1")
                        tabsView->onTabCommand(TabCommand::OpenTrack);

                    //getMaster()->changeChild(tabsView);

                    clock_t after2T = getTime();
                    clock_t diffT = after2T - afterT;
                    qDebug() <<"Opening gtp "<<diffT;
                }
            }
        }



        if (press == "tests")
        {
            //getMaster()->changeChild(testsView);
             changeCurrentView(testsView.get());
        }

        if ((press == "config")) //sepparate to log view
        {
                //getMaster()->changeChild(configView);
                 changeCurrentView(configView.get());
        }

        if (press == "tap")
        {
            //tap Ry
             //getMaster()->changeChild(tapRyView);
             tapRyView->measureTime();
             changeCurrentView(tapRyView.get());
        }

        if (press == "pattern")
        {
            //pattern input
            //getMaster()->changeChild(patternInp);
             changeCurrentView(patternInp.get());
        }

        if (press == "tabview")
        {
             changeCurrentView(tabsView.get());
             //tabsView->keyevent("track");
        }

        if (press == "trackview")
        {
            changeCurrentView(tabsView->tracksView[tabsView->getLastOpenedTrack()].get());
        }

        if (press == "info")
        {
            changeCurrentView(infView.get());
        }

        if (press == "morze")
        {
            changeCurrentView(morzeInp.get());
        }

        if (press == "newtab")
        {
            if (tabsView->gotChanges()==false)
            {
                //NEW tab

                auto newTab = std::make_unique<Tab>();
                newTab->setBPM(120);

                //newTab->connectTracks();
                auto track = std::make_unique<Track>();
                track->setParent(newTab.get());
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


                auto bar = std::make_unique<Bar>();
                bar->flush();
                bar->setSignDenum(4); bar->setSignNum(4);
                bar->setRepeat(0);


                for (int iB=0; iB <4; ++iB)
                {
                    auto beat = std::make_unique<Beat>();
                    beat->setPause(true);
                    beat->setDotted(0);
                    beat->setDuration(3);
                    beat->setDurationDetail(0);

                    bar->push_back(std::move(beat));
                }

                track->push_back(std::move(bar));
                newTab->push_back(std::move(track));
                newTab->connectTracks();

                changeCurrentView(tabsView.get());
                tabsView->setTab(std::move(newTab));
                if (CONF_PARAM("skipTabView")=="1")
                tabsView->onTabCommand(TabCommand::OpenTrack); // keyevent("opentrack");
                //getMaster()->changeChild(tabsView);

            }

        }
}

void MainView::draw(QPainter *painter)
{
    int wi = getMaster()->getWidth();
    int hi = getMaster()->getHeight();

    //pan->setW(wi);
    //pan->resetButtons();
    //pan->draw(painter);

    if (currentView)
    {

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
    dependent=false;
    tabsView = std::make_unique<TabView>();
    configView = std::make_unique<ConfigView>();
    tapRyView = std::make_unique<TapRyView>();
    testsView = std::make_unique<TestsView>(this, tabsView.get());
    patternInp = std::make_unique<PatternInput>();

    infView = std::make_unique<InfoView>();
    morzeInp = std::make_unique<MorzeInput>();

    welcome = std::make_unique<WelcomeView>();

    bendInp = std::make_unique<BendInput>();
    chanInp = std::make_unique<ChangesInput>();
    chordInp = std::make_unique<ChordInput>();

    currentView  =  welcome.get(); //testsView;

    pan = std::make_unique<GStickPannel>(0,60,800);
    pan->setPressView(this);
    pan->setNoOpenButton();

}



ConfigView::ConfigView():GView()
{
    AConfig& config = AConfig::getInst();
    connectConfigs(config);

    std::string sP = config.logsNames[0] + " " +
        std::to_string((int)*(config.logs[0]))  + " press 1 to chng";
    labA = std::make_unique<GLabel>(20,80+200-55,sP.c_str());

    sP = config.logsNames[2] + " " +
        std::to_string((int)*(config.logs[2]))  + " press 1 to chng";
    labC = std::make_unique<GLabel>(20,120+200-55,sP.c_str());

    sP = config.logsNames[3] + " " +
        std::to_string((int)*(config.logs[3]))  + " press 1 to chng";
    labD = std::make_unique<GLabel>(20,120+200-55,sP.c_str());
}

void ConfigView::keyevent(std::string press)
{
    //if (press == "m")
        //getMaster()->resetToFirstChild();

    AConfig& config = AConfig::getInst();

    if (press == "1")
    {
      bool *x = config.logs[0];
      bool inv = !(*x);
      *x = inv;

      std::string sP =config.logsNames[0] + " " +
        std::to_string((int)*(config.logs[0]))  + " press 1 to chng";
      labA->setText(sP.c_str());
    }
    if (press == "2")
    {
      bool *x = config.logs[1];
      bool inv = !(*x);
      *x = inv;

      std::string sP =config.logsNames[1] + " " +
        std::to_string((int)*(config.logs[1]))  + " press 2 to chng";

      labB->setText(sP.c_str());
    }
    if (press == "3")
    {
      bool *x = config.logs[2];
      bool inv = !(*x);
      *x = inv;

      std::string sP =config.logsNames[2] + " " +
        std::to_string((int)*(config.logs[2]))  + " press 3 to chng";

      labC->setText(sP.c_str());
    }
    if (press == "4")
    {
      bool *x = config.logs[3];
      bool inv = !(*x);
      *x = inv;


      std::string sP =config.logsNames[3] + " " +
        std::to_string((int)*(config.logs[3]))  + " press 4 to chng";


      labD->setText(sP.c_str());
    }
}

void ConfigView::onclick([[maybe_unused]]int x1, [[maybe_unused]]int y1)
{
    //if (labA->hit(x1,y1)) keyevent("1");
    //if (labB->hit(x1,y1)) keyevent("2");
    //if (labC->hit(x1,y1)) keyevent("3");
    //if (labD->hit(x1,y1)) keyevent("4");

    /*
    if (labScaleMinus->hit(x1,y1))
    {
        double currentScale = AConfig::getInstance().getScaleCoef();

        if (currentScale >= 0.5)
        {
            currentScale -= 0.25;
            AConfig::getInstance().setScaleCoef(currentScale);
        }
    }

    if (labScalePlus->hit(x1,y1))
    {
        double currentScale = AConfig::getInstance().getScaleCoef();

        if (currentScale <= 4.0)
        {
            currentScale += 0.25;
            AConfig::getInstance().setScaleCoef(currentScale);
        }
    }
    */
}


void ConfigView::draw([[maybe_unused]]QPainter *painter)
{
    //labA->draw(painter);
    //labB->draw(painter);
    //labC->draw(painter);
    //labD->draw(painter);

    //labScaleMinus->draw(painter);
    //labScalePlus->draw(painter);

    //size_t i = 0;
    //int limitY = getMaster()->getHeight();
    //int curX = 200;
    //int curY = 75;

    /*
    for (std::map<std::string,std::string>::iterator itV=AConfig::getInstance().values.begin();
         itV!=AConfig::getInstance().values.end(); ++itV, ++i)
    {
            stri ngExtended line;
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




void TestsView::openTestNumber(int num) {
    if (tabsView->gotChanges()==false) {

        qDebug() << "Opening test "<<buttons[num].getText().c_str();

        std::string fn = AConfig::getInst().testsLocation + buttons[num].getText().c_str() + ".gp4";
        //WIN? inverted?

        std::ifstream importFile(fn, std::ios::binary);
         if (importFile.is_open() == false) {
             std::cerr << "Failed to open " << fn << std::endl;
         }

        auto forLoad = std::make_unique<Tab>();
        Gp4Import importer;
        importer.import(importFile, forLoad.get());
        forLoad->postGTP();
        forLoad->connectTracks();
        qDebug() << "file v 4 was opened: "<<fn.c_str();

        MainView *mainView = (MainView*)getMaster()->getFirstChild();
        mainView->changeCurrentView(tabsView);
        tabsView->setTab(std::move(forLoad));
        if (CONF_PARAM("skipTabView")=="1")
        tabsView->onTabCommand(TabCommand::OpenTrack);
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


QByteArray alphaCut(QByteArray &src)
{
    QByteArray newBytes;
    for (size_t i = 0; i < src.size(); ++i)
    {
        int letter = src.at(i)+128;
        int hi = letter / 16;
        int lo = letter % 16;

        char valueHi = 97 + hi;
        char valueLo = 97 + lo;
        newBytes.append(valueHi);
        newBytes.append(valueLo);

       // std::cout << "B1 = " << valueHi <<" "<<int(valueHi)<<"; B2 = "<<valueLo<<" "<<int(valueLo)<<"; SOurce value ="<<letter<<std::endl;
    }

    char endingZero = 0;
    newBytes.append(endingZero);
    return newBytes;
}


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


        QString crashName = AConfig::getInst().testsLocation.c_str() + QString("crashs.glog");
        if (QFile::exists(crashName))
        {
            QFile logData;
            logData.setFileName(crashName);
            logData.open(QIODevice::ReadOnly);
            QByteArray allLogData = logData.readAll();

            std::string lines[7];
            lines[0] = "File exists";
            lines[1] = "Full size " + std::to_string( allLogData.size() );
            lines[2] = "Could be sent in ~ " +  std::to_string(  allLogData.size()/1000 )
                + " iterations";

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
            std::string lines[7];
            lines[0] = "File doesn't exists - no crashes saved";
            labelA->setText(lines[0].c_str());
        }


       return;
    }

    if (sendCrash->hit(x1,y1))
    {
        std::string requestLine;

        std::string sendData = "crashlog";

        QString crashName = AConfig::getInst().testsLocation.c_str() + QString("crashs.glog");
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

        /*
        std::string userIdStd = getUserId();
        QDateTime timeDate(QDateTime::currentDateTime());
        QString st = timeDate.toString(Qt::ISODate);
        st.replace(":","");
        std::string time = st.toStdString();
        requestLine = "http://guitarmy.in/glogs/crash.php?log="
                     + sendData + "&date=" + time + "&user=" + userIdStd;
        */ //TODO новая отправка логов

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




void BendInput::draw(QPainter *painter)
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

        for (size_t i = 0; i < 12; ++i)
            painter->drawLine(20,80+i*20,500,80+i*20);

        for (size_t i = 0; i < 60; ++i)
            painter->drawLine(20+i*8,80,20+i*8,320);

        //8 px for horizon step, 20 for vertical
        int prevX = 0; int prevY = 0;

        gtmy::changeColor("red", painter);
        for (size_t i = 0; i < ptrToBend->size(); ++i)
        {
            std::uint8_t vertical = ptrToBend->at(i).vertical;
            std::uint8_t horizontal = ptrToBend->at(i).horizontal;
            //qDebug() << i << ") v= "<<vertical<<"; hor = "<<horizontal;

            int localY = vertical*-20; localY += 80+240;
            int localX = horizontal*8; localX += 20; //second op - shift in rect

            if (i) //skip first
                painter->drawLine(prevX,prevY,localX,localY);

            prevX = localX; prevY = localY;
            gtmy::drawEllipse(QColor("red"), painter, localX-3,localY-3,6,6);
        }
        //qDebug() << "Bend outpited";
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
            qDebug() << "Bend c type "<<bendType<<"; h "<<bendHeight;
            fillBend(bendType,bendHeight);
        }
        qDebug() << "comBend";
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
                ptrToNote->getEffectsRef().setEffectAt(Effect::Bend,true);
                //ptrToNote->getEffects().addPack(17,2,ptrToBend); //insure but it must be already inside
             }
             getMaster()->pushForceKey("esc"); //go prev
             //better have another function atention
         }
    }
    if (delButton->hit(x1,y1))
    {
        if (ptrToNote)
        {
            bool whatIsThere = ptrToNote->getEffects().getEffectAt(Effect::Bend);
            qDebug() << "Deleting bend that is "<<(int)whatIsThere;
            ptrToNote->getEffectsRef().setEffectAt(Effect::Bend, false);
        }
    }
    //shoud react on press and calculate
    ///vertical and horizontal
    //to print it in the top value

    int localX = x1-20;
    int localY = y1-80; localY = 240 - localY;

    qDebug() << "Locals "<<localX<<"; "<<localY;

    //ATTENTION please note that there are yen no chanes

    if ((localX < 480) & (localY < 240))
    {
        std::uint8_t vertical = localY/20;
        std::uint8_t horizontal = localX/8;
        qDebug() << "hit "<<vertical<<"; "<<horizontal;

        if (ptrToBend)
        {
            for (size_t i = 0; i < ptrToBend->size(); ++i)
            {
                int pointV= ptrToBend->at(i).vertical;
                int pointH= ptrToBend->at(i).horizontal;

                //diff + - 1 ?

                if ((pointV==vertical) && (pointH==horizontal))
                {
                    qDebug() << "point really hitten";
                    ptrToBend->erase(ptrToBend->begin() + i);
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
        ptrToBend = new BendPoints; //simple escape then could be reason of leak TODO
    //first only type==0 (normal bend)
    if (type==0)
    {
        ptrToBend->clear();
        BendPoint p0; p0.vertical = 0; p0.horizontal = 0; p0.vFlag=0;
        BendPoint p1; p1.vertical = height; p1.horizontal = 15; p1.vFlag=0;
        BendPoint p2; p2.vertical = height; p2.horizontal = 60; p2.vFlag=0;
        ptrToBend->push_back(p0); ptrToBend->push_back(p1); ptrToBend->push_back(p2);
    }
    if (type==1)
    {
        ptrToBend->clear();
        BendPoint p0; p0.vertical = 0; p0.horizontal = 0; p0.vFlag=0;
        BendPoint p1; p1.vertical = height; p1.horizontal = 10; p1.vFlag=0;
        BendPoint p2; p2.vertical = height; p2.horizontal = 20; p2.vFlag=0;
        BendPoint p3; p3.vertical = 0; p3.horizontal = 30; p3.vFlag=0;
        BendPoint p4; p4.vertical = 0; p4.horizontal = 60; p4.vFlag=0;
        ptrToBend->push_back(p0); ptrToBend->push_back(p1); ptrToBend->push_back(p2); ptrToBend->push_back(p3); ptrToBend->push_back(p4);
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
        ptrToBend->push_back(p0); ptrToBend->push_back(p1); ptrToBend->push_back(p2); ptrToBend->push_back(p3);
        ptrToBend->push_back(p4); ptrToBend->push_back(p5); ptrToBend->push_back(p6);
    }
    if (type==3)
    {
        ptrToBend->clear();
        BendPoint p0; p0.vertical = height; p0.horizontal = 0; p0.vFlag=0;
        BendPoint p1; p1.vertical = height; p1.horizontal = 60; p1.vFlag=0;
        ptrToBend->push_back(p0); ptrToBend->push_back(p1);
    }
    if (type==4)
    {
        ptrToBend->clear();
        BendPoint p0; p0.vertical = height; p0.horizontal = 0; p0.vFlag=0;
        BendPoint p1; p1.vertical = height; p1.horizontal = 15; p1.vFlag=0;
        BendPoint p2; p2.vertical = 0; p2.horizontal = 30; p2.vFlag=0;
        BendPoint p3; p3.vertical = 0; p3.horizontal = 60; p3.vFlag=0;
        ptrToBend->push_back(p0); ptrToBend->push_back(p1); ptrToBend->push_back(p2);  ptrToBend->push_back(p3);
    }
}

//CHANGES

void ChangesInput::draw(QPainter *painter)
{
    top->draw(painter);

    if (ptrToBeat)
    {
        painter->drawText(70,30,"instruments change:");
        painter->drawText(70,80,"bpm change:");
        painter->drawText(70,130,"volume change:");
        painter->drawText(70,180,"pan change:");


        top->setText("beat ptr was set");
        if (ptrToBeat->getEffects().getEffectAt(Effect::Changes)==true)
        {

            {
                Beat::ChangesList *changes = &ptrToBeat->changes;
                if (changes)
                {
                    getMaster()->setComboBox(5,"changecombo4",550,70,50,40,0);
                    getMaster()->setComboBox(2,"changecombo4",550,20,50,40,0);
                    getMaster()->setComboBox(8,"changecombo4",550,120,50,40,0);
                    getMaster()->setComboBox(11,"changecombo4",550,170,50,40,0);


                    for (size_t i = 0; i < changes->size(); ++i)
                    {
                        std::uint8_t changeType = changes->at(i).changeType;
                        size_t changeValue = changes->at(i).changeValue;
                        std::uint8_t changeCount = changes->at(i).changeCount;

                        std::string sX = "Type " + std::to_string( changeType ) + "; Value " +
                            std::to_string( changeValue )  + "; count " + std::to_string( changeCount );
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

    Beat::ChangesList *changes = &ptrToBeat->changes;

    std::uint8_t awaitType = 255;

    if (combo=="2")
        awaitType = 1;
    if (combo=="5")
        awaitType = 8;
    if (combo=="8")
        awaitType = 2;
    if (combo=="11")
        awaitType = 3;

    if (changes)
        for (size_t i = 0; i < changes->size(); ++i)
        {
            if (changes->at(i).changeType == awaitType)
            {
                changes->erase(changes->begin() + i);
                if (changes->size() == 0)
                {
                    ptrToBeat->getEffects().setEffectAt(Effect::Changes,false);
                }
                return;
            }
        }
}

void ChangesInput::turnOnChange(std::string combo)
{
    Beat::ChangesList *changes = &ptrToBeat->changes;

    std::uint8_t awaitType = 255;
    size_t changeValue = 0;
    std::uint8_t changeAfter = 0;

    if (ptrToBeat->getEffects().getEffectAt(Effect::Changes)==false)
        ptrToBeat->getEffects().setEffectAt(Effect::Changes,true);

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
        ptrToBeat->getEffects().setEffectAt(Effect::Changes,true);
        changes = &ptrToBeat->changes;
    }
    else
    {
        for (size_t i = 0; i < changes->size(); ++i)
        {
            if (changes->at(i).changeType == awaitType)
            {
                changes->at(i).changeType = awaitType;
                changes->at(i).changeValue = changeValue;
                changes->at(i).changeCount = changeAfter;
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
        changes->push_back(newChange);
    }

}

void ChangesInput::changeMainValue(int combo, int newValue)
{
    Beat::ChangesList *changes = &ptrToBeat->changes;
    std::uint8_t awaitType = 255;

    if (combo==0)
        awaitType = 1;
    if (combo==3)
        awaitType = 8;
    if (combo==6)
        awaitType = 2;
    if (combo==9)
        awaitType = 3;

    if (changes)
        for (size_t i = 0; i < changes->size(); ++i)
        {
            if (changes->at(i).changeType == awaitType)
            {
                changes->at(i).changeValue = newValue;
                return;
            }
        }
}

void ChangesInput::changeSubValue(int combo, int newValue)
{
    Beat::ChangesList *changes = &ptrToBeat->changes;

    std::uint8_t awaitType = 255;

    if (combo==0)
        awaitType = 1;
    if (combo==3)
        awaitType = 8;
    if (combo==6)
        awaitType = 2;
    if (combo==9)
        awaitType = 3;

    if (changes)
        for (size_t i = 0; i < changes->size(); ++i)
        {
            if (changes->at(i).changeType == awaitType)
            {
                changes->at(i).changeCount = newValue;
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

