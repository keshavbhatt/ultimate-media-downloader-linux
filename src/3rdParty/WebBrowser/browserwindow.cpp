/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browserwindow.h"
#include "browser.h"
#include "cookiedialog.h"
#include "downloadmanagerwidget.h"
#include "tabwidget.h"
#include "webview.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QScreen>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#include <QWebEngineFindTextResult>
#endif
#include <QSettings>
#include <QWebEngineProfile>

BrowserWindow::BrowserWindow(Browser *browser, QWebEngineProfile *profile,
                             bool forDevTools,
                             RequestInterceptor *adBlockRequestInterceptor)
    : m_browser(browser), m_profile(profile),
      m_tabWidget(new TabWidget(profile, this)), m_progressBar(nullptr),
      m_historyBackAction(nullptr), m_historyForwardAction(nullptr),
      m_stopAction(nullptr), m_reloadAction(nullptr),
      m_stopReloadAction(nullptr), m_urlLineEdit(nullptr), m_favAction(nullptr),
      m_adBlockRequestInterceptor(adBlockRequestInterceptor) {
  setAttribute(Qt::WA_DeleteOnClose, true);
  setFocusPolicy(Qt::ClickFocus);

  if (m_adBlockRequestInterceptor == nullptr) {
    m_adBlockRequestInterceptor = new RequestInterceptor(this);
  }

  connect(tabWidget(), &TabWidget::fullScreenRequested, this,
          &BrowserWindow::onFullScreenRequested);

  if (!m_profile->isOffTheRecord()) {
    connect(m_profile->cookieStore(), &QWebEngineCookieStore::cookieAdded,
            m_profile->cookieStore(), [this](auto cookie) {
              if (containsCookie(cookie))
                return;
              m_cookies.append(cookie);
            });
  }

  if (!forDevTools) {
    m_progressBar = new QProgressBar(this);

    QToolBar *toolbar = createToolBar();
    addToolBar(toolbar);
    menuBar()->addMenu(createFileMenu(m_tabWidget));
    menuBar()->addMenu(createEditMenu());
    menuBar()->addMenu(createViewMenu(toolbar));
    menuBar()->addMenu(createAdBlockMenu(m_tabWidget));
    menuBar()->addMenu(createWindowMenu(m_tabWidget));
  }

  QWidget *centralWidget = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  if (!forDevTools) {
    addToolBarBreak();

    m_progressBar->setMaximumHeight(1);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        QStringLiteral("QProgressBar {border: 0px} QProgressBar::chunk "
                       "{background-color: #da4453}"));

    layout->addWidget(m_progressBar);
  }

  layout->addWidget(m_tabWidget);
  centralWidget->setLayout(layout);
  setCentralWidget(centralWidget);

  connect(m_tabWidget, &TabWidget::titleChanged, this,
          &BrowserWindow::handleWebViewTitleChanged);
  if (!forDevTools) {
    connect(m_tabWidget, &TabWidget::linkHovered,
            [this](const QString &url) { statusBar()->showMessage(url); });
    connect(m_tabWidget, &TabWidget::loadProgress, this,
            &BrowserWindow::handleWebViewLoadProgress);
    connect(m_tabWidget, &TabWidget::webActionEnabledChanged, this,
            &BrowserWindow::handleWebActionEnabledChanged);
    connect(m_tabWidget, &TabWidget::urlChanged, [this](const QUrl &url) {
      m_urlLineEdit->setText(url.toDisplayString());
    });
    connect(m_tabWidget, &TabWidget::favIconChanged, m_favAction,
            &QAction::setIcon);
    connect(m_tabWidget, &TabWidget::devToolsRequested, this,
            &BrowserWindow::handleDevToolsRequested);
    connect(m_urlLineEdit, &QLineEdit::returnPressed, [this]() {
      QString input = m_urlLineEdit->text();
      QUrl url = QUrl(input);

      if (url.isValid() && !url.isRelative()) {
        m_tabWidget->setUrl(url);
      } else {
        QString searchQuery =
            QString("https://www.google.com/search?q=%1").arg(input);
        m_tabWidget->setUrl(QUrl(searchQuery));
      }
    });
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    connect(m_tabWidget, &TabWidget::findTextFinished, this,
            &BrowserWindow::handleFindTextFinished);
#endif

    QAction *focusUrlLineEditAction = new QAction(this);
    addAction(focusUrlLineEditAction);
    focusUrlLineEditAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(focusUrlLineEditAction, &QAction::triggered, this,
            [this]() { m_urlLineEdit->setFocus(Qt::ShortcutFocusReason); });
  }

  handleWebViewTitleChanged(QString());
  m_tabWidget->createTab();
}

