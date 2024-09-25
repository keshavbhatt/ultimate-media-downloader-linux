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

#include "tabwidget.h"
#include "cookiedialog.h"
#include "webpage.h"
#include "webview.h"

#include <QFile>
#include <QTimer>

TabWidget::TabWidget(QWebEngineProfile *profile, QWidget *parent)
    : FancyTabWidget(parent, QIcon(":add-box-fill.png")), m_profile(profile) {

  connect(this, &FancyTabWidget::tabAddRequested, [this]() { createTab(); });
  connect(this, &QTabWidget::currentChanged, this,
          &TabWidget::handleCurrentChanged);

  this->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &TabWidget::customContextMenuRequested, this,
          &TabWidget::handleContextMenuRequested);

  QTabBar *tabBar = this->tabBar();
  tabBar->setElideMode(Qt::ElideRight);
  tabBar->setTabsClosable(true);
  tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
  tabBar->setMovable(true);
  tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(tabBar, &QTabBar::customContextMenuRequested, this,
          &TabWidget::handleContextMenuRequested);
  connect(tabBar, &QTabBar::tabCloseRequested, this, &TabWidget::closeTab);
  connect(tabBar, &QTabBar::tabBarDoubleClicked, this, [=](int index) {
    if (index == -1)
      createTab();
  });

  if (profile->isOffTheRecord()) {
    QLabel *icon = new QLabel(this);
    QPixmap pixmap(QStringLiteral(":ninja.png"));
    icon->setPixmap(pixmap.scaledToHeight(tabBar->height()));
    auto localPixmap = icon->pixmap(Qt::ReturnByValue);
    setStyleSheet(QStringLiteral("QTabWidget::tab-bar { left: %1px; }")
                      .arg(localPixmap.width()));
  }

  readHomePage();
}

void TabWidget::handleCurrentChanged(int index) {
  if (index != -1) {
    WebView *view = webView(index);
    if (!view->url().isEmpty())
      view->setFocus();
    emit titleChanged(view->title());
    emit loadProgress(view->loadProgress());
    emit urlChanged(view->url());
    emit favIconChanged(view->favIcon());
    emit webActionEnabledChanged(
        QWebEnginePage::Back, view->isWebActionEnabled(QWebEnginePage::Back));
    emit webActionEnabledChanged(
        QWebEnginePage::Forward,
        view->isWebActionEnabled(QWebEnginePage::Forward));
    emit webActionEnabledChanged(
        QWebEnginePage::Stop, view->isWebActionEnabled(QWebEnginePage::Stop));
    emit webActionEnabledChanged(
        QWebEnginePage::Reload,
        view->isWebActionEnabled(QWebEnginePage::Reload));
  } else {
    emit titleChanged(QString());
    emit loadProgress(0);
    emit urlChanged(QUrl());
    emit favIconChanged(QIcon());
    emit webActionEnabledChanged(QWebEnginePage::Back, false);
    emit webActionEnabledChanged(QWebEnginePage::Forward, false);
    emit webActionEnabledChanged(QWebEnginePage::Stop, false);
    emit webActionEnabledChanged(QWebEnginePage::Reload, true);
  }
}

void TabWidget::handleContextMenuRequested(const QPoint &pos) {
  QMenu menu;
  menu.addAction(tr("New &Tab"), this, &TabWidget::createTab,
                 QKeySequence::AddTab);
  int index = tabBar()->tabAt(pos);
  if (index != -1) {
    QAction *action = menu.addAction(tr("Clone Tab"));
    connect(action, &QAction::triggered, this,
            [this, index]() { cloneTab(index); });
    menu.addSeparator();
    action = menu.addAction(tr("&Close Tab"));
    action->setShortcut(QKeySequence::Close);
    connect(action, &QAction::triggered, this,
            [this, index]() { closeTab(index); });
    action = menu.addAction(tr("Close &Other Tabs"));
    connect(action, &QAction::triggered, this,
            [this, index]() { closeOtherTabs(index); });
    menu.addSeparator();
    action = menu.addAction(tr("Reload Tab"));
    action->setShortcut(QKeySequence::Refresh);
    connect(action, &QAction::triggered, this,
            [this, index]() { reloadTab(index); });
  } else {
    menu.addSeparator();
  }
  menu.addAction(tr("Reload All Tabs"), this, &TabWidget::reloadAllTabs);
  menu.exec(QCursor::pos());
}

WebView *TabWidget::currentWebView() const { return webView(currentIndex()); }

WebView *TabWidget::webView(int index) const {
  return qobject_cast<WebView *>(widget(index));
}

