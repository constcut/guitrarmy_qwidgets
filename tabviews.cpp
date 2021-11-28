#include "tabviews.h"

#include "g0/tab.h"
//#include "g0/gtpfiles.h"
#include "g0/midifile.h"

#include "g0/aconfig.h"

#include "g0/gmyfile.h"
#include "g0/aexpimp.h" //loader

#include "athread.h"

#include "mainviews.h"

#include <QInputDialog>
#include <QMessageBox>

#include "midiengine.h"

#include <QDebug>




int translateDefaulColor(const std::string& confParam){
    int numColor = -1;
    if (confParam == "black") numColor = 0;
    if (confParam == "red") numColor = 1;
    if (confParam == "yellow") numColor = 2;
    if (confParam == "orange") numColor = 3;
    if (confParam == "green") numColor = 4;
    if (confParam == "blue") numColor = 5;
    if (confParam == "darkblue") numColor = 6;
    if (confParam == "violet") numColor = 7;
    if (confParam == "white") numColor = 8;
    if (confParam == "gray") numColor = 9;
    if (confParam == "lightgray") numColor = 10;
    if (confParam == "darkgray") numColor = 11;
    if (confParam == "cyan") numColor = 12;
    if (confParam == "mageta") numColor = 13;
    if (confParam == "darkgreen") numColor = 14;
    if (confParam == "darkcyan") numColor = 15;
    if (confParam == "darkred") numColor = 16;
    return numColor;
}

void changeColor(const std::string& color, QPainter* src)
{
    //saveColor();
    int colorS = translateDefaulColor(color);
    switch (colorS)
    {
        case 0: src->setPen(Qt::black); break;
        case 1: src->setPen(Qt::red); break;
        case 2: src->setPen(Qt::yellow); break; //orange
        case 3: src->setPen(Qt::darkYellow); break;
        case 4: src->setPen(Qt::green); break;
        case 5: src->setPen(Qt::blue); break;
        case 6: src->setPen(Qt::darkBlue); break;
        case 7: src->setPen(Qt::magenta); break; //violet
        case 8: src->setPen(Qt::white); break;
        case 9: src->setPen(Qt::gray); break;
        case 10: src->setPen(Qt::lightGray); break;
        case 11: src->setPen(Qt::darkGray); break;
        case 12: src->setPen(Qt::cyan); break;
        case 13: src->setPen(Qt::darkMagenta); break;
        case 14: src->setPen(Qt::darkGreen); break;
        case 15: src->setPen(Qt::darkCyan); break;
        case 16: src->setPen(Qt::darkRed); break;

    }
    //storeValue = colorS;
}

void drawEllipse(QColor c, QPainter *painter, int x, int y, int w, int h) {
    QBrush startBra = painter->brush();
    painter->setBrush(c);
    painter->drawEllipse(x,y,w,h);
    painter->setBrush(startBra); //return brach back
}

void drawEllipse(QPainter *painter, int x, int y, int w, int h) {
    painter->drawEllipse(x,y,w,h);
}



int scaleCoef = 1;


//Tab view
TabView::TabView():GView(0,0,w,h),pTab(0),localThr(0)
{
    statusLabel = new GLabel(50,460,"file was loaded.");
    bpmLabel = new GLabel(300,460,"bpm=notsetyet");

    pan = new GTabPannel(300,480,800);
    pan->setPressView(this);
}

void TabView::addSingleTrack(Track *track)
{
    TrackView *tV = new TrackView(track);
    tV->setPa(this);

    int wSet = getMaster()->getWidth();
    int hSet = getMaster()->getHeight();

    tV->setH(hSet);
    tV->setW(wSet);

    tracksView.push_back(tV);
}

void TabView::setUI()
{
    if (getMaster())
    {
        getMaster()->SetButton(0,"back to track",630,20,100,15,"opentrack");
    }
}

void TabView::setTab(Tab* point2Tab)
{
    //refact note - list of deleted pTabs

    pTab = point2Tab;

    MidiEngine::stopDefaultFile();
    stopAllThreads();
    setPlaying(false);

    if (tracksView.empty() == false)
        for (ul i = 0; i <tracksView.size(); ++i)
            delete tracksView[i];

    tracksView.clear();

    if (pTab) {
        pTab->getDisplayBar() = 0;
        pTab->getDisplayTrack() = 0;
        pTab->getCurrentTrack() = 0;
        pTab->getCurrentBar() = 0;
        bpmLabel->setText("bpm=" + std::to_string(pTab->getBPM()));
        for (ul i = 0; i < pTab->size(); ++i)
            addSingleTrack(pTab->at(i));
        std::string statusBar1,statusBar2;
        statusBar1 = "Tab Name";
        statusBar2 = "BPM = " + std::to_string(pTab->getBPM());
        getMaster()->setStatusBarMessage(1,statusBar1.c_str());
        getMaster()->setStatusBarMessage(2,statusBar2.c_str());
        getMaster()->setStatusBarMessage(0,"Tab was loaded",500);
    }

}


void TabView::onclick(int x1, int y1)
{
    if (pTab==0)
        return;
    if (bpmLabel->hit(x1,y1)) {
        //if (pan->isOpenned()) return;
        //keyevent("bpm");
        //return;
    }
    if (pan->hit(x1,y1)){
        pan->onclick(x1,y1);
        return;
    }

    size_t awaitBar = (x1-200)/30;
    int toolBarHeight = getMaster()->getToolBarHeight();
    size_t awaitTrack = (y1-toolBarHeight)/30;
    awaitTrack-=1;

    if (awaitBar >= 0){
        awaitBar += pTab->getDisplayBar();
        if (awaitBar==pTab->getCurrentBar()){
            qDebug() << "Track pressed "<<awaitTrack<<"; Bar "<<awaitBar;
            if (awaitTrack >= 0){
                if (awaitTrack < pTab->size()) {
                    TrackView *trackView = tracksView[awaitTrack];
                    pTab->getLastOpenedTrack() = awaitTrack;
                    trackView->setDisplayBar(awaitBar);
                    MainView *mainView = (MainView*)getMaster()->getFirstChild();
                    mainView->changeCurrentView(trackView);
                    std::string statusBar1,statusBar2;
                    statusBar1 = pTab->at(awaitTrack)->getName();
                    statusBar2 = "bar " + std::to_string( pTab->getCurrentBar() );
                    getMaster()->setStatusBarMessage(1,statusBar1.c_str());
                    getMaster()->setStatusBarMessage(2,statusBar2.c_str());
                }
            }
        }
        else
            if (awaitBar < pTab->at(0)->size()) {
                pTab->getCurrentBar() = awaitBar;
                ul chosenTrack = pTab->getDisplayTrack() + awaitTrack;
                if ((awaitTrack>=0) && (chosenTrack < pTab->size()))
                        pTab->getCurrentTrack() = chosenTrack;
            }
    }
    else {
        ++awaitTrack;
        ul chosenTrack = pTab->getDisplayTrack() + awaitTrack;
        if ((awaitTrack>=0) && (chosenTrack < pTab->size()))
                pTab->getCurrentTrack() = chosenTrack;
    }
}


void TabView::ondblclick(int x1, int y1) {
   onclick(x1,y1);
}


