/*
 *  Copyright (c) 2021 Keshav Bhatt
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "mainwindow.h"
#include "tabmanager.h"
#include "ui_mainwindow.h"
#include "utility.h"

#include "context.hpp"
#include "settings.h"
#include "translator.h"

#include <QScreen>
#include <browser.h>
#include <browserwindow.h>
#include <tabwidget.h>

#include <about/about.h>

static std::unique_ptr<Ui::MainWindow> _init_ui(QMainWindow &mw) {
  auto m = std::make_unique<Ui::MainWindow>();
  m->setupUi(&mw);
  return m;
}

static QString _debug(const QStringList &e) {
  if (e.contains("--debug")) {

    return "--debug";

  } else if (e.contains("--qdebug")) {

    return "--qdebug";
  } else {
    return "";
  }
}

MainWindow::MainWindow(QApplication &app, settings &s, translator &t,
                       const QStringList &args)
    : m_qApp(app), m_ui(_init_ui(*this)),
      m_logger(*m_ui->plainTextEditLogger, this, s), m_engines(m_logger, s),
      m_cou(*m_ui), m_tabManager(s, t, m_engines, m_logger, *m_ui, *this, *this,
                                 m_cou, _debug(args)),
      m_settings(s), m_defaultWindowTitle(APPLICATION_NAME) {
  QIcon icon = QIcon::fromTheme(QApplication::applicationName(),
                                QIcon(":/icons/app/icon-64.png"));
  this->window()->setWindowIcon(icon);
  initRateWidget();
  m_ui.get()->ytSearchResultsListWidget->installEventFilter(this);
  restoreMainWindow();

  // init engine version check
  auto engine_path = engines::enginePaths(m_settings).binPath() + "/" +
                     m_settings.defaultEngineName();
  m_engineUpdateCheck = new EngineUpdateCheck(this, engine_path);
  connect(m_engineUpdateCheck, &EngineUpdateCheck::updateFound, this,
          &MainWindow::handleEngineUpdateAvailable);
  m_engineUpdateCheck->checkForUpdate();

  // init player service
  PlayerService *playerService =
      new PlayerService(this, engine_path, m_settings);
  playerService->clearSocketDir();
  playerService->deleteLater();

  initBrowser();
  initToolbar();
}

void MainWindow::handleEngineUpdateAvailable() {
  qWarning() << "ENGINE: handleEngineUpdate...";
  m_ui.get()->pbConfigureDownload->setEnabled(true);
  m_ui.get()->pbConfigureDownload->click();
}

void MainWindow::restoreMainWindow() {
  if (m_settings.getValue("MainWindowGeometry", QByteArray())
          .toByteArray()
          .isEmpty() == false) {
    restoreGeometry(
        m_settings.getValue("MainWindowGeometry", QByteArray()).toByteArray());
  } else {
    this->resize(870, 620);
  }

  restoreState(
      m_settings.getValue("MainWindowState", QByteArray()).toByteArray());
}

void MainWindow::initToolbar() {

  if (m_toolbar == nullptr) {
    m_toolbar = new QToolBar("Toolbar", this);
    m_toolbar->setObjectName("toolBar");

    addToolBar(Qt::LeftToolBarArea, m_toolbar);
    m_toolbar->setFloatable(false);
    m_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_toolbar->setStyleSheet("QToolBar { border: none; }");

    m_browserAction =
        new QAction(QIcon(":/icons/webbrowser.png"), "Browser", this);
    connect(m_browserAction, &QAction::triggered, this,
            [=]() { initAndShowBrowserWindow(true); });

    m_toolbar->addAction(m_browserAction);

    m_toolbar->addSeparator();

    m_aboutAction = new QAction(QIcon(":/icons/info_blue.png"), "About", this);
    connect(m_aboutAction, &QAction::triggered, this,
            &MainWindow::onAboutActionTriggered);

    m_toolbar->addAction(m_aboutAction);

    // restore pos
    Qt::ToolBarArea area = static_cast<Qt::ToolBarArea>(
        m_settings.getValue("toolbar/area", Qt::LeftToolBarArea).toInt());
    addToolBar(area, m_toolbar);
  }
}

void MainWindow::initBrowser() {
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::PluginsEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::FullScreenSupportEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::JavascriptCanAccessClipboard, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::AutoLoadImages, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::JavascriptEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::JavascriptCanOpenWindows, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::LocalStorageEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::XSSAuditingEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::LocalContentCanAccessFileUrls, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::ScrollAnimatorEnabled, false);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::DnsPrefetchEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::LinksIncludedInFocusChain, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::FocusOnNavigationEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::SpatialNavigationEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::JavascriptCanPaste, true);

  dumpBrowserCookie();
}

void MainWindow::onAboutActionTriggered() {
  About *about = new About(this);
  about->setWindowFlag(Qt::Dialog);
  about->setMinimumSize(about->sizeHint());
  about->adjustSize();
  about->setAttribute(Qt::WA_DeleteOnClose, true);
  connect(about, &About::browserOpenLinkRequested, this,
          &MainWindow::browserOpenLink);
  about->show();
}

void MainWindow::browserOpenLink(QUrl url) {
  initAndShowBrowserWindow(true);
  m_browser_window->tabWidget()->setUrl(url);
  m_browser_window->show();
  m_browser_window->raise();
  m_browser_window->activateWindow();
  m_browser_window->setFocus();
}

QString MainWindow::dumpBrowserCookie() {

  initAndShowBrowserWindow(false);
  if (m_browser_window != nullptr) {
    auto path = m_browser_window->dumpCookiesToNetscapeFormat();
    if (m_settings.getValue("browserSessionSharing", true).toBool()) {
      m_settings.setCookieFilePath(m_settings.defaultEngineName(), path);
    }
    return path;
  } else {
    qWarning() << "MainWindow::dumpBrowserCookie: "
               << "Unable to dump browser cookies, m_browser or "
                  "m_browser_window is not instanciated";
    return "";
  }
}

void MainWindow::initAndShowBrowserWindow(bool show) {

  if (m_browser_window == nullptr) {
    m_browser_window = m_browser.createWindowHidden();

    connect(m_browser_window, &BrowserWindow::cookiesCleared, this,
            [=]() { this->dumpBrowserCookie(); });

    // required to access browser window from configure
    m_browser_window->setParent(this);
    m_browser_window->setWindowFlag(Qt::Window);
    // end required to access browser window from configure

    m_browser_window->tabWidget()->setUrl(QUrl("https://google.com"));
    connect(m_browser_window, &QObject::destroyed, this,
            [this]() { m_browser_window = nullptr; });
  }
  if (m_browser_window && show) {
    if (!m_browser_window->isVisible() || !m_browser_window->hasFocus()) {
      // If the window is minimized, restore it
      if (m_browser_window->windowState() & Qt::WindowMinimized) {
        m_browser_window->setWindowState(m_browser_window->windowState() &
                                         ~Qt::WindowMinimized);
      }

      // Bring the window to the front
      m_browser_window->show();
      m_browser_window->raise();
      m_browser_window->activateWindow();
      m_browser_window->setFocus();
    }
  } else {
    qDebug() << "Window is not available.";
  }
}

bool MainWindow::eventFilter(QObject *o, QEvent *e) {
  if (e->type() == QEvent::Resize &&
      o == m_ui.get()->ytSearchResultsListWidget) {
    m_tabManager.YoutubeSearch().resizeTrendingView();
  }

  return QMainWindow::eventFilter(o, e);
}

void MainWindow::initRateWidget() {
  RateApp *rateApp =
      new RateApp(this, "snap://ultimate-media-downloader", 2, 5, 1000 * 30);
  rateApp->setWindowTitle(QApplication::applicationName() + " | " +
                          tr("Rate Application"));
  rateApp->setVisible(false);
  rateApp->setWindowFlags(Qt::Dialog);
  rateApp->setAttribute(Qt::WA_DeleteOnClose, true);
  QPoint centerPos = this->geometry().center() - rateApp->geometry().center();
  connect(rateApp, &RateApp::showRateDialog, rateApp, [=]() {
    if (this->windowState() != Qt::WindowMinimized && this->isVisible() &&
        isActiveWindow()) {
      rateApp->move(centerPos);
      rateApp->show();
    } else {
      rateApp->delayShowEvent();
    }
  });
}

void MainWindow::retranslateUi() { m_ui->retranslateUi(this); }

void MainWindow::setTitle(const QString &m) {
  if (m.isEmpty()) {

    this->resetTitle();
  } else {
    this->window()->setWindowTitle(m);
  }
}

void MainWindow::resetTitle() { this->setTitle(m_defaultWindowTitle); }

void MainWindow::Show() { this->show(); }

void MainWindow::processEvent(const QByteArray &e) { m_tabManager.gotEvent(e); }

void MainWindow::quitApp() { m_tabManager.basicDownloader().appQuit(); }

void MainWindow::log(const QByteArray &e) { m_logger.add(e, -1); }

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent *e) {

  m_settings.setValue("MainWindowGeometry", saveGeometry());

  m_settings.setValue("MainWindowState", saveState());

  if (m_toolbar != nullptr) {
    m_settings.setValue("toolbar/area", this->toolBarArea(m_toolbar));
  }

  e->ignore();

  // foreach (BrowserWindow *browser_window, m_browser->windows()) {
  //   if (browser_window != nullptr)
  //     browser_window->quit();
  // }

  // if (m_browser) {
  //   delete m_browser;
  // }

  this->hide();

  m_tabManager.basicDownloader().appQuit();
}
