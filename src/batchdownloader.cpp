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

#include "batchdownloader.h"
#include "tabmanager.h"

#include <QClipboard>
#include <QFileDialog>
#include <QMetaObject>
#include <mainwindow.h>

batchdownloader::batchdownloader(const Context &ctx)
    : m_ctx(ctx), m_settings(m_ctx.Settings()), m_ui(m_ctx.Ui()),
      m_mainWindow(m_ctx.mainWidget()), m_tabManager(m_ctx.TabManager()),
      m_showThumbnails(m_settings.showThumbnails()),
      m_table(*m_ui.tableWidgetBD, m_ctx.mainWidget().font(), 1),
      m_tableWidgetBDList(*m_ui.TableWidgetBatchDownloaderList,
                          m_ctx.mainWidget().font()),
      m_debug(ctx.debug()),
      m_defaultVideoThumbnail(
          m_settings.defaultVideoThumbnailIcon(settings::tabName::batch)),
      m_ccmd(m_ctx, *m_ui.pbBDCancel, m_settings) {
  qRegisterMetaType<ItemEntry>();

  m_tableWidgetBDList.setTableWidget([]() {
    tableWidget::tableWidgetOptions opts;

    opts.customContextPolicy = Qt::NoContextMenu;
    opts.selectionMode = QAbstractItemView::ExtendedSelection;

    return opts;
  }());

  m_table.connect(&QTableWidget::currentItemChanged,
                  [this](QTableWidgetItem *c, QTableWidgetItem *p) {
                    m_table.selectRow(c, p, m_table.startPosition());
                  });

  this->setThumbnailColumnSize(m_showThumbnails);

  m_ui.pbBDDownload->setEnabled(false);

  m_ui.pbBDCancel->setEnabled(false);

  m_ui.BDFrame->hide();

  this->resetMenu();

  m_tableWidgetBDList.connect(&QTableWidget::itemClicked,
                              [this](QTableWidgetItem *item) {
                                if (item) {

                                  m_tableWidgetBDList.selectMediaOptions(
                                      m_optionsList, *item, m_lineEdit);
                                }
                              });

  m_tableWidgetBDList.connect(
      &QTableWidget::itemDoubleClicked, [this](QTableWidgetItem *item) {
        if (item) {

          auto m = m_tableWidgetBDList.item(item->row(), 0).text();

          if (!m.isEmpty()) {

            auto u = tableWidget::type::DownloadOptions;

            m_table.setDownloadingOptions(u, m_table.currentRow(), m);
          }

          m_ui.BDFrame->hide();
        }
      });

  connect(m_ui.pbBatchDownloaderSet, &QPushButton::clicked, [this]() {
    auto m = m_lineEdit.text();

    if (!m.isEmpty()) {

      auto u = tableWidget::type::DownloadOptions;

      m_table.setDownloadingOptions(u, m_table.currentRow(), m);
    }

    m_ui.BDFrame->hide();
  });

  connect(m_ui.pbCancelBatchDownloder, &QPushButton::clicked,
          [this]() { m_ui.BDFrame->hide(); });

  connect(m_ui.pbBDOptionsHistory, &QPushButton::clicked, [this]() {
    auto s = utility::showHistory(
        *m_ui.lineEditBDUrlOptions,
        m_settings.getOptionsHistory(settings::tabName::batch), m_settings,
        settings::tabName::batch);

    if (s) {

      this->download(this->defaultEngine());
    }
  });

  connect(m_ui.pbBDDownload, &QPushButton::clicked,
          [this]() { this->download(this->defaultEngine()); });

  connect(m_ui.pbBDCancel, &QPushButton::clicked,
          [this]() { m_terminator.terminateAll(m_table.get()); });

  connect(m_ui.pbBDPasteClipboard, &QPushButton::clicked, [this]() {
    auto m = utility::clipboardText();

    if (!m.isEmpty()) {

      m_ui.lineEditBDUrl->setText(m);
    }
  });

  m_table.connect(&QTableWidget::cellDoubleClicked,
                  [this](int row, int column) {
                    Q_UNUSED(column)

                    const auto &engine = utility::resolveEngine(
                        m_table, this->defaultEngine(), m_ctx.Engines(), row);

                    m_ctx.Engines().openUrls(m_table, row, engine);
                  });

  // auto s = static_cast< void( QComboBox::* )( int ) >( &QComboBox::activated
  // ) ;

  m_table.connect(&QTableWidget::customContextMenuRequested, [this](QPoint) {
    auto row = m_table.currentRow();

    auto function = [this](const utility::contextState &c) {
      if (c.showLogWindow()) {

        m_ctx.logger().showLogWindow();

      } else if (c.clear()) {

        m_table.clear();
      }
    };

    QMenu m;

    if (row == -1) {

      this->loadListFromLastSession(m);
      this->getListFromFile(m);

      return utility::appendContextMenu(m, m_table.noneAreRunning(), function);
    }

    auto txt = m_table.runningState(row);

    auto running = downloadManager::finishedStatus::running(txt);
    auto finishSuccess =
        downloadManager::finishedStatus::finishedWithSuccess(txt);

    auto ac = m.addAction(tr("Open"));

    ac->setEnabled(finishSuccess);

    connect(ac, &QAction::triggered, [this, row]() {
      const auto &engine = utility::resolveEngine(
          m_table, this->defaultEngine(), m_ctx.Engines(), row);

      m_ctx.Engines().openUrls(m_table, row, engine);
    });

    ac = m.addAction(tr("Cancel Download"));

    ac->setEnabled(running);

    connect(ac, &QAction::triggered,
            [this, row]() { m_terminator.terminate(row); });

    ac = m.addAction(tr("Copy Url"));

    connect(ac, &QAction::triggered, [this, row]() {
      auto m = QApplication::clipboard();

      if (m) {

        m->setText(m_table.url(row));
      }
    });

    ac = m.addAction(tr("Remove"));

    ac->setEnabled(m_table.noneAreRunning());

    connect(ac, &QAction::triggered, [this, row]() {
      m_table.removeRow(row);

      m_ui.pbBDDownload->setEnabled(m_table.rowCount());
    });

    const auto &engine = this->defaultEngine();

    ac = m.addAction(tr("List available formats"));
    ac->setEnabled(!running && engine.likeYoutubeDl());

    connect(ac, &QAction::triggered, [this]() { this->showList(); });

    utility::addDownloadContextMenu(
        running, finishSuccess, m, row,
        [this](int row) {
          auto m = m_table.uiText(row);

          return m.endsWith("\n" + engines::engine::mediaAlreadInArchiveText());
        },
        [this, &engine](QAction *ac, bool forceDownload, int row) {
          connect(
              ac, &QAction::triggered, [&engine, this, row, forceDownload]() {
                downloadManager::index indexes(m_table);

                auto e = m_table.runningState(row);

                if (!downloadManager::finishedStatus::finishedWithSuccess(e) ||
                    forceDownload) {

                  auto u = m_table.downloadingOptions(row);

                  if (u.isEmpty()) {

                    indexes.add(row, m_ui.lineEditBDUrlOptions->text(),
                                forceDownload);
                  } else {
                    indexes.add(row, u, forceDownload);
                  }
                }

                this->download(engine, std::move(indexes));
              });
        });

    // START HIDE ENGINE SELECTOR MENU SINCE WE ONLY HAVE YT_DLP

    //		auto mm = m.addMenu( tableWidget::engineName().replace( ":","" )
    //)
    //;

    //		mm->setEnabled( !finishSuccess ) ;

    //		for( const auto& it : m_ctx.Engines().getEngines() ){

    //			if( it.mainEngine() ){

    //				const auto& e = it.name() ;

    //				mm->addAction( e )->setObjectName( e ) ;
    //			}
    //		}

    //		connect( mm,&QMenu::triggered,[ this ]( QAction * ac ){

    //			auto u = tableWidget::type::EngineName ;

    //			m_table.setDownloadingOptions(
    // u,m_table.currentRow(),ac->objectName() ) ; 		} ) ;

    // END HIDE ENGINE SELECTOR MENU SINCE WE ONLY HAVE YT_DLP

    auto subMenu = utility::setUpMenu(m_ctx, {}, false, false, true, &m);

    subMenu->setTitle(QObject::tr("Format Presets"));

    subMenu->setEnabled(!finishSuccess);

    connect(subMenu, &QMenu::triggered, [this, row](QAction *ac) {
      auto m = util::split(ac->objectName(), '\n', true);

      auto u = tableWidget::type::DownloadOptions;

      if (m.size() > 1) {

        m_table.setDownloadingOptions(u, row, m[0], m[1]);
      } else {
        m_table.setDownloadingOptions(u, row, m[0]);
      }
    });

    m.addMenu(subMenu);

    m.addSeparator();

    utility::saveDownloadList(m_ctx, m, m_table);

    this->loadListFromLastSession(m);
    this->getListFromFile(m);

    utility::appendContextMenu(m, {m_table.noneAreRunning(), finishSuccess},
                               function);
  });

  connect(m_ui.pbBDCancel, &QPushButton::clicked,
          [this]() { m_ccmd.cancelled(); });

  connect(m_ui.pbBDAdd, &QPushButton::clicked, [this]() {
    auto m = m_ui.lineEditBDUrl->text();

    if (!m.isEmpty()) {

      m_ui.lineEditBDUrl->clear();

      this->addToList(m);
    }
  });
}