void TabView::ongesture(int offset, bool horizontal) {
    int quant = offset/25;
    if (horizontal) {
        size_t nextCursor = quant + pTab->getDisplayBar();
        if (nextCursor<0)
            nextCursor=0;
        else
            if (nextCursor > pTab->at(0)->size())
                nextCursor = pTab->at(0)->size()-1;
        pTab->getDisplayBar() = nextCursor;
    }
    else {
        if (quant >= 2)
            keyevent("^^^"); //TODO with command
        else
            if (quant <= -2)
                keyevent("vvv");
    }
}


void TabView::draw(QPainter *painter)
{
    //statusLabel->draw(painter);
    //bpmLabel->draw(painter);

   if (pTab != 0)
    {
        if (pTab->playing())
            pTab->getDisplayBar()=pTab->getCurrentBar();

        int yLimit = getMaster()->getHeight();
        int xLimit = getMaster()->getWidth();

        //TrackView should be agregated
        for (ul i = 0 ; i < pTab->size(); ++i)
        {
           ul trackIndex = i + pTab->getDisplayTrack();
           if (trackIndex >= pTab->size())
            break;
           std::string trackVal = std::to_string(trackIndex+1) +" " + pTab->at(trackIndex)->getName();
           //int pannelShift = getMaster()->getToolBarHeight();
           int yPos = (i+1)*30; //pannelShift+(i+2)*30;
           if (yPos > (yLimit-100))
               break;
           if (trackIndex == pTab->getCurrentTrack())
                changeColor(CONF_PARAM("colors.curTrack"), painter);

           painter->drawText(20,yPos,trackVal.c_str());
           //painter->drawEllipse(10,yPos,5,5);
           painter->drawRect(7,yPos-10,10,10);

           if (trackIndex == pTab->getCurrentTrack())
               changeColor(CONF_PARAM("colors.default"), painter);

           byte trackStat = pTab->at(trackIndex)->getStatus();
           if (trackStat==1)
            painter->drawText(9,yPos+3,"m");
           else
               if (trackStat==2)
                painter->drawText(9,yPos+3,"s");


           Track *tr = pTab->at(trackIndex);
           for (ul j = 0 ; j < tr->size(); ++j)
           {
               ul barIndex = j + pTab->getDisplayBar();
               if (barIndex >= tr->size())
                    break;

               std::string sX = std::to_string(barIndex+1);
               Bar *cB= tr->at(barIndex);
               if (cB->size() == 1)
               {
                   Beat *beat = cB->at(0);
                   Note *note = 0;
                   if (beat->size())
                    note = beat->at(0);
                   if (note == 0)
                      sX+="*";
               }

               int reprize = cB->getRepeat();
               std::string markerText;
               ul markerColor;
               cB->getGPCOMPMarker(markerText,markerColor);
               bool isMarkerHere = markerText.empty() == false;

                //hi light color bar
               if (barIndex == pTab->getCurrentBar())
                   painter->fillRect(200+30*j,yPos,20,20,QColor(CONF_PARAM("colors.curBar").c_str()));

               painter->drawText(200+30*j,yPos+10,sX.c_str());
               if (i==0)  {
                   if (reprize) {
                       std::string rep;
                       if (reprize == 1)
                           rep = "|:";
                       if (reprize == 2)
                       {
                           byte repTimes = cB->getRepeatTimes();
                           if (repTimes != 2)
                               rep += std::to_string(repTimes);

                           rep += ":|";
                       }
                       if (reprize == 3)
                           rep = ":|:";

                       painter->drawText(200+30*j,80-60,rep.c_str());
                   }

                   if (isMarkerHere){
                        painter->drawText(200+30*j,70-60,markerText.c_str());
                   }
               }
                painter->drawRect(200+30*j,yPos,20,20);

                if ((200+j*30) > (xLimit-100)) //800 border
                    break;
           }

           std::string sX = std::to_string(tr->size());
           int border = getMaster()->getWidth()-20;
           painter->drawText(border,10+yPos,sX.c_str());

           sX.clear();
           byte vol =  tr->getVolume();//pTab->GpCompMidiChannels[port*chan].volume; //tr->getVolume();
           sX = "vol " + std::to_string(vol);
           painter->drawText(70,10+yPos,sX.c_str());

           sX.clear();
           byte pan =  tr->getPan();//pTab->GpCompMidiChannels[port*chan].balance;//tr->getPan();
           int intPan = pan - 7;
           sX = "pan " + std::to_string(intPan);
           painter->drawText(110,10+yPos,sX.c_str());

           sX.clear();
           byte ins =  tr->getInstrument();//pTab->GpCompMidiChannels[port*chan].instrument;//tr->getPan();

           if (tr->isDrums() == false)
            sX = std::to_string(ins) + "i";
           else
               sX ="d" + std::to_string(ins);

           painter->drawText(170,10+yPos,sX.c_str()); //mute or solo
        }
    }
    pan->draw(painter);
}


void TabView::prepareAllThreads(ul shiftTheCursor)
{
    for (ul i = 0; i <tracksView.size(); ++i)
    {
        tracksView[i]->prepareThread(shiftTheCursor);
        int thrLenSeconds = tracksView[i]->threadSeconds();
        qDebug() << "Thread "<<i<<" seconds "<<thrLenSeconds;
    }

    Track *pTrack = pTab->at(0);

    if (localThr)
    {
        localThr->requestStop();
        localThr->terminate();
        if (CONF_PARAM("crashOnPlayHotFix") != "1")
        localThr->deleteLater();
        //delete localThr;
    }

    localThr = new ThreadLocal;

    localThr->setInc(&pTab->getCurrentBar(), nullptr); //oh shhhi 2nd arg
    localThr->setBPM(pTab->getBPM());

    ul timeLoopLen = pTrack->timeLoop.size();
    for (ul i = shiftTheCursor; i < timeLoopLen;++i)
    {
        localThr->addNumDenum(pTrack->timeLoop.at(i)->getSignNum(),
        pTrack->timeLoop.at(i)->getSignDenum(), pTrack->timeLoopIndexStore[i]);
    }
    localThr->setLimit(pTrack->timeLoop.size());

    qDebug() << "All threads prepared";
}

void TabView::launchAllThreads()
{


    for (ul i = 0; i <tracksView.size(); ++i)
        tracksView[i]->launchThread();

    //return;

    if (localThr)
        localThr->start();
}

void TabView::stopAllThreads()
{
    for (ul i = 0; i <tracksView.size(); ++i)
        tracksView[i]->stopThread();

    if (localThr)
    {
        localThr->terminate();
#ifdef __ANDROID_API__
         localThr->requestStop();
#endif
    }
}

void TabView::connectAllThreadsSignal(MasterView *masterView)
{
    for (ul i = 0; i <tracksView.size(); ++i)
        tracksView[i]->connectThreadSignal(masterView);

   // masterView->connectThread(localThr);

   masterView->connectMainThread(localThr);
}


void TrackView::launchThread()
{
    if (localThr)
    localThr->start();
}

void TrackView::stopThread()
{
    if (localThr)
    {
        localThr->terminate();

#ifdef __ANDROID_API__
      localThr->requestStop();
#endif
    }
}

void TrackView::connectThreadSignal(MasterView *masterView)
{
    masterView->connectThread(localThr);
}






