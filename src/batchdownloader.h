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
#ifndef BATCH_DOWNLOADER_URL_H
#define BATCH_DOWNLOADER_URL_H

#include "context.hpp"
#include "downloadmanager.h"
#include "settings.h"
#include "tableWidget.h"
#include "utility.h"
#include <QMenu>
#include <QString>
#include <QStringList>

class tabManager;

class Items {
public:
  struct entry {
    entry(const QString &uiText, const QString &url)
        : uiText(uiText), url(url) {}
    QString uiText;
    QString url;
  };
  Items() = default;
  Items(const QString &url) { m_entries.emplace_back(url, url); }
  Items(const QString &uiText, const QString &url) {
    m_entries.emplace_back(uiText, url);
  }
  void add(const QString &uiText, const QString &url) {
    m_entries.emplace_back(uiText, url);
  }
  void add(const QString &url) { m_entries.emplace_back(url, url); }
  const Items::entry &at(size_t s) const { return m_entries[s]; }
  const Items::entry &first() const { return m_entries[0]; }
  size_t size() const { return m_entries.size(); }
  bool hasOneEntry() const { return m_entries.size() == 1; }
  Items::entry takeFirst() {
    auto m = m_entries[0];

    m_entries.erase(m_entries.begin());

    return m;
  }
  bool isEmpty() const { return m_entries.size() == 0; }
  auto begin() { return m_entries.begin(); }
  auto end() { return m_entries.end(); }
  auto begin() const { return m_entries.begin(); }
  auto end() const { return m_entries.end(); }

private:
  std::vector<entry> m_entries;
};

class ItemEntry {
public:
  ItemEntry() = default;
  ItemEntry(const engines::engine &engine, Items list)
      : m_engine(&engine), m_list(std::move(list)) {}
  Items::entry next() { return m_list.takeFirst(); }
  bool hasNext() const { return !m_list.isEmpty(); }
  const engines::engine &engine() { return *m_engine; }

private:
  const engines::engine *m_engine;
  Items m_list;
};

Q_DECLARE_METATYPE(ItemEntry)

class batchdownloader : public QObject {
  Q_OBJECT
public:
  batchdownloader(const Context &);
  ~batchdownloader();
  void init_done();
  void enableAll();
  void disableAll();
  void resetMenu();
  void retranslateUi();
  void tabEntered();
  void tabExited();
  void gotEvent(const QByteArray &);
  //	void updateEnginesList( const QStringList& ) ;
  void setThumbnailColumnSize(bool);
private slots:
  void addItemUiSlot(ItemEntry);

private:
  void getListFromFile(QMenu &);
  QString defaultEngineName();
  const engines::engine &defaultEngine();
  void clearScreen();
  void showList();
  void addToList(const QString &, bool autoDownload = false,
                 bool showThumbnails = true);
  void download(const engines::engine &, downloadManager::index);
  void download(const engines::engine &);
  void download(const engines::engine &, int);
  void addItem(int, bool, const utility::MediaEntry &);
  void addItemUi(int, bool, const utility::MediaEntry &);
  void addItemUi(const QPixmap &pixmap, int, bool, const utility::MediaEntry &);
  void showThumbnail(const engines::engine &, int, const QString &url, bool);

  void showThumbnail(const engines::engine &, Items, bool = false,
                     bool = false);

  const Context &m_ctx;
  settings &m_settings;
  Ui::MainWindow &m_ui;
  QWidget &m_mainWindow;
  tabManager &m_tabManager;
  bool m_showThumbnails;
  tableWidget m_table;
  tableMiniWidget<int> m_tableWidgetBDList;
  QString m_debug;
  int m_networkRunning = false;
  QStringList m_optionsList;
  QLineEdit m_lineEdit;
  QPixmap m_defaultVideoThumbnail;

  utility::Terminator m_terminator;

  downloadManager m_ccmd;

  class BatchLogger {
  public:
    BatchLogger(Logger &l) : m_logger(l), m_id(utility::concurrentID()) {}
    void add(const QString &e) { this->add(e.toUtf8()); }
    void add(const QByteArray &e) { m_logger.add(e, m_id); }
    void clear() {}
    template <typename Function> void add(const Function &function) {
      m_logger.add(function, m_id);
      function(m_lines, m_id, false);
    }
    void logError(const QByteArray &data) { m_logger.logError(data, m_id); }
    QByteArray data() const { return m_lines.toLine(); }

  private:
    Logger::Data m_lines;
    Logger &m_logger;
    int m_id;
  };

  class BatchLoggerWrapper {
  public:
    BatchLoggerWrapper(Logger &l)
        : m_logger(std::make_shared<BatchLogger>(l)) {}
    void add(const QByteArray &e) { m_logger->add(e); }
    void clear() { m_logger->clear(); }
    template <typename Function> void add(const Function &function) {
      m_logger->add(function);
    }
    QByteArray data() const { return m_logger->data(); }
    void logError(const QByteArray &data) { m_logger->logError(data); }

  private:
    std::shared_ptr<BatchLogger> m_logger;
  };

  struct opts {
    const Context &ctx;
    QString debug;
    bool listRequested;
    int index;
    BatchLoggerWrapper batchLogger;
  };

  template <typename Functions>

  auto make_options(batchdownloader::opts opts, Functions f) {
    return utility::options<batchdownloader::opts, Functions>(std::move(opts),
                                                              std::move(f));
  }
  void processSavedBtachDownloadFile(const QString &filePath);
  void saveBatchDownloadList(tableWidget &t_tableWidget);
  void loadListFromLastSession(QMenu &m);
};

#endif
