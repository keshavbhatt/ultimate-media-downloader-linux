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

#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QClipboard>
#include <QFile>
#include <QMainWindow>
#include <QNetworkCookie>
#include <QSettings>
#include <QTemporaryFile>
#include <QTime>
#include <QVector>
#include <QWebEngineFullScreenRequest>
#include <QWebEnginePage>

#include <3rdParty/AdBlocker/requestinterceptor.h>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QProgressBar;
QT_END_NAMESPACE

class Browser;
class TabWidget;
class WebView;

class BrowserWindow : public QMainWindow {
  Q_OBJECT

public:
  BrowserWindow(Browser *browser, QWebEngineProfile *profile,
                bool forDevTools = false,
                RequestInterceptor *adBlockRequestInterceptor = nullptr);
  QSize sizeHint() const override;
  TabWidget *tabWidget() const;
  WebView *currentTab() const;
  Browser *browser() { return m_browser; }

  void showAllCookies();
  QString dumpCookiesToNetscapeFormat();

  void quit();

signals:
  void cookiesCleared();

protected:
  void closeEvent(QCloseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private slots:
  void handleNewWindowTriggered();
  void handleNewIncognitoWindowTriggered();
  void handleFileOpenTriggered();
  void handleFindActionTriggered();
  void handleShowWindowTriggered();
  void handleWebViewLoadProgress(int);
  void handleWebViewTitleChanged(const QString &title);
  void handleWebActionEnabledChanged(QWebEnginePage::WebAction action,
                                     bool enabled);
  void handleDevToolsRequested(QWebEnginePage *source);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  void handleFindTextFinished(const QWebEngineFindTextResult &result);
#endif

  void onFullScreenRequested(QWebEngineFullScreenRequest request);

private:
  Browser *m_browser;
  QWebEngineProfile *m_profile;
  TabWidget *m_tabWidget;
  QProgressBar *m_progressBar;
  QAction *m_historyBackAction;
  QAction *m_historyForwardAction;
  QAction *m_stopAction;
  QAction *m_reloadAction;
  QAction *m_stopReloadAction;
  QAction *m_homePageAction;
  QLineEdit *m_urlLineEdit;
  QAction *m_favAction;
  QString m_lastSearch;
  bool m_quitting;

  QToolBar *navigationBar;
  QMenu *createFileMenu(TabWidget *tabWidget);
  QMenu *createEditMenu();
  QMenu *createViewMenu(QToolBar *toolBar);
  QMenu *createWindowMenu(TabWidget *tabWidget);
  QMenu *createAdBlockMenu(TabWidget *tabWidget);
  QToolBar *createToolBar();
  RequestInterceptor *m_adBlockRequestInterceptor = nullptr;

  bool containsCookie(const QNetworkCookie &cookie);
  QVector<QNetworkCookie> m_cookies;
  Qt::WindowStates windowStateBeforeFullScreen;
  void setFullscreen(bool fullscreen);
  void toggleFullScreen();

  QSettings settings;
};

#endif // BROWSERWINDOW_H
