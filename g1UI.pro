#-------------------------------------------------
#
# Project created by QtCreator 2015-03-23T11:22:08
#
#-------------------------------------------------


#       core gui
QT += widgets multimedia network
CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17


android:QT += androidextras


TARGET = Guitarmy
TEMPLATE = app


QMAKE_CFLAGS +=  -fpermissive
QMAKE_CXXFLAGS +=  -fpermissive
QMAKE_LFLAGS +=  -fpermissive



SOURCES += main.cpp\
    audio/AudioSpeaker.cpp \
    g0/Base.cpp \
    g0/Config.cpp \
    g0/Init.cpp \
    g0/Regression.cpp \
    g0/Tests.cpp \
    g0/Threads.cpp \
    libs/sf/tsf.cpp \
    midi/MidiExport.cpp \
    midi/MidiFile.cpp \
    midi/MidiRender.cpp \
    midi/MidiSignal.cpp \
    midi/MidiTrack.cpp \
    tab/Bar.cpp \
    tab/Beat.cpp \
    tab/tools/Commands.cpp \
    tab/Effects.cpp \
    tab/tools/GmyFile.cpp \
    tab/tools/GtpFiles.cpp \
    tab/Note.cpp \
    tab/Tab.cpp \
    tab/tools/TabClipboard.cpp \
    tab/tools/TabLoader.cpp \
    tab/Track.cpp \
    ui/BarView.cpp \
    ui/CenterView.cpp \
    ui/Components.cpp \
    ui/GPannel.cpp \
    ui/GView.cpp \
    ui/ImagePreloader.cpp \
    ui/InputViews.cpp \
    ui/MainViews.cpp \
    ui/MainWindow.cpp \
    ui/TabCommands.cpp \
    ui/TabViews.cpp \
    ui/TrackView.cpp \
    tab/tools/tab_com.cpp \
    tab/tools/track_com.cpp


HEADERS  += \
    audio/AudioSpeaker.hpp \
    g0/Base.hpp \
    g0/Config.hpp \
    g0/Init.hpp \
    g0/Regression.hpp \
    g0/Tests.hpp \
    g0/Threads.hpp \
    midi/MidiExport.hpp \
    midi/MidiFile.hpp \
    midi/MidiRender.hpp \
    midi/MidiSignal.hpp \
    midi/MidiTrack.hpp \
    tab/Bar.hpp \
    tab/Beat.hpp \
    tab/BeatStructs.hpp \
    tab/Chain.hpp \
    tab/tools/Commands.hpp \
    tab/Effects.hpp \
    tab/tools/GmyFile.hpp \
    tab/tools/GtpFiles.hpp \
    tab/Note.hpp \
    tab/NoteStructs.hpp \
    tab/Tab.hpp \
    tab/tools/TabClipboard.hpp \
    tab/tools/TabLoader.hpp \
    tab/TabStructs.hpp \
    tab/Track.hpp \
    tab/TrackStructs.hpp \
    ui/BarView.hpp \
    ui/CenterView.hpp \
    ui/Components.hpp \
    libs/sf/tml.h \
    libs/sf/tsf.h \
    ui/GPannel.hpp \
    ui/GView.hpp \
    ui/ImagePreloader.hpp \
    ui/InputViews.hpp \
    ui/MainViews.hpp \
    ui/MainWindow.hpp \
    ui/TabViews.hpp \
    ui/TrackView.hpp



DISTFILES += \
    g0/todo.txt \
    android/AndroidManifest.xml \
    android/res/values/libs.xml \
    libs/sf/LICENSE \
    libs/sf/README.md \
    android/src/in/guitarmy/app/AndroidHelper.java

INSTALLS += target


RESOURCES += \
    resources/icons.qrc \
    resources/tests.qrc \
    resources/info.qrc \
    resources/icons2.qrc

