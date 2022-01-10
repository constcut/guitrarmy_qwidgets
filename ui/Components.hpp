#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "MainViews.hpp"

#include <QPushButton>
#include <QComboBox>


namespace gtmy {


    class GQButton : public QPushButton //TODO в отдельные файлы
    {
        Q_OBJECT
    protected:
            int buttonNumber;
            std::string pressSyn;
            MasterView *keyPress;
    public:
        GQButton(QWidget *pa=0);

        void setButtonNum(int newBN)
        {    buttonNumber = newBN; }

        void setKeyPress(MasterView *mast)
        { keyPress = mast;}

        void setPressSyn(std::string newSyn)
        { pressSyn = newSyn; }

    public slots:

        void buttonWasClicked();

    };


    class GQCombo : public QComboBox
    {
        Q_OBJECT
    protected:
        int elementNumber;
        MasterView *keyPress;

        std::string params;
        bool pushItem;
    public:

        bool getPushItem() {return pushItem;}//option
        void setPushItem(bool pushNew){pushItem=pushNew;}

        void setElementNum(int newBN)
        {    elementNumber = newBN; }

        void setKeyPress(MasterView *mast)
        { keyPress = mast;}

        void setParams(std::string newParams)
        {params = newParams;}

        std::string getParams(){return params;}

        GQCombo(QWidget *pa=0);

    public slots:
        void elementChosen(int index);
    };



}

#endif // COMPONENTS_H
