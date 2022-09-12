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

#include "basicdownloader.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMenu>

#include "tabmanager.h"

basicdownloader::basicdownloader(const Context &ctx)
    : m_ctx(ctx), m_settings(m_ctx.Settings()), m_debug(ctx.debug()),
      m_ui(m_ctx.Ui()), m_tabManager(m_ctx.TabManager()),
      m_tableList(*m_ui.bdTableWidgetList, m_ctx.mainWidget().font()),
      m_bogusTable(m_bogusTableOriginal, m_ctx.mainWidget().font(), 0) {
  this->setAsActive();

  m_ui.pbCancel->setEnabled(false);

  m_ui.formatSelectionWidget->setVisible(false);

  tableWidget::tableWidgetOptions opts;

  opts.customContextPolicy = Qt::NoContextMenu;
  opts.selectionMode = QAbstractItemView::ExtendedSelection;

  m_tableList.setTableWidget(opts);

  connect(m_ui.pbResetBD, &QPushButton::clicked, this,
          &basicdownloader::resetView);

  connect(m_ui.pbCustomFormatSelector, &QPushButton::clicked, this,
          &basicdownloader::showCustomFormatSelector);

  connect(m_ui.pbPasteClipboard, &QPushButton::clicked,
          [this]() { m_ui.lineEditURL->setText(utility::clipboardText()); });

  m_tableList.connect(&QTableWidget::itemClicked,
                      [this](QTableWidgetItem *item) {
                        if (item) {

                          m_tableList.selectMediaOptions(m_optionsList, *item,
                                                         *m_ui.lineEditOptions);
                        }
                      });

  connect(m_ui.pbList, &QPushButton::clicked, [this]() { this->list(); });

  this->resetMenu();

  connect(m_ui.pbDownload, &QPushButton::clicked, [this]() {
    m_ui.formatSelectionWidget->setVisible(false);

    this->download(m_ui.lineEditURL->text());
  });

  connect(m_ui.pbOptionsHistory, &QPushButton::clicked, [this]() {
    auto s = utility::showHistory(
        *m_ui.lineEditOptions,
        m_settings.getOptionsHistory(settings::tabName::basic), m_settings,
        settings::tabName::basic);

    if (s) {

      m_ui.formatSelectionWidget->setVisible(false);

      this->download(m_ui.lineEditURL->text());
    }
  });

  // auto s = static_cast< void( QComboBox::* )( int ) >( &QComboBox::activated
  // ) ;

  auto &table = m_bogusTable.get();

  table.insertRow(0);

  for (int s = 0; s < 3; s++) {

    table.insertColumn(s);
    table.setItem(0, s, new QTableWidgetItem);
  }
}

void basicdownloader::init_done() {
  // Engine presence check
  if (m_ctx.Engines().defaultEngineExists()) {
    m_ctx.logger().clear();
    m_ctx.logger().add(tr("Ready"));
  } else {
    // m_ui.pbConfigureDownload->click();
  }
}

void basicdownloader::resetMenu(const QStringList &args) {
  utility::setMenuOptions(
      m_ctx, args, false, false, m_ui.pbEntries, [this](QAction *aa) {
        utility::selectedAction ac(aa);

        if (ac.clearOptions()) {

          m_ui.lineEditOptions->clear();

        } else if (ac.clearScreen()) {

          m_ctx.logger().clear();

        } else if (ac.openFolderPath()) {

          utility::openDownloadFolderPath(m_settings.downloadFolder());
        } else {
          m_ui.lineEditOptions->setText(ac.objectName());

          if (m_settings.autoDownload()) {

            this->download(m_ui.lineEditURL->text());
          }
        }
      });
}

void basicdownloader::showCustomFormatSelector() {
  CustomFormatSelector *customFormatSelector = new CustomFormatSelector(
      m_ui.tabWidget->parentWidget(), *m_ui.bdTableWidgetList);
  customFormatSelector->setAttribute(Qt::WA_DeleteOnClose, true);
  customFormatSelector->setWindowFlags(Qt::Window);
  customFormatSelector->setWindowModality(Qt::ApplicationModal);
  connect(customFormatSelector, &CustomFormatSelector::formatSelected,
          m_ui.lineEditOptions, &QLineEdit::setText);
  customFormatSelector->show();
}

