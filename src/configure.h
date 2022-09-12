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
#ifndef CONFIGURE_H
#define CONFIGURE_H

#include "context.hpp"
#include "networkAccess.h"
#include "settings.h"
#include "tableWidget.h"
#include "utility.h"

#include <QMenu>

class tabManager;

class configure : public QObject {
  Q_OBJECT
public:
  configure(const Context &);
  void init_done();
  void enableAll();
  void disableAll();
  void resetMenu();
  void retranslateUi();
  void downloadFromGitHub(const engines::Iterator &);
  void tabExited();
  void tabEntered();
  void updateEnginesList(const QStringList &e);
  void setDownloadOptions(int row, tableWidget &table);
  template <typename Function>
  void presetOptionsForEach(const Function &function) {
    m_presetOptions.forEach(function);
  }
  void enableXpSearchSilently();
  void toggleXpSearch(bool enable);

  void updateCookiePath();
  protected:
  bool eventFilter(QObject *obj, QEvent *event);

private slots:
  void updatePreferredYtPlaybackQuality(int preferedQuality);

private:
  class presetOptions {
  public:
    presetOptions(const Context &, settings &);
    ~presetOptions();
    void clear();
    void setDefaults();
    void add(const QString &uiName, const QString &options);
    template <typename Function> void forEach(const Function &function) {
      for (const auto &it : util::asConst(m_array)) {

        auto obj = it.toObject();

        if (!obj.isEmpty()) {

          auto a = obj.value("uiName").toString();
          auto b = obj.value("options").toString();

          if (!a.isEmpty() && !b.isEmpty()) {

            function(a, b);
          }
        }
      }
    }

  private:
    QByteArray defaultData();
    QString m_path;
    QJsonArray m_array;
  };
  class downloadDefaultOptions {
  public:
    struct opts {
      const QString &comparator;
      const QString &downloadOptions;
      const QString &engine;
      const QString &url;
    };
    downloadDefaultOptions(const Context &);
    void save();
    QJsonObject add(const QString &url, const QString &opts,
                    const QString &engineName);
    QJsonObject add(const configure::downloadDefaultOptions::opts &);
    void remove(const QJsonObject &);
    template <typename Function> void forEach(const Function &function) {
      for (const auto &it : util::asConst(m_array)) {

        auto obj = it.toObject();

        if (!obj.isEmpty()) {

          auto a = obj.value("comparator").toString();
          auto b = obj.value("downloadOption").toString();
          auto c = obj.value("engine").toString();
          auto d = obj.value("url").toString();

          if (function({a, b, c, d}, std::move(obj))) {

            break;
          }
        }
      }
    }
    void setDownloadOptions(int row, tableWidget &table) {
      this->forEach([&](const opts &e, const QJsonObject &) {
        const auto &rowUrl = table.url(row);

        const auto &cmp = e.comparator;
        const auto &url = e.url;
        const auto &eng = e.engine;
        const auto &opt = e.downloadOptions;

        auto dlo = tableWidget::type::DownloadOptions;
        auto en = tableWidget::type::EngineName;

        if (cmp == "contains" && rowUrl.contains(url)) {

          if (!opt.isEmpty()) {

            table.setDownloadingOptions(dlo, row, opt);
          }

          if (!eng.isEmpty()) {

            table.setDownloadingOptions(en, row, eng);
          }

          return true;

        } else if (cmp == "startsWith" && rowUrl.startsWith(url)) {

          if (!opt.isEmpty()) {

            table.setDownloadingOptions(dlo, row, opt);
          }

          if (!eng.isEmpty()) {

            table.setDownloadingOptions(en, row, eng);
          }

          return true;

        } else if (cmp == "endsWith" && rowUrl.endsWith(url)) {

          if (!opt.isEmpty()) {

            table.setDownloadingOptions(dlo, row, opt);
          }

          if (!eng.isEmpty()) {

            table.setDownloadingOptions(en, row, eng);
          }

          return true;
        } else {
          return false;
        }
      });
    }

  private:
    QString m_path;
    QJsonArray m_array;
  };

  void saveOptions();
  void setEngineOptions(const QString &);

  const Context &m_ctx;
  settings &m_settings;
  Ui::MainWindow &m_ui;
  QWidget &m_mainWindow;
  tabManager &m_tabManager;
  engines &m_engines;
  presetOptions m_presetOptions;
  downloadDefaultOptions m_downloadDefaultOptions;
  bool isChildOf(QObject *Of, QObject *self);
};

#endif