batchdownloader::~batchdownloader() { this->saveBatchDownloadList(m_table); }

void batchdownloader::init_done() {}

void batchdownloader::resetMenu() {
  utility::setMenuOptions(
      m_ctx, {}, false, false, m_ui.pbBDOptions, [this](QAction *aa) {
        utility::selectedAction ac(aa);

        if (ac.clearOptions()) {

          m_ui.lineEditBDUrlOptions->clear();

        } else if (ac.clearScreen()) {

          this->clearScreen();

        } else if (ac.openFolderPath()) {

          utility::openDownloadFolderPath(m_settings.downloadFolder());
        } else {
          m_ui.lineEditBDUrlOptions->setText(ac.objectName());

          this->download(this->defaultEngine());
        }
      });
}

void batchdownloader::retranslateUi() { this->resetMenu(); }

void batchdownloader::tabEntered() {
  auto accountManager = m_tabManager.AccountManager();
  if (accountManager.evaluationUsed()) {
    this->disableAll();
    accountManager.showPurchaseMessage();
  }
}

void batchdownloader::tabExited() {}

void batchdownloader::gotEvent(const QByteArray &m) {
  QJsonParseError err;
  auto jsonDoc = QJsonDocument::fromJson(m, &err);

  if (err.error == QJsonParseError::NoError) {

    QJsonObject jsonArgs = jsonDoc.object();

    auto url = jsonArgs.value("-u").toString();

    if (!url.isEmpty()) {

      m_ui.tabWidget->setCurrentIndex(1);

      auto autoDownload = jsonArgs.value("-a").toBool(false);
      auto showThumbnail = jsonArgs.value("-s").toBool(false);

      this->addToList(url, autoDownload, showThumbnail);
    }
  }
}

