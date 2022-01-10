// This deprecated file was autoformated with clang-format
#include "MainWindow.hpp"

#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QDebug>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScroller>
#include <QStandardPaths>
#include <QStatusBar>
#include <QSwipeGesture>
#include <QTapGesture>
#include <QTextBlock>
#include <QTextCodec>
#include <QToolButton>
#include <fstream>
#include <sstream>
#include <vector>

#include "ui/GPannel.hpp"
#include "ui/GView.hpp"
#include "ui/ImagePreloader.hpp"
#include "ui/MainViews.hpp"
#include "ui/TabViews.hpp"

#define QWIDGET_ALLOC new
// https://doc.qt.io/qt-5/objecttrees.html - we don't need to free any memory
// here, so unique is danger

using namespace gtmy;


void addToolButtonGrid(MainWindow *mainWindow, QDockWidget *dock,
                       std::string button, std::string confValue = "",
                       bool secondLines = false);
QAction *addToolButton(QToolBar *toolBar, std::string button,
                       std::string confValue);



void MainWindow::createMenuTool() {
  _menuToolBar = addToolBar("mainMenu");

  QWidget *sep1 = QWIDGET_ALLOC QWidget(this);
  sep1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep1);

  addToolButton(_menuToolBar, "backview", "esc");
  QAction *hideP1 = addToolButton(_menuToolBar, "new", "newtab");
  QAction *hideP2 =
      addToolButton(_menuToolBar, "open", CONF_PARAM("Main.open"));

  QWidget *sep2 = QWIDGET_ALLOC QWidget(this);
  sep2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep2);

  addToolButton(_menuToolBar, "tab", "tabview");

  QWidget *sep3 = QWIDGET_ALLOC QWidget(this);
  sep3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep3);

  _hideA1 = addToolButton(_menuToolBar, "tests", "tests");  // HIDE

  QWidget *sep32 = QWIDGET_ALLOC QWidget(this);
  sep32->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep32);

  _hideA2 = addToolButton(_menuToolBar, "tap", "tap");  // HIDE

  QWidget *sep31 = QWIDGET_ALLOC QWidget(this);
  sep31->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep31);

  addToolButton(_menuToolBar, "pattern", "pattern");

  QWidget *sep4 = QWIDGET_ALLOC QWidget(this);
  sep4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep4);

  addToolButton(_menuToolBar, "config", "config");

  QWidget *sep5 = QWIDGET_ALLOC QWidget(this);
  sep5->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep5);

  _hideA3 = addToolButton(_menuToolBar, "record", "rec");   // hide
  _hideA4 = addToolButton(_menuToolBar, "morze", "morze");  // hide

  QWidget *sep52 = QWIDGET_ALLOC QWidget(this);
  sep52->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep52);

  _hideA5 = addToolButton(_menuToolBar, "info", "info");  // hide

  QWidget *sep51 = QWIDGET_ALLOC QWidget(this);
  sep51->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep51);

  QAction *hideP3 = addToolButton(_menuToolBar, "undo", "undo");
  addToolButton(_menuToolBar, "play", CONF_PARAM("TrackView.playMidi"));

  QWidget *sep6 = QWIDGET_ALLOC QWidget(this);
  sep6->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep6);

  addToolButton(_menuToolBar, "openPannel", "openPannel");

  QWidget *sep7 = QWIDGET_ALLOC QWidget(this);
  sep7->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _menuToolBar->addWidget(sep7);

  hideP1->setVisible(false);
  hideP2->setVisible(false);
  hideP3->setVisible(false);
  // hideP4->setVisible(false);
  _hideA4->setVisible(false);

  _hideA1->setVisible(false);

  this->connect(_menuToolBar, SIGNAL(actionTriggered(QAction *)),
                SLOT(actionNow(QAction *)));

  if (CONF_PARAM("toolBar") == "right") {
    _menuToolBar->setOrientation(Qt::Vertical);
    addToolBar(Qt::RightToolBarArea, _menuToolBar);
  }

  if (CONF_PARAM("toolBar") == "left") {
    _menuToolBar->setOrientation(Qt::Vertical);
    addToolBar(Qt::LeftToolBarArea, _menuToolBar);
  }

  if (CONF_PARAM("toolBar") == "top") {
    _menuToolBar->setOrientation(Qt::Horizontal);
    addToolBar(Qt::TopToolBarArea, _menuToolBar);
  }

  if (CONF_PARAM("toolBar") == "bottom") {
    _menuToolBar->setOrientation(Qt::Horizontal);
    addToolBar(Qt::BottomToolBarArea, _menuToolBar);
  }

  _menuToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  _menuToolBar->setMovable(false);
  _menuToolBar->setFloatable(true);

  QScreen *screen = QApplication::screens().at(0);
  double autoCoef = screen->geometry().width() > screen->geometry().height()
                        ? screen->geometry().width()
                        : screen->geometry().height();

  autoCoef /= 800;

  std::stringstream menuStyleStr;
  int menuPlatformCoef = 1;

  if (AConfig::getInst().platform == "windows")  // check
    menuPlatformCoef = 4;

  menuStyleStr << "QMenu { font-size:"
               << (int)(autoCoef * 30 / menuPlatformCoef)
               << "px;}QToolBar {border: 1px solid black;}";
  _menuToolBar->setStyleSheet(
      menuStyleStr.str()
          .c_str());  //"QMenu { font-size:40px;}"); //QToolBar{spacing:15px;}

  int iconSize = 36 * autoCoef;  // 36

  if (AConfig::getInst().isMobile == false) {
    iconSize = 36;  // 36 debug
    autoCoef = 1.0;
  }

  _menuToolBar->setIconSize(QSize(iconSize, iconSize));

  AConfig::getInst().setScaleCoef(autoCoef);

  if (CONF_PARAM("toolBar") == "0") {
    _menuToolBar->hide();
  }

  if (CONF_PARAM("statusBar") == "0") {
    statusBar()->hide();
  }
  else
    statusBar()->show();
}


void MainWindow::createMainToolbar() {
  if (CONF_PARAM("mainMenu") == "1") {
    // create menu

    QMenu *m1 = menuBar()->addMenu("tab");
    m1->addAction("tab");
    m1->addAction("opentrack");
    m1->addAction("new");
    m1->addAction("open");
    m1->addAction("save");

    QMenu *m1Addition = QWIDGET_ALLOC QMenu(m1);
    m1->addMenu(m1Addition)->setIconText("export");
    m1Addition->addAction("midi");
    m1Addition->addAction("text");
    m1->addAction("exit");

    QMenu *m2 = menuBar()->addMenu("inputs");
    m2->addAction("pattern");
    m2->addAction("tap");
    m2->addAction("rec");
    m2->addAction("morze");
    // piano roll

    QMenu *m3 = menuBar()->addMenu("edit");
    m3->addAction("copy");
    m3->addAction("cut");
    m3->addAction("paste");

    QMenu *m3Addition = QWIDGET_ALLOC QMenu(m3);
    m3Addition->addAction("bufer1");
    m3Addition->addAction("bufer2");
    m3Addition->addAction("bufer3");
    m3->addMenu(m3Addition)->setIconText("chose bufer");

    QMenu *m3Addition2 = QWIDGET_ALLOC QMenu(m3);
    m3Addition2->addAction("effects");

    QMenu *m3Addition3 = QWIDGET_ALLOC QMenu(m3);
    m3Addition3->addAction("specif");
    m3->addMenu(m3Addition2)->setIconText("effects");
    m3->addMenu(m3Addition3)->setIconText("specif");

    QMenu *m4 = menuBar()->addMenu("info");
    m4->addAction("info");
    m4->addAction("config");
    m4->addAction("about");
    m4->addAction("tests");
    menuBar()->setFont(QFont("Arial", 30));
  }

  createMenuTool();
}


