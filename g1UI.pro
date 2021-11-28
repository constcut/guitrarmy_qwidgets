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
    g0/bar.cpp \
    g0/beat.cpp \
    g0/note.cpp \
    g0/tab_com.cpp \
    g0/track.cpp \
    g0/track_com.cpp \
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
    g0/rec.cpp


HEADERS  += mainwindow.h \
    g0/bar.h \
    g0/beat.h \
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
    mainviews.h \
    tabviews.h \
    g0/aclipboard.h \
    tabcommands.h \
    g0/wavefile.h \
    g0/fft.h \
    g0/waveanalys.h \
    othLibs/ffft/Array.h \
    othLibs/ffft/Array.hpp \
    othLibs/ffft/def.h \
    othLibs/ffft/DynArray.hpp \
    othLibs/ffft/FFTReal.hpp \
    othLibs/ffft/FFTRealFixLen.hpp \
    othLibs/ffft/FFTRealPassDirect.hpp \
    othLibs/ffft/FFTRealPassInverse.hpp \
    othLibs/ffft/FFTRealSelect.hpp \
    othLibs/ffft/FFTRealUseTrigo.hpp \
    othLibs/ffft/OscSinCos.hpp \
    g0/ptbfile.h \
    android_helper.h \
    centerview.h \
    qthelper.h \
    inputviews.h \
    midiengine.h \
    g0/rec.h



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
    othLibs/ffft/license.txt \
    othLibs/ffft/readme.txt \
    android/src/in/guitarmy/app/AndroidHelper.java

INSTALLS += target

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

RESOURCES += \
    icons.qrc \
    tests.qrc \
    info.qrc \
    icons2.qrc