void basicdownloader::resetView() {

  m_tableList.get().setProperty("url", "");

  m_ui.lineEditURL->clear();

  m_ui.lineEditOptions->clear();

  m_ctx.logger().clear();

  m_tableList.clear();

  m_optionsList.clear();

  m_ui.formatSelectionWidget->hide();

  m_ctx.logger().add(tr("Ready"));
}

basicdownloader &basicdownloader::setAsActive() {
  m_ui.tabWidget->setCurrentIndex(0);
  return *this;
}

basicdownloader &basicdownloader::hideTableList() {
  m_ui.formatSelectionWidget->setVisible(false);
  return *this;
}

QString basicdownloader::defaultEngineName() {
  return "core";
  // we are not using the settings value since it will break many old installs
  // do not return from settings cause we changed name to core
  // return m_settings.defaultEngine(
  // settings::tabName::basic,m_ctx.Engines().defaultEngineName() ) ;
}

const engines::engine &basicdownloader::defaultEngine() {
  return m_ctx.Engines().defaultEngine(this->defaultEngineName());
}

void basicdownloader::retranslateUi() { this->resetMenu(); }

void basicdownloader::listRequested(const QByteArray &a) {
  if (a.isEmpty()) {
    m_ui.formatSelectionWidget->setVisible(false);
  } else {
    auto m = "core";

    const auto &engine = m_ctx.Engines().defaultEngine(m);

    for (const auto &m : engine.mediaProperties(a)) {

      m_tableList.add(m);
    }

    m_tableList.setEnabled(true);

    m_tableList.get().setProperty("url", m_ui.lineEditURL->text());

    m_ui.formatSelectionWidget->setVisible(true);

    m_tableList.get().resizeRowsToContents();
  }
}

void basicdownloader::list() {

  // already have format data
  if (m_tableList.get().property("url").toString() ==
      m_ui.lineEditURL->text()) {

    if (m_ui.formatSelectionWidget->isVisible()) {
      m_ui.formatSelectionWidget->setVisible(false);
    } else if (m_ui.formatSelectionWidget->isVisible() == false) {
      m_ui.formatSelectionWidget->setVisible(true);
    }

    m_tableList.get().clearSelection();

  } else {
    m_ui.formatSelectionWidget->setVisible(false);

    m_tableList.setEnabled(false);

    m_ui.pbResetBD->setEnabled(false);

    m_ui.pbCancel->setEnabled(true);

    m_tableList.clear();

    m_optionsList.clear();

    m_ui.lineEditOptions->clear();

    auto url = m_ui.lineEditURL->text();

    const auto &backend = this->defaultEngine();

    auto args = backend.defaultListCmdOptions();

    args.append(url.split(' '));

    m_tabManager.dumpCookie();

    auto cookiePath = m_settings.cookieFilePath(backend.name());
    const auto &ca = backend.cookieArgument();

    if (!cookiePath.isEmpty() && !ca.isEmpty()) {
      args.append(ca);
      args.append(cookiePath);
    }

    m_ctx.logger().clear();

    m_ctx.logger().add(tr("Listing available formats..."));

    this->run(backend, args, "", true);
  }
}

void basicdownloader::download(const QString &url) {
  if (url.isEmpty()) {

    return;
  }

  m_ctx.logger().clear();

  m_ctx.logger().add(tr("Starting download..."));

  m_settings.setLastUsedOption("core", m_ui.lineEditOptions->text(),
                               settings::tabName::basic);

  auto m = util::split(url, ' ', true);

  const auto &engine = this->defaultEngine();

  m_bogusTable.clear();

  auto uiText = m.at(0);

  auto state = downloadManager::finishedStatus::notStarted();

  m_bogusTable.addItem({uiText, uiText, state});

  auto s = m_ui.lineEditOptions->text();

  m_settings.addOptionsHistory(s, settings::tabName::basic);

  this->download(engine, s, m, false);
}

