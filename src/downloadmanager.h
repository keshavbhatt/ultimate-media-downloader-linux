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

#ifndef CCDOWNLOAD_MG_H
#define CCDOWNLOAD_MG_H

#include "context.hpp"
#include "engines.h"
#include "tableWidget.h"
#include "utility.h"
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <memory>

class downloadManager {
public:
  class finishedStatus {
  public:
    static QString notStarted() { return "Not Started"; }
    static QString running() { return "Running"; }
    static QString finishedCancelled() { return "FinishedCancelled"; }
    static QString finishedWithError() { return "FinishedWithError"; }
    static QString finishedWithSuccess() { return "FinishedWithSuccess"; }
    static bool notStarted(const QString &e) { return notStarted() == e; }
    static bool running(const QString &e) { return running() == e; }
    static bool finishedCancelled(const QString &e) {
      return finishedCancelled() == e;
    }
    static bool finishedWithError(const QString &e) {
      return finishedWithError() == e;
    }
    static bool finishedWithSuccess(const QString &e) {
      return finishedWithSuccess() == e;
    }
    static bool finishedCancelled(tableWidget &e, int row) {
      return finishedCancelled(e.runningState(row));
    }
    static bool finishedWithError(tableWidget &e, int row) {
      return finishedWithError(e.runningState(row));
    }
    static bool finishedWithSuccess(tableWidget &e, int row) {
      return finishedWithSuccess(e.runningState(row));
    }
    QString setState() const {
      if (this->exitState().cancelled()) {

        return finishedCancelled();

      } else if (this->exitState().success()) {

        return finishedWithSuccess();
      } else {
        return finishedWithError();
      }
    }
    int index() const { return m_index; }
    bool allFinished() const { return m_allFinished; }
    const utility::ProcessExitState &exitState() const { return m_exitState; }
    finishedStatus(int i, bool s, utility::ProcessExitState e)
        : m_index(i), m_allFinished(s), m_exitState(std::move(e)) {}

  private:
    int m_index;
    bool m_allFinished;
    utility::ProcessExitState m_exitState;
  };

  class index {
  public:
    index(tableWidget &t) : m_table(t) {}
    int value(int s) const { return this->Entry(s).index; }
    const QString &options(int s) const { return this->Entry(s).options; }
    bool forceDownload(int s) const { return this->Entry(s).forceDownload; }
    bool forceDownload() const { return this->forceDownload(m_index); }
    int value() const { return this->value(m_index); }
    size_t count() const { return m_entries.size(); }
    void next() { m_index++; }
    bool hasNext() const {
      return static_cast<size_t>(m_index) < m_entries.size();
    }
    tableWidget &table() const { return m_table; }
    void add(int index, const QString &url, bool forceUpdate = false) {
      m_entries.emplace_back(index, url, forceUpdate);
    }
    bool empty() const { return m_entries.empty(); }
    const QString &options() const { return this->options(m_index); }
    QString indexAsString() const { return this->indexAsString(m_index); }
    template <typename T> QString indexAsString(T s) const {
      auto m = QString::number(s + 1);
      auto r = QString::number(m_table.rowCount() + 1);

      while (r.size() > m.size()) {

        m = "0" + m;
      }

      return m;
    }

  private:
    struct entry {
      entry(int i, const QString &o, bool s)
          : index(i), options(o), forceDownload(s) {}
      int index;
      QString options;
      bool forceDownload;
    };
    const entry &Entry(int s) const {
      return m_entries[static_cast<size_t>(s)];
    }
    int m_index = 0;
    std::vector<entry> m_entries;
    tableWidget &m_table;
  };

  downloadManager(const Context &ctx, QPushButton &cancelButton, settings &s)
      : m_ctx(ctx), m_cancelButton(cancelButton), m_settings(s) {}
  void cancelled() { m_cancelled = true; }
  template <typename Function, typename Finished>
  void monitorForFinished(const engines::engine &engine, int index,
                          utility::ProcessExitState exitState,
                          Function function, Finished finished) {
    if (m_cancelled) {

      m_cancelButton.setEnabled(false);

      finished({index, true, std::move(exitState)});
    } else {
      m_counter++;

      if (m_counter == m_index->count()) {

        if (m_index->table().noneAreRunning()) {

          m_cancelButton.setEnabled(false);
        }

        finished({index, true, std::move(exitState)});
      } else {
        finished({index, false, std::move(exitState)});

        if (m_index->hasNext()) {

          function(engine, m_index->value());
        }
      }
    }
  }
  template <typename ConcurrentDownload>
  void download(downloadManager::index index, const engines::engine &engine,
                size_t maxNumberOfConcurrency,
                ConcurrentDownload concurrentDownload) {
    m_index = std::move(index);

    m_counter = 0;
    m_cancelled = false;

    this->uiEnableAll(false);
    m_cancelButton.setEnabled(true);
    m_index->table().setEnabled(true);

    auto min = std::min(m_index->count(), maxNumberOfConcurrency);

    for (size_t s = 0; s < min; s++) {
      concurrentDownload(engine, m_index->value(s));
    }
  }
  template <typename Options, typename Logger, typename TermSignal>
  void download(const engines::engine &engine, QStringList cliOptions,
                const QString &url, TermSignal conn, Options opts,
                Logger logger,
                utility::ProcessOutputChannels channel =
                    utility::ProcessOutputChannels()) {
    m_index->next();

    cliOptions.append(url);

    auto ctx = utility::make_ctx(engine, std::move(opts), std::move(logger),
                                 std::move(conn), channel);

    utility::run(cliOptions, QString(), std::move(ctx));
  }
  template <typename Options, typename Logger, typename TermSignal,
            typename OptionUpdater>
  void download(const engines::engine &engine, const OptionUpdater &optsUpdater,
                const engines::enginePaths &ep, const QString &url,
                TermSignal terminator, Options opts, Logger logger,
                utility::ProcessOutputChannels channel =
                    utility::ProcessOutputChannels()) {
    const auto &m = m_index->options();

    auto iString = m_index->indexAsString();

    bool fd = m_index->forceDownload();

    m_index->next();

    utility::args args(m);

    utility::updateOptionsStruct opt{engine,  ep, m_settings, args,
                                     iString, fd, {url}};

    auto ctx = utility::make_ctx(engine, std::move(opts), std::move(logger),
                                 std::move(terminator), channel);

    utility::run(optsUpdater(utility::updateOptions(opt)), args.quality(),
                 std::move(ctx));
  }

private:
  void uiEnableAll(bool e);
  size_t m_counter;
  util::storage<downloadManager::index> m_index;
  bool m_cancelled;
  const Context &m_ctx;
  QPushButton &m_cancelButton;
  settings &m_settings;
};

#endif
