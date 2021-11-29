#-------------------------------------------------
#
# Project created by QtCreator 2015-03-23T11:22:08
#
#-------------------------------------------------


#       core gui
QT   += widgets multimedia network
CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17

#

android:QT += androidextras


TARGET = Guitarmy
TEMPLATE = app


QMAKE_CFLAGS +=  -fpermissive
QMAKE_CXXFLAGS +=  -fpermissive
QMAKE_LFLAGS +=  -fpermissive


win32:LIBS += -lWinMM


SOURCES += main.cpp\
    barview.cpp \
    tab/bar.cpp \
    tab/beat.cpp \
    midi/midiexport.cpp \
    midi/midisignal.cpp \
    tab/note.cpp \
    tab/tab_com.cpp \
    tab/track.cpp \
    tab/track_com.cpp \
    libs/sf/tsf.cpp \
    mainwindow.cpp \
    gview.cpp \
    tab/abitarray.cpp \
    g0/aconfig.cpp \
    tab/aexpimp.cpp \
    tab/gmyfile.cpp \
    tab/gtpfiles.cpp \
    midi/midifile.cpp \
    g0/scenario.cpp \
    tab/tab.cpp \
    athread.cpp \
    gpannel.cpp \
    mainviews.cpp \
    midi/midirender.cpp \
    tabviews.cpp \
    tab/aclipboard.cpp \
    tabcommands.cpp \
    g0/wavefile.cpp \
    g0/fft.cpp \
    g0/waveanalys.cpp \
    android_helper.cpp \
    centerview.cpp \
    qthelper.cpp \
    inputviews.cpp \
    midi/midiengine.cpp \
    g0/build.cpp \
    g0/rec.cpp \
    trackview.cpp


HEADERS  += mainwindow.h \
    barview.h \
    tab/bar.h \
    tab/beat.h \
    midi/midisignal.h \
    tab/note.h \
    tab/track.h \
    tab/types.h \
    gview.h \
    tab/abitarray.h \
    g0/aconfig.h \
    tab/aexpimp.h \
    tab/gmyfile.h \
    tab/gtpfiles.h \
    midi/midifile.h \
    tab/tab.h \
    athread.h \
    g0/buildheader.h \
    gpannel.h \
    libs/sf/tml.h \
    libs/sf/tsf.h \
    mainviews.h \
    midirender.h \
    tabviews.h \
    tab/aclipboard.h \
    tabcommands.h \
    g0/wavefile.h \
    g0/fft.h \
    g0/waveanalys.h \
    android_helper.h \
    centerview.h \
    qthelper.h \
    inputviews.h \
    midi/midiengine.h \
    g0/rec.h \
    trackview.h



DISTFILES += \
    g0/actionPlan.txt \
    g0/todo.txt \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    libs/sf/LICENSE \
    libs/sf/README.md \
    othLibs/ffft/readme.txt \
    android/src/in/guitarmy/app/AndroidHelper.java

INSTALLS += target

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

RESOURCES += \
    icons.qrc \
    tests.qrc \
    info.qrc \
    icons2.qrc