void TrackView::ongesture(int offset, bool horizontal)
{
    if (horizontal){
        //x offset
        /* forbiden for a while
        int restOffset = offset;
        while (restOffset != 0)
        restOffset = horizonMove(restOffset);
        */
        //there could be selection for
    }
    else{
        size_t& cursor = pTrack->cursor();
        size_t& displayIndex = pTrack->displayIndex();
        //y offset
        if (offset < 0) {
            /*
            int quant = offset/-80;
                displayIndex = cursor += quant;
            cursorBeat=0;
            if (displayIndex > pTrack->size())
                cursor = displayIndex = pTrack->size()-1;
            */

            int absOffset = -1*offset;
            int shiftTo = 0;
            int curY = barsPull[0].getY();

            while (absOffset>0) {
                int barY = barsPull[shiftTo].getY();
                if (barY > curY) {
                    absOffset -= (barY-curY);
                    curY=barY;
                    if (absOffset < barsPull[0].getH()) {
                        //--shiftTo;
                        break;
                    }
                }
                ++shiftTo;
            }


            ul trackLen = pTrack->size();
            displayIndex = cursor  +=shiftTo;

            if (trackLen <= displayIndex){

                displayIndex = trackLen;
                if (trackLen)
                    --displayIndex;

                cursor=displayIndex;
            }
            qDebug()<<"Shifting to "<<shiftTo;
        }
        else {
            size_t& cursorBeat = pTrack->cursorBeat();
            size_t quant = offset/80;
            if (cursor > quant)
                displayIndex = cursor -= quant;
            else
                displayIndex = cursor = 0;
            cursorBeat=0;

        }
        tabParrent->setCurrentBar(cursor);
        //verticalMove - same way but skips whole line - height is always the same
    }
}

int TrackView::horizonMove(int offset)
{
    //index 0 only for first iteration

    GView *bar = &(barsPull.at(0)); //over display index
    BarView *bV = (BarView*)bar;

    int absOffset = offset > 0? offset: offset*-1;
    int rest = offset;

    if (absOffset > bV->getW()){

        size_t& displayIndex = pTrack->displayIndex();
        size_t& lastSeen = pTrack->lastSeen();

            if (offset > 0){
                if ((displayIndex+1) < (lastSeen-1))
                    ++displayIndex;
                rest -= bV->getW();
            }
            else {
                if (displayIndex > 0)
                    --displayIndex;
                rest += bV->getW();
            }
       return rest;
    }

    return 0;
}

void TrackView::onclick(int x1, int y1)
{
    if (pan->hit(x1,y1))
    {
        pan->onclick(x1,y1);
        //aware of open pannel
        return;
    }

    if (tabParrent->getPlaying())
    {
        return; //skip
    }

    size_t& cursor = pTrack->cursor();
    size_t& cursorBeat = pTrack->cursorBeat();
    size_t& stringCursor = pTrack->stringCursor();
    size_t& displayIndex = pTrack->displayIndex();

    //touch and mouse events on first note
    for (ul i = 0; i < barsPull.size(); ++i)
    {
        /*
        log << "Bar "<<i<<" "<<barsPull.getV(i).getX()<<
               " "<<barsPull.getV(i).getY()<<" "<<
               " "<<barsPull.getV(i).getW()<<" "<<
               " "<<barsPull.getV(i).getH()<<"; hits-"<<
               (int)(barsPull.getV(i).hit(x1,y1));
               */

        if (barsPull.at(i).hit(x1,y1))
        {
            GView *bar = &(barsPull.at(i));
            BarView *bV = (BarView*)bar; //it must be that way i know it
            //may be refact to make Poly<BarView>

            ul realIndex = i + displayIndex;
            //set cursor from press
            cursor = realIndex;

            int beatClick = bV->getClickBeat(x1);
            int stringClick = bV->getClickString(y1);

            int stringUpperBarrier = pTrack->tuning.getStringsAmount();

            //++beatClick;
            if (beatClick >= pTrack->at(cursor)->size())
                --beatClick;

            //log<<"beat click "<<beatClick<<"; stringClick "<<stringClick;
            //log<<"log";

            if (beatClick>0)
                cursorBeat = beatClick;
            else
                cursorBeat = 0;

            if ((stringClick >= 0) && (stringClick < stringUpperBarrier))
                stringCursor = stringClick;

            pTrack->digitPress() = -1;

            tabParrent->setCurrentBar(cursor);
            //getMaster()->pleaseRepaint();
        }
    }



    //log << "Press "<<x1<<" "<<y1;
}