// void batchdownloader::updateEnginesList( const QStringList& e )
//{
//	auto& comboBox = *m_ui.cbEngineTypeBD ;

//	comboBox.clear() ;

//	for( const auto& it : e ){

//		comboBox.addItem( it ) ;
//	}

//	auto s = settings::tabName::batch ;

//	utility::setUpdefaultEngine( comboBox,
//				     this->defaultEngineName(),
//				     [ this,s ]( const QString& e ){
// m_settings.setDefaultEngine( e,s ) ; } ) ;
//}

void batchdownloader::showThumbnail(const engines::engine &engine, Items list,
                                    bool autoDownload, bool showThumbnails) {
  if (list.isEmpty()) {

    return;
  }

  if (autoDownload && !showThumbnails && list.hasOneEntry()) {

    const auto &s = list.first();

    auto state = downloadManager::finishedStatus::running();

    int row =
        m_table.addItem({m_defaultVideoThumbnail, s.uiText, s.url, state});

    m_table.selectLast();

    downloadManager::index index(m_table);

    index.add(row, m_ui.lineEditBDUrlOptions->text());

    this->download(engine, std::move(index));

  } else if (m_showThumbnails && engine.likeYoutubeDl()) {

    for (const auto &it : list) {

      downloadManager::index indexes(m_table);

      auto state = downloadManager::finishedStatus::running();

      auto uiText = it.uiText;

      int row = m_table.addItem(
          {m_defaultVideoThumbnail, "...\n" + it.uiText, it.url, state});

      util::Timer(1000, [this, row, uiText](int counter) {
        if (downloadManager::finishedStatus::running(
                m_table.runningState(row))) {

          QString m = "...";

          int max = counter % 4;

          for (int s = 0; s < max; s++) {

            m += " ...";
          }

          m_table.setUiText(m + "\n" + uiText, row);

          return false;
        } else {
          return true;
        }
      });

      m_table.selectLast();

      indexes.add(row, m_ui.lineEditBDUrlOptions->text());

      auto url = it.url;

      m_ccmd.download(
          std::move(indexes), engine,
          [this]() { return m_settings.maxConcurrentDownloads(); }(),
          [this, url, autoDownload](const engines::engine &engine, int index) {
            this->showThumbnail(engine, index, url, autoDownload);
          });
    }
  } else {
    this->addItemUiSlot({engine, std::move(list)});
  }
}