QDockWidget *MainWindow::createToolDock(std::string dockname, GPannel *pan) {
  QDockWidget *dockTab = QWIDGET_ALLOC QDockWidget(dockname.c_str(), this);
  QMainWindow *dockWin = QWIDGET_ALLOC QMainWindow(0);

  dockWin->setWindowFlags(Qt::Widget);
  QToolBar *men = dockWin->addToolBar(dockname.c_str());
  men->setMovable(false);
  men->setFloatable(false);

  QWidget *spacerWidget = QWIDGET_ALLOC QWidget(this);
  spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  spacerWidget->setVisible(true);
  men->addWidget(spacerWidget);

  QWidget *titleWidget = QWIDGET_ALLOC QWidget(this);
  dockTab->setTitleBarWidget(titleWidget);
  QToolBar *men2 = 0;

  size_t middleBreak = pan->_buttons.size() / 2;
  if (middleBreak < 10)
      middleBreak = 4;

  if (CONF_PARAM("pannels") == "oneline")
      middleBreak = 100;  // force one line

  for (size_t i = 0; i < pan->_buttons.size(); ++i)
    if (i > middleBreak)
    {
      if (men2 == 0) {
        dockWin->addToolBarBreak();
        men2 = dockWin->addToolBar("second");
        men2->setMovable(false);
        men2->setFloatable(false);

        QWidget *spacerWidget2 = QWIDGET_ALLOC QWidget(this);
        spacerWidget2->setSizePolicy(QSizePolicy::Expanding,
                                     QSizePolicy::Preferred);
        spacerWidget2->setVisible(true);
        men2->addWidget(spacerWidget2);
      }

      addToolButton(men2, pan->_buttons[i].getText(),
                    pan->_buttons[i].getPressSyn());

    }
    else {  // first
      addToolButton(men, pan->_buttons[i].getText(),
                    pan->_buttons[i].getPressSyn());
    }

  dockTab->resize(dockTab->minimumSizeHint());
  dockTab->setWidget(dockWin);
  removeDockWidget(dockTab);

  int iconSize = _menuToolBar->iconSize().width();
  men->setIconSize(QSize(iconSize, iconSize));
  int iCoef = 4;  // for the clip

  if (men2) {
    iCoef = 4;
    men2->setIconSize(QSize(iconSize, iconSize));
    this->connect(men2, SIGNAL(actionTriggered(QAction *)),
                  SLOT(actionNow(QAction *)));
  }

  dockWin->setMaximumHeight(iconSize * iCoef);
  dockWin->setMaximumWidth(maximumWidth());
  dockWin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  dockTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  this->connect(men, SIGNAL(actionTriggered(QAction *)),
                SLOT(actionNow(QAction *)));

  dockTab->setAllowedAreas(Qt::BottomDockWidgetArea);
  addDockWidget(Qt::BottomDockWidgetArea, dockTab);  //
  dockTab->show();
  return dockTab;
}


QMenu *MainWindow::createToolMenu(GPannel *pan) {
  QMenu *menu = QWIDGET_ALLOC QMenu(0);

  std::string iconsSet;
  if (CONF_PARAM("iconsSet") == "1")
    iconsSet = ":/icons/";
  else
    iconsSet = ":/icons2/";

  for (size_t i = 0; i < pan->_buttons.size(); ++i) {
    std::string button = pan->_buttons[i].getText();
    std::string iconPlace = iconsSet + button + std::string(".png");
    QIcon icon(iconPlace.c_str());
    std::string press = pan->_buttons[i].getPressSyn();
    menu->addAction(icon, press.c_str());
  }

  return menu;
}


void MainWindow::createUI() {

  GPannel *pan1 = QWIDGET_ALLOC GTabPannel(1, 2, 3);
  GPannel *pan2 = QWIDGET_ALLOC GTrackPannel(1, 2, 3);
  GPannel *pan3 = QWIDGET_ALLOC GEffectsPannel(1, 2, 3);
  GPannel *pan4 = QWIDGET_ALLOC GClipboardPannel(1, 2, 3);
  _menu1 = createToolMenu(pan1);
  _menu2 = createToolMenu(pan2);
  _menu3 = createToolMenu(pan3);
  _menu4 = createToolMenu(pan4);

  createMainToolbar();
  createFloDocks();

  _statusLabel = QWIDGET_ALLOC QLabel(this);
  _statusLabelSecond = QWIDGET_ALLOC QLabel(this);
  _winCombo = QWIDGET_ALLOC GQCombo(this);
  _winCombo->addItem("welcome");

  if (CONF_PARAM("") == "1")
      _winCombo->addItem("tabview");  // tab

  _winCombo->addItem("trackview");
  _winCombo->addItem("tap");
  _winCombo->addItem("rec");
  _winCombo->addItem("pattern");
  _winCombo->addItem("tests");
  _winCombo->addItem("config");
  _winCombo->addItem("info");
  _winCombo->setElementNum(0);

  GQCombo *win2 = QWIDGET_ALLOC GQCombo(this);
  win2->addItem("menu");
  win2->addItem("track / bar");
  win2->addItem("effects");
  win2->addItem("edit / clipboard");
  win2->addItem("num / move");  // tab
  win2->addItem("all");

  win2->setElementNum(0);

  statusBar()->addWidget(_statusLabel, 4);
  statusBar()->addWidget(_statusLabelSecond, 4);
  statusBar()->addWidget(_winCombo, 3);
  statusBar()->addWidget(win2, 3);

  initAudioInput();
  initAudioOutput();

  // showHelp();

  if (_center == 0)
      _center = QWIDGET_ALLOC CenterView();

  if (_centerScroll == 0)
      _centerScroll = QWIDGET_ALLOC QScrollArea(this);

  _centerScroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  _center->setFatherScroll(_centerScroll);

  _winCombo->setKeyPress(_center);
  _winCombo->setPushItem(true);  // REFACT - rename
  win2->setKeyPress(_center);
  win2->setPushItem(true);

  double coef = AConfig::getInst().getScaleCoef();

  int initWidth = 770 * coef;
  int initHeightMin = 300 * coef;

  _center->setMinimumHeight(initHeightMin);
  _center->setMinimumWidth(initWidth);
  _center->setGeometry(0, 0, initWidth, initHeightMin);
  _center->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  _centerScroll->setWidget(_center);
  _centerScroll->setWidgetResizable(true);

  /*
  #ifndef __ANDROID_API__
      QScroller::grabGesture(centerScroll, QScroller::LeftMouseButtonGesture);
  #else
      QScroller::grabGesture(centerScroll, QScroller::TouchGesture);
  #endif
  */
  _center->setStyleSheet("background-color:rgba(0,0,0,0);");
  setCentralWidget(_centerScroll);
  _centerScroll->show();
  _center->show();

  setDockOptions(QMainWindow::AllowTabbedDocks | QMainWindow::AnimatedDocks |
                 QMainWindow::AllowNestedDocks);

  setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  if (CONF_PARAM("pannels") == "tabbed") {
    QDockWidget *tabDock = createToolDock("tab", pan1);
    QDockWidget *trackDock = createToolDock("track", pan2);
    QDockWidget *effectsDock = createToolDock("effects", pan3);
    QDockWidget *clipDock = createToolDock("clipboard", pan4);
    QDockWidget *userDock = createToolDock("user", new GUserPannel(1, 2, 3));
    // Menu from pannel QMenu *tabMenu = createToolMenu("tab",pan1);

    _dock5 = tabDock;
    _dock6 = trackDock;
    _dock7 = effectsDock;
    _dock8 = clipDock;
    _dock9 = userDock;

    tabifyDockWidget(trackDock, tabDock);
    tabifyDockWidget(trackDock, effectsDock);
    tabifyDockWidget(trackDock, clipDock);
    tabifyDockWidget(trackDock, userDock);
    tabifyDockWidget(userDock, trackDock);

    _dock5->hide();
    _dock6->hide();
    _dock7->hide();
    _dock8->hide();
    _dock9->hide();
  }
  else if (CONF_PARAM("pannels") == "oneline")  // classic is banned
  {
    GPannel *pan1 = new GNumPannel();
    GPannel *pan3 = new GEditPannel();
    GPannel *pan6 = new GNotePannel();
    GPannel *pan7 = new GTrackNewPannel();
    GPannel *pan9 = new GUserPannel(1, 2, 3);

    _dock5 = createToolDock("num / move", pan1);
    _dock6 = createToolDock("edit / clipboard", pan3);
    _dock7 = createToolDock("effects", pan6);
    _dock8 = createToolDock("track / bar", pan7);
    _dock9 = createToolDock("menu", pan9);

    tabifyDockWidget(_dock9, _dock5);
    tabifyDockWidget(_dock9, _dock6);
    tabifyDockWidget(_dock9, _dock7);
    tabifyDockWidget(_dock9, _dock8);
  }
  if (CONF_PARAM("pannels") == "docked") {
    _dock->show();
    _dock2->show();
    _dock3->show();
    _dock4->show();
  }
}


