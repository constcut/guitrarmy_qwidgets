#include "trackview.h"
#include "tabviews.h"


void TrackView::launchThread()
{
    if (localThr)
        localThr->start();
}

void TrackView::stopThread()
{
    if (localThr) {
        localThr->requestStop();
        localThr->wait();
    }
}

void TrackView::connectThreadSignal(MasterView *masterView)
{
    masterView->connectThread(localThr.get());
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


            size_t trackLen = pTrack->size();
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
    for (size_t i = 0; i < barsPull.size(); ++i)
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

            size_t realIndex = i + displayIndex;
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
    for (size_t i = 0; i < barsPull.size(); ++i)
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

        std::uint8_t soloMute = pTrack->getStatus();
        tabParrent->getMaster()->setComboBox(5,"mutesolo",510+centerX,5,50,30,soloMute);

        tabParrent->getMaster()->setComboBox(6,"pan",570+centerX,5,50,30,pTrack->getPan());
    }


}

void TrackView::draw(QPainter *painter)
{
    Track *track1 = pTrack;
    size_t trackLen = track1->size();
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

    std::uint8_t lastNum = 0;
    std::uint8_t lastDen = 0;

    if (pTrack->isDrums())
    {
        painter->drawText(220,55,"!Drum track!");
    }


    for (size_t i = displayIndex; i < trackLen; ++i) //trackLen
    {
        auto& curBar = track1->at(i);

        std::uint8_t curNum = curBar->getSignNum();
        std::uint8_t curDen = curBar->getSignDenum();
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

        BarView bView(curBar.get(),stringsN,i);

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
        std::uint8_t barCompleteStatus = curBar->getCompleteStatus(); //avoid recalculations

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

void TrackView::prepareThread(size_t shiftTheCursor)
{
    //prepare for the animation

    if (localThr) {
        localThr->requestStop();
        localThr->wait();
    }

    localThr = std::make_unique<ThreadLocal>();

    size_t& cursor = pTrack->cursor();
    size_t& cursorBeat = pTrack->cursorBeat();

    localThr->setInc(&cursor,&cursorBeat);
    localThr->setupValues(tabParrent->getTab(),pTrack,shiftTheCursor);
}