void batchdownloader::addItemUiSlot(ItemEntry m) {
  if (m.hasNext()) {

    auto s = m.next();

    this->addItemUi(m_defaultVideoThumbnail, -1, false, {s.uiText, s.url});

    QMetaObject::invokeMethod(this, "addItemUiSlot", Qt::QueuedConnection,
                              Q_ARG(ItemEntry, m));
  }
}

void batchdownloader::saveBatchDownloadList(tableWidget &t_tableWidget) {
  QJsonArray arr;
  t_tableWidget.forEach([&](const tableWidget::entry &e) {
    using df = downloadManager::finishedStatus;
    auto m = df::finishedWithSuccess(e.runningState);
    if (e.url.isEmpty() || m) {
      return;
    }
    arr.append([&]() {
      QJsonObject obj;
      obj.insert("url", e.url);
      obj.insert("uiText", e.uiText);
      return obj;
    }());
  });
  auto stuff = QJsonDocument(arr);
  Util::saveJson(
      stuff, QStandardPaths::writableLocation(QStandardPaths::DataLocation) +
                 QDir::separator() + "BatchDownloadsState.json");
}

void batchdownloader::loadListFromLastSession(QMenu &m) {
  auto ac_load = m.addAction(QObject::tr("Load List from Last Session"));
  QObject::connect(ac_load, &QAction::triggered, this, [this]() {
    this->processSavedBtachDownloadFile(
        QStandardPaths::writableLocation(QStandardPaths::DataLocation) +
        QDir::separator() + "BatchDownloadsState.json");
  });
}

void batchdownloader::getListFromFile(QMenu &m) {

  auto ac = m.addAction(QObject::tr("Get List From File"));

  QObject::connect(ac, &QAction::triggered, this, [this]() {
    auto e = QFileDialog::getOpenFileName(
        &m_ctx.mainWindow(), tr("Select Batch File"), utility::homePath(),
        tr("JSON/Text (*.json *.txt)"));

    if (!e.isEmpty()) {
      processSavedBtachDownloadFile(e);
    }
  });
}

void batchdownloader::processSavedBtachDownloadFile(const QString &filePath) {
  auto list = engines::file(filePath, m_ctx.logger()).readAll();

  if (list.isEmpty()) {
    return;
  }

  if (list.startsWith('[')) {
    QJsonParseError err;
    auto json = QJsonDocument::fromJson(list, &err);
    if (err.error == QJsonParseError::NoError) {
      const auto array = json.array();
      Items items;
      for (const auto &it : array) {
        auto obj = it.toObject();
        auto url = obj.value("url").toString();
        auto uiText = obj.value("uiText").toString();
        items.add(uiText, url);
      }
      const auto &engine = this->defaultEngine();
      return this->showThumbnail(engine, std::move(items));
    }
  }

  Items items;
  const auto localSplit = util::split(list, '\n', true);
  for (const auto &it : localSplit) {
    items.add(it);
  }

  const auto &engine = this->defaultEngine();
  this->showThumbnail(engine, std::move(items));
}

QString batchdownloader::defaultEngineName() {
  return m_settings.defaultEngine(settings::tabName::batch,
                                  m_ctx.Engines().defaultEngineName());
}

const engines::engine &batchdownloader::defaultEngine() {
  return m_ctx.Engines().defaultEngine(this->defaultEngineName());
}

