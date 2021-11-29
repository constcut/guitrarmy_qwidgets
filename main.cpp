#include "ui/mainwindow.h"
#include <QApplication>

#include "ui/mainviews.h"

#include "g0/config.h"

#include "tab/tabclipboard.h"

#include <QTextCodec>
//#include <QSettings>

#include <QMediaPlayer>
#include <QAudioFormat>

#include "g0/wavefile.h"

#include <QStandardPaths>
#include <QScreen>
#include <QDir>

#include "android_helper.h"


#include <signal.h>

#include "qthelper.h"

#include <QNetworkInterface>
#include <QDateTime>
//DATE N TIME TO QT HELPER

#ifdef WIN32
#include <windows.h>
#endif

#include "midi/midiengine.h"
#include "tab/tabloader.h"

#include <QDebug>

#include "midi/midirender.h"



//OMGC its a nightmare

void posix_death_signal(int signum) //TODO move whole init actions into another file
{
    std::cerr << "Crash happend signal:"<<signum;
    signal(signum, SIG_DFL);


    std::string logName = getTestsLocation() + std::string("log.txt");


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

    QString baseLocation = getTestsLocation();

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
    /*
    for(NetworkInterface interface=QNetworkInterface::allInterfaces().first();
        interface != )
    {
        text += interface.hardwareAddress();
    }
    */
    //mac id broken
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






int main(int argc, char *argv[])
{

#ifdef WIN32
    QStringList libPath = QCoreApplication::libraryPaths();

    libPath.append(".");
    libPath.append("platforms");
    libPath.append("imageformats");

    QCoreApplication::setLibraryPaths(libPath);
#endif

    QApplication::setStyle("fusion");

    QApplication a(argc, argv);

    getTime();
    initGlobals();
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(icons2);
    Q_INIT_RESOURCE(tests);
    Q_INIT_RESOURCE(info);

    //PREPARE USER ID
    //QString userId = getUserID();
    //userId.replace(":","");
    //userId = userId.mid(0,7);

    //KOI8-R
    //UTF-8
    //Windows-1251
    //ISO 8859-5
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251"));

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


    AConfig configuration;
    configuration.checkConfig();
    std::string confFileName = currentPath + "g.config";
    if (QFile::exists(confFileName.c_str())) {
        std::ifstream confFile(confFileName);
        if (confFile.is_open())
            configuration.load(confFile);
    }

    configuration.printValues();
    AConfig::setInstance(&configuration);
    configuration.imageLoader.loadImages();


    MainWindow w;
    a.setApplicationVersion(getGuitarmyVersion());
    a.setOrganizationName("Guitarmy union");

    if (globals.isMobile == false) //for Desktops
        w.setGeometry(30,30,800,480);

    if (globals.platform == "windows")
        MidiEngine midInit;

    MainView *mw = new MainView();
    mw->setMaster(w.getCenterView());
    w.getCenterView()->changeChild(mw);
    w.getCenterView()->setStatusSetter(&w);

    MainView *m2 = new MainView();
    m2->setSlave();
    w.getCenterView()->ownChild = new CenterView(w.getCenterView());
    w.getCenterView()->ownChild ->changeChild(m2);

    w.getCenterView()->ownChild->setStatusSetter(w.getCenterView());

    std::string winIconName = currentPath + "Icons/winIcon.png";
    QIcon winIcon(winIconName.c_str());
    w.setWindowIcon(winIcon);

    std::string logName = getTestsLocation() + std::string("log.txt"); //TODO log init
    //TODO add init logger form gtab3

    if (globals.platform == "android")
        if (CONF_PARAM("sdcardLogDebug")=="1") //very temp
            logName = "/sdcard/Guitarmy.log";




    w.setAttribute(Qt::WA_AcceptTouchEvents);
    w.grabGesture(Qt::SwipeGesture);
    w.grabGesture(Qt::TapAndHoldGesture);
    w.grabGesture(Qt::TapGesture);
    w.grabGesture(Qt::PanGesture);
    w.grabGesture(Qt::PinchGesture);

    if (globals.platform == "android") {
        w.show();
        if (CONF_PARAM("fullscreen")=="1")
            w.showFullScreen();
    }
    else
        w.show();
    ///checkBase(); //if param set - moved to center view
    //for the self chosen rotation
    //android_helper and_help;
    //and_help.setLandscape();
    w.setWindowTitle(getGuitarmyVersion());
    signal(SIGSEGV, posix_death_signal);
    signal(SIGILL, posix_death_signal);
    signal(SIGFPE, posix_death_signal);
    //w.getCenterView()->addComboBox("ohhsome",50,50,70,30);
    w.getCenterView()->pushForceKey("newtab");
    //because now we debug recording
    ///w.getCenterView()->pushForceKey("rec");
    if (CONF_PARAM("skipTabView")=="1")
        w.getCenterView()->pushForceKey("opentrack");
    //TODO  отправка краш логов?       QString crashName = getTestsLocation() + QString("crashs.glog");
                //if (QFile::exists(crashName) {
    //TODO окно для загрузки разных тестов
    //testScenario();


    int out = 0;
    try{
       out = a.exec();
       //__except (EXCEPTION_EXECUTE_HANDLER) // __try{}  __finally
    }
    catch(...){
        qDebug() << "Exception thrown";
        //crash log saver
    }

    delete mw;
    qDebug() << "Main function done" ;
    std::cout << "Main end reached"<<std::endl;


    return out;
}