void basicdownloader::download(const engines::engine &engine,
                               const utility::args &args, const QString &url,
                               bool s) {
  this->download(engine, args, QStringList(url), s);
}

void basicdownloader::download(const engines::engine &engine,
                               const utility::args &args,
                               const QStringList &urls, bool update) {
  m_ui.formatSelectionWidget->setVisible(false);

  if (update) {

    m_ui.lineEditOptions->setText(args.quality() + " " +
                                  args.otherOptions().join(' '));

    m_ui.lineEditURL->setText(urls.join(' '));
  }

  m_ui.tabWidget->setCurrentIndex(0);

  m_ui.pbCancel->setEnabled(true);

  m_ui.pbResetBD->setEnabled(false);

  const auto &ep = m_ctx.Engines().engineDirPaths();

  m_tabManager.dumpCookie();

  auto opts = utility::updateOptions(
      {engine, ep, m_settings, args, QString(), false, urls});

  this->run(engine, opts, args.quality(), false);
}

void basicdownloader::run(const engines::engine &engine,
                          const QStringList &args, const QString &quality,
                          bool list_requested) {
  auto functions = utility::OptionsFunctions(
      [this](const QByteArray &args) { this->listRequested(args); },
      [](const basicdownloader::opts &opts) {
        opts.ctx.TabManager().disableAll();
      },
      [this](utility::ProcessExitState m, const basicdownloader::opts &opts) {
        opts.ctx.TabManager().enableAll();

        m_ctx.logger().add(tr("Done"));

        m_ui.pbCancel->setEnabled(false);

        m_ui.pbResetBD->setEnabled(true);

        if (!opts.listRequested) {

          auto a = downloadManager::finishedStatus(0, true, std::move(m));

          auto &s = opts.ctx.Settings();

          utility::updateFinishedState(opts.engine, s, opts.table,
                                       std::move(a));
        }
      });

  basicdownloader::opts opts{engine,  m_bogusTable,   m_ctx,
                             m_debug, list_requested, -1};

  auto oopts =
      basicdownloader::make_options(std::move(opts), std::move(functions));
  auto logger = LoggerWrapper(m_ctx.logger(), utility::concurrentID());
  auto term = m_terminator.setUp(m_ui.pbCancel, &QPushButton::clicked, -1);

  auto ctx = utility::make_ctx(
      engine, std::move(oopts), std::move(logger), std::move(term), [&]() {
        if (list_requested) {

          return utility::ProcessOutputChannels(
              QProcess::ProcessChannel::StandardOutput);
        } else {
          return utility::ProcessOutputChannels();
        }
      }());

  utility::run(args, quality, std::move(ctx));
}

void basicdownloader::tabEntered() { m_ctx.logger().updateView(true); }

void basicdownloader::tabExited() { m_ctx.logger().updateView(false); }

void basicdownloader::enableAll() {
  m_ui.pbOptionsHistory->setEnabled(true);
  m_ui.pbPasteClipboard->setEnabled(true);
  m_ui.pbEntries->setEnabled(true);
  m_ui.label_2->setEnabled(true);
  m_ui.label->setEnabled(true);
  m_ui.pbList->setEnabled(true);
  m_ui.pbDownload->setEnabled(true);
  m_ui.lineEditURL->setEnabled(true);
  m_ui.lineEditOptions->setEnabled(true);
}

void basicdownloader::disableAll() {
  m_ui.pbOptionsHistory->setEnabled(false);
  m_ui.pbPasteClipboard->setEnabled(false);
  m_ui.pbEntries->setEnabled(false);
  m_ui.label_2->setEnabled(false);
  m_ui.label->setEnabled(false);
  m_ui.pbList->setEnabled(false);
  m_ui.pbDownload->setEnabled(false);
  m_ui.lineEditURL->setEnabled(false);
  m_ui.lineEditOptions->setEnabled(false);
}

void basicdownloader::appQuit() {
  m_settings.setTabNumber(m_ui.tabWidget->currentIndex());

  QCoreApplication::quit();
}

void basicdownloader::gotEvent(const QByteArray &) {}