void batchdownloader::showThumbnail(const engines::engine &engine, int index,
                                    const QString &url, bool autoDownload) {
  auto aa = [&engine, index, this, url,
             autoDownload](utility::ProcessExitState e,
                           const batchdownloader::opts &opts) {
    auto aa = [this, autoDownload](const engines::engine &engine, int index) {
      this->showThumbnail(engine, index, {}, autoDownload);
    };

    auto bb = [this, &engine, &opts, url,
               autoDownload](const downloadManager::finishedStatus &f) {
      auto allFinished = f.allFinished();

      if (f.exitState().cancelled()) {

        this->addItem(f.index(), allFinished, url);
      } else {
        utility::MediaEntry m(url, opts.batchLogger.data());

        if (m.valid()) {

          this->addItem(f.index(), allFinished, std::move(m));
        } else {
          this->addItem(f.index(), allFinished, url);
        }

        if (allFinished && autoDownload) {

          this->download(engine);
        }
      }
    };

    m_ccmd.monitorForFinished(engine, index, std::move(e), std::move(aa),
                              std::move(bb));
  };

  auto functions = utility::OptionsFunctions(
      [this](const batchdownloader::opts &) {
        m_ui.pbBDPasteClipboard->setEnabled(true);

        m_ui.pbBDAdd->setEnabled(true);

        m_ui.lineEditBDUrl->setEnabled(true);
      },
      std::move(aa));

  //  auto m = m_ui.lineEditBDUrlOptions->text();

  BatchLoggerWrapper wrapper(m_ctx.logger());

  auto dumpjsonArgs = engine.dumpJsonArguments();

  m_tabManager.dumpCookie();

  const QString cookiePath = m_settings.cookieFilePath(engine.name());
  const QString ca = engine.cookieArgument();

  if (!cookiePath.isEmpty() && !ca.isEmpty()) {
    dumpjsonArgs.append(ca);
    dumpjsonArgs.append(cookiePath);
  }

  m_ccmd.download(
      engine, dumpjsonArgs, index == -1 ? url : m_table.url(index),
      m_terminator.setUp(m_ui.pbBDCancel, &QPushButton::clicked, index),
      batchdownloader::make_options({m_ctx, m_debug, false, index, wrapper},
                                    std::move(functions)),
      wrapper, QProcess::ProcessChannel::StandardOutput);
}

void batchdownloader::setThumbnailColumnSize(bool e) {
  m_showThumbnails = e;

  if (m_showThumbnails) {
    m_table.get().setColumnWidth(
        0, m_settings.thumbnailWidth(settings::tabName::batch));
  } else {
    auto w = m_settings.thumbnailWidth(settings::tabName::batch);
    auto h = m_settings.thumbnailHeight(settings::tabName::batch);

    auto pixmap = QIcon(":/icons/video.png").pixmap(w, h);

    m_table.get().setColumnWidth(0, pixmap.width());
  }
}

void batchdownloader::clearScreen() {
  m_table.clear();
  m_ui.lineEditBDUrlOptions->clear();
  m_ui.lineEditBDUrl->clear();
}

void batchdownloader::showList() {
  auto &table = m_table.get();

  auto row = table.currentRow();

  if (row == -1) {

    return;
  }

  const auto &engine = utility::resolveEngine(m_table, this->defaultEngine(),
                                              m_ctx.Engines(), row);

  auto args = engine.defaultListCmdOptions();

  if (args.isEmpty()) {

    return;
  }

  m_tabManager.dumpCookie();

  auto cookiePath = m_settings.cookieFilePath(engine.name());
  const auto &ca = engine.cookieArgument();

  if (!cookiePath.isEmpty() && !ca.isEmpty()) {

    args.append(ca);
    args.append(cookiePath);
  }

  m_optionsList.clear();

  m_tableWidgetBDList.clear();

  m_ui.BDFrame->show();

  args.append(m_table.url(row));

  m_ctx.TabManager().disableAll();

  auto functions = utility::OptionsFunctions(
      [this, &engine](const QByteArray &a) {
        if (!a.isEmpty()) {

          for (const auto &m : engine.mediaProperties(a)) {

            m_tableWidgetBDList.add(m);
          }
        }
      },
      [this](const batchdownloader::opts &opts) {
        opts.ctx.TabManager().disableAll();

        m_ui.pbCancelBatchDownloder->setEnabled(true);
      },
      [](utility::ProcessExitState, const batchdownloader::opts &opts) {
        opts.ctx.TabManager().enableAll();
      });

  batchdownloader::opts opts{m_ctx, m_debug, true, -1,
                             BatchLoggerWrapper(m_ctx.logger())};

  auto oopts =
      batchdownloader::make_options(std::move(opts), std::move(functions));
  auto logger = LoggerWrapper(m_ctx.logger(), utility::concurrentID());
  auto term = m_terminator.setUp(m_ui.pbCancelBatchDownloder,
                                 &QPushButton::clicked, -1);
  auto ch = QProcess::ProcessChannel::StandardOutput;

  auto ctx = utility::make_ctx(engine, std::move(oopts), std::move(logger),
                               std::move(term), ch);

  utility::run(args, QString(), std::move(ctx));
}