void MainWindow::recreateUI() {
  _dock5->setVisible(false);
  _dock6->setVisible(false);
  _dock7->setVisible(false);
  _dock8->setVisible(false);
  _dock9->setVisible(false);

  removeDockWidget(_dock5);
  removeDockWidget(_dock6);
  removeDockWidget(_dock7);
  removeDockWidget(_dock8);
  removeDockWidget(_dock9);

  _dock5->deleteLater();
  _dock6->deleteLater();
  _dock7->deleteLater();
  _dock8->deleteLater();
  _dock9->deleteLater();

  _dock5 = nullptr;
  _dock6 = nullptr;
  _dock7 = nullptr;
  _dock8 = nullptr;
  _dock9 = nullptr;

  removeToolBar(_menuToolBar);
  createMenuTool();

  if (CONF_PARAM("pannels") == "tabbed") {
    GPannel *pan1 = QWIDGET_ALLOC GTabPannel(1, 2, 3);
    GPannel *pan2 = QWIDGET_ALLOC GTrackPannel(1, 2, 3);
    GPannel *pan3 = QWIDGET_ALLOC GEffectsPannel(1, 2, 3);
    GPannel *pan4 = QWIDGET_ALLOC GClipboardPannel(1, 2, 3);
    QDockWidget *tabDock = createToolDock("tab", pan1);
    QDockWidget *trackDock = createToolDock("track", pan2);
    QDockWidget *effectsDock = createToolDock("effects", pan3);
    QDockWidget *clipDock = createToolDock("clipboard", pan4);

    QDockWidget *userDock = createToolDock("user", new GUserPannel(1, 2, 3));

    _dock5 = tabDock;
    _dock6 = trackDock;
    _dock7 = effectsDock;
    _dock8 = clipDock;
    _dock9 = userDock;

    tabifyDockWidget(trackDock, tabDock);
    tabifyDockWidget(trackDock, effectsDock);
    tabifyDockWidget(trackDock, clipDock);
    tabifyDockWidget(trackDock, userDock);
    tabifyDockWidget(userDock, trackDock);

  }
  else if (CONF_PARAM("pannels") == "oneline")  // classic is banned
  {
    GPannel *pan1 = new GNumPannel();
    GPannel *pan3 = new GEditPannel();
    GPannel *pan6 = new GNotePannel();
    GPannel *pan7 = new GTrackNewPannel();
    GPannel *pan9 = new GUserPannel(1, 2, 3);

    _dock5 = createToolDock("num / move", pan1);
    _dock6 = createToolDock("edit / clipboard", pan3);
    _dock7 = createToolDock("effects", pan6);
    _dock8 = createToolDock("track / bar", pan7);
    _dock9 = createToolDock("menu", pan9);

    tabifyDockWidget(_dock9, _dock5);
    tabifyDockWidget(_dock9, _dock6);
    tabifyDockWidget(_dock9, _dock7);
    tabifyDockWidget(_dock9, _dock8);
  }
}


bool MainWindow::eventFilter([[maybe_unused]] QObject *object, QEvent *e) {
  if (e->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
    if ((keyEvent->key() == Qt::Key_Right) ||
        (keyEvent->key() == Qt::Key_Left) || (keyEvent->key() == Qt::Key_Up) ||
        (keyEvent->key() == Qt::Key_Down) ||
        (keyEvent->key() == Qt::Key_Space)) {
      keyPressEvent(keyEvent);
      return true;
    }

  }
  return false;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), MasterView(), _centerScroll(0), _center(0) {
  _dock5 = 0;
  _dock6 = 0;
  _dock7 = 0;
  _dock8 = 0;
  _dock9 = 0;
  _winCombo = 0;

  AClipboard::setCurrentClip(&_clip1);

  qApp->installEventFilter(this);

  createUI();
}


void MainWindow::resizeEvent([[maybe_unused]] QResizeEvent *event) {
  if (geometry().width() < geometry().height()) {
    _hideA2->setVisible(false);
    _hideA3->setVisible(false);
    _hideA5->setVisible(false);
  } else {
    _hideA2->setVisible(true);
    _hideA3->setVisible(true);
    _hideA5->setVisible(true);
  }
}



int MainWindow::getCurrentViewType() {
  GView *gview = getChild();
  MainView *mv = dynamic_cast<MainView *>(gview);

  if (mv) {
    gview = mv->getCurrenView();
    if (dynamic_cast<TabView *>(gview) != 0) return 1;
    if (dynamic_cast<TrackView *>(gview) != 0) return 2;
    if (dynamic_cast<PatternInput *>(gview) != 0) return 3;
    if (dynamic_cast<TapRyView *>(gview) != 0) return 4;
    if (dynamic_cast<ConfigView *>(gview) != 0) return 6;
    if (dynamic_cast<InfoView *>(gview) != 0) return 7;
    if (dynamic_cast<TestsView *>(gview) != 0) return 8;
    if (dynamic_cast<MorzeInput *>(gview) != 0) return 9;
  }
  return 0;
}


void MainWindow::threadFinished() {
  if (isPlaylistHere()) {
    goOnPlaylist();
  }
}



void MainWindow::pushForceKey(std::string keyevent) {
  if (keyevent == "openPannel") {
    if (_dock5) {
      if (_dock5->isVisible()) {
        _dock5->setVisible(false);
        _dock6->setVisible(false);
        _dock7->setVisible(false);
        _dock8->setVisible(false);
        _dock9->setVisible(false);
      } else {
        _dock5->setVisible(true);
        _dock6->setVisible(true);
        _dock7->setVisible(true);
        _dock8->setVisible(true);
        _dock9->setVisible(true);
      }
    }
  }
  if (keyevent == "playMidi") {
    if (_center) _center->pushForceKey(CONF_PARAM("TrackView.playMidi"));
  }
  if (keyevent == "menu")  // REFACT - make checkable
  {
    _dock5->setVisible(false);
    _dock6->setVisible(false);
    _dock7->setVisible(false);
    _dock8->setVisible(false);
    _dock9->setVisible(true);
  }
  if (keyevent == "track / bar") {
    _dock5->setVisible(false);
    _dock6->setVisible(false);
    _dock7->setVisible(false);
    _dock8->setVisible(true);
    _dock9->setVisible(false);
  }
  if (keyevent == "effects") {
    _dock5->setVisible(false);
    _dock6->setVisible(false);
    _dock7->setVisible(true);
    _dock8->setVisible(false);
    _dock9->setVisible(false);
  }
  if (keyevent == "edit / clipboard") {
    _dock5->setVisible(false);
    _dock6->setVisible(true);
    _dock7->setVisible(false);
    _dock8->setVisible(false);
    _dock9->setVisible(false);
  }
  if (keyevent == "num / move") {
    _dock5->setVisible(true);
    _dock6->setVisible(false);
    _dock7->setVisible(false);
    _dock8->setVisible(false);
    _dock9->setVisible(false);
  }
  if (keyevent == "all") {
    _dock5->setVisible(true);
    _dock6->setVisible(true);
    _dock7->setVisible(true);
    _dock8->setVisible(true);
    _dock9->setVisible(true);
  }

  if (keyevent == "start_audioi")
    startAudioInput();
  if (keyevent == "stop_audioi")
    stopAudioInput();

  if (keyevent == "stop_record_output")
    stopAudioOutput();

  if (keyevent.substr(0, 19) == "start_record_output") {
    std::string stdFileName = keyevent.substr(20);
    startAudioOutput(stdFileName);
  }
}


