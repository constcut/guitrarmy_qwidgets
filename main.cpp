#include "g0/Init.hpp"

#include <QApplication>
#include <QTextCodec>


#include "g0/Config.hpp"
#include "g0/Regression.hpp"
#include "g0/Base.hpp"
#include "ui/MainWindow.hpp"


using namespace gtmy;


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
    auto mainViewLayer1 = std::make_unique<MainView>(); //Sorry for this layers, MainView and CenterView are not used to be supported furter
    mainViewLayer1->setMaster(w.getCenterView());           //This is abadoned part of a project, that would be fully replaces with QML
    w.getCenterView()->changeChild(mainViewLayer1.get());
    w.getCenterView()->setStatusSetter(&w);
    auto mainViewLayer2 = std::make_unique<MainView>();
    mainViewLayer2->setDependent();
    auto centerView = std::make_unique<CenterView>(w.getCenterView());
    w.getCenterView()->ownChild = centerView.get();
    w.getCenterView()->ownChild ->changeChild(mainViewLayer2.get());
    w.getCenterView()->ownChild->setStatusSetter(w.getCenterView());

    setWindowIcon(w);
    configureScreen(w);
    setPosixSignals();
    initNewTab(w);
    setLogFilename();

    copyResourcesIntoTempDir();
    runRegressionTests();
    checkMidiIORegression();

    //BaseStatistics base; //Later sepparate into another project
    //base.start("/home/punnalyse/dev/g/base/", 166000); //166000

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