static int _addItemUi(const QPixmap &pixmap, int index, tableWidget &table,
                      Ui::MainWindow &ui, const utility::MediaEntry &media) {
  auto state = downloadManager::finishedStatus::notStarted();

  int row;
  if (index == -1) {

    row = table.addItem({pixmap, media.uiText(), media.url(), state});
    table.selectLast();
  } else {
    row = index;
    table.replace({pixmap, media.uiText(), media.url(), state}, index);
  }

  ui.lineEditBDUrl->clear();

  ui.lineEditBDUrl->setFocus();

  return row;
}

void batchdownloader::addItemUi(const QPixmap &pixmap, int index,
                                bool enableAll,
                                const utility::MediaEntry &media) {
  auto row = _addItemUi(pixmap, index, m_table, m_ui, media);

  m_ctx.TabManager().Configure().setDownloadOptions(row, m_table);

  m_ui.pbBDDownload->setEnabled(true);

  if (enableAll) {

    m_ctx.TabManager().enableAll();
    m_ui.pbBDCancel->setEnabled(false);
  }
}

void batchdownloader::addItemUi(int index, bool enableAll,
                                const utility::MediaEntry &media) {
  this->addItemUi(m_defaultVideoThumbnail, index, enableAll, media);
}

void batchdownloader::addItem(int index, bool enableAll,
                              const utility::MediaEntry &media) {
  if (media.thumbnailUrl().isEmpty()) {

    this->addItemUi(index, enableAll, media);
  } else {
    if (networkAccess::hasNetworkSupport()) {

      auto &network = m_ctx.versionInfo().network();

      m_networkRunning++;

      const auto &u = media.thumbnailUrl();

      network.getResource(u, [this, media, index](const QByteArray &data) {
        QPixmap pixmap;

        if (pixmap.loadFromData(data)) {

          auto w = m_settings.thumbnailWidth(settings::tabName::batch);
          auto h = m_settings.thumbnailHeight(settings::tabName::batch);

          pixmap = pixmap.scaled(w, h, Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);

        } else {
          pixmap = m_defaultVideoThumbnail;
        }

        _addItemUi(pixmap, index, m_table, m_ui, media);

        m_ctx.TabManager().Configure().setDownloadOptions(index, m_table);

        if (m_table.noneAreRunning()) {

          m_ui.pbBDDownload->setEnabled(true);

          m_ctx.TabManager().enableAll();

          m_ui.pbBDCancel->setEnabled(false);
        }

        m_networkRunning--;
      });
    } else {
      this->addItemUi(index, enableAll, media);
    }
  }
}

void batchdownloader::addToList(const QString &url, bool autoDownload,
                                bool showThumbnails) {
  this->showThumbnail(this->defaultEngine(), url, autoDownload, showThumbnails);
}

void batchdownloader::download(const engines::engine &engine,
                               downloadManager::index indexes) {
  if (indexes.empty()) {

    return;
  }

  m_settings.setLastUsedOption("core", m_ui.lineEditBDUrlOptions->text(),
                               settings::tabName::batch);

  m_ctx.TabManager().basicDownloader().hideTableList();

  m_ctx.mainWindow().setTitle(QString());

  m_ccmd.download(
      std::move(indexes), engine,
      [this]() { return m_settings.maxConcurrentDownloads(); }(),
      [this](const engines::engine &engine, int index) {
        this->download(engine, index);
      });
}