void BrowserWindow::toggleFullScreen() {
  if (isFullScreen()) {
    setFullscreen(false);
  } else {
    setFullscreen(true);
  }
}

void BrowserWindow::setFullscreen(bool fullscreen) {
  if (fullscreen) {
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->tabBar()->hide();
    statusBar()->hide();
    navigationBar->hide();
    menuBar()->hide();
    windowStateBeforeFullScreen = this->windowState();
    this->setWindowState(Qt::WindowState::WindowFullScreen |
                         windowStateBeforeFullScreen);
  } else {
    m_tabWidget->setDocumentMode(false);
    m_tabWidget->tabBar()->show();
    statusBar()->show();
    navigationBar->show();
    menuBar()->show();
    this->setWindowState(windowStateBeforeFullScreen);
  }
}

void BrowserWindow::onFullScreenRequested(QWebEngineFullScreenRequest request) {
  setFullscreen(request.toggleOn());
  request.accept();
}

QSize BrowserWindow::sizeHint() const {
  QRect desktopRect = QApplication::primaryScreen()->geometry();
  QSize size = desktopRect.size() * qreal(0.9);
  return size;
}

QMenu *BrowserWindow::createFileMenu(TabWidget *tabWidget) {
  QMenu *fileMenu = new QMenu(tr("&File"));
  fileMenu->addAction(tr("&New Window"), this,
                      &BrowserWindow::handleNewWindowTriggered,
                      QKeySequence::New);
  fileMenu->addAction(tr("New &Incognito Window"), this,
                      &BrowserWindow::handleNewIncognitoWindowTriggered);

  QAction *newTabAction = new QAction(tr("New &Tab"), this);
  newTabAction->setShortcuts(QKeySequence::AddTab);
  connect(newTabAction, &QAction::triggered, this, [this]() {
    m_tabWidget->createTab();
    m_urlLineEdit->setFocus();
  });
  fileMenu->addAction(newTabAction);

  fileMenu->addAction(tr("&Open File..."), this,
                      &BrowserWindow::handleFileOpenTriggered,
                      QKeySequence::Open);
  fileMenu->addSeparator();

  QAction *closeTabAction = new QAction(tr("&Close Tab"), this);
  closeTabAction->setShortcuts(QKeySequence::Close);
  connect(closeTabAction, &QAction::triggered,
          [tabWidget]() { tabWidget->closeTab(tabWidget->currentIndex()); });
  fileMenu->addAction(closeTabAction);

  QAction *closeAction = new QAction(tr("&Quit"), this);
  closeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
  connect(closeAction, &QAction::triggered, this, &QWidget::close);
  fileMenu->addAction(closeAction);

  connect(fileMenu, &QMenu::aboutToShow, [this, closeAction]() {
    if (m_browser->windows().count() == 1)
      closeAction->setText(tr("&Quit"));
    else
      closeAction->setText(tr("&Close Window"));
  });
  return fileMenu;
}

QMenu *BrowserWindow::createEditMenu() {
  QMenu *editMenu = new QMenu(tr("&Edit"));
  QAction *findAction = editMenu->addAction(tr("&Find"));
  findAction->setShortcuts(QKeySequence::Find);
  connect(findAction, &QAction::triggered, this,
          &BrowserWindow::handleFindActionTriggered);

  QAction *findNextAction = editMenu->addAction(tr("Find &Next"));
  findNextAction->setShortcut(QKeySequence::FindNext);
  connect(findNextAction, &QAction::triggered, [this]() {
    if (!currentTab() || m_lastSearch.isEmpty())
      return;
    currentTab()->findText(m_lastSearch);
  });

  QAction *findPreviousAction = editMenu->addAction(tr("Find &Previous"));
  findPreviousAction->setShortcut(QKeySequence::FindPrevious);
  connect(findPreviousAction, &QAction::triggered, [this]() {
    if (!currentTab() || m_lastSearch.isEmpty())
      return;
    currentTab()->findText(m_lastSearch, QWebEnginePage::FindBackward);
  });

  QAction *clearCookiesAction = new QAction("Clear Cookies", this);
  connect(clearCookiesAction, &QAction::triggered, [this]() {
    if (m_profile) {
      m_cookies.clear();
      m_profile->cookieStore()->deleteAllCookies();
      emit cookiesCleared();
    }
  });
  editMenu->addAction(clearCookiesAction);

  return editMenu;
}

