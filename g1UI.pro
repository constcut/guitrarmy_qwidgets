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
    g0/bar.cpp \
    g0/beat.cpp \
    g0/midiexport.cpp \
    g0/midisignal.cpp \
    g0/note.cpp \
    g0/tab_com.cpp \
    g0/track.cpp \
    g0/track_com.cpp \
    libs/sf/tsf.cpp \
    mainwindow.cpp \
    gview.cpp \
    g0/abitarray.cpp \
    g0/aconfig.cpp \
    g0/aexpimp.cpp \
    g0/gmyfile.cpp \
    g0/gtpfiles.cpp \
    g0/midifile.cpp \
    g0/scenario.cpp \
    g0/tab.cpp \
    athread.cpp \
    gpannel.cpp \
    mainviews.cpp \
    midirender.cpp \
    tabviews.cpp \
    g0/aclipboard.cpp \
    tabcommands.cpp \
    g0/wavefile.cpp \
    g0/fft.cpp \
    g0/waveanalys.cpp \
    android_helper.cpp \
    centerview.cpp \
    qthelper.cpp \
    inputviews.cpp \
    midiengine.cpp \
    g0/build.cpp \
    g0/rec.cpp \
    trackview.cpp


HEADERS  += mainwindow.h \
    barview.h \
    g0/bar.h \
    g0/beat.h \
    g0/midisignal.h \
    g0/note.h \
    g0/track.h \
    g0/types.h \
    gview.h \
    g0/abitarray.h \
    g0/aconfig.h \
    g0/aexpimp.h \
    g0/gmyfile.h \
    g0/gtpfiles.h \
    g0/midifile.h \
    g0/tab.h \
    athread.h \
    g0/buildheader.h \
    gpannel.h \
    libs/sf/tml.h \
    libs/sf/tsf.h \
    mainviews.h \
    midirender.h \
    tabviews.h \
    g0/aclipboard.h \
    tabcommands.h \
    g0/wavefile.h \
    g0/fft.h \
    g0/waveanalys.h \
    g0/ptbfile.h \
    android_helper.h \
    centerview.h \
    qthelper.h \
    inputviews.h \
    midiengine.h \
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

