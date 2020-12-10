#include "qthelper.h"

#ifdef __ANDROID_API__
#include <QtAndroidExtras/QAndroidJniObject>
#endif

QByteArray alphaCut(QByteArray &src)
{
    QByteArray newBytes;
    for (int i = 0; i < src.size(); ++i)
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


void notifyAndroid()
{
    #ifdef __ANDROID_API__

QString m_notification = "Olololo? Lolololo!";
QAndroidJniObject javaNotification = QAndroidJniObject::fromString(m_notification);
QAndroidJniObject::callStaticMethod<void>("in/guitarmy/app/AndroidHelper",
                                   "notify",
                                   "(Ljava/lang/String;)V",
                                   javaNotification.object<jstring>());
#endif
}