void BrowserWindow::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_F11) {
    toggleFullScreen();
  }
  QMainWindow::keyPressEvent(event);
}

QMenu *BrowserWindow::createViewMenu(QToolBar *toolbar) {
  QMenu *viewMenu = new QMenu(tr("&View"));
  m_stopAction = viewMenu->addAction(tr("&Stop"));
  QList<QKeySequence> shortcuts;
  shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Period));
  shortcuts.append(Qt::Key_Escape);
  m_stopAction->setShortcuts(shortcuts);
  connect(m_stopAction, &QAction::triggered, [this]() {
    m_tabWidget->triggerWebPageAction(QWebEnginePage::Stop);
  });

  m_reloadAction = viewMenu->addAction(tr("Reload Page"));
  m_reloadAction->setShortcuts(QKeySequence::Refresh);
  connect(m_reloadAction, &QAction::triggered, [this]() {
    m_tabWidget->triggerWebPageAction(QWebEnginePage::Reload);
  });

  QAction *zoomIn = viewMenu->addAction(tr("Zoom &In"));
  zoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
  connect(zoomIn, &QAction::triggered, [this]() {
    if (currentTab())
      currentTab()->setZoomFactor(currentTab()->zoomFactor() + 0.1);
  });

  QAction *zoomOut = viewMenu->addAction(tr("Zoom &Out"));
  zoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
  connect(zoomOut, &QAction::triggered, [this]() {
    if (currentTab())
      currentTab()->setZoomFactor(currentTab()->zoomFactor() - 0.1);
  });

  QAction *resetZoom = viewMenu->addAction(tr("Reset &Zoom"));
  resetZoom->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
  connect(resetZoom, &QAction::triggered, [this]() {
    if (currentTab())
      currentTab()->setZoomFactor(1.0);
  });

  QAction *toggleFullScreenAction =
      viewMenu->addAction(tr("Toggle &FullScreen"));
  toggleFullScreenAction->setShortcut(QKeySequence(Qt::Key_F11));
  connect(toggleFullScreenAction, &QAction::triggered, this,
          [this]() { toggleFullScreen(); });
  viewMenu->addAction(toggleFullScreenAction);

  viewMenu->addSeparator();
  QAction *viewToolbarAction = new QAction(tr("Hide Toolbar"), this);
  viewToolbarAction->setShortcut(tr("Ctrl+|"));
  connect(viewToolbarAction, &QAction::triggered,
          [toolbar, viewToolbarAction]() {
            if (toolbar->isVisible()) {
              viewToolbarAction->setText(tr("Show Toolbar"));
              toolbar->close();
            } else {
              viewToolbarAction->setText(tr("Hide Toolbar"));
              toolbar->show();
            }
          });
  viewMenu->addAction(viewToolbarAction);

  QAction *viewStatusbarAction = new QAction(tr("Hide Status Bar"), this);
  viewStatusbarAction->setShortcut(tr("Ctrl+/"));
  connect(viewStatusbarAction, &QAction::triggered,
          [this, viewStatusbarAction]() {
            if (statusBar()->isVisible()) {
              viewStatusbarAction->setText(tr("Show Status Bar"));
              statusBar()->close();
            } else {
              viewStatusbarAction->setText(tr("Hide Status Bar"));
              statusBar()->show();
            }
          });
  viewMenu->addAction(viewStatusbarAction);

  QAction *viewCookiesAction = new QAction("View Cookies", this);
  connect(viewCookiesAction, &QAction::triggered,
          [this]() { showAllCookies(); });
  viewMenu->addAction(viewCookiesAction);

  // QAction *dumpCookiesAction = new QAction("Dump Cookies", this);
  // connect(dumpCookiesAction, &QAction::triggered, [this]() {
  //   auto filename = dumpCookiesToNetscapeFormat();
  //   qDebug() << filename;
  // });
  // viewMenu->addAction(dumpCookiesAction);

  return viewMenu;
}

bool BrowserWindow::containsCookie(const QNetworkCookie &cookie) {
  for (const auto &c : qAsConst(m_cookies)) {
    if (c.hasSameIdentifier(cookie))
      return true;
  }
  return false;
}

void BrowserWindow::showAllCookies() {
  CookieDialog dialog(m_cookies);
  dialog.exec();
}

