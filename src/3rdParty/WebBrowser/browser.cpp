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

#include "browser.h"
#include "browserwindow.h"

#include <QPointer>

Browser::Browser() {
  // Quit application if the download manager window is the only remaining
  // window
  m_downloadManagerWidget.setAttribute(Qt::WA_QuitOnClose, false);

  QObject::connect(QWebEngineProfile::defaultProfile(),
                   &QWebEngineProfile::downloadRequested,
                   &m_downloadManagerWidget,
                   &DownloadManagerWidget::downloadRequested);
}

BrowserWindow *Browser::createWindow(bool offTheRecord) {
  BrowserWindow *mainWindow = createBrowserWindow(offTheRecord);
  QObject::connect(mainWindow, &QObject::destroyed, mainWindow,
                   [this, mainWindow]() {
                     if (mainWindow) {
                       m_windows.removeOne(mainWindow);
                     }
                   });
  mainWindow->show();
  return mainWindow;
}

BrowserWindow *Browser::createWindowHidden() {
  BrowserWindow *mainWindow = createBrowserWindow(false);
  QObject::connect(mainWindow, &QObject::destroyed, mainWindow, [mainWindow]() {
    if (mainWindow) {
      mainWindow->deleteLater();
    }
  });
  return mainWindow;
}

BrowserWindow *Browser::createBrowserWindow(bool offTheRecord) {
  if (offTheRecord && !m_otrProfile) {
    m_otrProfile.reset(new QWebEngineProfile);
    QObject::connect(m_otrProfile.get(), &QWebEngineProfile::downloadRequested,
                     &m_downloadManagerWidget,
                     &DownloadManagerWidget::downloadRequested);
  }

  QWebEngineProfile *profile =
      offTheRecord ? m_otrProfile.get() : QWebEngineProfile::defaultProfile();

  // profile->setHttpUserAgent("Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:90.0)
  // "
  //                           "Gecko/20100101 Firefox/90.0");

  profile->setHttpUserAgent(
      "Mozilla/5.0 (X11; CrOS x86_64 10895.56.0) AppleWebKit/537.36 (KHTML, "
      "like Gecko) Chrome/130.0.3497.102 Safari/537.36");

  // Mozilla/5.0 ({os_info}; rv:90.0) Gecko/20100101 Firefox/90.0

  QPointer<BrowserWindow> mainWindow = new BrowserWindow(this, profile, false);
  m_windows.append(mainWindow);

  return mainWindow;
}

BrowserWindow *Browser::createDevToolsWindow() {
  auto profile = QWebEngineProfile::defaultProfile();

  QPointer<BrowserWindow> mainWindow = new BrowserWindow(this, profile, true);
  m_windows.append(mainWindow);

  QObject::connect(mainWindow, &QObject::destroyed, mainWindow,
                   [this, mainWindow]() {
                     if (mainWindow) {
                       m_windows.removeOne(mainWindow);
                     }
                   });

  mainWindow->show();
  return mainWindow;
}
