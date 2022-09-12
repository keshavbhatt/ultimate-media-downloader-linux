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

#include "tabmanager.h"

#include <QMessageBox>
#include <mainwindow.h>

tabManager::tabManager(settings &s, translator &t, engines &e, Logger &l,
                       Ui::MainWindow &ui, QWidget &w, MainWindow &mw,
                       utility::versionInfo &u, QString debug)
    : m_currentTab(s.tabNumber()),
      m_ctx(s, t, ui, w, mw, l, e, u, *this, debug), m_accountManager(m_ctx),
      m_configure(m_ctx), m_basicdownloader(m_ctx), m_batchdownloader(m_ctx),
      m_playlistdownloader(m_ctx), m_ytSearch(m_ctx), m_xpSearch(m_ctx),
      m_library(m_ctx), m_trackingService(new PostHogTrackingService(
                            nullptr, "https://us.posthog.com",
                            "phc_ctU9sizbaLzNk1LNVFjxGp4eE3LnAWz",
                            m_accountManager.getAccountId())) {
  u.setContext(m_ctx);

  const auto &engines = m_ctx.Engines().getEngines();

  if (engines.size() > 0) {

    if (s.showVersionInfoWhenStarting()) {

      s.setTabNumber(0);

      ui.tabWidget->setCurrentIndex(0);

      m_ctx.logger().updateView(true);

      auto &vinfo = m_ctx.versionInfo();

      m_initConnection = QObject::connect(
          &vinfo, &utility::versionInfo::vinfoDone, [this, &ui, &s]() {
            QObject::disconnect(m_initConnection);

            this->init_done(ui, s);
          });

      vinfo.check(engines);
    } else {
      this->init_done(ui, s);
    }
  } else {
    this->disableAll();
  }

  QJsonObject trackingData;
  trackingData.insert("accountIsPro", AccountManager().isPro());

  trackingService()->trackEvent("session_started", trackingData);
}

void tabManager::dumpCookie() { m_ctx.mainWindow().dumpBrowserCookie(); }

void tabManager::init_done(Ui::MainWindow &ui, settings &settings) {
  this->setDefaultEngines();

  m_accountManager.init_done();
  m_ytSearch.init_done();
  m_xpSearch.init_done();
  m_configure.init_done();
  m_basicdownloader.init_done();
  m_batchdownloader.init_done();
  m_playlistdownloader.init_done();
  m_library.init_done();

  auto &m_ui = ui;
  auto &s = settings;

  // hide weird base line in from tabbar in fusion theme
  m_ui.tabWidget->tabBar()->setDrawBase(false);

  m_ui.tabWidget->setCurrentIndex(s.tabNumber());

  switch (s.tabNumber()) {
  case 0:
    m_basicdownloader.tabEntered();
    break;
  case 1:
    m_ytSearch.tabEntered();
    break;
  case 2:
    m_xpSearch.tabEntered();
    break;
  case 3:
    m_batchdownloader.tabEntered();
    break;
  case 4:
    m_playlistdownloader.tabEntered();
    break;
  case 5:
    m_library.tabEntered();
    break;
  case 6:
    m_configure.tabEntered();
    break;
  case 7:
    m_accountManager.tabEntered();
    break;
  }

  QObject::connect(m_ui.tabWidget, &QTabWidget::currentChanged,
                   [this](int index) {
                     switch (index) {
                     case 0:
                       m_basicdownloader.tabEntered();
                       break;
                     case 1:
                       m_ytSearch.tabEntered();
                       break;
                     case 2:
                       m_xpSearch.tabEntered();
                       break;
                     case 3:
                       m_batchdownloader.tabEntered();
                       break;
                     case 4:
                       m_playlistdownloader.tabEntered();
                       break;
                     case 5:
                       m_library.tabEntered();
                       break;
                     case 6:
                       m_configure.tabEntered();
                       break;
                     case 7:
                       m_accountManager.tabEntered();
                       break;
                     }

                     if (m_currentTab != index) {

                       switch (m_currentTab) {
                       case 0:
                         m_basicdownloader.tabExited();
                         break;
                       case 1:
                         m_ytSearch.tabExited();
                         break;
                       case 2:
                         m_xpSearch.tabExited();
                         break;
                       case 3:
                         m_batchdownloader.tabExited();
                         break;
                       case 4:
                         m_playlistdownloader.tabExited();
                         break;
                       case 5:
                         m_library.tabExited();
                         break;
                       case 6:
                         m_configure.tabExited();
                         break;
                       case 7:
                         m_accountManager.tabExited();
                         break;
                       }

                       m_currentTab = index;
                     }
                   });
}

void tabManager::setDefaultEngines() {
  QStringList s;

  for (const auto &engine : m_ctx.Engines().getEngines()) {
    if (engine.mainEngine() && engine.backendExists() && !engine.broken()) {
      s.append(engine.name());
    }
  }
  m_configure.updateEnginesList(s);
}

tabManager &tabManager::gotEvent(const QByteArray &e) {
  m_basicdownloader.gotEvent(e);
  m_batchdownloader.gotEvent(e);
  m_playlistdownloader.gotEvent(e);

  return *this;
}

tabManager &tabManager::enableAll() {
  m_accountManager.enableAll();
  m_ytSearch.enableAll();
  m_xpSearch.enableAll();
  m_configure.enableAll();
  m_basicdownloader.enableAll();
  m_batchdownloader.enableAll();
  m_playlistdownloader.enableAll();
  m_library.enableAll();

  m_uiEnabled = true;

  return *this;
}

tabManager &tabManager::disableAll() {
  m_accountManager.disableAll();
  // m_ytSearch.disableAll(); //keep this enabled
  // m_xpSearch.disableAll(); //keep this enabled
  m_configure.disableAll();
  m_basicdownloader.disableAll();
  m_batchdownloader.disableAll();
  m_playlistdownloader.disableAll();
  m_library.disableAll();

  m_uiEnabled = false;

  return *this;
}

tabManager &tabManager::resetMenu() {
  m_accountManager.resetMenu();
  m_ytSearch.resetMenu();
  m_configure.resetMenu();
  m_basicdownloader.resetMenu();
  m_batchdownloader.resetMenu();
  m_playlistdownloader.resetMenu();
  m_library.resetMenu();

  return *this;
}

tabManager &tabManager::reTranslateUi() {
  m_accountManager.retranslateUi();
  m_ytSearch.retranslateUi();
  m_configure.retranslateUi();
  m_basicdownloader.retranslateUi();
  m_batchdownloader.retranslateUi();
  m_playlistdownloader.retranslateUi();
  m_library.retranslateUi();

  return *this;
}

void tabManager::toggleXpSearch(bool enable) {
  m_configure.toggleXpSearch(enable);
}

ITrackingService *tabManager::trackingService() const {
  return m_trackingService;
}