QString BrowserWindow::dumpCookiesToNetscapeFormat() {

  QTemporaryFile file;
  file.setAutoRemove(false); // todo: delete all files on close
  if (!file.open()) {
    qWarning() << "Cannot open file for writing:" << file.errorString();
    return "";
  }

  QTextStream out(&file);
  out.setCodec("UTF-8");
  out << "# Netscape HTTP Cookie File\n";
  out << "# This file is auto generated.  Do not edit.\n\n";

  for (const QNetworkCookie &cookie : qAsConst(m_cookies)) {
    QString domain = cookie.domain();
    bool isSecure = cookie.isSecure();
    QString path = cookie.path();
    QString name = cookie.name();
    QString value = cookie.value();
    QString expiration =
        QString::number(cookie.expirationDate().toSecsSinceEpoch());

    out << (domain.startsWith('.') ? domain : '.' + domain) << "\t"
        << "TRUE" << "\t" << path << "\t" << (isSecure ? "TRUE" : "FALSE")
        << "\t" << expiration << "\t" << name << "\t" << value << "\n";
  }

  file.close();

  return file.fileName();
}

QMenu *BrowserWindow::createWindowMenu(TabWidget *tabWidget) {
  QMenu *menu = new QMenu(tr("&Window"));

  QAction *nextTabAction = new QAction(tr("Show Next Tab"), this);
  QList<QKeySequence> shortcuts;
  shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceRight));
  shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
  shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
  shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Less));
  nextTabAction->setShortcuts(shortcuts);
  connect(nextTabAction, &QAction::triggered, tabWidget, &TabWidget::nextTab);

  QAction *previousTabAction = new QAction(tr("Show Previous Tab"), this);
  shortcuts.clear();
  shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceLeft));
  shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
  shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
  shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Greater));
  previousTabAction->setShortcuts(shortcuts);
  connect(previousTabAction, &QAction::triggered, tabWidget,
          &TabWidget::previousTab);

  connect(menu, &QMenu::aboutToShow,
          [this, menu, nextTabAction, previousTabAction]() {
            menu->clear();
            menu->addAction(nextTabAction);
            menu->addAction(previousTabAction);
            menu->addSeparator();

            QVector<BrowserWindow *> windows = m_browser->windows();
            int index(-1);
            for (auto window : windows) {
              QAction *action =
                  menu->addAction(window->windowTitle(), this,
                                  &BrowserWindow::handleShowWindowTriggered);
              action->setData(++index);
              action->setCheckable(true);
              if (window == this)
                action->setChecked(true);
            }
          });
  return menu;
}

QMenu *BrowserWindow::createAdBlockMenu(TabWidget *tabWidget) {
  QMenu *menu = new QMenu(tr("&AdBlock"));

  QAction *toggleAdBlockAction = new QAction(tr("Enable AdBlocker"), this);
  toggleAdBlockAction->setCheckable(true);
  toggleAdBlockAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
  connect(toggleAdBlockAction, &QAction::toggled, tabWidget,
          [tabWidget, toggleAdBlockAction, this](bool checked) {
            if (checked) {
              if (m_adBlockRequestInterceptor) {
                tabWidget->currentWebView()
                    ->page()
                    ->profile()
                    ->setUrlRequestInterceptor(m_adBlockRequestInterceptor);
              } else {
                qWarning() << "Cannot enable adBlocker. "
                              "m_adBlockRequestInterceptor is null";
                toggleAdBlockAction->blockSignals(true);
                toggleAdBlockAction->setChecked(false);
                toggleAdBlockAction->blockSignals(false);
              }
            } else {
              tabWidget->currentWebView()
                  ->page()
                  ->profile()
                  ->setUrlRequestInterceptor(nullptr);
            }

            settings.setValue("browserAdblocker", checked);
          });

  connect(menu, &QMenu::aboutToShow, this, [this, menu, toggleAdBlockAction]() {
    menu->clear();
    menu->addAction(toggleAdBlockAction);

    toggleAdBlockAction->setChecked(
        settings.value("browserAdblocker", true).toBool());
  });
  return menu;
}

