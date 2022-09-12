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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "engines.h"
#include "engineupdatecheck.h"
#include "logger.h"
#include "logwindow.h"
#include "rateapp.h"
#include "tabmanager.h"

#include <QApplication>
#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QString>
#include <QStringList>
#include <QToolBar>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <browser.h>
#include <browserwindow.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(QApplication &, settings &, translator &, const QStringList &);
  void retranslateUi();
  void setTitle(const QString &m);
  void resetTitle();
  void Show();
  void processEvent(const QByteArray &e);
  void quitApp();
  void log(const QByteArray &);
  ~MainWindow() override;

public slots:
  QString dumpBrowserCookie();

  void browserOpenLink(QUrl url);
protected slots:
  bool eventFilter(QObject *o, QEvent *e) override;
  void closeEvent(QCloseEvent *) override;

private slots:
  void initRateWidget();
  void restoreMainWindow();
  void handleEngineUpdateAvailable();

  void initAndShowBrowserWindow(bool show = true);
  void onAboutActionTriggered();

private:
  QApplication &m_qApp;
  std::unique_ptr<Ui::MainWindow> m_ui;
  Logger m_logger;
  engines m_engines;
  utility::versionInfo m_cou;
  tabManager m_tabManager;
  settings &m_settings;
  QString m_defaultWindowTitle;

  EngineUpdateCheck *m_engineUpdateCheck = nullptr;
  void initToolbar();

  Browser m_browser;
  BrowserWindow *m_browser_window = nullptr;

  QAction *m_browserAction;
  QAction *m_aboutAction;
  void initBrowser();
  QToolBar *m_toolbar = nullptr;
};

#endif // MAINWINDOW_H