void MainWindow::actionNow(QAction *action) {
  QString textOut;
  textOut = action->iconText();

  if (textOut.size() > 0)
    qDebug() << "Action now trig: " << textOut.toStdString().c_str();

  if (textOut == "tab") {
    if (_dock) {
      if (_dock->isHidden()) {
        _dock->setGeometry(this->pos().x() + 50, this->pos().y() + 390, 750,
                           110);
        _dock->show();
      } else
        _dock->hide();
    }
  }

  if (textOut == "track_dock") {
    if (_dock3) {
      if (_dock3->isHidden()) {
        _dock3->setGeometry(this->pos().x() + 50, this->pos().y() + 390, 650,
                            110);
        _dock3->show();
      } else
        _dock3->hide();
    }
  }

  if (textOut == "effects") {
    if (_dock4) {
      if (_dock4->isHidden()) {
        _dock4->setGeometry(this->pos().x() + 50, this->pos().y() + 390, 750,
                            110);
        _dock4->show();
      } else
        _dock4->hide();
    }
  }

  if (textOut == "clipboard") {
    if (_dock2) {
      if (_dock2->isHidden()) {
        _dock2->setGeometry(this->pos().x() + 50, this->pos().y() + 390, 500,
                            60);
        _dock2->show();
      } else
        _dock2->hide();
    }
  }

  if (textOut == "zoomIn") {
    double currentScale = AConfig::getInst().getScaleCoef();

    if (currentScale <= 4.0) {
      currentScale += 0.25;
      AConfig::getInst().setScaleCoef(currentScale);

      int initWidth = 770 * currentScale;
      int initHeightMin = 300 * currentScale;
      // int initHeightMax = 500 * currentScale;

      _center->setMinimumHeight(initHeightMin);
      // center->setMaximumHeight(initHeightMax);
      _center->setMinimumWidth(initWidth);

      _center->setGeometry(0, 0, initWidth, initHeightMin);
    }
  }
  if (textOut == "zoomOut") {
    double currentScale = AConfig::getInst().getScaleCoef();

    if (currentScale >= 0.5) {
      currentScale -= 0.25;
      AConfig::getInst().setScaleCoef(currentScale);

      int initWidth = 770 * currentScale;
      int initHeightMin = 300 * currentScale;
      // int initHeightMax = 500 * currentScale;

      _center->setMinimumHeight(initHeightMin);
      // center->setMaximumHeight(initHeightMax);
      _center->setMinimumWidth(initWidth);

      _center->setGeometry(0, 0, initWidth, initHeightMin);
    }
  }

  if (textOut == "clip1")
      AClipboard::setCurrentClip(&_clip1);
  if (textOut == "clip2")
      AClipboard::setCurrentClip(&_clip2);
  if (textOut == "clip3")
      AClipboard::setCurrentClip(&_clip3);

  if (textOut == "start_audioi")  {
    startAudioInput();
    return;
  }
  if (textOut == "stop_audioi") {
    stopAudioInput();
    return;
  }
  if (textOut == "stop_record_output") {
    stopAudioOutput();
    return;
  }

  if (textOut.mid(0, 19) == "start_record_output") {
    QString fileName = textOut.mid(20);
    std::string stdFileName = fileName.toStdString();
    startAudioOutput(stdFileName);
    return;
  }

  if (textOut == "exit") {
    exit(0);
  }

  if (textOut == "darkSkin") {
    DarkSkin skin;
    skin.setIntoConfig(AConfig::getInst());
#ifndef __ANDROID_API__
    recreateUI();  // to remember the bug
#endif
    update();
    return;
  }
  if (textOut == "lightSkin") {
    LightSkin skin;
    skin.setIntoConfig(AConfig::getInst());
#ifndef __ANDROID_API__
    recreateUI();  // refact return back
#endif
    update();
    return;
  }
  if (textOut == "classicSkin") {
    ClassicSkin skin;
    skin.setIntoConfig(AConfig::getInst());
#ifndef __ANDROID_API__
    recreateUI();  // refact return back
#endif
    update();
    return;
  }
  if (textOut == "classicInvSkin") {
    ClassicInvertedSkin skin;
    skin.setIntoConfig(AConfig::getInst());
#ifndef __ANDROID_API__
    recreateUI();  // refact return back
#endif
    update();
    return;
  }

  if (textOut == "save config") {
    std::string confFileName =
        std::string(AConfig::getInst().testsLocation) + "g.config";
    std::ofstream confFile(confFileName);
    AConfig::getInst().save(confFile);
    return;
  }

  if (textOut == "recreateUI") {
    recreateUI();
    return;
  }

  if (textOut == "openPannel") {
    if (_dock5) {
      if (_dock5->isVisible()) {
        _dock5->setVisible(false);
        _dock6->setVisible(false);
        _dock7->setVisible(false);
        _dock8->setVisible(false);
        _dock9->setVisible(false);
      } else {
        _dock5->setVisible(true);
        _dock6->setVisible(true);
        _dock7->setVisible(true);
        _dock8->setVisible(true);
        _dock9->setVisible(true);
      }
    } else {
      if (CONF_PARAM("pannels") == "tabbed") {
        GPannel *pan1 = QWIDGET_ALLOC GTabPannel(1, 2, 3);
        GPannel *pan2 = QWIDGET_ALLOC GTrackPannel(1, 2, 3);  // memory leak
        GPannel *pan3 = QWIDGET_ALLOC GEffectsPannel(1, 2, 3);
        GPannel *pan4 = QWIDGET_ALLOC GClipboardPannel(1, 2, 3);  // memleak

        QDockWidget *tabDock = createToolDock("tab", pan1);
        QDockWidget *trackDock = createToolDock("track", pan2);
        QDockWidget *effectsDock = createToolDock("effects", pan3);
        QDockWidget *clipDock = createToolDock("clipboard", pan4);

        QDockWidget *userDock =
            createToolDock("user", new GUserPannel(1, 2, 3));
        // Menu from pannel QMenu *tabMenu = createToolMenu("tab",pan1);

        _dock5 = tabDock;
        _dock6 = trackDock;
        _dock7 = effectsDock;
        _dock8 = clipDock;
        _dock9 = userDock;
        tabifyDockWidget(trackDock, tabDock);
        tabifyDockWidget(trackDock, effectsDock);
        tabifyDockWidget(trackDock, clipDock);
        tabifyDockWidget(trackDock, userDock);
        tabifyDockWidget(userDock, trackDock);
      }
    }
    update();
    if (CONF_PARAM("pannels") != "classic")
      return;  // think about this pair..update in some destructor
  }

  if (textOut == "only user") {
    _dock5->setVisible(false);
    _dock6->setVisible(false);
    _dock7->setVisible(false);
    _dock8->setVisible(false);
    _dock9->setVisible(true);
    return;
  }
  if (textOut == "only clipboard") {
    _dock5->setVisible(false);
    _dock6->setVisible(false);
    _dock7->setVisible(false);
    _dock8->setVisible(true);
    _dock9->setVisible(false);
    return;
  }
  if (textOut == "only effects") {
    _dock5->setVisible(false);
    _dock6->setVisible(false);
    _dock7->setVisible(true);
    _dock8->setVisible(false);
    _dock9->setVisible(false);
    return;
  }
  if (textOut == "only track") {
    _dock5->setVisible(false);
    _dock6->setVisible(true);
    _dock7->setVisible(false);
    _dock8->setVisible(false);
    _dock9->setVisible(false);
    return;
  }
  if (textOut == "only tab") {
    _dock5->setVisible(true);
    _dock6->setVisible(false);
    _dock7->setVisible(false);
    _dock8->setVisible(false);
    _dock9->setVisible(false);
    return;
  }
  if (textOut == "all pannels") {
    _dock5->setVisible(true);
    _dock6->setVisible(true);
    _dock7->setVisible(true);
    _dock8->setVisible(true);
    _dock9->setVisible(true);
    return;
  }

  if (textOut.size() > 0)
    _center->pushForceKey(textOut.toStdString());
  update();
}


