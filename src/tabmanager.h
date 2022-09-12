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

#ifndef TABMANAGER_H
#define TABMANAGER_H

#include "ITrackingService.h"
#include "accountmanager.h"
#include "basicdownloader.h"
#include "batchdownloader.h"
#include "configure.h"
#include "context.hpp"
#include "library.h"
#include "playlistdownloader.h"
#include "posthogtrackingservice.h"
#include "xpsearch.h"
#include "ytsearch.h"

class tabManager {
public:
  tabManager(settings &s, translator &t, engines &e, Logger &l,
             Ui::MainWindow &ui, QWidget &w, MainWindow &mw,
             utility::versionInfo &u, QString debug);
  void init_done(Ui::MainWindow &ui, settings &settings);
  void setDefaultEngines();
  tabManager &gotEvent(const QByteArray &e);
  tabManager &enableAll();
  tabManager &disableAll();
  tabManager &resetMenu();
  tabManager &reTranslateUi();
  basicdownloader &basicDownloader() { return m_basicdownloader; }
  batchdownloader &batchDownloader() { return m_batchdownloader; }
  playlistdownloader &playlistDownloader() { return m_playlistdownloader; }
  configure &Configure() { return m_configure; }
  accountManager &AccountManager() { return m_accountManager; }
  YtSearch &YoutubeSearch() { return m_ytSearch; }
  XpSearch &XpornSearch() { return m_xpSearch; }
  ITrackingService *trackingService() const;
  bool uiEnabled() { return m_uiEnabled; }
  void toggleXpSearch(bool enable);

  void dumpCookie();
  private:
  int m_currentTab;
  bool m_uiEnabled = true;
  Context m_ctx;
  accountManager m_accountManager;
  configure m_configure;
  basicdownloader m_basicdownloader;
  batchdownloader m_batchdownloader;
  playlistdownloader m_playlistdownloader;
  YtSearch m_ytSearch;
  XpSearch m_xpSearch;
  library m_library;
  ITrackingService *m_trackingService = nullptr;
  QMetaObject::Connection m_initConnection;
  void enableXpSearchSilently();
};

#endif