QToolBar *BrowserWindow::createToolBar() {
  navigationBar = new QToolBar(tr("Navigation"));
  navigationBar->setMovable(false);
  navigationBar->toggleViewAction()->setEnabled(false);

  m_historyBackAction = new QAction(this);
  QList<QKeySequence> backShortcuts =
      QKeySequence::keyBindings(QKeySequence::Back);
  for (auto it = backShortcuts.begin(); it != backShortcuts.end();) {
    // Chromium already handles navigate on backspace when appropriate.
    if ((*it)[0] == Qt::Key_Backspace)
      it = backShortcuts.erase(it);
    else
      ++it;
  }
  // For some reason Qt doesn't bind the dedicated Back key to Back.
  backShortcuts.append(QKeySequence(Qt::Key_Back));
  m_historyBackAction->setShortcuts(backShortcuts);
  m_historyBackAction->setIconVisibleInMenu(false);
  m_historyBackAction->setIcon(QIcon(QStringLiteral(":go-previous.png")));
  m_historyBackAction->setToolTip(tr("Go back in history"));
  connect(m_historyBackAction, &QAction::triggered, [this]() {
    m_tabWidget->triggerWebPageAction(QWebEnginePage::Back);
  });
  navigationBar->addAction(m_historyBackAction);

  m_historyForwardAction = new QAction(this);
  QList<QKeySequence> fwdShortcuts =
      QKeySequence::keyBindings(QKeySequence::Forward);
  for (auto it = fwdShortcuts.begin(); it != fwdShortcuts.end();) {
    if (((*it)[0] & Qt::Key_unknown) == Qt::Key_Backspace)
      it = fwdShortcuts.erase(it);
    else
      ++it;
  }
  fwdShortcuts.append(QKeySequence(Qt::Key_Forward));
  m_historyForwardAction->setShortcuts(fwdShortcuts);
  m_historyForwardAction->setIconVisibleInMenu(false);
  m_historyForwardAction->setIcon(QIcon(QStringLiteral(":go-next.png")));
  m_historyForwardAction->setToolTip(tr("Go forward in history"));
  connect(m_historyForwardAction, &QAction::triggered, [this]() {
    m_tabWidget->triggerWebPageAction(QWebEnginePage::Forward);
  });
  navigationBar->addAction(m_historyForwardAction);

  m_stopReloadAction = new QAction(this);
  connect(m_stopReloadAction, &QAction::triggered, [this]() {
    m_tabWidget->triggerWebPageAction(
        QWebEnginePage::WebAction(m_stopReloadAction->data().toInt()));
  });
  navigationBar->addAction(m_stopReloadAction);

  m_homePageAction = new QAction(this);
  m_homePageAction->setIcon(QIcon(QStringLiteral(":home-3-fill.png")));
  m_homePageAction->setToolTip(tr("Go to Home Page"));
  connect(m_homePageAction, &QAction::triggered,
          [this]() { currentTab()->setUrl(QUrl("https://google.com")); });
  navigationBar->addAction(m_homePageAction);

  m_urlLineEdit = new QLineEdit(this);
  m_favAction = new QAction(this);
  m_urlLineEdit->addAction(m_favAction, QLineEdit::LeadingPosition);
  m_urlLineEdit->setClearButtonEnabled(true);
  navigationBar->addWidget(m_urlLineEdit);

  auto copyCurrentUrlAction = new QAction(this);
  copyCurrentUrlAction->setIcon(QIcon(QStringLiteral(":file-copy-fill.png")));
  copyCurrentUrlAction->setToolTip(tr("Copy Curernt URL"));
  navigationBar->addAction(copyCurrentUrlAction);
  connect(copyCurrentUrlAction, &QAction::triggered, [this]() {
    QApplication::clipboard()->setText(currentTab()->url().toString());
  });

  auto downloadsAction = new QAction(this);
  downloadsAction->setIcon(QIcon(QStringLiteral(":go-bottom.png")));
  downloadsAction->setToolTip(tr("Show downloads"));
  navigationBar->addAction(downloadsAction);
  connect(downloadsAction, &QAction::triggered,
          [this]() { m_browser->downloadManagerWidget().show(); });

  return navigationBar;
}

void BrowserWindow::handleWebActionEnabledChanged(
    QWebEnginePage::WebAction action, bool enabled) {
  switch (action) {
  case QWebEnginePage::Back:
    m_historyBackAction->setEnabled(enabled);
    break;
  case QWebEnginePage::Forward:
    m_historyForwardAction->setEnabled(enabled);
    break;
  case QWebEnginePage::Reload:
    m_reloadAction->setEnabled(enabled);
    break;
  case QWebEnginePage::Stop:
    m_stopAction->setEnabled(enabled);
    break;
  default:
    qWarning("Unhandled webActionChanged signal");
  }
}