void MainWindow::moveEvent(QMoveEvent *ev) {
  int x1 = ev->pos().x();
  int y1 = ev->pos().y();

  int x2 = ev->oldPos().x();
  int y2 = ev->oldPos().y();

  if (_dock) {
    int dockX = _dock->pos().x();
    int dockY = _dock->pos().y();

    int newDX = x1 - x2 + dockX;
    int newDY = y1 - y2 + dockY;

    _dock->setGeometry(newDX, newDY, 750, 110);
  }
}


void MainWindow::setStatusBarMessage(size_t index, std::string text,
                                     int timeOut) {
  QString uniText = QTextCodec::codecForLocale()->toUnicode(text.c_str());
  if (index == 0) {
    // if (timeOut==0) timeOut = 10000;

    statusBar()->showMessage(uniText, timeOut);
  }
  if (index == 1) _statusLabel->setText(uniText);
  if (index == 2) _statusLabelSecond->setText(uniText);
  if (index == 3) {
    qDebug() << "No 3rd status label now!!";
  }
}


void MainWindow::paintEvent([[maybe_unused]] QPaintEvent *event) {
  QPainter painter(this);

  QFont defaultFont("Times");  // arial-9
  defaultFont.setPixelSize(14);
  painter.setFont(defaultFont);

  double scaleCoef = AConfig::getInst().getScaleCoef();

  qreal scaleX = scaleCoef;
  qreal scaleY = scaleCoef;

  painter.scale(scaleX, scaleY);

  if (getChild()) getChild()->draw(&painter);
}


void MainWindow::mouseReleaseEvent(QMouseEvent *mEvent) {
  double scaleCoef = AConfig::getInst().getScaleCoef();

  int xPress = mEvent->pos().x() / scaleCoef;
  int yPress = mEvent->pos().y() / scaleCoef;

  int xDiff = _lastPressX - xPress;
  int yDiff = _lastPressY - yPress;

  bool isMoved = ((abs(xDiff) + abs(yDiff)) > 20);

  int gesturesMlt = CONF_PARAM("gestures") == "classic" ? -1 : 1;

  if ((abs(xDiff * 5) < abs(yDiff)) && (isMoved)) {
    // vertical definitly
    yDiff *= gesturesMlt;
    if (getChild()) getChild()->ongesture(yDiff, false);

  } else if ((abs(xDiff) > abs(yDiff * 5)) && (isMoved)) {
    // horizontal
    xDiff *= gesturesMlt;
    if ((abs(xDiff) + abs(yDiff)) > 20)
      if (getChild()) getChild()->ongesture(xDiff, true);

  } else {
    if (getChild()) getChild()->onclick(xPress, yPress);
  }

  update();
}


void MainWindow::keyPressEvent(QKeyEvent *event) {
  int value = event->key();
  char key = (char)value;
  char c_style[2];

  c_style[0] = tolower(key);
  c_style[1] = 0;

  std::string singleKey = "";
  singleKey = c_style;

  if (value == Qt::Key_Right)
    singleKey = ">>>";
  else if (value == Qt::Key_Left)
    singleKey = "<<<";
  else if (value == Qt::Key_Up)
    singleKey = "^^^";
  else if (value == Qt::Key_Down)
    singleKey = "vvv";
  else if (value == Qt::Key_Space)
    singleKey = "spc";
  else if (value == Qt::Key_Return)
    singleKey = "ent";
  else if (value == Qt::Key_Delete)
    singleKey = "del";
  else if (value == Qt::Key_Escape)
    singleKey = "esc";
  else if (value == Qt::Key_Back)
    singleKey = "esc";  // key back !!
  else if (value == Qt::Key_Insert)
    singleKey = "ins";

  if (event->modifiers() & Qt::ControlModifier)
    singleKey = std::string("ctrl+") + singleKey;
  if (event->modifiers() & Qt::AltModifier)
    singleKey = std::string("alt+") + singleKey;
  if (event->modifiers() & Qt::ShiftModifier)
    singleKey = std::string("shift+") + singleKey;
  if (_center)
      _center->pushForceKey(singleKey);

  event->accept();
  this->repaint();
}


void MainWindow::mousePressEvent(QMouseEvent *event) {
  int xPress = event->localPos().x();  // event->globalX();
  int yPress = event->localPos().y();  // event->globalY();

  if (event->buttons() & Qt::RightButton) {
    // not used yet
  }

  if (event->buttons() & Qt::LeftButton) {
    double scaleCoef = AConfig::getInst().getScaleCoef();

    _lastPressX = xPress / scaleCoef;
    _lastPressY = yPress / scaleCoef;
  }
}


void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    int xPress = event->localPos().x();  // event->globalX();
    int yPress = event->localPos().y();  // event->globalY();

    if (getChild()) getChild()->ondblclick(xPress, yPress);
  }
  // may be not always??
  update();
}



void MainWindow::on_action800x480_triggered() {
  int x = this->geometry().x();
  int y = this->geometry().y();
  this->setGeometry(x, y, 800, 480);
  getChild()->setW(800);
  getChild()->setH(480);
}


void MainWindow::on_action1024x600_triggered() {
  int x = this->geometry().x();
  int y = this->geometry().y();
  this->setGeometry(x, y, 1024, 600);
  getChild()->setW(1024);
  getChild()->setH(600);
}


void MainWindow::on_action1280x720_triggered() {
  int x = this->geometry().x();
  int y = this->geometry().y();
  this->setGeometry(x, y, 1280, 720);
  getChild()->setW(1280);
  getChild()->setH(720);
}


void MainWindow::on_actionExit_triggered() { exit(0); }


