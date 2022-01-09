#-------------------------------------------------
#
# Project created by QtCreator 2015-03-23T11:22:08
#
#-------------------------------------------------


#       core gui
QT += widgets multimedia network
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
    audio/AudioSpeaker.cpp \
    audio/FFT.cpp \
    audio/Recording.cpp \
    audio/WaveAnalys.cpp \
    g0/Base.cpp \
    g0/Config.cpp \
    g0/Init.cpp \
    g0/Regression.cpp \
    g0/Tests.cpp \
    g0/Threads.cpp \
    libs/sf/tsf.cpp \
    midi/MidiEngine.cpp \
    midi/MidiExport.cpp \
    midi/MidiFile.cpp \
    midi/MidiRender.cpp \
    midi/MidiSignal.cpp \
    midi/MidiTrack.cpp \
    tab/Bar.cpp \
    tab/Beat.cpp \
    tab/Commands.cpp \
    tab/Effects.cpp \
    tab/GmyFile.cpp \
    tab/GtpFiles.cpp \
    tab/Note.cpp \
    tab/Tab.cpp \
    tab/TabClipboard.cpp \
    tab/TabLoader.cpp \
    tab/Track.cpp \
    ui/barview.cpp \
    tab/tab_com.cpp \
    tab/track_com.cpp \
    ui/components.cpp \
    ui/imagepreloader.cpp \
    ui/mainwindow.cpp \
    ui/gview.cpp \
    ui/athread.cpp \
    ui/gpannel.cpp \
    ui/mainviews.cpp \
    ui/tabviews.cpp \
    ui/tabcommands.cpp \
    ui/centerview.cpp \
    ui/inputviews.cpp \
    ui/trackview.cpp


HEADERS  += ui/mainwindow.h \
    audio/AudioSpeaker.hpp \
    audio/FFT.hpp \
    audio/Recording.hpp \
    audio/WaveAnalys.hpp \
    g0/Base.hpp \
    g0/Config.hpp \
    g0/Init.hpp \
    g0/Regression.hpp \
    g0/Tests.hpp \
    g0/Threads.hpp \
    midi/MidiEngine.hpp \
    midi/MidiExport.hpp \
    midi/MidiFile.hpp \
    midi/MidiRender.hpp \
    midi/MidiSignal.hpp \
    midi/MidiTrack.hpp \
    tab/Bar.hpp \
    tab/Beat.hpp \
    tab/Commands.hpp \
    tab/Effects.hpp \
    tab/GmyFile.hpp \
    tab/GtpFiles.hpp \
    tab/Note.hpp \
    tab/Tab.hpp \
    tab/TabClipboard.hpp \
    tab/TabLoader.hpp \
    tab/Track.hpp \
    tab/Types.hpp \
    ui/barview.h \
    ui/components.h \
    ui/gpannel.h \
    ui/gview.h \
    ui/athread.h \
    libs/sf/tml.h \
    libs/sf/tsf.h \
    ui/imagepreloader.h \
    ui/mainviews.h \
    ui/tabviews.h \
    ui/centerview.h \
    ui/inputviews.h \
    ui/trackview.h



DISTFILES += \
    g0/todo.txt \
    android/AndroidManifest.xml \
    android/res/values/libs.xml \
    libs/sf/LICENSE \
    libs/sf/README.md \
    android/src/in/guitarmy/app/AndroidHelper.java

INSTALLS += target

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

RESOURCES += \
    resources/icons.qrc \
    resources/tests.qrc \
    resources/info.qrc \
    resources/icons2.qrc

