QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    g0/Base.cpp \
    g0/Threads.cpp \
    stats_main.cpp \
    tab/Bar.cpp \
    tab/Beat.cpp \
    tab/Effects.cpp \
    tab/Note.cpp \
    tab/Tab.cpp \
    tab/Track.cpp \
    tab/tools/Commands.cpp \
    tab/tools/GmyFile.cpp \
    tab/tools/GtpFiles.cpp \
    tab/tools/TabClipboard.cpp \
    tab/tools/TabLoader.cpp \
    tab/tools/tab_com.cpp \
    tab/tools/track_com.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    g0/Base.hpp \
    g0/Threads.hpp \
    tab/Bar.hpp \
    tab/Beat.hpp \
    tab/BeatStructs.hpp \
    tab/Chain.hpp \
    tab/Effects.hpp \
    tab/Note.hpp \
    tab/NoteStructs.hpp \
    tab/Tab.hpp \
    tab/TabStructs.hpp \
    tab/Track.hpp \
    tab/TrackStructs.hpp \
    tab/tools/Commands.hpp \
    tab/tools/GmyFile.hpp \
    tab/tools/GtpFiles.hpp \
    tab/tools/TabClipboard.hpp \
    tab/tools/TabLoader.hpp