bool MainWindow::gestureEvent(QGestureEvent *event) {
  if (QGesture *pinch = event->gesture(Qt::PinchGesture)) {
    QPinchGesture *pinchGest = (QPinchGesture *)pinch;

    if (pinch->state() == Qt::GestureFinished)  // Finished)
    {
      qreal scale = pinchGest->totalScaleFactor();
      double dScale = scale;
      // dScale += 1.0;

      double nowScale = AConfig::getInst().getScaleCoef();

      if (CONF_PARAM("turnPinchZoomOn") == "1") {
        //
        std::string line = "Scale " + std::to_string(dScale) + "; with conf ";
        std::to_string(nowScale);

        if (scale < 0.15) {
          // very low
          dScale *= 5.0;
          nowScale *= dScale;
        }

        if (scale > 5.0) {
          dScale /= 5.0;  // then mult
          nowScale *= dScale;
        }

        AConfig::getInst().setScaleCoef(nowScale);
        update();
      }
    }
  }

  if (QGesture *swipe = event->gesture(Qt::SwipeGesture)) {
    // sX << "Swipe"; //swipeTriggered(static_cast<QSwipeGesture *>(swipe));
    QSwipeGesture *swipeGesture = static_cast<QSwipeGesture *>(swipe);
    if (swipeGesture->horizontalDirection()) {
      // sX <<"Vert";
      if (_dock5->isVisible() == false) {
        _dock5->setVisible(true);
        _dock6->setVisible(true);
        _dock7->setVisible(true);
        _dock8->setVisible(true);
        _dock9->setVisible(true);
      }
    }
  }

  if (QGesture *tapNhold = event->gesture(Qt::TapAndHoldGesture)) {
    QTapAndHoldGesture *tapGest = dynamic_cast<QTapAndHoldGesture *>(tapNhold);

    {
      if (CONF_PARAM("tapAndHoldCopy") == "1") {
        qDebug() << "Tap and hold x=" << tapGest->position().x()
                 << "; y=" << tapGest->position().y();

        if (_center)
          _center->ondblclick(tapGest->position().x(), tapGest->position().y());

        event->accept();
      }
    }
  }

  return true;
}


bool MainWindow::event(QEvent *event) {
  if (event->type() == QEvent::Gesture)
    return gestureEvent(static_cast<QGestureEvent *>(event));
  return QWidget::event(event);
}


#define audioSampleRateG 8000

void MainWindow::initAudioInput() {
  QAudioFormat format;
  format.setSampleRate(audioSampleRateG);
  format.setChannelCount(1);
  format.setSampleSize(16);
  format.setSampleType(QAudioFormat::SignedInt);
  format.setByteOrder(QAudioFormat::LittleEndian);
  format.setCodec("audio/pcm");

  QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
  if (!info.isFormatSupported(format)) {
    qDebug() << "Default format not supported - trying to use nearest";
    format = info.nearestFormat(format);
  }

  _pAudioInfo = QWIDGET_ALLOC AudioInfo(format, this);
  _pAudioInput = QWIDGET_ALLOC QAudioInput(
      QAudioDeviceInfo::defaultInputDevice(), format, this);
}


void MainWindow::audioTimerSlot() {}


void MainWindow::initAudioOutput() {
  QAudioFormat format;
  format.setSampleRate(audioSampleRateG);
  format.setChannelCount(2);
  format.setSampleSize(16);
  format.setSampleType(QAudioFormat::SignedInt);
  format.setByteOrder(QAudioFormat::LittleEndian);
  format.setCodec("audio/pcm");

  QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
  if (!info.isFormatSupported(format)) {
    qDebug() << "Default format not supported - trying to use nearest";
    format = info.nearestFormat(format);
  }

  _audioSpeaker = QWIDGET_ALLOC AudioSpeaker(format, this);
  _audioOutput = QWIDGET_ALLOC QAudioOutput(
      QAudioDeviceInfo::defaultOutputDevice(), format, this);
}


void MainWindow::startAudioOutput(std::string localName) {
  QFile audioFile;
  QString defaultRecFile;

  if (localName.empty()) {
    localName =
        AConfig::getInst().testsLocation + std::string("waveOutput.wav");
    defaultRecFile = localName.c_str();
  } else
    defaultRecFile = QString(AConfig::getInst().testsLocation.c_str()) +
                     QString(localName.c_str());

  audioFile.setFileName(defaultRecFile);

  if (audioFile.open(QIODevice::ReadOnly) == false)
    qDebug() << "Failed to open audio for output";

  QByteArray allBytes = audioFile.readAll();
  audioFile.close();

  _audioSpeaker->setAudioBufer(allBytes);
  _audioSpeaker->start();
  _audioOutput->start(_audioSpeaker);

  // may be there is a way to wait..
}


void MainWindow::stopAudioOutput() {
  _audioSpeaker->stop();
  _audioOutput->stop();
}


void MainWindow::startAudioInput() {
  _pAudioInfo->collector().clear();

  _pAudioInfo->start();
  _pAudioInput->start(_pAudioInfo);
}


void MainWindow::stopAudioInput() {
  _pAudioInfo->stop();
  _pAudioInput->stop();

  QDateTime timeDate(QDateTime::currentDateTime());
  QString st = timeDate.toString(Qt::ISODate);
  std::string time = st.toStdString();
  size_t pos1 = time.find(":", 0);
  size_t pos2 = time.find(":", pos1 + 1);

  const std::string rep = ".";
  time.replace(pos1, 1, rep);
  time.replace(pos2, 1, rep);

  std::string saveName = "record.graw";

  QString recordNameV = "rec_";
  recordNameV += time.c_str();
  recordNameV += ".graw";

  bool ok = false;
  QString inputedText = QInputDialog::getText(
      0, "Input record name", "Input record name:", QLineEdit::Normal,
      recordNameV, &ok);

  if (ok) {
    saveName = inputedText.toStdString();
    if (saveName.find(".graw") == std::string::npos) saveName += ".graw";
  } else {
    // ask do you want to delete?
  }

  /// QByteArray compress = qCompress(audioInfo->collector,7);
  QString defaultRecFile = QString(AConfig::getInst().testsLocation.c_str()) +
                           QString(saveName.c_str());
  QFile f;
  f.setFileName(defaultRecFile);
  /// int compressedSize = compress.size();

  if (f.open(QIODevice::Append)) {
    qDebug() << "Collector size was " << _pAudioInfo->collector().size();

    f.write(_pAudioInfo->collector());
    f.flush();
    f.close();
  } else
    qDebug() << "Open file for raw record error;";

  _pAudioInfo->collector().clear();

  // check up

  startAudioOutput(saveName);
}