void TabWidget::setupView(WebView *webView) {
  QWebEnginePage *webPage = webView->page();

  connect(webView, &QWebEngineView::titleChanged, this,
          [this, webView](const QString &title) {
            int index = indexOf(webView);
            if (index != -1) {
              setTabText(index, title);
              setTabToolTip(index, title);
            }
            if (currentIndex() == index)
              emit titleChanged(title);
          });
  connect(webView, &QWebEngineView::urlChanged, this,
          [this, webView](const QUrl &url) {
            int index = indexOf(webView);
            if (index != -1)
              tabBar()->setTabData(index, url);
            if (currentIndex() == index)
              emit urlChanged(url);

            if (url.toString() == "browser://homepage") {
              currentWebView()->setHtml(m_homePageHtmlContent,
                                        QUrl("browser://homepage"));
            }
          });
  connect(webView, &QWebEngineView::loadProgress, this,
          [this, webView](int progress) {
            if (currentIndex() == indexOf(webView))
              emit loadProgress(progress);
          });
  connect(webPage, &QWebEnginePage::linkHovered, this,
          [this, webView](const QString &url) {
            if (currentIndex() == indexOf(webView))
              emit linkHovered(url);
          });
  connect(webView, &WebView::favIconChanged, this,
          [this, webView](const QIcon &icon) {
            int index = indexOf(webView);
            if (index != -1)
              setTabIcon(index, icon);
            if (currentIndex() == index)
              emit favIconChanged(icon);
          });
  connect(webView, &WebView::webActionEnabledChanged, this,
          [this, webView](QWebEnginePage::WebAction action, bool enabled) {
            if (currentIndex() == indexOf(webView))
              emit webActionEnabledChanged(action, enabled);
          });
  connect(webPage, &QWebEnginePage::windowCloseRequested, this,
          [this, webView]() {
            int index = indexOf(webView);
            if (webView->page()->inspectedPage())
              window()->close();
            else if (index >= 0)
              closeTab(index);
          });

  connect(webPage, &QWebEnginePage::fullScreenRequested, this,
          [=](QWebEngineFullScreenRequest request) {
            emit fullScreenRequested(request);
          });

  connect(webView, &WebView::devToolsRequested, this,
          &TabWidget::devToolsRequested);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  connect(webPage, &QWebEnginePage::findTextFinished, this,
          [this, webView](const QWebEngineFindTextResult &result) {
            if (currentIndex() == indexOf(webView))
              emit findTextFinished(result);
          });
#endif
}

WebView *TabWidget::createTab() {
  WebView *webView = createBackgroundTab();
  setCurrentWidget(webView);

  return webView;
}

void TabWidget::readHomePage() {
  QFile file(":/homepage.html");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    m_homePageHtmlContent = file.readAll();
  }
}

bool TabWidget::isDarkTheme(QWidget *widget) {
  QPalette palette = widget->palette();

  // Compare the background and text colors
  QColor backgroundColor = palette.color(QPalette::Window);
  QColor textColor = palette.color(QPalette::WindowText);

  // Calculate the brightness of the colors
  int backgroundBrightness =
      (backgroundColor.red() * 299 + backgroundColor.green() * 587 +
       backgroundColor.blue() * 114) /
      1000;
  int textBrightness = (textColor.red() * 299 + textColor.green() * 587 +
                        textColor.blue() * 114) /
                       1000;

  // If background is darker than the text, it's likely a dark theme
  return backgroundBrightness < textBrightness;
}

WebView *TabWidget::createBackgroundTab() {
  WebView *webView = new WebView;
  WebPage *webPage = new WebPage(m_profile, webView);
  webView->setPage(webPage);
  setupView(webView);
  int index = addTab(webView, tr("(Untitled)"));
  setTabIcon(index, webView->favIcon());

  if (isDarkTheme(this)) {
    webView->setHtml(m_homePageHtmlContent.replace("class=\"light-mode\"",
                                                   "class=\"dark-mode\""),
                     QUrl("browser://homepage"));
  } else {
    webView->setHtml(m_homePageHtmlContent, QUrl("browser://homepage"));
  }

  // Workaround for QTBUG-61770
  webView->resize(currentWidget()->size());
  webView->show();
  return webView;
}

void TabWidget::reloadAllTabs() {
  for (int i = 0; i < count(); ++i)
    webView(i)->reload();
}

void TabWidget::closeAllTabs() {
  for (int i = 0; i < count(); ++i)
    closeTab(i);
}

void TabWidget::closeOtherTabs(int index) {
  for (int i = count() - 1; i > index; --i)
    closeTab(i);
  for (int i = index - 1; i >= 0; --i)
    closeTab(i);
}

void TabWidget::closeTab(int index) {
  if (WebView *view = webView(index)) {
    bool hasFocus = view->hasFocus();
    removeTab(index);
    if (hasFocus && count() > 0)
      currentWebView()->setFocus();
    if (count() == 0)
      createTab();
    view->deleteLater();
  }
}

void TabWidget::cloneTab(int index) {
  if (WebView *view = webView(index)) {
    WebView *tab = createTab();
    tab->setUrl(view->url());
  }
}

void TabWidget::setUrl(const QUrl &url) {
  if (WebView *view = currentWebView()) {
    view->setUrl(url);
    view->setFocus();
  }
}

void TabWidget::triggerWebPageAction(QWebEnginePage::WebAction action) {
  if (WebView *webView = currentWebView()) {
    webView->triggerPageAction(action);
    webView->setFocus();
  }
}

void TabWidget::nextTab() {
  int next = currentIndex() + 1;
  if (next == count())
    next = 0;
  setCurrentIndex(next);
}

void TabWidget::previousTab() {
  int next = currentIndex() - 1;
  if (next < 0)
    next = count() - 1;
  setCurrentIndex(next);
}

void TabWidget::reloadTab(int index) {
  if (WebView *view = webView(index))
    view->reload();
}
