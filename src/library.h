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

#ifndef LIBRARY_H
#define LIBRARY_H

#include "context.hpp"
#include <QDir>
#include <QString>
#include <QStringList>

#include "settings.h"
#include "tableWidget.h"
#include "utility.h"

class tabManager;

#include <QObject>

class library : public QObject {
  Q_OBJECT
public:
  library(const Context &);
  void init_done();
  void enableAll();
  void disableAll();
  void resetMenu();
  void retranslateUi();
  void tabEntered();
  void tabExited();
  enum class ICON { FILE, FOLDER, VIDEO, AUDIO, IMAGE, UNKNOWN };
private slots:
  static library::ICON getIconTypeFromFileName(const QString &filename);

private:
  void enableAll(bool);
  void disableAll(bool);
  void internalEnableAll();
  void internalDisableAll();
  void showContents(const QString &, bool disableUi = true);
  void moveUp();
  void addItem(const QString &text, library::ICON);
  const Context &m_ctx;
  bool m_enableGlobalUiChanges;
  settings &m_settings;
  Ui::MainWindow &m_ui;
  tableMiniWidget<ICON> m_table;
  QString m_downloadFolder;
  QString m_currentPath;
  const QDir::Filters m_dirFilter =
      QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot;
  QPixmap m_folderIcon;
  QPixmap m_videoIcon;
  QPixmap m_audioIcon;
  QPixmap m_unknownIcon;
  QPixmap m_imageIcon;
};

#endif