QAction *MainWindow::addToolButton(QToolBar *toolBar, std::string button,
                                   std::string confValue) {
  std::string iconPlace = "";
  // std::string(AConfig::getInstance().testsLocation) + std::string("Icons/") +
  // button + std::string(".png");

  std::string iconsSet;
  if (CONF_PARAM("iconsSet") == "1")
    iconsSet = ":/icons/";
  else
    iconsSet = ":/icons2/";
  iconPlace = iconsSet + button + std::string(".png");

  QIcon icon(iconPlace.c_str());
  QPixmap iPix = icon.pixmap(72, 72);
  QImage imageIcon = iPix.toImage();

  if (imageIcon.isNull())
    qDebug() << "Image empty";
  else {
    if (CONF_PARAM("invertImages") == "1") imageIcon.invertPixels();
  }

  // no invert while

  QImage *imgs = (QImage *)ImagePreloader::getInstance().getImage(button);
  QPixmap result;
  if (imgs) {
    QImage scI = *imgs;
    QPixmap newPix(72, 72);
    result = newPix.fromImage(scI);
  }

  QIcon iconNew(result);
  QString actionName = button.c_str();
  if (confValue.size()) actionName = confValue.c_str();
  QAction *act = toolBar->addAction(iconNew, actionName);

  std::string sX = button + " [" + confValue + "]";
  act->setToolTip(sX.c_str());

  if (actionName == "esc") {
    QMenu *menu = QWIDGET_ALLOC QMenu(toolBar);
    menu->addAction("exit");
    menu->addAction("darkSkin");
    menu->addAction("lightSkin");
    // menu->addAction("classicInvSkin");
    menu->addAction("classicSkin");
    act->setMenu(menu);
  }

  if (actionName == "rec") {
    QMenu *menu = QWIDGET_ALLOC QMenu(toolBar);
    menu->addAction("start_audioi");
    menu->addAction("stop_audioi");
    menu->addAction("start_record_output");
    menu->addAction("stop_record_output");
    act->setMenu(menu);
  }

  if (actionName == "tabview") {
    QMenu *menu = QWIDGET_ALLOC QMenu(toolBar);
    QIcon icon2(":/icons/new.png");
    menu->addAction(icon2, "newtab");
    QIcon icon1(":/icons/open.png");
    menu->addAction(icon1, QString(CONF_PARAM("Main.open").c_str()));
    QIcon icon3(":/icons/save.png");
    menu->addAction(icon3, "quicksave");
    menu->addAction("export_midi");
    menu->addAction("save as");
    act->setMenu(menu);
  }

  if (actionName == "openPannel") {
    QMenu *menu = QWIDGET_ALLOC QMenu(toolBar);
    QMenu *docksMenu = QWIDGET_ALLOC QMenu("docks", menu);
    docksMenu->addAction("tab");
    docksMenu->addAction("track");
    docksMenu->addAction("effects");
    docksMenu->addAction("clipboard");
    menu->addMenu(docksMenu);
    QMenu *pans = QWIDGET_ALLOC QMenu(menu);
    pans->addAction("only tab");
    pans->addAction("only track");
    pans->addAction("only effects");
    pans->addAction("only clipboard");
    pans->addAction("only user");
    pans->addAction("all pannels");
    menu->addMenu(pans)->setIconText("pannels");
    QMenu *m6 = QWIDGET_ALLOC QMenu(menu);  // menuBar()->addMenu("pannels");
    m6->addMenu(_menu1)->setIconText("tab");
    m6->addMenu(_menu2)->setIconText("track_dock");
    m6->addMenu(_menu3)->setIconText("effects");
    m6->addMenu(_menu4)->setIconText("clipboard");
    _pannelsMenu = m6;
    menu->addMenu(_pannelsMenu)->setIconText("menu");
    act->setMenu(menu);
  }

  if (button == "pattern") {
    // inputs group
    std::string icon1Place = std::string(AConfig::getInst().testsLocation) +
                             std::string("Icons/") + std::string("tap.png");
    QIcon icon1(icon1Place.c_str());
    std::string icon2Place = std::string(AConfig::getInst().testsLocation) +
                             std::string("Icons/") + std::string("record.png");
    QIcon icon2(icon2Place.c_str());
    std::string icon3Place = std::string(AConfig::getInst().testsLocation) +
                             std::string("Icons/") + std::string("morze.png");
    QIcon icon3(icon3Place.c_str());
    QMenu *menu = QWIDGET_ALLOC QMenu(toolBar);
    QAction *act1 = menu->addAction(icon1, "tap");  // tap
    act1->setText("tap");
    menu->addAction(icon2, "rec");     // recording
    menu->addAction(icon3, "morze");   // morze input
    menu->addAction(icon, "pattern");  // and itself
    act->setMenu(menu);
  }

  if (button == "play") {
    QMenu *menu = QWIDGET_ALLOC QMenu(toolBar);
    menu->addAction("playMidi");
    menu->addAction("playAMusic");
    menu->addAction("playMerge");
    menu->addAction("playFromStart");
    menu->addAction("goToStart");
    act->setMenu(menu);
  }

  if (button == "config") {
    // conf group
    std::string icon1Place = std::string(AConfig::getInst().testsLocation) +
                             std::string("Icons/") + std::string("info.png");
    QIcon icon1(icon1Place.c_str());
    std::string icon2Place = std::string(AConfig::getInst().testsLocation) +
                             std::string("Icons/") + std::string("tests.png");
    QIcon icon2(icon2Place.c_str());
    QMenu *menu = QWIDGET_ALLOC QMenu(toolBar);
    menu->addAction(icon1, "info");
    menu->addAction(icon2, "tests");
    menu->addAction(icon, "config");
    menu->addAction("save config");
    menu->addAction("recreateUI");
    act->setMenu(menu);
  }

  if (confValue == CONF_PARAM("TrackView.deleteNote")) {
    QMenu *menu = QWIDGET_ALLOC QMenu(toolBar);
    menu->addAction("delete bar");
    menu->addAction("delete selected bars");
    menu->addAction("delete selected beats");
    act->setMenu(menu);
  }

  if (confValue == "signs") {
    QMenu *menu = QWIDGET_ALLOC QMenu(toolBar);
    menu->addAction("set till the end");  // signs
    menu->addAction("set for selected");  // signs too
    act->setMenu(menu);
  }

  return act;
}

void addToolButtonGrid(MainWindow *mainWindow, QDockWidget *dock,
                       std::string button, std::string confValue,
                       bool secondLines) {
  static QWidget *wi = 0;

  static int counter = -1;
  static int counter2 = -1;

  if (confValue.empty()) confValue = button;

  if (mainWindow == 0) {
    if (dock == 0) {
      counter = -1;
      counter2 = -1;
      wi = 0;
      return;
    }
  }

  if (wi == 0) {
    wi = QWIDGET_ALLOC QWidget(dock);
    dock->setWidget(wi);
    wi->setGeometry(0, 0, 700, 100);
  }

  QToolButton *but = QWIDGET_ALLOC QToolButton(wi);
  std::string iconPlace =
      "";  // std::string(AConfig::getInstance().testsLocation) +
           // std::string("Icons/") + button + std::string(".png");
  iconPlace = ":/icons/" + button + std::string(".png");
  QIcon icon(iconPlace.c_str());
  but->setIcon(icon);

  QString actionName = button.c_str();
  if (confValue.size()) actionName = confValue.c_str();

  but->setDefaultAction(new QAction(icon, confValue.c_str(), dock));
  but->setAccessibleName(confValue.c_str());
  but->setIconSize(QSize(50, 50));

  mainWindow->connect(but, SIGNAL(triggered(QAction *)),
                      SLOT(actionNow(QAction *)));

  int thisCounter = 0;
  if (secondLines)
    thisCounter = ++counter2;
  else
    thisCounter = ++counter;

  int yPos = secondLines ? 55 : 0;
  int xPos = thisCounter * 50;

  but->setGeometry(xPos, yPos, 50, 50);
}


