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

#include "library.h"
#include "basicdownloader.h"
#include "tableWidget.h"
#include "tabmanager.h"

#include <QDir>
#include <QIcon>
#include <QMessageBox>

library::library(const Context &ctx)
    : m_ctx(ctx), m_enableGlobalUiChanges(false), m_settings(m_ctx.Settings()),
      m_ui(m_ctx.Ui()),
      m_table(*m_ui.tableWidgetLibrary, m_ctx.mainWidget().font()),
      m_downloadFolder(QDir::fromNativeSeparators(m_settings.downloadFolder())),
      m_currentPath(m_downloadFolder),
      m_folderIcon(QIcon("://icons/folder.png").pixmap(28, 28)),
      m_videoIcon(QIcon("://icons/video.png").pixmap(28, 28)),
      m_audioIcon(QIcon("://icons/music.png").pixmap(28, 28)),
      m_unknownIcon(QIcon("://icons/puzzle.png").pixmap(28, 28)),
      m_imageIcon(QIcon("://icons/photo.png").pixmap(28, 28)) {
  m_table.connect(&QTableWidget::currentItemChanged,
                  [this](QTableWidgetItem *c, QTableWidgetItem *p) {
                    m_table.selectRow(c, p, 1);
                  });

  m_table.connect(&QTableWidget::customContextMenuRequested, [this](QPoint) {
    QMenu m;

    connect(m.addAction(QIcon(":/icons/copy.png"), tr("Copy Filename")),
            &QAction::triggered, [this]() {
              auto row = m_table.currentRow();

              if (row != -1 && m_table.isSelected(row)) {

                auto name = m_table.item(row, 1).text();

                // auto m = m_currentPath + "/" + name;

                qApp->clipboard()->setText(name);
              }
            });

    connect(m.addAction(QIcon(":/icons/copy.png"), tr("Copy Filepath")),
            &QAction::triggered, [this]() {
              auto row = m_table.currentRow();

              if (row != -1 && m_table.isSelected(row)) {

                auto name = m_table.item(row, 1).text();

                auto m = m_currentPath + "/" + name;

                qApp->clipboard()->setText(m);
              }
            });

    connect(m.addAction(QIcon(":/icons/delete.png"), tr("Delete")),
            &QAction::triggered, [this]() {
              auto row = m_table.currentRow();

              if (row != -1 && m_table.isSelected(row)) {

                auto m = m_currentPath + "/" + m_table.item(row, 1).text();

                this->internalDisableAll();

                util::runInBgThread(
                    [m]() {
                      if (QFileInfo(m).isFile()) {

                        QFile::remove(m);
                      } else {
                        QDir(m).removeRecursively();
                      }
                    },
                    [row, this]() {
                      m_table.removeRow(row);

                      this->internalEnableAll();
                    });
              }
            });

    connect(m.addAction(QIcon(":/icons/delete.png"), tr("Delete All")),
            &QAction::triggered, [this]() {
              QMessageBox msgBox;
              msgBox.setText("Delete All Warning");
              msgBox.setInformativeText(
                  "This will delete everything in current "
                  "directory, Do you want to continue?");
              msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
              msgBox.setDefaultButton(QMessageBox::Cancel);
              int ret = msgBox.exec();

              // cancel deleteAll Operation
              if (ret == QMessageBox::Cancel) {
                return;
              }

              // continue delete all
              this->internalDisableAll();

              util::runInBgThread(
                  [this]() {
                    for (const auto &it :
                         QDir(m_currentPath).entryList(m_dirFilter)) {

                      auto m = m_currentPath + "/" + it;

                      QFileInfo f(m);

                      if (f.isFile()) {

                        QFile::remove(m);

                      } else if (f.isDir()) {

                        QDir(m).removeRecursively();
                      }
                    }
                  },
                  [this]() {
                    this->showContents(m_currentPath);

                    this->internalEnableAll();
                  });
            });
    if (m_table.rowCount() > 0) {

      m.exec(QCursor::pos());
    }
  });

  connect(m_ui.pbLibraryDowloadFolder, &QPushButton::clicked,
          [this]() { utility::openDownloadFolderPath(m_currentPath); });

  connect(m_ui.pbLibraryHome, &QPushButton::clicked, [this]() {
    m_downloadFolder = QDir::fromNativeSeparators(m_settings.downloadFolder());

    if (m_downloadFolder != m_currentPath) {

      m_currentPath = m_downloadFolder;

      this->showContents(m_currentPath);
    }
  });

  connect(m_ui.pbLibraryUp, &QPushButton::clicked,
          [this]() { this->moveUp(); });

  connect(m_ui.pbLibraryRefresh, &QPushButton::clicked,
          [this]() { this->showContents(m_currentPath); });

  m_table.connect(&QTableWidget::cellDoubleClicked,
                  [this](int row, int column) {
                    Q_UNUSED(column)

                    auto s = m_table.item(row, 1).text();

                    if (m_table.stuffAt(row) == library::ICON::FOLDER) {

                      m_currentPath += "/" + s;

                      this->showContents(m_currentPath);
                    } else {
                      m_ctx.Engines().openUrls(m_currentPath + "/" + s);
                    }
                  });
}

void library::moveUp() {
  if (m_currentPath != m_downloadFolder) {

    auto m = m_currentPath.lastIndexOf('/');

    if (m != -1) {

      m_currentPath.truncate(m);
    }

    this->showContents(m_currentPath);
  }
}

void library::init_done() { this->showContents(m_currentPath); }

void library::enableAll() { this->enableAll(m_enableGlobalUiChanges); }

void library::disableAll() { this->disableAll(m_enableGlobalUiChanges); }

void library::resetMenu() {}

void library::retranslateUi() {}

