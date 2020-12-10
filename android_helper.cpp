#include "android_helper.h"


#ifdef __ANDROID__
# include <android/api-level.h>
#endif

#ifdef __ANDROID_API__
#include <QtAndroidExtras/QAndroidJniObject>
//#include <qpa/qplatformnativeinterface.h>

#include <QApplication>
#endif

#include "g0/astreaming.h"

static AStreaming logger("android_helper");

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