void MainWindow::createFloDocks() {
  _dock = QWIDGET_ALLOC QDockWidget(
      this, Qt::Window | Qt::WindowStaysOnTopHint |
                Qt::X11BypassWindowManagerHint |
                Qt::FramelessWindowHint);  //("pannel",this);

  addToolButtonGrid(0, 0, "", "");  // new dock

  QLabel *labelTitle = QWIDGET_ALLOC QLabel("pannel", _dock);
  _dock->setTitleBarWidget(labelTitle);

  _dock->setFeatures(QDockWidget::DockWidgetMovable |
                     QDockWidget::DockWidgetFloatable |
                     QDockWidget::DockWidgetVerticalTitleBar);

  _dock->setAllowedAreas(Qt::NoDockWidgetArea);
  this->addDockWidget(Qt::NoDockWidgetArea, _dock);
  addToolButtonGrid(this, _dock, "play", CONF_PARAM("TrackView.playAMusic"));

  for (size_t i = 0; i < 10; ++i)
    addToolButtonGrid(this, _dock, std::to_string(i), std::to_string(i));

  addToolButtonGrid(this, _dock, "prevBar", CONF_PARAM("TrackView.prevBar"));
  addToolButtonGrid(this, _dock, "upString", CONF_PARAM("TrackView.stringUp"));
  addToolButtonGrid(this, _dock, "nextBar", CONF_PARAM("TrackView.nextBar"));
  addToolButtonGrid(this, _dock, "play", CONF_PARAM("TrackView.playMidi"), true);
  addToolButtonGrid(this, _dock, "qp", "q", true);
  addToolButtonGrid(this, _dock, "qm", "w", true);  // refact conf prarm it
  addToolButtonGrid(this, _dock, "p", "p", true);
  addToolButtonGrid(this, _dock, "del", CONF_PARAM("TrackView.deleteNote"), true);
  addToolButtonGrid(this, _dock, "-3-", "-3-", true);
  addToolButtonGrid(this, _dock, "leeg", "leeg", true);
  addToolButtonGrid(this, _dock, "x", "dead", true);
  addToolButtonGrid(this, _dock, "newBar", "newBar", true);
  addToolButtonGrid(this, _dock, "save", "quicksave", true);  // CONF_PARAM("TrackView.save")
  addToolButtonGrid(this, _dock, "open", "quickopen", true);  // CONF_PARAM("TrackView.quickOpen")
  addToolButtonGrid(this, _dock, "prevBeat", CONF_PARAM("TrackView.prevBeat"), true);
  addToolButtonGrid(this, _dock, "downString", CONF_PARAM("TrackView.stringDown"), true);
  addToolButtonGrid(this, _dock, "nextBeat", CONF_PARAM("TrackView.nextBeat"), true);

  _dock->setGeometry(90, 390, 750, 110);
  _dock->show();

  QDockWidget *dock2 = QWIDGET_ALLOC QDockWidget(
      this, Qt::Window | Qt::WindowStaysOnTopHint |
                Qt::X11BypassWindowManagerHint |
                Qt::FramelessWindowHint);  //("pannel",this);

  QLabel *labelTitle2 = QWIDGET_ALLOC QLabel("clips ", dock2);
  dock2->setTitleBarWidget(labelTitle2);

  dock2->setFeatures(QDockWidget::DockWidgetMovable |
                     QDockWidget::DockWidgetFloatable |
                     QDockWidget::DockWidgetVerticalTitleBar);

  dock2->setAllowedAreas(Qt::NoDockWidgetArea);
  this->addDockWidget(Qt::NoDockWidgetArea, dock2);

  addToolButtonGrid(0, 0, "", "");  // new dock
  addToolButtonGrid(this, dock2, "copy", "copy");  // must config it
  addToolButtonGrid(this, dock2, "copyBeat", "copyBeat");
  addToolButtonGrid(this, dock2, "copyBars", "copyBars");
  addToolButtonGrid(this, dock2, "cut", "cut");
  addToolButtonGrid(this, dock2, "paste", "paste");
  addToolButtonGrid(this, dock2, "clip1", "clip1");
  addToolButtonGrid(this, dock2, "clip2", "clip2");
  addToolButtonGrid(this, dock2, "clip3", "clip3");
  dock2->setGeometry(90, 290, 440, 50);
  dock2->show();

  QDockWidget *dock3 = QWIDGET_ALLOC QDockWidget(
      this, Qt::Window | Qt::WindowStaysOnTopHint |
                Qt::X11BypassWindowManagerHint |
                Qt::FramelessWindowHint);  //("pannel",this);

  QLabel *labelTitle3 = QWIDGET_ALLOC QLabel("tabs  ", dock3);
  dock3->setTitleBarWidget(labelTitle3);

  dock3->setFeatures(QDockWidget::DockWidgetMovable |
                     QDockWidget::DockWidgetFloatable |
                     QDockWidget::DockWidgetVerticalTitleBar);

  dock3->setAllowedAreas(Qt::NoDockWidgetArea);

  this->addDockWidget(Qt::NoDockWidgetArea, dock3);
  addToolButtonGrid(0, 0, "", "");  // new dock
  addToolButtonGrid(this, dock3, "next", ">>>");  // must config it
  addToolButtonGrid(this, dock3, "prev", "<<<");
  addToolButtonGrid(this, dock3, "^", "^^^");
  addToolButtonGrid(this, dock3, "V", "vvv");
  addToolButtonGrid(this, dock3, "goToN");
  addToolButtonGrid(this, dock3, "opentrack");
  addToolButtonGrid(this, dock3, "newTrack");
  addToolButtonGrid(this, dock3, "deleteTrack");
  addToolButtonGrid(this, dock3, "deleteTrack");
  addToolButtonGrid(this, dock3, "marker");
  addToolButtonGrid(this, dock3, "44", "signs");
  addToolButtonGrid(this, dock3, "repBegin", "|:", true);
  addToolButtonGrid(this, dock3, "repEnd", ":|", true);
  addToolButtonGrid(this, dock3, "alt", "", true);
  addToolButtonGrid(this, dock3, "bpm", "", true);
  addToolButtonGrid(this, dock3, "instr", "", true);
  addToolButtonGrid(this, dock3, "volume", "", true);
  addToolButtonGrid(this, dock3, "pan", "", true);
  addToolButtonGrid(this, dock3, "name", "", true);
  addToolButtonGrid(this, dock3, "tune", "", true);
  addToolButtonGrid(this, dock3, "mute", "", true);
  addToolButtonGrid(this, dock3, "solo", "", true);
  dock3->setGeometry(90, 290, 600, 110);
  dock3->show();

  QDockWidget *dock4 = QWIDGET_ALLOC QDockWidget(
      this, Qt::Window | Qt::WindowStaysOnTopHint |
                Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);

  QLabel *labelTitle4 = QWIDGET_ALLOC QLabel("effects", dock4);
  dock4->setTitleBarWidget(labelTitle4);

  dock4->setFeatures(QDockWidget::DockWidgetMovable |
                     QDockWidget::DockWidgetFloatable |
                     QDockWidget::DockWidgetVerticalTitleBar);

  dock4->setAllowedAreas(Qt::NoDockWidgetArea);
  addToolButtonGrid(0, 0, "", "");  // new dock

  addToolButtonGrid(this, dock4, "vib",
                    CONF_PARAM("effects.vibrato"));  // must config it
  addToolButtonGrid(this, dock4, "sli", CONF_PARAM("effects.slide"));
  addToolButtonGrid(this, dock4, "ham", CONF_PARAM("effects.hammer"));
  addToolButtonGrid(this, dock4, "lr", CONF_PARAM("effects.letring"));
  addToolButtonGrid(this, dock4, "pm", CONF_PARAM("effects.palmmute"));
  addToolButtonGrid(this, dock4, "harm", CONF_PARAM("effects.harmonics"));
  addToolButtonGrid(this, dock4, "trem", CONF_PARAM("effects.tremolo"));
  addToolButtonGrid(this, dock4, "trill", CONF_PARAM("effects.trill"));
  addToolButtonGrid(this, dock4, "stok", CONF_PARAM("effects.stokatto"));
  addToolButtonGrid(this, dock4, "tapp");
  addToolButtonGrid(this, dock4, "slap");
  addToolButtonGrid(this, dock4, "pop");
  addToolButtonGrid(this, dock4, "fadeIn", CONF_PARAM("effects.fadein"));
  addToolButtonGrid(this, dock4, "upm", "up m", true);
  addToolButtonGrid(this, dock4, "downm", "down m", true);
  addToolButtonGrid(this, dock4, "acc", CONF_PARAM("effects.accent"), true);
  addToolButtonGrid(this, dock4, "downm", "down m", true);
  addToolButtonGrid(this, dock4, "acc", CONF_PARAM("effects.accent"), true);
  addToolButtonGrid(this, dock4, "hacc", "h acc", true);
  addToolButtonGrid(this, dock4, "bend", "", true);
  addToolButtonGrid(this, dock4, "chord", "", true);
  addToolButtonGrid(this, dock4, "txt", "text", true);
  addToolButtonGrid(this, dock4, "changes", "", true);
  addToolButtonGrid(this, dock4, "upstroke", "", true);
  addToolButtonGrid(this, dock4, "downstroke", "", true);

  this->addDockWidget(Qt::RightDockWidgetArea, dock4);
  _dock->hide();
  dock2->hide();
  dock3->hide();
  dock4->hide();

  this->_dock2 = dock2;
  this->_dock3 = dock3;
  this->_dock4 = dock4;
}
