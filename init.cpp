#include "init.h"

#include <iostream>
#include <signal.h>
#include <fstream>

#include "g0/config.h"
#include "ui/imagepreloader.h"
#include "g0/regression.h"
#include "tab/tabloader.h"
#include "midi/midiengine.h"

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QDateTime>

#include <QStandardPaths>
#include <QDir>
#include <QApplication>

#include <QDebug>

#ifdef WIN32
#include <windows.h>
#endif


void posix_death_signal(int signum) //TODO move whole init actions into another file
{
    std::cerr << "Crash happend signal:"<<signum;
    signal(signum, SIG_DFL);


    std::string logName = AConfig::getInst().testsLocation + std::string("log.txt");


    QFile logF;
    logF.setFileName(logName.c_str());
    logF.open(QIODevice::ReadOnly);

    QByteArray logWhole = logF.readAll();
    QByteArray logCompressed = qCompress(logWhole,9);

    std::cerr << "Compressed log size "<<logCompressed.size();

    QFile crashLog;


    QDateTime timeDate(QDateTime::currentDateTime());
    QString st = timeDate.toString(Qt::ISODate);
    st.replace(":","-");
    std::string time = st.toStdString();
    //time = time.substr(time.find("T")+1);

    std::cout << std::endl << time.c_str() << std::endl;

    QString baseLocation = AConfig::getInst().testsLocation.c_str();
    QString crashName = baseLocation + QString("crashs.glog");
    crashLog.setFileName(crashName);

    crashLog.open(QIODevice::Append);

    crashLog.write(time.c_str());
    crashLog.write(logCompressed);
    crashLog.close();

    if (CONF_PARAM("addRootCrashes")=="1")
    {
        QFile crashTextLog;
        QString textCrashFn;

#ifdef __ANDROID_API__
        textCrashFn = "/sdcard/glog.C";
#else
        textCrashFn = "glog.C";
#endif
        QDateTime timeDate(QDateTime::currentDateTime());
        QString st = timeDate.toString(Qt::ISODate);
        st.replace(":","-");

        textCrashFn+=st;
        textCrashFn+=QString(".txt");

        crashTextLog.setFileName(textCrashFn);
        crashTextLog.open(QIODevice::WriteOnly);
        crashTextLog.write(logWhole);
        crashTextLog.close();


    }

    exit(3);
}

QString getUserID(){
    QString text;
    //TODO анонимный айди
    return text;
}


int sayType(QByteArray &file)
{
    //plag imorter

    if (file[0]=='G')
    {
        if (file[1]=='A') return 1; //incomplete check



    }
    else if (file[0]=='p')
    {

        if (file[1]=='t')
            if (file[2]=='a')
               if (file[3]=='b') return 2;
    }
    else if (file[1]=='F')
    {
        if (file[2]=='I')
            if (file[3]=='C')
            {
                char v = file[21];
                //char subV = file[23];
                if (v==51) return 3;
                if (v==52) return 4;
                if (v==53) return 5;
            }
    }



    return -1;
}


void setLibPath() {
#ifdef WIN32
    QStringList libPath = QCoreApplication::libraryPaths();

    libPath.append(".");
    libPath.append("platforms");
    libPath.append("imageformats");

    QCoreApplication::setLibraryPaths(libPath);
#endif
}


void initResourses() {

    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(icons2);
    Q_INIT_RESOURCE(tests);
    Q_INIT_RESOURCE(info);
}


void setTestsPath(QApplication& a) {
    std::string currentPath;
    QString qPath = a.applicationDirPath();
    std::string appPath = qPath.toStdString();
    appPath += std::string("/tests/");
    QString pathStd = QStandardPaths::writableLocation(QStandardPaths::QStandardPaths::ApplicationsLocation); //AppDataLocation);
    currentPath =  pathStd.toStdString(); //"/sdcard/p";
    QDir dir;
    QString dirPath = currentPath.c_str();
    dir.mkdir(dirPath);
    currentPath += std::string("/");
    //SET PATH
    setTestLocation(currentPath);
}

void loadConfig() {
    std::string currentPath = AConfig::getInst().testsLocation;
    AConfig& configuration = AConfig::getInst();
    configuration.checkConfig();
    std::string confFileName = currentPath + "g.config";
    if (QFile::exists(confFileName.c_str())) {
        std::ifstream confFile(confFileName);
        if (confFile.is_open())
            configuration.load(confFile);
    }
    configuration.printValues();
}

