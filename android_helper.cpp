#include "android_helper.h"


#ifdef __ANDROID__
# include <android/api-level.h>
#endif

#ifdef __ANDROID_API__
#include <QtAndroidExtras/QAndroidJniObject>
//#include <qpa/qplatformnativeinterface.h>

#include <QApplication>
#endif




void android_helper::notifyAndroid()
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


android_helper::android_helper()
{

}

void android_helper::setPortrait()
{
#ifdef __ANDROID_API__

    //QPlatformNativeInterface *interface = QApplication::platformNativeInterface();
    //QAndroidJniObject activity = (jobject)interface->nativeResourceForIntegration("QtActivity");


    QAndroidJniObject helper("in/guitarmy/app/ahelper/AndroidHelper");
    helper.callMethod<void>("setPortOrt");
#endif
}

void android_helper::setLandscape()
{
#ifdef __ANDROID_API__

    //QPlatformNativeInterface *interface = QApplication::platformNativeInterface();
    //QAndroidJniObject activity = (jobject)interface->nativeResourceForIntegration("QtActivity");


    // QAndroidJniObject::callStaticMethod
   //         <void>("in/guitarmy/app/ahelper/AndroidHelper","msg");
    //copy notifier then(

    QAndroidJniObject helper("in/guitarmy/app/ahelper/AndroidHelper");
    helper.callMethod<void>("setLandOrt");


#endif
}