void library::tabEntered() {
  this->showContents(m_currentPath, m_ctx.TabManager().uiEnabled());
  m_table.get().resizeRowsToContents();
}

void library::tabExited() {}

void library::enableAll(bool e) {
  if (e) {

    m_table.setEnabled(true);
    m_ui.pbLibraryHome->setEnabled(true);
    m_ui.pbLibraryDowloadFolder->setEnabled(true);
    m_ui.pbLibraryRefresh->setEnabled(true);
    m_ui.pbLibraryUp->setEnabled(true);
  }
}

void library::disableAll(bool e) {
  if (e) {

    m_table.setEnabled(false);
    m_ui.pbLibraryHome->setEnabled(false);
    m_ui.pbLibraryDowloadFolder->setEnabled(false);
    m_ui.pbLibraryRefresh->setEnabled(false);
    m_ui.pbLibraryUp->setEnabled(false);
  }
}

void library::internalEnableAll() {
  if (m_enableGlobalUiChanges) {

    m_ctx.TabManager().enableAll();
  } else {
    this->enableAll(true);
  }
}

void library::internalDisableAll() {
  if (m_enableGlobalUiChanges) {

    m_ctx.TabManager().disableAll();
  } else {
    this->disableAll(true);
  }
}

void library::addItem(const QString &text, library::ICON type) {
  auto row = m_table.addRow(type);

  m_table.get().setCellWidget(row, 0, [&]() {
    auto label = new QLabel();

    if (type == library::ICON::FOLDER) {
      label->setPixmap(m_folderIcon);
    } else if (type == library::ICON::VIDEO) {
      label->setPixmap(m_videoIcon);
    } else if (type == library::ICON::AUDIO) {
      label->setPixmap(m_audioIcon);
    } else if (type == library::ICON::IMAGE) {
      label->setPixmap(m_imageIcon);
    } else {
      label->setPixmap(m_unknownIcon);
    }

    label->setAlignment(Qt::AlignCenter);

    return label;
  }());

  auto &item = m_table.item(row, 1);

  item.setText(text);
  item.setTextAlignment(Qt::AlignCenter);
  item.setFont(m_ctx.mainWidget().font());
}

static qint64 _created_time(QFileInfo &e) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
  return e.birthTime().toMSecsSinceEpoch();
#else
  return e.created().toMSecsSinceEpoch();
#endif
}

void library::showContents(const QString &path, bool disableUi) {
  m_table.clear();

  m_table.get().setHorizontalHeaderItem(1, new QTableWidgetItem(m_currentPath));

  if (disableUi) {

    this->internalDisableAll();
  }

  auto showCompletedOnly =
      m_settings.getValue("onlyCompletedCheckBox", true).toBool();

  util::runInBgThread(
      [path, this]() {
        auto entries =
            QDir(path).entryList(m_dirFilter, QDir::Time | QDir::DirsFirst);

        // only show first 100 items
        if (entries.count() > 100) {
          while (entries.count() > 100) {
            entries.removeLast();
          }
        }
        return entries;
      },
      [path, disableUi, showCompletedOnly, this](const QStringList &m) {
        if (disableUi) {
          this->internalEnableAll();
        }

        struct entry {
          entry(bool f, qint64 d, QString p)
              : file(f), dateCreated(d), path(std::move(p)) {}
          bool file;
          qint64 dateCreated;
          QString path;
        };

        std::vector<entry> folders;
        std::vector<entry> files;

        for (const auto &it : m) {

          if (it.startsWith("info_") && it.endsWith(".log")) {

            continue;
          }

          if (showCompletedOnly && it.endsWith(".part")) {
            continue;
          }

          if (showCompletedOnly && it.endsWith(".ytdl")) {
            continue;
          }

          auto q = path + "/" + it;

          auto w = QDir::fromNativeSeparators(it);

          QFileInfo s(q);

          if (s.isFile()) {

            files.emplace_back(true, _created_time(s), w);

          } else if (s.isDir()) {

            folders.emplace_back(false, _created_time(s), w);
          }
        }

        // SORT FILES FORLDER, WE ARE NOW USING QDIR::SORTFLAG
        //        std::sort(folders.begin(), folders.end(),
        //                  [](const entry &lhs, const entry &rhs) {
        //                    return lhs.dateCreated > rhs.dateCreated;
        //                  });

        //        std::sort(files.begin(), files.end(),
        //                  [](const entry &lhs, const entry &rhs) {
        //                    return lhs.dateCreated > rhs.dateCreated;
        //                  });

        for (const auto &it : folders) {
          this->addItem(it.path, library::ICON::FOLDER);
        }

        for (const auto &it : files) {
          this->addItem(it.path, getIconTypeFromFileName(it.path));
        }

        // SELECT LAST ITEM IN TABLE
        //        if (m_table.rowCount() > 0) {

        //          auto &t = m_table.get();

        //          t.setCurrentCell(m_table.rowCount() - 1, t.columnCount() -
        //          1);
        //        }
      });
}

library::ICON library::getIconTypeFromFileName(const QString &filename) {
  QMimeDatabase mimeDatabase;
  QMimeType mimeType;

  QList<QMimeType> mimeTypes = mimeDatabase.mimeTypesForFileName(filename);

  if (mimeTypes.isEmpty() == false) {
    mimeType = mimeTypes.first();
    QString typeStr =
        mimeType.iconName().split("-").first().trimmed().toLower();
    if (typeStr == "video") {
      return ICON::VIDEO;
    } else if (typeStr == "audio") {
      return ICON::AUDIO;
    } else if (typeStr == "image") {
      return ICON::IMAGE;
    } else {
      return library::ICON::UNKNOWN;
    }
  } else {
    return library::ICON::UNKNOWN;
  }
}