void TrackView::ondblclick(int x1, int y1)
{
    bool wasPressed = false;
    for (ul i = 0; i < barsPull.size(); ++i)
    {
        /*
        log << "Bar "<<i<<" "<<barsPull.getV(i).getX()<<
               " "<<barsPull.getV(i).getY()<<" "<<
               " "<<barsPull.getV(i).getW()<<" "<<
               " "<<barsPull.getV(i).getH()<<"; hits-"<<
               (int)(barsPull.getV(i).hit(x1,y1));
               */

        if (barsPull.at(i).hit(x1,y1))
        {
            BarView *bar = &(barsPull.at(i));
            BarView *bV = bar; //(dynamic_cast<BarView*>(bar)); //it must be that way i know it
            //may be refact to make Poly<BarView>

            if (bV == 0) continue;

            int beatClick = bV->getClickBeat(x1);
            Bar *hitBar = bV->getBar();
            int fullBar = bV->getBarsize();


            if (fullBar <= beatClick)
                continue;

            qDebug() << "Bar hits "<<beatClick<<" of "<<fullBar;

            size_t& displayIndex = pTrack->displayIndex();
            int& selectionBeatFirst = pTrack->selectBeatFirst();
            int& selectionBeatLast = pTrack->selectBeatLast();
            int& selectionBarFirst = pTrack->selectBarFirst();
            int& selectionBarLast = pTrack->selectBarLast();

            if (selectionBeatFirst == -1) {
                selectionBeatFirst = selectionBeatLast =  bV->getClickBeat(x1);
                selectionBarFirst = selectionBarLast = i+displayIndex;
            }
            else
            {
                if (i+displayIndex > selectionBarLast)
                {
                    selectionBeatLast =  bV->getClickBeat(x1);
                    selectionBarLast = i+displayIndex;
                }
                else
                if (i+displayIndex < selectionBarFirst)
                {
                    selectionBeatFirst =   bV->getClickBeat(x1);
                    selectionBarFirst = i+displayIndex;
                }
                else
                {
                    if (selectionBarFirst == selectionBarLast)
                    {
                        int addBeat = bV->getClickBeat(x1);
                        if (addBeat > selectionBeatLast)
                            selectionBeatLast = addBeat;
                        if (addBeat < selectionBarFirst)
                            selectionBeatFirst = addBeat;
                    }
                    else
                    {
                        int addBeat = bV->getClickBeat(x1);
                        //if (addBeat > selectionBeatLast)
                        if (i+displayIndex ==selectionBarLast)
                        {
                            selectionBeatLast = addBeat;
                        }
                        else //if (addBeat < selectionBeatfirstt)
                        if (i+displayIndex ==selectionBarFirst)
                        {
                            selectionBeatFirst = addBeat;
                        }
                        else
                        {
                            if (i+displayIndex ==selectionBarLast-1)
                            {
                                //pre last bar
                                if (addBeat==pTrack->at(i+displayIndex)->size()-1)
                                {
                                    //its last beat
                                    if (selectionBeatLast == 0)
                                    {
                                        //and current beat is irst in last bar
                                        selectionBeatLast = addBeat;
                                        --selectionBarLast;
                                    }
                                }
                            }

                            if (i+displayIndex ==selectionBarFirst+1)
                            {
                                //pre last bar
                                if (addBeat==0)
                                {
                                    //its last beat
                                    if (selectionBeatFirst == pTrack->at(i+displayIndex-1)->size()-1)
                                    {
                                        //and current beat is irst in last bar
                                        selectionBeatFirst = 0;
                                        ++selectionBarFirst;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            wasPressed = true;
        }
    }

    if (wasPressed == false) {
        int& selectionBeatFirst = pTrack->selectBeatFirst();
        int& selectionBeatLast = pTrack->selectBeatLast();
        int& selectionBarFirst = pTrack->selectBarFirst();
        int& selectionBarLast = pTrack->selectBarLast();
        selectionBeatFirst = selectionBeatLast =  -1;
        selectionBarFirst = selectionBarLast = -1;

    }
}

void TrackView::setDisplayBar(int barPosition)
{
    size_t& cursor = pTrack->cursor();
    size_t& cursorBeat = pTrack->cursorBeat();
    size_t& displayIndex = pTrack->displayIndex();

    displayIndex = barPosition;
    cursor = displayIndex;
    tabParrent->setCurrentBar(cursor);
    cursorBeat = 0;
}

void TrackView::setUI()
{
    if (tabParrent->getMaster())
    {
        int centerX=0;

        std::string trackNames="";

        for (size_t i = 0; i < tabParrent->getTab()->size(); ++i)
        {
          trackNames += tabParrent->getTab()->at(i)->getName();
          trackNames += std::string(";");
        }

        //now-debug:here
        tabParrent->getMaster()->setComboBox(0,trackNames,centerX+20,5,210,30,tabParrent->getLastOpenedTrack());
        tabParrent->getMaster()->setComboBox(1,"instruments",240+centerX,5,200,30,pTrack->getInstrument());
        tabParrent->getMaster()->setComboBox(2,"volume",450+centerX,5,50,30,pTrack->getVolume());

        int butShift = 50;
        tabParrent->getMaster()->SetButton(3,"open tab view",640+butShift,20,90,15,"tabview");
        tabParrent->getMaster()->SetButton(4,"play",570+butShift,20,45,15,"playMidi");

        byte soloMute = pTrack->getStatus();
        tabParrent->getMaster()->setComboBox(5,"mutesolo",510+centerX,5,50,30,soloMute);

        tabParrent->getMaster()->setComboBox(6,"pan",570+centerX,5,50,30,pTrack->getPan());
    }


}

void TrackView::draw(QPainter *painter)
{
    Track *track1 = pTrack;
    ul trackLen = track1->size();
    int stringsN = track1->tuning.getStringsAmount();

    int pannelShift = getMaster()->getToolBarHeight();
    //double scaleValue = AConfig::getInstance()->getScaleCoef();
    //pannelShift /= scaleValue;

    int xSh=0;
    int ySh=pannelShift; //shift for the pannel

    ySh += 30;

    int hLimit = (h-50)/100;
    hLimit *= 100;

    size_t& cursor = pTrack->cursor();
    size_t& cursorBeat = pTrack->cursorBeat();
    size_t& stringCursor = pTrack->stringCursor();
    size_t& lastSeen = pTrack->lastSeen();
    size_t& displayIndex = pTrack->displayIndex();
    int& selectionBeatFirst = pTrack->selectBeatFirst();
    int& selectionBeatLast = pTrack->selectBeatLast();
    int& selectionBarFirst = pTrack->selectBarFirst();
    int& selectionBarLast = pTrack->selectBarLast();

    //to automate scroll
    if (cursor < displayIndex)
        displayIndex = cursor;

    if (cursor > (lastSeen-1))
        displayIndex = cursor;

    barsPull.clear(); //not always - to optimize

    byte lastNum = 0;
    byte lastDen = 0;

    if (pTrack->isDrums())
    {
        painter->drawText(220,55,"!Drum track!");
    }


    for (ul i = displayIndex; i < trackLen; ++i) //trackLen
    {
        Bar *curBar = track1->at(i);

        byte curNum = curBar->getSignNum();
        byte curDen = curBar->getSignDenum();
        bool sameSign = true;

        static bool alwaysShowSign = CONF_PARAM("TrackView.alwaysShowBarSign") == "1";

        if (alwaysShowSign)
            sameSign = false;

        if ((curNum != lastNum) ||(curDen != lastDen))
        {
                sameSign = false;
                lastNum = curNum;
                lastDen = curDen;
        }

        BarView bView(curBar,stringsN,i);

        if (selectionBarFirst!=-1)
        {
            if ((i >= selectionBarFirst)&&
                (i <= selectionBarLast))
            {
                //if one - then same (first)
                //if last - then second (last)
                //if middle - -1(whole)
                if (selectionBarLast==selectionBarFirst)
                    bView.setSelectors(selectionBeatFirst,selectionBeatLast);
                else
                {
                    if (selectionBarFirst==i)
                        bView.setSelectors(selectionBeatFirst,-1);
                    else if (selectionBarLast==i)
                         bView.setSelectors(0,selectionBeatLast);
                    else
                        bView.setSelectors(0,-1); //in the middle
                }

            }
        }

        bView.setSameSign(sameSign);

        if (ySh <= (hLimit))
            lastSeen = i;

        int xShNEXT = xSh + bView.getW()+15;
        int border = getMaster()->getWidth();

        if (xShNEXT > border) {
            xSh = 0;
            ySh += bView.getH(); // there was 100 hardcoded

            if (ySh >= (hLimit+480)){
                pan->draw(painter);
                return; //stop that
            }
        }

        bView.setShifts(xSh,ySh);
        byte barCompleteStatus = curBar->getCompleteStatus(); //avoid recalculations

        if ( i == cursor ) {
            changeColor(CONF_PARAM("colors.curBar"), painter);
            if (i==cursor)
                bView.setCursor(cursorBeat,stringCursor+1);
        }
        else {
            //refact add another color
            if ((barCompleteStatus==2)||(barCompleteStatus==1))
                changeColor(CONF_PARAM("colors.exceed"), painter);
        }


        bView.draw(painter);

        if (( i == cursor ) || (barCompleteStatus==2)||(barCompleteStatus==1))
             changeColor(CONF_PARAM("colors.default"), painter);


        xSh += bView.getW();

        //if (ySh <= (hLimit))
        {
            barsPull.push_back(bView);
        }

        if (i == cursor)
        {
            //old cursor lines
            //painter->drawLine(xSh,ySh+20*track1->tuning.getStringsAmount(),xSh+bView.getW(),ySh+20*track1->tuning.getStringsAmount());
            //painter->drawLine(xSh+cursorBeat*12,5+ySh+20*track1->tuning.getStringsAmount(),xSh+cursorBeat*12+12,5+ySh+20*track1->tuning.getStringsAmount());
        }
    }

    //if (ySh <= (hLimit))
    {
        ++lastSeen;
    }

    pan->draw(painter);

}

void TrackView::prepareThread(ul shiftTheCursor)
{
    //prepare for the animation

    if (localThr) {
        localThr->requestStop();
        localThr->terminate();
        if (CONF_PARAM("crashOnPlayHotFix") != "1")
        localThr->deleteLater();
        //local
    }

    localThr = new ThreadLocal;

    size_t& cursor = pTrack->cursor();
    size_t& cursorBeat = pTrack->cursorBeat();

    localThr->setInc(&cursor,&cursorBeat);
    localThr->setupValues(tabParrent->getTab(),pTrack,shiftTheCursor);
}


///////////Bars------------------------------
void BarView::drawMidiNote(QPainter *painter, byte noteDur, byte dotted, byte durDet, int midiNote,
                  int x1, int y1)
{
    int xPoint = x1;
    int yPoint = y1;

    //most important is verical shft thats made from
    //midi note value

   // int sevenTh = midiNote/7;

    int lineToStand = midiNote - 20; //40


    yPoint += (stringWidth-3)*11; //was *6
    yPoint -=2; //now set on 40

    int yNote = (stringWidth-3)/2;

    ///if (lineToStand == position[i])
    //THEN sign is used to be #


    int position[] = {2,5,7,10,12,14,17,19, 22,24,26,29,31,34,36,38,41,43,46,48,
                     50,53,55,58,60,62,65,67,70,72,74,77,79,82,84,86};



    bool needSignSharp =false;
    for (size_t i = 0; i < sizeof(position)/sizeof(int); ++i)
    {
        if (position[i]==lineToStand)
            needSignSharp = true;
    }


    int diezCount = 0;

    for (size_t i = 0; i < sizeof(position)/sizeof(int); ++i)
    {
        int index = sizeof(position)/sizeof(int) - 1 -i;
        //from the end
        if (lineToStand >= position[index])
        {
            --lineToStand;
            ++diezCount;
       }
    }



    lineToStand -= 12; //20 more notes were addded for bass

    yNote -= lineToStand*(stringWidth-3)/2;



    //prepare all default parameters here

    int mainRadius = 5;

    int radiusShift = 0;

    int note32 = 4;
    int note16 = 6;
    int note8 = 8;
    int note4 = 10; //names are wrong.. abit

    int noteTail = 6;
    int noteTailEnd = 10;


    //shift them
    if (CONF_PARAM("TrackView.largeNotes")=="1")
    {
        note4 *= 2;
        note8 *= 2;
        note16 *= 2;
        note32 *= 2;

        noteTailEnd += 3;

        yPoint += 10;

        mainRadius += 3;
        radiusShift = 2;
        //and fill inside(
    }


    yNote -= radiusShift;



    if (CONF_PARAM("TrackView.largeNotes")=="1")
        drawEllipse(QColor("black"), painter, xPoint-radiusShift,yPoint-radiusShift+yNote,mainRadius,mainRadius);
    else
    {
        drawEllipse(painter, xPoint-radiusShift,yPoint-radiusShift+yNote,mainRadius,mainRadius); //TODO тут вообще можно даже без функции
        drawEllipse(painter, xPoint+1,yPoint+1+yNote,3,3);
        drawEllipse(painter, xPoint+2,yPoint+2+yNote,2,2);
    }

    if (needSignSharp)
    {
        painter->drawLine(xPoint-radiusShift-5,yPoint-radiusShift+yNote,
                          xPoint-radiusShift,yPoint-radiusShift+yNote);

        painter->drawLine(xPoint-radiusShift-5,yPoint-radiusShift+yNote-2,
                          xPoint-radiusShift,yPoint-radiusShift+yNote-2);

        painter->drawLine(xPoint-radiusShift-2,yPoint-radiusShift+yNote-3,
                          xPoint-radiusShift-2,yPoint-radiusShift+yNote+1);

        painter->drawLine(xPoint-radiusShift-4,yPoint-radiusShift+yNote-3,
                          xPoint-radiusShift-4,yPoint-radiusShift+yNote+1);
    }

    int nU = 1; //note up or normal -1 \ 1

    if (CONF_PARAM("upsideDownNotes")=="1")
        nU = -1;


    if (dotted)
        painter->drawEllipse(xPoint+7,yPoint+yNote,2,2); //check fine on large


    if (noteDur >= 1)
        painter->drawLine(xPoint+noteTail,yPoint+yNote,xPoint+noteTail,yPoint-note4*nU+yNote); //line (1+)

    if (noteDur >=3)
        painter->drawLine(xPoint+noteTail,yPoint-note4*nU+yNote,xPoint+noteTailEnd,yPoint-note4*nU+yNote-3); //flow 1 (3+)
    if (noteDur >=4)
        painter->drawLine(xPoint+noteTail,yPoint-note8*nU+yNote,xPoint+noteTailEnd,yPoint-note8*nU+yNote-3); //flow 2 (4+)
    if (noteDur >=5)
        painter->drawLine(xPoint+noteTail,yPoint-note16*nU+yNote,xPoint+noteTailEnd,yPoint-note16*nU+yNote-3); //flow 3 (5+)
    if (noteDur >=6)
        painter->drawLine(xPoint+noteTail,yPoint-note32*nU+yNote,xPoint+noteTailEnd,yPoint-note32*nU+yNote-3); //flow 4 (6)

    if (durDet)
        painter->drawText(xPoint+6,yPoint+5,std::to_string(durDet).c_str());

    //DRAW LINE AS ITERATION

///     painter->drawLine(xPoint,cY+(stringWidth-3)
///     *(amountStr+5+lines),cX+10+barLen*inbarWidth,
///     cY+(stringWidth-3)*(amountStr+5+lines));


    int beginIndex = 0; ///0
    int endIndex = lineToStand/2; //14

    beginIndex = lineToStand/2;
    //endIndex = 12;

    int lowerestBorder =  0;

    if (lineToStand<5)
    {
        if (lineToStand < -6)
        {
            beginIndex = -6;

            int smoothIndex = beginIndex*-1 - 2;

            yPoint += (stringWidth-3)*smoothIndex;
        }
    }
    else
    {
        if (lineToStand >= 16)
        {
            beginIndex = 6;
        }
        //beginIndex = 7;
    }

    //

    for (int i = lowerestBorder; i < beginIndex; ++i)
        yPoint -= stringWidth-3;

    for (int i = beginIndex ; i <= endIndex; ++i)
    {
        int smallMove = -3;
        int decreaseElse = 0; //-3

        //if (i == lineToStand/2)
           // decreaseElse = 0;


        painter->drawLine(xPoint-radiusShift+smallMove,yPoint+radiusShift/2,
                          xPoint+radiusShift+mainRadius/2+smallMove+decreaseElse,yPoint+radiusShift/2);

        yPoint -= stringWidth-3;
    }
}


void BarView::drawNote(QPainter *painter, byte noteDur, byte dotted, byte durDet,
                       int x1, int y1)
{
    int xPoint = x1;
    int yPoint = y1;


    //prepare all default parameters here

    int mainRadius = 5;

    int radiusShift = 0;

    int note32 = 4;
    int note16 = 6;
    int note8 = 8;
    int note4 = 10; //names are wrong.. abit

    int noteTail = 6;
    int noteTailEnd = 10;


    //shift them
    if (CONF_PARAM("TrackView.largeNotes")=="1")
    {
        note4 *= 2;
        note8 *= 2;
        note16 *= 2;
        note32 *= 2;

        noteTailEnd += 3;

        yPoint += 10;

        mainRadius += 3;
        radiusShift = 2;
        //and fill inside(
    }

    //paint now

    int nU = 1; //note up or normal -1 \ 1

    if (CONF_PARAM("upsideDownNotes")=="1")
        nU = -1;

    if (noteDur >= 2)
    {
        //Must get filled - in another manner

        if (CONF_PARAM("TrackView.largeNotes")=="1")
            drawEllipse(QColor("black"), painter, xPoint-radiusShift,yPoint-radiusShift,mainRadius,mainRadius);
        else
        {
            painter->drawEllipse(xPoint-radiusShift,yPoint-radiusShift,mainRadius,mainRadius);

        painter->drawEllipse(xPoint+1,yPoint+1,3,3); //inner circle ( 2+)
        painter->drawEllipse(xPoint+2,yPoint+2,2,2); //inner circle ( 2+)
        }
    }
    else
    {
        painter->drawEllipse(xPoint-radiusShift,yPoint-radiusShift,mainRadius,mainRadius); //default circle
    }


    if (dotted)
        painter->drawEllipse(xPoint+7,yPoint,2,2); //check fine on large


    if (noteDur >= 1)
        painter->drawLine(xPoint+noteTail,yPoint,xPoint+noteTail,yPoint-note4*nU); //line (1+)

    if (noteDur >=3)
        painter->drawLine(xPoint+noteTail,yPoint-note4*nU,xPoint+noteTailEnd,yPoint-note4*nU); //flow 1 (3+)
    if (noteDur >=4)
        painter->drawLine(xPoint+noteTail,yPoint-note8*nU,xPoint+noteTailEnd,yPoint-note8*nU); //flow 2 (4+)
    if (noteDur >=5)
        painter->drawLine(xPoint+noteTail,yPoint-note16*nU,xPoint+noteTailEnd,yPoint-note16*nU); //flow 3 (5+)
    if (noteDur >=6)
        painter->drawLine(xPoint+noteTail,yPoint-note32*nU,xPoint+noteTailEnd,yPoint-note32*nU); //flow 4 (6)

    if (durDet)
        painter->drawText(xPoint+6,yPoint+5,std::to_string(durDet).c_str());
}

BarView::BarView(Bar *b,int nstr, int barNum): //stringWidth(12),inbarWidth(20),
    pBar(b),xShift(0),yShift(0),nStrings(nstr),cursor(-1),stringCursor(-1)
  ,barNumber(barNum),sameSign(false)
{
    selectorBegin=-1;
    selectorEnd=-1;

    repBegin=false;
    repEnd=false;



    int barLen = b->size();
    h = stringWidth*(nstr+1); //

    if (CONF_PARAM("TrackView.largeNotes")=="1")
        h = stringWidth*(nstr+2);


    h+= 10; //mini shift from last
    w = (barLen+1)*inbarWidth;

    if (CONF_PARAM("upsideDownNotes")=="1")
        h+= 10;


    if (CONF_PARAM("showNotesView")=="1")
    {
       h += (stringWidth-3)*(5);
       h += 75;

       h+= 20; //to ensure bass would be ok

       if (CONF_PARAM("upsideDownNotes")=="0")
           h+=10;
    }

    nBeats=barLen;

    if (b->getRepeat()&1)
    { repBegin = true; w += 15; }
    if (b->getRepeat()&2)
    { repEnd = true; w += 15; }
}

void BarView::draw(QPainter *painter)
{
    Bar *bar1 = pBar;
    int cX = 20+xShift;
    int cY = 20+yShift;

    Track *track = bar1->getParent();
    bool isSelected = false;

    if (selectorBegin!=-1)
    {
        int skipFromStart = (selectorBegin)*inbarWidth;
        int skipFromEnd = (bar1->size() - selectorEnd)*inbarWidth;

        if (selectorEnd == -1)
        {
            painter->fillRect(getX()+skipFromStart,getY(),
                              getW()-skipFromStart,getH(),QColor(CONF_PARAM("colors.selection").c_str()));
        }   //to the end
        else
        {
            painter->fillRect(getX()+skipFromStart,getY(),
                              getW()-skipFromStart-skipFromEnd,getH(),QColor(CONF_PARAM("colors.selection").c_str()));
        }
        isSelected = true;


    }

    //check widht and height
    //painter->drawRect(cX,cY,w,h);

    if (sameSign == false)
    {
        //SKIP in another mode
        std::string numVal,denVal;
        numVal = std::to_string( bar1->getSignNum() );
        denVal = std::to_string( bar1->getSignDenum() );

        //Paint properly - not in signs!!!
        int repeat = pBar->getRepeat();

        /* to new
        if (repeat == 1)
            numVal<<".B";
        if (repeat == 2)
            denVal<<".E";
        if (repeat == 3)
            numVal<<".C";
            */


        auto f = painter->font();
        auto fontSize = f.pixelSize();
        f.setPixelSize(18);
        painter->setFont(f);
        painter->drawText(cX-15,cY-15+50+3,numVal.c_str());
        painter->drawText(cX-15,cY+15+50+3,denVal.c_str());
        f.setPixelSize(fontSize);
        painter->setFont(f);
    }

    if (repBegin)
        cX += 15;
    //else
    {  //always
        cX -=10; //decreace bar size
    }

    if (barNumber!=-1)
    {
        std::string numberLabel = std::to_string(barNumber + 1);
        int xMiniSHift = 0;
        if ((barNumber+1) >= 10)
            xMiniSHift -= 5;
        if ((barNumber+1) >= 100)
            xMiniSHift -= 5;

        if (repBegin) xMiniSHift-=15;

        painter->drawText(cX+xMiniSHift,cY+15+3,numberLabel.c_str());
    }

    ul barLen = bar1->size();
    int amountStr = nStrings;

    if (repBegin)
    {
        int toBegin = -3;

        for (int l=0; l<2; ++l)
        painter->drawLine(toBegin+cX+l,cY+stringWidth/2,toBegin+cX+l,cY+amountStr*stringWidth-stringWidth/2);

        painter->drawEllipse(toBegin+cX+5,cY+stringWidth*2,3,3);
        painter->drawEllipse(toBegin+cX+5,cY+stringWidth*5,3,3);
    }

    if (repEnd)
    {
        int toBegin = w-10;

        if (repBegin)
            toBegin -= 15;

        for (int l=0; l<2; ++l)
        painter->drawLine(toBegin+cX+l,cY+stringWidth/2,toBegin+cX+l,cY+amountStr*stringWidth-stringWidth/2);

        std::string repCount =  std::to_string(bar1->getRepeatTimes());
        painter->drawText(toBegin+cX-9,cY+stringWidth*2-3,repCount.c_str());
        painter->drawEllipse(toBegin+cX-9,cY+stringWidth*2,3,3); //default color fill refact?
        painter->drawEllipse(toBegin+cX-9,cY+stringWidth*5,3,3);
    }



    //const int stringWidth = 12;
    //const int inbarWidth = 20;

    for (int i = 1 ; i < (amountStr+1); ++i)
    {
        if (i == stringCursor)
        {
            //painter->changeColor(APainter::colorRed);
            changeColor(CONF_PARAM("colors.curString"), painter);
        }

        painter->drawLine(cX+10,cY+i*stringWidth-stringWidth/2,cX+10+barLen*inbarWidth,cY+i*stringWidth-stringWidth/2);


        if (i == stringCursor)
        {
            //painter->changeColor(APainter::colorGreen);
            changeColor(CONF_PARAM("colors.curBar"), painter);
        }
    }

    //start and end lines
    painter->drawLine(cX+10+0*inbarWidth,cY + stringWidth/2,cX+10+0*inbarWidth,cY+stringWidth*amountStr - stringWidth/2);
    painter->drawLine(cX+10+barLen*inbarWidth,cY + stringWidth/2,cX+10+barLen*inbarWidth,cY+stringWidth*amountStr - stringWidth/2);

    ul colorMark =0;
    std::string barMarker;
    bar1->getGPCOMPMarker(barMarker,colorMark);

    bool markerPrec = barMarker.empty()==false;

    bool wasNoBeatEffects = true;

    for (ul i = 0; i < barLen; ++i)
    {

        if (i==cursor)
        {
            //painter->changeColor(APainter::colorBlue);
             changeColor(CONF_PARAM("colors.curBeat"), painter);
        }


        Beat *curBeat = bar1->at(i);

        for (ul j =0; j < curBeat->size(); ++j)
        {
            Note *curNote = curBeat->at(j);



            if (curNote)
            {
                int noteTune = 0;
                int midiNote = 0;

                if (track)
                   noteTune = track->tuning.getTune(curNote->getStringNumber()-1);

                midiNote = curNote->getMidiNote(noteTune);

                if (CONF_PARAM("showNotesView")=="1")
                {
                drawMidiNote(painter,curBeat->getDuration(),
                             curBeat->getDotted(),curBeat->getDurationDetail(),
                             midiNote,
                             cX+10+i*inbarWidth + inbarWidth/2,
                             cY+(stringWidth-3)*(amountStr+5));
                }
                else
                {
                    /*
                    if (j==0)
                    {
                        drawMidiNote(painter,curBeat->getDuration(),
                                     curBeat->getDotted(),curBeat->getDurationDetail(),
                                     midiNote,
                                     cX+10+i*inbarWidth + inbarWidth/2,
                                     cY+(stringWidth-3)*(amountStr+5));
                    }
                    */
                }

                bool letRing = curNote->effPack.get(18);
                bool palmMute = curNote->effPack.get(2);
                bool ghostNote = curNote->effPack.get(21);
                bool harmonics = curNote->effPack.get(14);

                std::string noteVal;

                byte noteState = curNote->getState();

                if (noteState != 3) //not x note
                {
                    if ((noteState==2))

                    {
                        noteVal = "_" + std::to_string( curNote->getFret() );
                    }
                    else
                        noteVal = std::to_string( curNote->getFret() );
                }
                else
                    noteVal = "x"; //x note

                if (ghostNote) {
                    std::string sX ="(" +  noteVal + ")";
                    noteVal.clear();
                    noteVal = sX.c_str();
                }

                int miniShift = inbarWidth/2;

                if (curNote->getFret() >= 10)
                    miniShift /= 2;

                int currentString = curNote->getStringNumber();

                std::string thatBack = CONF_PARAM("colors.background");

                if (isSelected)
                {
                    if (selectorEnd == -1)//till end
                    {
                        if (i>=selectorBegin)
                            thatBack = CONF_PARAM("colors.selection");
                    }
                    else
                    {
                        if (selectorBegin == 0)
                        {
                            if (i<=selectorEnd)
                                thatBack = CONF_PARAM("colors.selection");
                        }
                        else
                        {
                            //not zero start not -1 end
                            if ((i<=selectorEnd) && (i >= selectorBegin))
                                    thatBack = CONF_PARAM("colors.selection");
                        }

                    }
                }

                painter->fillRect(cX+10+i*inbarWidth + inbarWidth/4,
                                  cY+stringWidth*currentString-stringWidth,
                                  inbarWidth - inbarWidth/4, stringWidth, QColor(thatBack.c_str())); //BG color
                //DRAW effects

                if (palmMute)
                    painter->drawRect(cX+10+i*inbarWidth + inbarWidth/4,
                                      cY+stringWidth*currentString-stringWidth,
                                      inbarWidth - inbarWidth/4, stringWidth);

                if (letRing)
                    painter->drawEllipse(cX+10+i*inbarWidth + inbarWidth/4,
                                         cY+stringWidth*currentString-stringWidth,
                                         inbarWidth - inbarWidth/4, stringWidth);
                if (harmonics)
                {
                    //change midi note?
                    int XBegin = cX+10+i*inbarWidth + inbarWidth/4;
                    int XEnd = inbarWidth - inbarWidth/4 + XBegin;

                    int YBegin = cY+stringWidth*currentString-stringWidth;
                    int YEnd = stringWidth + YBegin;

                    XBegin -= 2; YBegin -=2;
                    XEnd += 2; YEnd += 2;

                    painter->drawLine(XBegin,(YBegin+YEnd)/2,
                                      (XBegin+XEnd)/2,YBegin);
                    painter->drawLine((XBegin+XEnd)/2,YBegin,XEnd,(YBegin+YEnd)/2);
                    painter->drawLine(XEnd,(YBegin+YEnd)/2,(XBegin+XEnd)/2,YEnd);
                    painter->drawLine((XBegin+XEnd)/2,YEnd,XBegin,(YBegin+YEnd)/2);
                }


                drawEffects(painter, cX+10+i*inbarWidth,
                            cY+stringWidth*currentString,              
                            inbarWidth - inbarWidth/4, stringWidth,
                            &(curNote->effPack));

                int centerBoeder = 1;

                if (ghostNote) miniShift -= 3;

                painter->drawText(cX+10+i*inbarWidth + miniShift,
                                  cY+stringWidth*currentString
                                  -centerBoeder,noteVal.c_str());

                if (ghostNote) miniShift += 3;
            }
        }

        if (curBeat->getPause())
        {
            //3 and 4 is like current string
            painter->drawText(cX+10+i*inbarWidth+ inbarWidth/2,cY+stringWidth*3,"z");
            painter->drawText(cX+10+i*inbarWidth+ inbarWidth/2,cY+stringWidth*4,"z");
        }


        std::string durVal = std::to_string(curBeat->getDuration());

        if (curBeat->getDotted())
            durVal += ".";

        byte trump = curBeat->getDurationDetail();
        if (trump)
            durVal += "[" + std::to_string( trump );


        //painter->drawText(cX+10+i*inbarWidth,cY+stringWidth*(amountStr+1), // downerdurVal.c_str());
        //draw beat eff
        bool upStroke = curBeat->effPack.get(25);
        bool downStroke = curBeat->effPack.get(26);

        bool changes = curBeat->effPack.get(28);

        std::string textBeat; curBeat->getGPCOMPText(textBeat);
        bool textPrec = textBeat.empty()==false;



        //chord SKIPPED

        bool fadeIn = curBeat->effPack.get(20);

        if (downStroke)
        {
            painter->drawLine(cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(0),
                              cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(amountStr));

            painter->drawLine(cX+10+i*inbarWidth + inbarWidth/2, cY+stringWidth*(amountStr),
                              cX+10+i*inbarWidth + inbarWidth/2-2, cY+stringWidth*(amountStr)-3);

            painter->drawLine(cX+10+i*inbarWidth + inbarWidth/2, cY+stringWidth*(amountStr),
                              cX+10+i*inbarWidth + inbarWidth/2+2, cY+stringWidth*(amountStr)-3);
        }

        if (upStroke)
        {
            painter->drawLine(cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(0),
                              cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(amountStr));

            painter->drawLine(cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(0),
                              cX+10+i*inbarWidth + inbarWidth/2-2,cY+3);

            painter->drawLine(cX+10+i*inbarWidth + inbarWidth/2,cY,
                              cX+10+i*inbarWidth + inbarWidth/2+2,cY+3);
        }

        if (fadeIn)
        {
           // painter->drawRect(cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(amountStr),inbarWidth,stringWidth);
            painter->drawLine(cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(amountStr)+stringWidth/2,
                              cX+10+i*inbarWidth + inbarWidth/2+5,cY+stringWidth*(amountStr)+stringWidth/2-3);

            painter->drawLine(cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(amountStr)+stringWidth/2,
                              cX+10+i*inbarWidth + inbarWidth/2+5,cY+stringWidth*(amountStr)+stringWidth/2+3);

            wasNoBeatEffects=false;
        }

        if (changes)
        {
            painter->drawRect(cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(amountStr),inbarWidth,stringWidth);
            painter->drawText(cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(amountStr+1),"ch");//could replace with bpm value

            wasNoBeatEffects=false;
        }

        if (textPrec)
        {
            auto f = painter->font();
            auto fontSize = f.pixelSize();
            f.setPixelSize(9);
            painter->setFont(f);
            painter->drawText(cX+10+i*inbarWidth + inbarWidth/2,cY+stringWidth*(amountStr+1),textBeat.c_str());
            f.setPixelSize(fontSize);
            painter->setFont(f);

        }


        if (CONF_PARAM("showNotesView")=="1")
        {
            //later
        }
        else
            drawNote(painter, curBeat->getDuration(),curBeat->getDotted(),curBeat->getDurationDetail(),
                     cX+10+i*inbarWidth + inbarWidth/2,
                     cY+stringWidth*(amountStr+1));

        //OR DRAW MANY NOTES
        if (i==cursor)
        {
             //painter->changeColor(APainter::colorGreen);
             changeColor(CONF_PARAM("colors.curBar"), painter);
        }
    }


    if (wasNoBeatEffects) //check for whole: (changes==false) && (fadeIn==false))
    {
        if (markerPrec)
        {
            painter->drawText(cX+10+inbarWidth/2,cY+stringWidth*(amountStr+1),barMarker.c_str());//could replace with bpm value
        }
    }



    if (CONF_PARAM("showNotesView")=="1")
    {

    //draw 5 lines first


        int prefLines = 10;

        for (int lines = 0; lines < 11; ++lines)
        {
            if (lines==5) continue;

            painter->drawLine(cX+10,cY+(stringWidth-3)*(amountStr+prefLines+lines),cX+10+barLen*inbarWidth,cY+(stringWidth-3)*(amountStr+prefLines+lines));


        }

        //S
        painter->drawLine(cX+10,cY+(stringWidth-3)*(amountStr+prefLines+0),
                cX+10,cY+(stringWidth-3)*(amountStr+prefLines+4));
        painter->drawLine(cX+10+barLen*inbarWidth,cY+(stringWidth-3)*(amountStr+prefLines+0),
                cX+10+barLen*inbarWidth,cY+(stringWidth-3)*(amountStr+prefLines+4));

        //BASs
        painter->drawLine(cX+10,cY+(stringWidth-3)*(amountStr+prefLines+6),
                cX+10,cY+(stringWidth-3)*(amountStr+prefLines+10));

        painter->drawLine(cX+10+barLen*inbarWidth,cY+(stringWidth-3)*(amountStr+prefLines+6),
                cX+10+barLen*inbarWidth,cY+(stringWidth-3)*(amountStr+prefLines+10));


    }


    //auto clean them
    flushSelectors();

    //h = stringWidth*amountStr;
    //w = (barLen+2)*inbarWidth;
}

void BarView::drawEffects(QPainter *painter, int x1, int y1, int w1, int h1, EffectsPack *eff)
{
    if (eff->get(27))
    {
        //accented
        painter->drawLine(x1+w1/2,y1-h1,x1+2+w1/2,y1-3-h1);
        painter->drawLine(x1+2+w1/2,y1-3-h1,x1+4+w1/2,y1-h1);
    }

    if (eff->get(17))
    {
        //bend
        painter->drawLine(x1+w1, y1-h1/2, x1+w1+6, y1-h1-6);

    }

    if (eff->get(24))
    {
        painter->drawLine(x1+w1/2,y1-h1,
                          x1+w1/2+3,y1-h1-3);

        painter->drawLine(x1+w1/2+2,y1-h1,
                          x1+w1/2+5,y1-h1-3);

        painter->drawLine(x1+w1/2+4,y1-h1,
                          x1+w1/2+7,y1-h1-3);
    }

    if (eff->get(1))
    {
        //vibratto turned on
        x1+=inbarWidth/2;
        y1-=3*stringWidth/4;

        painter->drawLine(x1,y1,x1+2,y1-2);
        painter->drawLine(x1+2,y1-2,x1+4,y1);
        painter->drawLine(x1+4,y1,x1+6,y1-2);
        painter->drawLine(x1+6,y1-2,x1+8,y1);
        painter->drawLine(x1+8,y1,x1+10,y1-2);
    }

    if (eff->get(10))
    {
        //legato turned on
        x1+=inbarWidth/2;
        y1-=3*stringWidth/4;

        painter->drawLine(x1,y1,x1+7,y1-3);
        painter->drawLine(x1+7,y1-3,x1+14,y1);
    }

    //by the way third left free
    if (eff->inRange(3,9))
    {
        //some of slides turned on

        if (eff->get(4)||eff->get(5))
        {
            //4 normal
            //5 legatto slide

            x1+=3*inbarWidth/4;
            y1-=stringWidth/2;

            painter->drawLine(x1,y1-2,x1+14,y1+4);

            x1-=inbarWidth/4;
            y1-=stringWidth/4;

            if (eff->get(5))
            {
                painter->drawLine(x1,y1,x1+7,y1-3);
                painter->drawLine(x1+7,y1-3,x1+14,y1);
            }
        }

        //8+6 slides down
        if (eff->get(8)||eff->get(6))
        {
            x1+=3*inbarWidth/4;
            y1-=stringWidth/2;

            painter->drawLine(x1,y1-2,x1+10,y1+7);
        }

        //9+7 slides up

        if (eff->get(9)||eff->get(7))
        {
            x1+=3*inbarWidth/4;
            y1-=stringWidth/2;

            painter->drawLine(x1,y1+7,x1+10,y1-2);
        }
        ///2.19 is only guidence
        //1 normal 2 from -1 to 12
        //3 normal 4 from 99 to 1
        //5 from 1 to 99
    }
}

int BarView::getClickString(int y1)
{
    int yOffset = y1-y;
    int stringIndex = yOffset/12; //12 stringWidth
    //attention
    //unknown issue
    return stringIndex;
}

int BarView::getClickBeat(int x1)
{
    if (repBegin) x1 -= 15;

    int xOffset = x1-x;
    int beatIndex = xOffset/inbarWidth;
    //--beatIndex;
    return beatIndex;
}


// ------------OVER the changes--------------------


bool TabView::gotChanges()
{
    for (int i = 0; i < tracksView.size(); ++i)
        if (tracksView[i]->gotChanges())
        {
            //REQUEST DIALOG - if no then return true

            QMessageBox dialog("Escape changes?","Leave modification not saved?",
                               QMessageBox::Information,
                               QMessageBox::Yes,
                               QMessageBox::No,
                               QMessageBox::Cancel |QMessageBox::Escape);
            //dialog.set
            if (dialog.exec() == QMessageBox::Yes)
                return false; //escape

            return true;
        }

    return false;
}

bool TrackView::gotChanges()
{
    if (pTrack->commandSequence.size())
        return true;
    return false;
}