void BrowserWindow::handleWebViewTitleChanged(const QString &title) {
  QString suffix =
      m_profile->isOffTheRecord() ? tr("Browser (Incognito)") : tr("Browser");

  if (title.isEmpty())
    setWindowTitle(suffix);
  else
    setWindowTitle(title + " - " + suffix);
}

void BrowserWindow::handleNewWindowTriggered() {
  BrowserWindow *window = m_browser->createWindow();
  window->m_urlLineEdit->setFocus();
}

void BrowserWindow::handleNewIncognitoWindowTriggered() {
  BrowserWindow *window = m_browser->createWindow(/* offTheRecord: */ true);
  window->m_urlLineEdit->setFocus();
}

void BrowserWindow::handleFileOpenTriggered() {
  QUrl url =
      QFileDialog::getOpenFileUrl(this, tr("Open Web Resource"), QString(),
                                  tr("Web Resources (*.html *.htm *.svg *.png "
                                     "*.gif *.svgz);;All files (*.*)"));
  if (url.isEmpty())
    return;
  currentTab()->setUrl(url);
}

void BrowserWindow::handleFindActionTriggered() {
  if (!currentTab())
    return;
  bool ok = false;
  QString search = QInputDialog::getText(this, tr("Find"), tr("Find:"),
                                         QLineEdit::Normal, m_lastSearch, &ok);
  if (ok && !search.isEmpty()) {
    m_lastSearch = search;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    currentTab()->findText(m_lastSearch);
#else
    currentTab()->findText(m_lastSearch, 0, [this](bool found) {
      if (!found)
        statusBar()->showMessage(tr("\"%1\" not found.").arg(m_lastSearch));
    });
#endif
  }
}

void BrowserWindow::quit() {
  m_quitting = true;
  this->close();
}

void BrowserWindow::closeEvent(QCloseEvent *event) {

  if (m_quitting) {
    m_tabWidget->closeAllTabs();
    event->accept();
    return;
  }
  if (m_tabWidget->count() > 1) {
    int ret = QMessageBox::warning(
        this, tr("Confirm close"),
        tr("Are you sure you want to close the window ?\n"
           "There are %1 tabs open.")
            .arg(m_tabWidget->count()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret == QMessageBox::No) {
      event->ignore();
      return;
    }
  }
  if (browser()->windows().count() > 1) {
    event->accept();
  } else {
    event->ignore();
    this->hide();
    m_tabWidget->closeAllTabs();
    setFullscreen(false);
  }
}

TabWidget *BrowserWindow::tabWidget() const { return m_tabWidget; }

WebView *BrowserWindow::currentTab() const {
  return m_tabWidget->currentWebView();
}

void BrowserWindow::handleWebViewLoadProgress(int progress) {
  static QIcon stopIcon(QStringLiteral(":process-stop.png"));
  static QIcon reloadIcon(QStringLiteral(":view-refresh.png"));

  if (0 < progress && progress < 100) {
    m_stopReloadAction->setData(QWebEnginePage::Stop);
    m_stopReloadAction->setIcon(stopIcon);
    m_stopReloadAction->setToolTip(tr("Stop loading the current page"));
    m_progressBar->setValue(progress);
  } else {
    m_stopReloadAction->setData(QWebEnginePage::Reload);
    m_stopReloadAction->setIcon(reloadIcon);
    m_stopReloadAction->setToolTip(tr("Reload the current page"));
    m_progressBar->setValue(0);
  }
}

void BrowserWindow::handleShowWindowTriggered() {
  if (QAction *action = qobject_cast<QAction *>(sender())) {
    int offset = action->data().toInt();
    QVector<BrowserWindow *> windows = m_browser->windows();
    windows.at(offset)->activateWindow();
    windows.at(offset)->currentTab()->setFocus();
  }
}

void BrowserWindow::handleDevToolsRequested(QWebEnginePage *source) {
  source->setDevToolsPage(
      m_browser->createDevToolsWindow()->currentTab()->page());
  source->triggerAction(QWebEnginePage::InspectElement);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
void BrowserWindow::handleFindTextFinished(
    const QWebEngineFindTextResult &result) {
  if (result.numberOfMatches() == 0) {
    statusBar()->showMessage(tr("\"%1\" not found.").arg(m_lastSearch));
  } else {
    statusBar()->showMessage(
        tr("\"%1\" found: %2/%3")
            .arg(m_lastSearch, QString::number(result.activeMatch()),
                 QString::number(result.numberOfMatches())));
  }
}
#endif
