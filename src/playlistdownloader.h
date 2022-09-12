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
#ifndef PLAYLIST_DOWNLOADER
#define PLAYLIST_DOWNLOADER

#include "utility.h"

#include "context.hpp"
#include "downloadmanager.h"
#include "settings.h"
#include "tableWidget.h"

class tabManager;

class playlistdownloader : public QObject {
  Q_OBJECT
public:
  playlistdownloader(Context &);
  void init_done();
  void enableAll();
  void disableAll();
  void resetMenu();
  void retranslateUi();
  void tabEntered();
  void tabExited();
  void gotEvent(const QByteArray &);

private:
  QString defaultEngineName();
  const engines::engine &defaultEngine();

  void download();
  void download(const engines::engine &, downloadManager::index);
  void download(const engines::engine &);
  void download(const engines::engine &, int);

  void clearScreen();
  bool enabled();

  Context &m_ctx;
  settings &m_settings;
  Ui::MainWindow &m_ui;
  QWidget &m_mainWindow;
  tabManager &m_tabManager;
  tableWidget m_table;
  bool m_gettingPlaylist = false;
  bool m_showThumbnails;
  bool m_showTimer;
  bool m_autoDownload;
  bool m_stoppedOnExisting;
  bool m_meaw;
  bool m_dataReceived;

  int m_networkRunning = 0;

  downloadManager m_ccmd;

  utility::Terminator m_terminator;

  QPixmap m_defaultVideoThumbnailIcon;

  class customOptions;

  enum class Loop { Continue, Break };
  Loop parseJson(const playlistdownloader::customOptions &, tableWidget &table,
                 Logger::Data &data);

  struct opts {
    const Context &ctx;
    QString debug;
    bool listRequested;
    int index;
  };

  template <typename Functions>
  auto make_options(playlistdownloader::opts opts, Functions f) {
    return utility::options<playlistdownloader::opts, Functions>(
        std::move(opts), std::move(f));
  }

  class subscription {
  public:
    subscription(const Context &, tableMiniWidget<int> &, QWidget &);
    void add(const QString &uiName, const QString &url, const QString &Opts);
    void remove(int);
    void setVisible(bool);
    const QString &archivePath() const;
    struct entry {
      entry() {}
      entry(const QString &u) : url(u) {}
      entry(QString u, QString l, QString o)
          : uiName(std::move(u)), url(std::move(l)),
            getListOptions(std::move(o)) {}
      QString uiName;
      QString url;
      QString getListOptions;
    };
    std::vector<subscription::entry> entries();

  private:
    void save();
    QString m_path;
    QString m_archivePath;
    tableMiniWidget<int> &m_table;
    QWidget &m_ui;
    QJsonArray m_array;
  };

  class listIterator {
  public:
    listIterator(std::vector<subscription::entry> &&s) : m_list(std::move(s)) {}
    listIterator(const QString &s) { m_list.emplace_back(s); }
    bool hasNext() const { return m_list.size() > 1; }
    const QString &url() const { return m_list.back().url; }
    const QString &uiName() const { return m_list.back().uiName; }
    const QString &listOptions() const { return m_list.back().getListOptions; }
    listIterator next() const {
      m_list.pop_back();
      return std::move(m_list);
    }

  private:
    mutable std::vector<subscription::entry> m_list;
  };

  void getList(playlistdownloader::listIterator);
  void getList(customOptions &&, const engines::engine &, listIterator);

  // subscription m_subscription ;
};

#endif
