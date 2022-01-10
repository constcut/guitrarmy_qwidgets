#include "MainViews.hpp"

#include <QFileDialog>
#include <QMessageBox>

#include "tab/Tab.hpp"
#include "tab/tools/GtpFiles.hpp"
#include "midi/MidiFile.hpp"

#include "g0/Config.hpp"
#include "tab/tools/GmyFile.hpp"
#include "tab/tools/TabLoader.hpp" //loader

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
    return _tabsView->getPlaying();
}

void MainView::onclick(int x1, int y1)
{
    if (_currentView)
        _currentView->onclick(x1,y1);
}

void MainView::ondblclick(int x1, int y1)
{
    if (_currentView)
        _currentView->ondblclick(x1,y1);
}

void MainView::onTabCommand(TabCommand command) {
    if (_currentView)
        _currentView->onTabCommand(command);
}

void MainView::onTrackCommand(TrackCommand command) {
    if (_currentView)
        _currentView->onTrackCommand(command);
}

void MainView::keyevent(std::string press)
{
    if (press=="welcome")
    {
        changeCurrentView(_welcome.get());
        return;
    }
    if (press=="bend_view")
    {
        changeCurrentView(_bendInp.get());
        return;
    }
    if (press=="chord_view")
    {
        changeCurrentView(_chordInp.get());
        return;
    }
    if (press=="change_view")
    {
        changeCurrentView(_chanInp.get());
        return;
    }



        if (_currentView)
            _currentView->keyevent(press);

        if (press == "esc")
            changeViewToLast();

        if ((press == CONF_PARAM("TrackView.quickOpen"))||(press=="quickopen")) {

            if (_tabsView->gotChanges()==false) {
                GmyFile gmyFile; 
                std::string gfileName =  std::string(AConfig::getInst().testsLocation)  + "first.gmy";
                if (QFile::exists(gfileName.c_str()))
                {
                    std::ifstream file(gfileName.c_str(), std::ios::binary);

                    auto newTab = std::make_unique<Tab>();
                    gmyFile.loadFromFile(file,newTab.get());

                    newTab->connectTracks();

                    changeCurrentView(_tabsView.get());
                    _tabsView->setTab(std::move(newTab));
                    if (CONF_PARAM("skipTabView")=="1")
                    _tabsView->onTabCommand(TabCommand::OpenTrack);
                }

            }
        }
        if (press == CONF_PARAM("Main.open")) {

            if (_tabsView->gotChanges()==false) {
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
                    changeCurrentView(_tabsView.get());
                    _tabsView->setTab(std::move(tabLoader.getTab()));
                    if (CONF_PARAM("skipTabView")=="1")
                        _tabsView->onTabCommand(TabCommand::OpenTrack);

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
             changeCurrentView(_testsView.get());
        }

        if ((press == "config")) //sepparate to log view
        {
                //getMaster()->changeChild(configView);
                 changeCurrentView(_configView.get());
        }

        if (press == "tap")
        {
            //tap Ry
             //getMaster()->changeChild(tapRyView);
             _tapRyView->measureTime();
             changeCurrentView(_tapRyView.get());
        }

        if (press == "pattern")
        {
            //pattern input
            //getMaster()->changeChild(patternInp);
             changeCurrentView(_patternInp.get());
        }

        if (press == "tabview")
        {
             changeCurrentView(_tabsView.get());
             //tabsView->keyevent("track");
        }

        if (press == "trackview")
        {
            changeCurrentView(_tabsView->getTracksViewRef()[_tabsView->getLastOpenedTrack()].get());
        }

        if (press == "info")
        {
            changeCurrentView(_infView.get());
        }

        if (press == "morze")
        {
            changeCurrentView(_morzeInp.get());
        }

        if (press == "newtab")
        {
            if (_tabsView->gotChanges()==false)
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

                auto& tuning = track->getTuningRef();
                tuning.setStringsAmount(6);
                tuning.setTune(0,64);
                tuning.setTune(1,59);
                tuning.setTune(2,55);
                tuning.setTune(3,50);
                tuning.setTune(4,45);
                tuning.setTune(5,40);

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

                changeCurrentView(_tabsView.get());
                _tabsView->setTab(std::move(newTab));
                if (CONF_PARAM("skipTabView")=="1")
                _tabsView->onTabCommand(TabCommand::OpenTrack); // keyevent("opentrack");
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

    if (_currentView)
    {

        _currentView->setW(wi);
        _currentView->setH(hi);

        if (_currentView->getMaster() == 0)
            _currentView->setMaster(getMaster());

        _currentView->draw(painter);
    }

}


void MainView::setMaster(MasterView *mast)
{
    GView::setMaster(mast);
    if (_testsView) _testsView->setMaster(mast);
    if (_tabsView) _tabsView->setMaster(mast);
    if (_configView) _configView->setMaster(mast);
    if (_tapRyView) _tapRyView->setMaster(mast);
    if (_patternInp) _patternInp->setMaster(mast);
    //log <<"Main view set master called";
}


MainView::MainView():GView(0,0,800,480), _currentView(0)
{
    _dependent=false;
    _tabsView = std::make_unique<TabView>();
    _configView = std::make_unique<ConfigView>();
    _tapRyView = std::make_unique<TapRyView>();
    _testsView = std::make_unique<TestsView>(this, _tabsView.get());
    _patternInp = std::make_unique<PatternInput>();

    _infView = std::make_unique<InfoView>();
    _morzeInp = std::make_unique<MorzeInput>();

    _welcome = std::make_unique<WelcomeView>();

    _bendInp = std::make_unique<BendInput>();
    _chanInp = std::make_unique<ChangesInput>();
    _chordInp = std::make_unique<ChordInput>();

    _currentView  =  _welcome.get(); //testsView;

    _pan = std::make_unique<GStickPannel>(0,60,800);
    _pan->setPressView(this);
    _pan->setNoOpenButton();

}



ConfigView::ConfigView():GView()
{
    AConfig& config = AConfig::getInst();
    connectConfigs(config);

    std::string sP = config.logsNames[0] + " " +
        std::to_string((int)*(config.logs[0]))  + " press 1 to chng";
    _labA = std::make_unique<GLabel>(20,80+200-55,sP.c_str());

    sP = config.logsNames[2] + " " +
        std::to_string((int)*(config.logs[2]))  + " press 1 to chng";
    _labC = std::make_unique<GLabel>(20,120+200-55,sP.c_str());

    sP = config.logsNames[3] + " " +
        std::to_string((int)*(config.logs[3]))  + " press 1 to chng";
    _labD = std::make_unique<GLabel>(20,120+200-55,sP.c_str());
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
      _labA->setText(sP.c_str());
    }
    if (press == "2")
    {
      bool *x = config.logs[1];
      bool inv = !(*x);
      *x = inv;

      std::string sP =config.logsNames[1] + " " +
        std::to_string((int)*(config.logs[1]))  + " press 2 to chng";

      _labB->setText(sP.c_str());
    }
    if (press == "3")
    {
      bool *x = config.logs[2];
      bool inv = !(*x);
      *x = inv;

      std::string sP =config.logsNames[2] + " " +
        std::to_string((int)*(config.logs[2]))  + " press 3 to chng";

      _labC->setText(sP.c_str());
    }
    if (press == "4")
    {
      bool *x = config.logs[3];
      bool inv = !(*x);
      *x = inv;


      std::string sP =config.logsNames[3] + " " +
        std::to_string((int)*(config.logs[3]))  + " press 4 to chng";


      _labD->setText(sP.c_str());
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
}


WelcomeView::WelcomeView()
{
    _top = std::make_unique<GLabel>(50,100,"Guitarmy - tablature compose tool","",false);
    _mid = std::make_unique<GLabel>(50,150,"This is early beta version - thank you for download","",false);
    _bot = std::make_unique<GLabel>(50,200,";)","",false);
}

void WelcomeView::draw(QPainter *painter)
{
    _top->draw(painter);
    _mid->draw(painter);
    _bot->draw(painter);
}



TestsView::TestsView(MainView *mainV, TabView *tabV): _upper(30, 30, "Start all tests!(dbl click)"),
    _bottom(250, 30,"Information about full tests: not started"),
    _g3(600, 30, "G3____"), _g5(700, 30, "G5_____") ,_mainView(mainV),_tabsView(tabV)
{
    _playlistButton = std::make_unique<GLabel>(400,0+30,"playlist");
    _playlist2Button = std::make_unique<GLabel>(400,115-85+30,"playlist2");
    _stopPlaylist = std::make_unique<GLabel>(450,0+30,"stop playlist");
    setAllButtons();
}

void TestsView::openTestNumber(int num) {
    if (_tabsView->gotChanges()==false) {

        qDebug() << "Opening test "<<_buttons[num].getText().c_str();

        std::string fn = AConfig::getInst().testsLocation + _buttons[num].getText().c_str() + ".gp4";
        //WIN? inverted?

        std::ifstream importFile(fn, std::ios::binary);
         if (importFile.is_open() == false) {
             std::cerr << "Failed to open " << fn << std::endl;
         }

        auto forLoad = std::make_unique<Tab>();
        Gp4Import importer;
        importer.import(importFile, forLoad.get());
        forLoad->postLoading();
        forLoad->connectTracks();
        qDebug() << "file v 4 was opened: "<<fn.c_str();

        MainView *mainView = (MainView*)getMaster()->getFirstChild();
        mainView->changeCurrentView(_tabsView);
        _tabsView->setTab(std::move(forLoad));
        if (CONF_PARAM("skipTabView")=="1")
        _tabsView->onTabCommand(TabCommand::OpenTrack);
    }

}

void TestsView::onclick(int x1, int y1)
{
    for (size_t i = 0; i < _buttons.size(); ++i)
        if (_buttons[i].hit(x1,y1))
        {
            openTestNumber(i);
            return;
        }

    if (_stopPlaylist->hit(x1,y1))
    {
        getMaster()->cleanPlayList();
    }

    if (_playlistButton->hit(x1,y1))
    {
        getMaster()->pushForceKey(_playlistButton->getPressSyn());
    }

    if (_playlist2Button->hit(x1,y1))
    {
        getMaster()->pushForceKey(_playlist2Button->getPressSyn());
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
    if (_labelB->hit(x1,y1))
        QMessageBox::aboutQt(0,"About Qt");

    if (_labelA->hit(x1,y1))
    {

    }

    if (_help->hit(x1,y1))
    {
        getMaster()->showHelp();

        _help->setVisible(false);
        _logShow->setVisible(false);

        _sendCrash->setVisible(false);

        _labelA->setVisible(false);
        _labelA2->setVisible(false);
        _icons->setVisible(false);

        _labelB->setVisible(false);
        _labelC->setVisible(false);
        _labelD->setVisible(false);
        _labelE->setVisible(false);
        return;
    }

    if (_logShow->hit(x1,y1))
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

            _labelA->setText(lines[0].c_str());
            _labelA2->setText(lines[1].c_str());
            _icons->setText(lines[2].c_str());

            _labelB->setText("");
            _labelC->setText("");
            _labelD->setText("");
            _labelE->setText("");

            logData.close();
        }
        else
        {
            std::string lines[7];
            lines[0] = "File doesn't exists - no crashes saved";
            _labelA->setText(lines[0].c_str());
        }


       return;
    }

    if (_sendCrash->hit(x1,y1))
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

    if (_help->isVisible()==false)
    {
        _help->setVisible(true);
    }

    if (_logShow->isVisible()==false)
    {
        _logShow->setVisible(true);
    }

    if (_sendCrash->isVisible()==false)
    {
        _sendCrash->setVisible(true);
    }

}

//Bend View




void BendInput::draw(QPainter *painter)
{
    if (ptrToBend)
        _top->setText("Bend is set!");
    else
        _top->setText("Bend not set!");

    _top->draw(painter);
    _okButton->draw(painter);
    _delButton->draw(painter);

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
    if (_okButton->hit(x1,y1))
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
    if (_delButton->hit(x1,y1))
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
    _top->draw(painter);

    if (ptrToBeat)
    {
        painter->drawText(70,30,"instruments change:");
        painter->drawText(70,80,"bpm change:");
        painter->drawText(70,130,"volume change:");
        painter->drawText(70,180,"pan change:");


        _top->setText("beat ptr was set");
        if (ptrToBeat->getEffects().getEffectAt(Effect::Changes)==true)
        {

            {
                ChangesList *changes = ptrToBeat->getChangesPtr();
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
        _top->setText("beat ptr was not set");
}


void ChangesInput::turnOffChange(std::string combo)
{

    ChangesList* changes = ptrToBeat->getChangesPtr();

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
    ChangesList *changes = ptrToBeat->getChangesPtr();

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
        changes = ptrToBeat->getChangesPtr();
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
        SingleChange newChange;
        newChange.changeType = awaitType;
        newChange.changeValue = changeValue;
        newChange.changeCount = changeAfter;
        changes->push_back(newChange);
    }

}

void ChangesInput::changeMainValue(int combo, int newValue)
{
    ChangesList *changes = ptrToBeat->getChangesPtr();
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
    ChangesList *changes = ptrToBeat->getChangesPtr();

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
    }
}



void MainView::changeViewToLast()
{
    if (_lastViews.empty())
        return;

    GView *lastOne = _lastViews[_lastViews.size()-1];
    _lastViews.pop_back();
    _currentView = lastOne;
    if (_dependent==false)
        _currentView->setUI();
}



void MainView::changeCurrentView(GView* newView)
{
    _lastViews.push_back(_currentView);
    _currentView = newView;

    _currentView->setMaster(getMaster());
    if (_dependent==false)
        newView->setUI();
}

InfoView::InfoView()
{
    int verticalShift = -60; //-10; //40

    _labelA = std::make_unique<GLabel>(10,100+verticalShift,"Here is Guitarmy early beta test. This software is provided 'as-is', without any express or implied ");
    _labelA2 = std::make_unique<GLabel>(10,125+verticalShift,
                        "warranty. In no event will the authors be held liable for any damages arising from the use of this software.");

    _icons  = std::make_unique<GLabel>(10,150+verticalShift,
                        "Some of icons used for app taken from icons8.com");

    _labelB = std::make_unique<GLabel>(10, 175+verticalShift, "This software is developed using Qt library under LGPL licence. Press here for information");
    _labelC = std::make_unique<GLabel>(10, 200+verticalShift, "Link to the sources of Qt library used for development(5.4.2): guitarmy.in/lgpl/src.zip");

    _labelD = std::make_unique<GLabel>(10, 225+verticalShift, "Link to the licence text: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html");
    _labelE = std::make_unique<GLabel>(10, 250+verticalShift, "Address to contact author about LGPL topics: support@guitarmy.in");

    _help = std::make_unique<GLabel>(10,300+verticalShift,"Show help");

    _logShow = std::make_unique<GLabel>(10,330+verticalShift,"Check crashes");

    _sendCrash = std::make_unique<GLabel>(10,360+verticalShift,"Send crashed");

    _labelA->setBorder(false);
    _labelA2->setBorder(false);
    _icons->setBorder(false);

    _labelB->setBorder(false);
    _labelC->setBorder(false);
    _labelD->setBorder(false);
    _labelE->setBorder(false);
}


void InfoView::draw(QPainter *painter)
{
    _labelA->draw(painter);
    _labelA2->draw(painter);

    _icons->draw(painter);

    _labelB->draw(painter);
    _labelC->draw(painter);

    _labelD->draw(painter);
    _labelE->draw(painter);

    _help->draw(painter);
    _logShow->draw(painter);
    _sendCrash->draw(painter);
}


void TestsView::setUI()
{

    for (size_t i = 0; i < _buttons.size(); ++i)  {
       if (_mainView->getMaster())
            _mainView->getMaster()->SetButton(i,_buttons[i].getText(),
                               _buttons[i].getX(),_buttons[i].getY(),
                               _buttons[i].getW(),_buttons[i].getH(),"");

       }

    if (_mainView->getMaster())
    {
        size_t i = _buttons.size();

        _mainView->getMaster()->SetButton(i,_playlistButton->getText(),
                               _playlistButton->getX(),_playlistButton->getY(),
                               _playlistButton->getW(),_playlistButton->getH(),_playlistButton->getText());
        ++i;

        _mainView->getMaster()->SetButton(i,_stopPlaylist->getText(),
                               _stopPlaylist->getX(),_stopPlaylist->getY(),
                               _stopPlaylist->getW(),_stopPlaylist->getH(),_stopPlaylist->getText());
        ++i;

        _mainView->getMaster()->SetButton(i,_playlist2Button->getText(),
                               _playlist2Button->getX(),_playlist2Button->getY(),
                               _playlist2Button->getW(),_playlist2Button->getH(),_playlist2Button->getText());

    }
}


void TestsView::setAllButtons()
{
    int wi = 800;
    int hi = 480;
    if (_mainView->getMaster()) {
        wi = _mainView->getMaster()->getWidth();
        hi = _mainView->getMaster()->getHeight();
    }
    _buttons.clear();

    int xSh = 30;
    int ySh = 120-85+30;

    for (size_t i = 0; i < 11; ++i) {
        GLabel button(xSh,ySh,"1." + std::to_string(i+1));
        button.setW(40);
        _buttons.push_back(std::move(button));

        xSh += 20 + button.getW();
        if (xSh >= (wi-50)) {
            xSh = 30;
            ySh += button.getH()*2;
        }
    }

    for (size_t i = 0; i < 38; ++i)
    {
        GLabel button(xSh,ySh,"2." + std::to_string(i+1));
        button.setW(40);
        _buttons.push_back(std::move(button));

        xSh += 20 + button.getW();
       if (xSh >= (wi-50))
        {
            xSh = 30;
            ySh += button.getH()*2;
        }
    }

    for (size_t i = 0; i < 70; ++i) {
        GLabel button(xSh,ySh,"3." + std::to_string(i+1));
        button.setW(40);
        _buttons.push_back(std::move(button));
        xSh += 20 + button.getW();
        if (xSh >= (wi-50)) {
            xSh = 30;
            ySh += button.getH()*2;
        }
    }

    //move after first buttons add
     ySh += 50;
     xSh = 30;
     _playlistButton->setX(xSh);
     _playlistButton->setY(ySh);
     xSh += _playlistButton->getW() + 10;
     _stopPlaylist->setX(xSh);;
     _stopPlaylist->setY(ySh);
     xSh += _stopPlaylist->getW() + 10;
     _playlist2Button->setX(xSh);
     _playlist2Button->setY(ySh);


     if (_mainView->getMaster())
     {
         double scaleCoef = AConfig::getInst().getScaleCoef();
         ySh += 50;
         ySh *= scaleCoef; // YET FOR VIEWS MUST DONE REQUEST
         //REFACT
         _mainView->getMaster()->requestHeight(ySh);
     }

}


void TestsView::draw(QPainter *painter)
{
    setAllButtons();

    _upper.draw(painter);
    _bottom.draw(painter);

    _g3.draw(painter);
    _g5.draw(painter);

    _playlistButton->draw(painter);
    _playlist2Button->draw(painter);
    _stopPlaylist->draw(painter);

    for (size_t i = 0; i < _buttons.size(); ++i)
        _buttons[i].draw(painter);
}


void TestsView::ondblclick(int x1, int y1)
{
    if (_bottom.hit(x1,y1))
    {
        //fastTestAll();
        keyevent("1");
        getMaster()->pleaseRepaint();
        keyevent("2");
        getMaster()->pleaseRepaint();
        keyevent("3");
        getMaster()->pleaseRepaint();
        keyevent("4");
        getMaster()->pleaseRepaint();

    }
    if (_upper.hit(x1,y1)) {
        keyevent("playlist");
        getMaster()->pleaseRepaint();
    }
    if (_g3.hit(x1,y1)) {
        clock_t now = getTime();
        _bottom.setText("starting 1 pack");
        greatCheckScenarioCase(1,1,12,3);
        greatCheckScenarioCase(2,1,35,3);
        greatCheckScenarioCase(3,1,70,3);
        clock_t after = getTime();
        clock_t diff = after-now;
        std::string sX = "TIme spent for g3 ; - " + std::to_string(diff) + " ms";;
        _bottom.setText(sX.c_str());
    }
    if (_g5.hit(x1,y1)) {
        clock_t now = getTime();
        _bottom.setText("starting 1 pack");
        greatCheckScenarioCase(1,1,12,5);
        greatCheckScenarioCase(2,1,35,5);
        greatCheckScenarioCase(3,1,70,5);
        greatCheckScenarioCase(4,1,109,5);
        clock_t after = getTime();
        clock_t diff = after-now;
        std::string sX = "TIme spent for g3 ; - " + std::to_string(diff) + " ms";;
        _bottom.setText(sX.c_str());
    }
}


void TestsView::keyevent(std::string press)
        {
            if (press=="playlist")
            {
                //Start the playlist
                //Try from 1?
                for (size_t i = 1; i < _buttons.size(); ++i) //48- start of 3?
                {
                    std::string onText = "on:" + std::to_string(i);
                    std::vector<std::string> playlistElement;

                    playlistElement.push_back("tests");
                    playlistElement.push_back(onText.c_str());
                    playlistElement.push_back("spc");

                    getMaster()->addToPlaylist(playlistElement);
                }
                qDebug() << "Playlist configured of " << _buttons.size() << " entries ";

                getMaster()->goOnPlaylist();
            }

            if (press=="playlist2")
            {
                for (size_t i = 51; i < _buttons.size(); ++i)
                {
                    std::string onText = "on:" + std::to_string(i);
                    std::vector<std::string> playlistElement;

                    playlistElement.push_back("tests");
                    playlistElement.push_back(onText.c_str());
                    playlistElement.push_back("ent");

                    getMaster()->addToPlaylist(playlistElement);
                }

                getMaster()->goOnPlaylist();
            }

            if (press=="stop playlist")
            {
                getMaster()->cleanPlayList();
            }

            size_t separatorOn = press.find(":");

            if (separatorOn != std::string::npos)
            {
                std::string theNumber = press.substr(separatorOn+1);
                int buttonNumber = atoi(theNumber.c_str());
                openTestNumber(buttonNumber);
            }

            if (press == "1")
            {
                clock_t now = getTime();
                _bottom.setText("starting 1 pack");
                greatCheckScenarioCase(1,1,12,4);
                clock_t after = getTime();
                clock_t diff = after-now;
                std::string sX = "TIme spent for pack 1; - " + std::to_string(diff) + " ms";;
                _bottom.setText(sX.c_str());
            }
            if (press == "2")
            {
                clock_t now = getTime();

                _bottom.setText("starting 2 pack");
                greatCheckScenarioCase(2,1,35,4);
                clock_t after = getTime();
                clock_t diff = after-now;
                std::string sX = "TIme spent for pack 2; - " + std::to_string(diff) + " ms";
                _bottom.setText(sX.c_str());

            }
            if (press == "3")
            {
                clock_t now = getTime();

                _bottom.setText("starting 3 pack");
                greatCheckScenarioCase(3,1,70,4);
                _bottom.setText("pack 3 finished");
                clock_t after = getTime();
                clock_t diff = after-now;

                std::string sX = "TIme spent for pack 3; - " + std::to_string(diff) + " ms";

                _bottom.setText(sX.c_str());
            }
            if (press == "4")
            {
                clock_t now = getTime();
                _bottom.setText("starting 4 pack");
                greatCheckScenarioCase(4,1,109,4);
                clock_t after = getTime();
                clock_t diff = after-now;
                std::string sX = "TIme spent for pack 4; - " + std::to_string(diff) + " ms";
                _bottom.setText(sX.c_str());
            }
            if (press=="m")
            {
               // getMaster()->resetToFirstChild();
            }

        }



void ChordInput::setUI()
{
    if (getMaster())
    {
        getMaster()->setComboBox(0,"chordcombo1",50,20,100,40,-1); //note
        getMaster()->setComboBox(1,"chordcombo2",170,20,100,40,-1); //postfix

        getMaster()->setComboBox(2,"chordcombo3",340,20,100,40,-1); // 9 11 13 - not everywhere awailible
        getMaster()->setComboBox(3,"chordcombo4",510,20,100,40,-1); // _ +5 -5 - not everywhere

        //then only on 9 11 13

        getMaster()->setComboBox(4,"chordcombo5",340,70,100,40,-1); // _ +9 -9
        getMaster()->setComboBox(5,"chordcombo6",510,70,100,40,-1); // _ +11 -11
        getMaster()->setComboBox(6,"chordcombo7",660,70,100,40,-1); // _ + -

        getMaster()->SetButton(7,"back",  600, 20, 70,  30, "esc"); //refact

    }
}


void ChangesInput::setUI()
{
    if (getMaster())
    {
        //getMaster()->setComboBox(0,"changecombo1",50,20,150,40,-1); //change type
        getMaster()->setComboBox(0,"instruments",220,20,150,40,-1);
        getMaster()->setComboBox(1,"changecombo3",390,20,150,40,-1);
        getMaster()->setComboBox(2,"changecombo4",550,20,50,40,-1);

        getMaster()->setComboBox(3,"bpm",220,70,150,40,-1);
        getMaster()->setComboBox(4,"changecombo3",390,70,150,40,-1);
        getMaster()->setComboBox(5,"changecombo4",550,70,50,40,-1);

        getMaster()->setComboBox(6,"volume",220,120,150,40,-1);
        getMaster()->setComboBox(7,"changecombo3",390,120,150,40,-1);
        getMaster()->setComboBox(8,"changecombo4",550,120,50,40,-1);

        getMaster()->setComboBox(9,"pan",220,170,150,40,-1);
        getMaster()->setComboBox(10,"changecombo3",390,170,150,40,-1);
        getMaster()->setComboBox(11,"changecombo4",550,170,50,40,-1);

        getMaster()->SetButton(12,"back", 600, 20, 70,  30, "esc"); //refact

    }
}


void BendInput::setUI()
{
    if (getMaster())
    {
        getMaster()->setComboBox(0,"bendcombo1",50,20,150,40,-1);
        getMaster()->setComboBox(1,"bendcombo2",350,20,150,40,-1);
    }
}

BendInput::BendInput()
{
    _top = std::make_unique<GLabel>(10,10,"Bend input","",false);
    _okButton = std::make_unique<GLabel>(530,35,"OK");
    _delButton = std::make_unique<GLabel>(630,35,"Delete");
}