void batchdownloader::download(const engines::engine &engine) {
  downloadManager::index indexes(m_table);

  for (int s = 0; s < m_table.rowCount(); s++) {

    auto e = m_table.runningState(s);

    if (!downloadManager::finishedStatus::finishedWithSuccess(e)) {

      auto u = m_table.downloadingOptions(s);

      if (u.isEmpty()) {

        indexes.add(s, m_ui.lineEditBDUrlOptions->text());
      } else {
        indexes.add(s, u);
      }
    }
  }

  this->download(engine, std::move(indexes));
}

void batchdownloader::download(const engines::engine &eng, int index) {
  const auto &engine =
      utility::resolveEngine(m_table, eng, m_ctx.Engines(), index);

  auto aa = [&engine, index, this](utility::ProcessExitState e,
                                   const batchdownloader::opts &) {
    auto aa = [this](const engines::engine &engine, int index) {
      this->download(engine, index);
    };

    auto bb = [&engine, index, this](const downloadManager::finishedStatus &f) {
      utility::updateFinishedState(engine, m_settings, m_table, f);

      if (m_table.noneAreRunning()) {

        m_ctx.TabManager().enableAll();

        m_ui.pbBDCancel->setEnabled(false);
      }

      m_ctx.mainWindow().setTitle(m_table.completeProgress(index));
    };

    m_ccmd.monitorForFinished(engine, index, std::move(e), std::move(aa),
                              std::move(bb));
  };

  auto functions = utility::OptionsFunctions(
      [](const batchdownloader::opts &) {}, std::move(aa));

  auto m = m_ui.lineEditBDUrlOptions->text();

  m_settings.addOptionsHistory(m, settings::tabName::batch);

  batchdownloader::opts opts{m_ctx, m_debug, false, index,
                             BatchLoggerWrapper(m_ctx.logger())};

  auto oopts =
      batchdownloader::make_options(std::move(opts), std::move(functions));

  auto updater = [this, index](const QByteArray &e) {
    m_table.setUiText(e, index);
  };

  auto error = [](const QByteArray &) {};

  auto logger = make_loggerBatchDownloader(
      engine.filter(utility::args(m).quality()), m_ctx.logger(),
      std::move(updater), std::move(error), utility::concurrentID());

  m_table.setRunningState(downloadManager::finishedStatus::running(), index);

  m_ccmd.download(
      engine, [](QStringList opts) { return opts; },
      m_ctx.Engines().engineDirPaths(), m_table.url(index),
      m_terminator.setUp(), std::move(oopts), std::move(logger));
}

void batchdownloader::enableAll() {
  m_table.setEnabled(true);
  m_ui.pbBDPasteClipboard->setEnabled(true);
  m_ui.pbBDDownload->setEnabled(m_table.rowCount());
  m_ui.pbBDAdd->setEnabled(true);
  m_ui.pbBDOptions->setEnabled(true);
  m_ui.labelBDEnterOptions->setEnabled(true);
  m_ui.labelBDEnterUrl->setEnabled(true);
  m_ui.lineEditBDUrl->setEnabled(true);
  m_ui.lineEditBDUrlOptions->setEnabled(true);
  m_ui.pbBatchDownloaderSet->setEnabled(true);
  m_ui.pbCancelBatchDownloder->setEnabled(true);
  m_ui.TableWidgetBatchDownloaderList->setEnabled(true);
  m_ui.pbBDOptionsHistory->setEnabled(true);
}

void batchdownloader::disableAll() {
  m_table.setEnabled(false);
  m_ui.pbBDOptionsHistory->setEnabled(false);
  m_ui.pbBDPasteClipboard->setEnabled(false);

  m_ui.pbBDDownload->setEnabled(false);
  m_ui.pbBDAdd->setEnabled(false);
  m_ui.pbBDOptions->setEnabled(false);
  m_ui.labelBDEnterOptions->setEnabled(false);
  m_ui.labelBDEnterUrl->setEnabled(false);
  m_ui.lineEditBDUrl->setEnabled(false);
  m_ui.lineEditBDUrlOptions->setEnabled(false);
  m_ui.pbBatchDownloaderSet->setEnabled(false);
  m_ui.pbCancelBatchDownloder->setEnabled(false);
  m_ui.TableWidgetBatchDownloaderList->setEnabled(false);
}
