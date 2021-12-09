#include <QApplication>
#include <QTextCodec>

#include "ui/mainwindow.h"
#include "g0/config.h"
#include "init.h"

#include <QTemporaryDir>
#include <iostream>



int main(int argc, char *argv[])
{
    setLibPath();
    QApplication::setStyle("fusion");
    QApplication a(argc, argv);
    initResourses();

    initGlobals();
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251")); //TODO configuration

    loadConfig();
    preloadImages();

    MainWindow w;
    initMainWindow(w, a);
    initMidi(); //Раньше было посередие initMainWindow
    setWindowIcon(w);
    configureScreen(w);
    setPosixSignals();
    initNewTab(w);
    setLogFilename();


    QTemporaryDir tempDir;
    setTestLocation(tempDir.path().toStdString() + "/");
    std::unordered_map<size_t, size_t> groupLength = {
        {1, 12},
        {2, 39},
        {3, 70},
        {4, 109}
    };
    for (size_t groupIdx = 1; groupIdx <= 4; ++groupIdx) {
        size_t from = 1;
        size_t to = groupLength[groupIdx] - 1;
        for (size_t fileIndx = from; fileIndx <= to; ++fileIndx) {
            std::string testName = std::to_string(groupIdx) + "." + std::to_string(fileIndx);
            QString resourse = QString(":/own_tests/") + testName.c_str() + ".gp4";
            QString copy = tempDir.path() + "/" + testName.c_str() + ".gp4";
            QFile::copy(resourse, copy);
        }
    }

    runRegressionTests();
    std::cout << "TEST LOCATION " << AConfig::getInst().testsLocation << std::endl;

    int out = 0;
    try{
       out = a.exec();
    }
    catch(...){
        qDebug() << "Exception thrown"; //TODO save logs into file
    }
    qDebug() << "Main function done" ;
    return out;
}
