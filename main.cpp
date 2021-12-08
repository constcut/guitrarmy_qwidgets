#include <QApplication>
#include <QTextCodec>

#include "ui/mainwindow.h"
#include "g0/config.h"
#include "init.h"



int main(int argc, char *argv[])
{
    setLibPath();
    QApplication::setStyle("fusion");
    QApplication a(argc, argv);
    initResourses();
    initGlobals();
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251")); //TODO configuration
    setTestsPath(a);
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
    runRegressionTests();

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
