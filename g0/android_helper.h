#ifndef ANDROID_HELPER_H
#define ANDROID_HELPER_H


class android_helper
{
public:
    android_helper();

    static void setPortrait();
    static void setLandscape();

    static void notifyAndroid();
};



#endif // ANDROID_HELPER_H
