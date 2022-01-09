#ifndef INIT_H
#define INIT_H

#include <QApplication>
#include "ui/MainWindow.hpp"


void setPosixSignals();
void preloadImages();
void loadConfig();

void setTestsPath(QApplication& a);
void initResourses();
void setLibPath();

void initMidi();
void initMainWindow(MainWindow& w, QApplication& a);

void setWindowIcon(MainWindow& w);
void configureScreen(MainWindow& w);
void initNewTab(MainWindow& w);

void setLogFilename();

void copyResourcesIntoTempDir();

#endif // INIT_H
