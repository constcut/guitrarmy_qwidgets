#include "Components.hpp"


using namespace gtmy;


GQButton::GQButton(QWidget *pa):QPushButton(pa)
{
    buttonNumber=-1;
    keyPress = 0;
    connect(this, SIGNAL (clicked()), SLOT (buttonWasClicked()));
}

void GQButton::buttonWasClicked() {
    if (buttonNumber!= -1) {
        if (pressSyn.empty() == false) {
             keyPress->pushForceKey(pressSyn);
        }
        else
            if (keyPress) {
                keyPress->pushForceKey("b:" + std::to_string(buttonNumber));
                //audio_qDebug()<<"log from button"<<genPress.c_str();
            }
    }
}



GQCombo::GQCombo(QWidget *pa):QComboBox(pa)
{
    elementNumber = -1;
    pushItem=false;
    keyPress = 0;
    connect(this,SIGNAL(activated(int)),SLOT(elementChosen(int)));
    //connect(this,SIGNAL(currentIndexChanged(int)),SLOT(elementChosen(int)));
}

void GQCombo::elementChosen(int index)
{
    qDebug()<<"log from combo "<<elementNumber<<" "<<index;
    std::string line = "com:" + std::to_string(elementNumber) + ":" + std::to_string(index);
    if (keyPress) {
        if (pushItem==false)
        keyPress->pushForceKey(line.c_str());
        else {
            QString itemStr = this->currentText();
            std::string itemStd = itemStr.toStdString();
            keyPress->pushForceKey(itemStd);
        }
    }
}