void preloadImages() {
    ImagePreloader& imageLoader = ImagePreloader::getInstance();
    imageLoader.loadImages();
}

void setPosixSignals() {
    signal(SIGSEGV, posix_death_signal);
    signal(SIGILL, posix_death_signal);
    signal(SIGFPE, posix_death_signal);
}

void initMidi() {
    if (AConfig::getInst().platform == "windows")
        MidiEngine midInit;
}

void initMainWindow(MainWindow& w, QApplication& a) {
    a.setApplicationVersion("Guitarmy v 0.5 final");
    a.setOrganizationName("KK");
    if (AConfig::getInst().isMobile == false) //for Desktops
        w.setGeometry(30,30,800,480);
    //block2
    static auto mainViewLayer1 = std::make_unique<MainView>(); //Sorry for this layers, MainView and CenterView are not used to be supported furter
    mainViewLayer1->setMaster(w.getCenterView());           //This is abadoned part of a project, that would be fully replaces with QML
    w.getCenterView()->changeChild(mainViewLayer1.get());
    w.getCenterView()->setStatusSetter(&w);
    static auto mainViewLayer2 = std::make_unique<MainView>();
    mainViewLayer2->setDependent();
    static auto centerView = std::make_unique<CenterView>(w.getCenterView());
    w.getCenterView()->ownChild = centerView.get();
    w.getCenterView()->ownChild ->changeChild(mainViewLayer2.get());
    w.getCenterView()->ownChild->setStatusSetter(w.getCenterView());
    //block 3
    w.setAttribute(Qt::WA_AcceptTouchEvents);
    w.grabGesture(Qt::SwipeGesture);
    w.grabGesture(Qt::TapAndHoldGesture);
    w.grabGesture(Qt::TapGesture);
    w.grabGesture(Qt::PanGesture);
    w.grabGesture(Qt::PinchGesture);
}


void setWindowIcon(MainWindow& w) {
    std::string currentPath = AConfig::getInst().testsLocation;
    std::string winIconName = currentPath + "Icons/winIcon.png";
    QIcon winIcon(winIconName.c_str());
    w.setWindowIcon(winIcon);
}


void setLogFilename() {
    std::string logName = AConfig::getInst().testsLocation + std::string("log.txt"); //TODO log init
    //TODO add init logger form gtab3

    if (AConfig::getInst().platform == "android")
        if (CONF_PARAM("sdcardLogDebug")=="1") //very temp
            logName = "/sdcard/Guitarmy.log";
    //TODO log hanler for gtab3
}


void configureScreen(MainWindow& w) {
    ////and_help.setLandscape();
    if (AConfig::getInst().platform == "android") {
        w.show();
        if (CONF_PARAM("fullscreen")=="1")
            w.showFullScreen();
    }
    else
        w.show();
}


void initNewTab(MainWindow& w) {
    w.setWindowTitle("Guitarmy");
    w.getCenterView()->pushForceKey("newtab");

    if (CONF_PARAM("skipTabView")=="1")
        w.getCenterView()->pushForceKey("opentrack");
}




void runRegressionTests() {
    //greatCheckScenarioCase(1, 1, 12, 4);
    //greatCheckScenarioCase(2, 1, 38, 4);
    if (checkHasRegression()) {
        qDebug() << "Has regression, terminating";
        return;
    }
    else
        qDebug() << "Has no regression";

    //TODO midi read write test (streaming operator check)

    {
        Tab t;
        t.onTabCommand(TabCommand::NewTrack);
        t.onTabCommand(TabCommand::Solo);
        {
            std::ofstream os("/home/punnalyse/dev/g/_wgtab/gtab/og/macro", std::ios::binary);
            saveMacroComannds(t.getMacro(), os);
            qDebug() << t.getMacro().size() << " commands written";
        }
        {
            Tab t2;
            std::ifstream is("/home/punnalyse/dev/g/_wgtab/gtab/og/macro", std::ios::binary);
            auto commands = loadMacroCommands(is);
            for (auto& c: commands)
                t2.playCommand(c);
            qDebug() << commands.size() << " commands read";
            if (t2.at(0)->getStatus() != 2) {
                qDebug() << "ERROR: Tab commands failed!";
            }
            else
                qDebug() << "Simple tab commands fine";
        }
    }
}

