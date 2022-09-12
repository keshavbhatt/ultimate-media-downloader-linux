#include "ytsearch.h"

#include "tabmanager.h"
#include <QMessageBox>
#include <QScreen>
#include <services/xp/xpsearchservice.h>

// NOTE: ignore clazy warning about Qt's unique connection in lambda functor
//  unique connection are established since we are passing this object as
//  context

YtSearch::YtSearch(const Context &ctx)
    : m_ctx(ctx), m_settings(m_ctx.Settings()), m_ui(m_ctx.Ui()),
      m_mainWindow(m_ctx.mainWidget()), m_tabManager(m_ctx.TabManager()),
      searchServiceRetries(0),
      m_engine_path(m_ctx.Engines().defaultEngine("core").exePath().realExe()) {

  const QIcon youtubeIcon(":/icons/search.png");
  m_ui.ytSearchLineEdit->addAction(youtubeIcon, QLineEdit::LeadingPosition);

#ifndef QT_DEBUG
  m_ui.ytSearchInstanceWidget->hide();
#endif

  this->disableAll();

  m_ui.ytSearchButton->setEnabled(m_ui.ytSearchLineEdit->text().isEmpty() ==
                                  false);

  m_ui.ytSearchSwitchInstance->setEnabled(false);

  auto placeHolder = m_ui.ytSearchSearchTypeComboBox->currentIndex() == 0
                         ? "Search YouTube Videos"
                         : "Search YouTube Playlists";
  m_ui.ytSearchLineEdit->setPlaceholderText(placeHolder);

  m_accessManager = new QNetworkAccessManager(this);
  QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
  diskCache->setCacheDirectory(
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
  m_accessManager->setCache(diskCache);

  invidiousInstances = new InvidiousInstances(this, m_accessManager);

  // init tredingWidget
  trendingWidget = new TrendingWidget(m_ui.ytSearchResultsListWidget);
  trendingWidget->setWindowFlags(Qt::Widget);
  trendingWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  trendingWidget->hide();

  connect(trendingWidget, &TrendingWidget::loadRequested, this,
          &YtSearch::getTrending, Qt::UniqueConnection);

  connect(
      invidiousInstances, &InvidiousInstances::ready, this,
      [=]() {
        enableAll();
        m_ui.ytSearchInstancesCombo->blockSignals(true);
        m_ui.ytSearchInstancesCombo->clear();
        m_ui.ytSearchInstancesCombo->blockSignals(false);
        m_ui.ytSearchInstancesCombo->addItems(
            invidiousInstances->getInstances());

        m_ui.ytSearchGetInstances->setEnabled(true);

        initSearchService();
      },
      Qt::UniqueConnection);

  connect(
      m_ui.ytSearchSwitchInstance, &QPushButton::clicked, this,
      [=]() {
        QString instanceBaseUrl = invidiousInstances->getRandomInstance();
        m_ui.ytSearchInstancesCombo->setCurrentText(instanceBaseUrl);
        // reload current request
        if (m_searchService != nullptr) {
          cancelAllsearchServiceRequests();
          qDebug() << "Retrying with random server";
          m_searchService->setInstanceBaseUrl(instanceBaseUrl);
          m_searchService->get();
          setSearching(true, false);
        }
      },
      Qt::UniqueConnection);

  connect(
      m_ui.ytSearchGetInstances, &QPushButton::clicked, this,
      [=]() {
        m_ui.ytSearchGetInstances->setEnabled(false);
        invidiousInstances->reload();
      },
      Qt::UniqueConnection);

  connect(
      m_ui.ytSearchInstancesCombo,
      QOverload<int>::of(&QComboBox::currentIndexChanged), this,
      [=](int index) { invidiousInstances->setCurrentInstance(index); },
      Qt::UniqueConnection);

  connect(
      m_ui.ytSearchLineEdit, &QLineEdit::textChanged, this,
      [=](const QString &str) {
        m_ui.ytSearchButton->setEnabled(str.isEmpty() == false);
      },
      Qt::UniqueConnection);

  connect(
      m_ui.ytSearchLineEdit, &QLineEdit::returnPressed, this,
      [=]() {
        if (m_ui.ytSearchButton->isEnabled()) {
          m_ui.ytSearchButton->click();
        };
      },
      Qt::UniqueConnection);

  connect(
      m_ui.ytSearchLoadMore, &QPushButton::clicked, this,
      [=]() {
        setSearching(true);
        m_searchService->loadMore();
      },
      Qt::UniqueConnection);

  connect(
      m_ui.ytSearchButton, &QPushButton::clicked, this,
      [=]() {
        if (this->searching) {
          // cancel search
          cancelAllsearchServiceRequests();
          setSearching(false);
        } else {
          // start search
          this->search(m_ui.ytSearchLineEdit->text());
        }
      },
      Qt::UniqueConnection);

  connect(
      m_ui.ytSearchSearchTypeComboBox,
      QOverload<int>::of(&QComboBox::currentIndexChanged), this,
      [=](int index) {
        auto placeHolder =
            index == 0 ? "Search YouTube Videos" : "Search YouTube Playlists";
        m_ui.ytSearchLineEdit->setPlaceholderText(placeHolder);
      },
      Qt::UniqueConnection);

  connect(
      m_ui.ytSearchClear, &QPushButton::clicked, this,
      [=]() {
        cancelAllsearchServiceRequests();
        setSearching(false);
        m_ui.ytSearchLineEdit->clear();
        m_ui.ytSearchResultsListWidget->clear();
        m_ui.ytSearchLoadMore->setEnabled(false);
        m_ui.ytSearchClear->setEnabled(m_ui.ytSearchResultsListWidget->count() >
                                       0);
      },
      Qt::UniqueConnection);
}

void YtSearch::searchServiceRetry() {
  this->searchServiceRetries = this->searchServiceRetries + 1;
  // 2 retries max (0,1)
  if (searchServiceRetries <= 1) {
    m_ui.ytSearchSwitchInstance->click(); // retry req using random instance
  } else {
    // reset retries
    searchServiceRetries = 0;
    // switch back to healtiest server
    m_ui.ytSearchInstancesCombo->setCurrentText(
        invidiousInstances->getFirstInstance());
  }
}

void YtSearch::initSearchService() {
  m_searchService = new YtSearchService(this, m_accessManager);
  connect(
      m_searchService, &YtSearchService::finished, this,
      [=](YtSearchService::SearchMode serchMode, const QByteArray &data) {
        switch (serchMode) {
        case YtSearchService::SearchMode::VIDEO:
          processVideoResults(data);
          break;
        case YtSearchService::SearchMode::PLAYLIST:
          processPlaylistResults(data);
          break;
        default: // since other cases are only trending
          processTrendingResult(data, serchMode);
          m_ui.ytSearchLoadMore->setEnabled(false);
          break;
        }
        // retry if data is empty without updating searching
        QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
        if (jsonResponse.isNull() || jsonResponse.isEmpty() || data.isEmpty() ||
            !jsonResponse.isArray() || jsonResponse.array().isEmpty()) {
          qDebug() << "RTR";
          searchServiceRetry();
        }
        setSearching(false);
      },
      Qt::UniqueConnection);

  connect(
      m_searchService, &YtSearchService::error, this,
      [=](const QString &error) {
        processError(error);
        setSearching(false);
      },
      Qt::UniqueConnection);

  connect(m_ui.ytSearchLoadTrending, &QPushButton::clicked, this,
          &YtSearch::showTrendingSelector, Qt::UniqueConnection);

  m_ui.ytSearchLoadMore->setEnabled(m_searchService->getPage() > 0);
}

void YtSearch::cancelAllsearchServiceRequests() {
  foreach (YtSearchService *srv, this->findChildren<YtSearchService *>()) {
    srv->cancel();
  }
  m_ui.ytSearchResultsListWidget->clear();
  addPsudoItemToListWidget("Request cancelled");
}

void YtSearch::search(const QString &searchTerm) {

  // if (m_xpTriggerWords.contains(searchTerm)) {
  //   int ret = QMessageBox::warning(
  //       m_ui.ytSearchTab, QApplication::applicationName() + " | " +
  //       tr("Warning"), tr("Based on the searched keyword, the result page
  //       could contain age "
  //          "restricted content!\n\nProceed showing results?"),
  //       , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

  //   if (ret == QMessageBox::No) {
  //     return;
  //   }
  // }

  if (m_xpTriggerWords.contains(searchTerm)) {
    QMessageBox msgBox(
        QMessageBox::Warning, qApp->applicationName() + " | " + tr("Warning"),
        tr("Based on the searched keyword, the result page could contain age "
           "restricted content!\n\nProceed showing results?"),
        QMessageBox::Yes | QMessageBox::No);

    QPushButton *enableXpSearchButton = msgBox.addButton(
        tr("Enable XP Search Plugin"), QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == msgBox.button(QMessageBox::No)) {
      return;
    } else if (msgBox.clickedButton() == enableXpSearchButton) {
      m_tabManager.toggleXpSearch(true);
      return;
    }
  }

  YtSearchService::SearchMode searchMode =
      m_ui.ytSearchSearchTypeComboBox->currentIndex() == 0
          ? YtSearchService::SearchMode::VIDEO
          : YtSearchService::SearchMode::PLAYLIST;

  // cancel all
  cancelAllsearchServiceRequests();

  m_searchService->init(invidiousInstances->getCurrentInstance(), searchTerm,
                        searchMode);

  setSearching(true);

  QJsonObject trackingData{{"searchTerm", searchTerm}};

  m_tabManager.trackingService()->trackEvent("ytSearchQuery", trackingData);

  m_searchService->get();
}

// inline void CenterWidget(QWidget *widget, QWidget *host = Q_NULLPTR) {
//   if (!host)
//     host = widget->parentWidget();

//  if (host) {
//    QPoint centre =
//        host->mapToGlobal(host->rect().center() - widget->rect().center());
//    widget->move(centre);
//  } else {
//    QRect screenGeometry = QGuiApplication::screens().first()->geometry();
//    int x = (screenGeometry.width() - widget->width()) / 2;
//    int y = (screenGeometry.height() - widget->height()) / 2;
//    widget->move(x, y);
//  }
//}

void YtSearch::showTrendingSelector() {
  firstLaunch = false;
  if (trendingWidget != nullptr)
    trendingWidget->resize(m_ui.ytSearchResultsListWidget->rect().size());
  auto localPalette = m_ui.centralWidget->palette();
  auto localBackground = localPalette.window();
  QColor darker = localBackground.color().darker(102);
  // bug background is showing transparent if not set
  QString style =
      "QFrame#frame{background-color:" + localBackground.color().name() + ";}" +
      "QWidget#widget{background-color:" + darker.name() + ";}";
  trendingWidget->setStyleSheet(style);
  if (trendingWidget->isVisible() == false)
    trendingWidget->show();
  else
    trendingWidget->hide();
}

void YtSearch::hideTrendingSelector() { trendingWidget->hide(); }

void YtSearch::getTrending(const YtSearchService::SearchMode &searchMode,
                           const QString &region) {

  cancelAllsearchServiceRequests();

  m_searchService->init(invidiousInstances->getCurrentInstance(), "",
                        searchMode, region);

  setSearching(true);

  QJsonObject trackingData{{"searchMode", searchMode}, {"region", region}};
  m_tabManager.trackingService()->trackEvent("ytSearchgetTrending",
                                             trackingData);

  m_searchService->get();
}

void YtSearch::setSearching(const bool &searching, const bool &resetRetires) {
  this->searching = searching;
  if (searching) {
    hideTrendingSelector();
    m_ui.ytSearchButton->setText(tr("Cancel"));
    m_ui.ytSearchButton->setEnabled(true);
    m_ui.ytSearchLineEdit->setEnabled(false);
    m_ui.ytSearchSearchTypeComboBox->setEnabled(false);
    m_ui.ytSearchLoadMore->setEnabled(false);
    m_ui.ytSearchResultsListWidget->clear();
    addPsudoItemToListWidget("Loading...");
    m_ui.ytSearchSwitchInstance->setEnabled(true);
    if (resetRetires) {
      searchServiceRetries = 0;
    }
  } else {
    m_ui.ytSearchButton->setText(tr("Search"));
    m_ui.ytSearchButton->setEnabled(m_ui.ytSearchLineEdit->text().isEmpty() ==
                                    false);
    m_ui.ytSearchLineEdit->setEnabled(true);
    m_ui.ytSearchSearchTypeComboBox->setEnabled(true);
    m_ui.ytSearchLoadMore->setEnabled(m_searchService->getPage() > 0);
  }
}

void YtSearch::addPsudoItemToListWidget(const QString &labelStr) {
  QLabel *label = new ElidedLabel(m_ui.ytSearchResultsListWidget);
  label->setText(labelStr);
  label->setToolTip(labelStr);
  label->setAlignment(Qt::AlignHCenter);
  label->setEnabled(false);
  QListWidgetItem *item = new QListWidgetItem(m_ui.ytSearchResultsListWidget);
  m_ui.ytSearchResultsListWidget->setItemWidget(item, label);
  item->setFlags(Qt::NoItemFlags);
  m_ui.ytSearchResultsListWidget->addItem(item);
}

void YtSearch::addNewVideoItemToListWidget(const QJsonObject &videoObject) {

  YtVideo *video =
      new YtVideo(m_ui.ytSearchResultsListWidget, videoObject, m_accessManager);

  QListWidgetItem *item = new QListWidgetItem(m_ui.ytSearchResultsListWidget);

  m_ui.ytSearchResultsListWidget->setItemWidget(item, video);

  item->setSizeHint(video->sizeHint());

  connect(video, &YtVideo::playAudio, this, &YtSearch::playAudio);

  connect(video, &YtVideo::playVideo, this, &YtSearch::playVideo);

  m_ui.ytSearchResultsListWidget->addItem(item);
}

void YtSearch::addNoResultReturnedForQueryToListWidget() {
  m_ui.ytSearchResultsListWidget->clear();
  auto searchTerm = m_ui.ytSearchLineEdit->text();
  QString label = QString("No results returned for \"%1\"").arg(searchTerm);
  this->addPsudoItemToListWidget(label);
}

void YtSearch::addTrendingResultInfoHeaderItemToListWidget(
    const YtSearchService::SearchMode &searchMode) {
  QString pageNumberLabel;
  switch (searchMode) {
  case YtSearchService::SearchMode::TRENDING_DEFAULT:
    pageNumberLabel = "Trending Videos";
    break;
  case YtSearchService::SearchMode::TRENDING_MUSIC:
    pageNumberLabel = "Trending Music";
    break;
  case YtSearchService::SearchMode::TRENDING_GAMES:
    pageNumberLabel = "Trending Gaming";
    break;
  case YtSearchService::SearchMode::TRENDING_MOVIES:
    pageNumberLabel = "Trending Movies";
    break;
  default:
    pageNumberLabel = "Trending Uncategorized";
    break;
  }
  addPsudoItemToListWidget(pageNumberLabel);
}

void YtSearch::addResultInfoHeaderItemToListWidget() {
  auto searchTerm = m_ui.ytSearchLineEdit->text();
  QString pageNumberLabel =
      QString("Results for \"%1\" on page %2")
          .arg(searchTerm, QString::number(m_searchService->getPage()));
  addPsudoItemToListWidget(pageNumberLabel);
}

void YtSearch::processTrendingResult(
    const QByteArray &data, const YtSearchService::SearchMode &searchMode) {
  m_ui.ytSearchResultsListWidget->clear();

  QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
  if (jsonResponse.isArray()) {

    addTrendingResultInfoHeaderItemToListWidget(searchMode);

    QJsonArray jsonArray = jsonResponse.array();
    if (jsonArray.isEmpty()) {
      addPsudoItemToListWidget("No results returned, try again!");
    } else {
      foreach (const QJsonValue &val, jsonArray) {
        addNewVideoItemToListWidget(val.toObject());
      }
    }
    m_ui.ytSearchClear->setEnabled(m_ui.ytSearchResultsListWidget->count() > 0);
  }
}

void YtSearch::processError(const QString &error) {
  m_ui.ytSearchResultsListWidget->clear();
  addPsudoItemToListWidget(error);
}

void YtSearch::processVideoResults(const QByteArray &data) {

  m_ui.ytSearchResultsListWidget->clear();

  QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
  if (jsonResponse.isArray()) {

    addResultInfoHeaderItemToListWidget();

    QJsonArray jsonArray = jsonResponse.array();
    if (jsonArray.isEmpty()) {
      addNoResultReturnedForQueryToListWidget();
    } else {
      foreach (const QJsonValue &val, jsonArray) {
        addNewVideoItemToListWidget(val.toObject());
      }
    }
    m_ui.ytSearchClear->setEnabled(m_ui.ytSearchResultsListWidget->count() > 0);
  }
}

void YtSearch::processPlaylistResults(const QByteArray &data) {
  m_ui.ytSearchResultsListWidget->clear();

  QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
  if (jsonResponse.isArray()) {

    addResultInfoHeaderItemToListWidget();

    QJsonArray jsonArray = jsonResponse.array();
    if (jsonArray.isEmpty()) {
      addNoResultReturnedForQueryToListWidget();
    } else {
      foreach (const QJsonValue &val, jsonArray) {
        addNewPlaylistItemToListWidget(val.toObject());
      }
    }
    m_ui.ytSearchClear->setEnabled(m_ui.ytSearchResultsListWidget->count() > 0);
  }
}

void YtSearch::addNewPlaylistItemToListWidget(
    const QJsonObject &playlistObject) {

  YtPlaylist *playlist = new YtPlaylist(m_ui.ytSearchResultsListWidget,
                                        playlistObject, m_accessManager);
  QListWidgetItem *item = new QListWidgetItem(m_ui.ytSearchResultsListWidget);
  m_ui.ytSearchResultsListWidget->setItemWidget(item, playlist);
  item->setSizeHint(playlist->sizeHint());
  m_ui.ytSearchResultsListWidget->addItem(item);
}

void YtSearch::resizeTrendingView() {
  if (trendingWidget != nullptr) {
    trendingWidget->resize(m_ui.ytSearchResultsListWidget->rect().size());
  }
}

void YtSearch::playAudio(const QString &audioLink) {

  YtVideo *videoItem = qobject_cast<YtVideo *>(sender());

  YtdlGetFormat *getFormatService =
      new YtdlGetFormat(this, m_engine_path, audioLink);

  connect(getFormatService, &YtdlGetFormat::ready, videoItem, [=]() {
    if (getFormatService->audio_url().isEmpty() == false) {

      auto playerService = createPlayerService(videoItem);

      playerService->play(getFormatService->audio_url());

      getFormatService->deleteLater();
    }
  });

  connect(getFormatService, &YtdlGetFormat::error, videoItem,
          [=](const QString &error) {
            qDebug() << error;
            videoItem->setStatusLabelText("Error!");
            getFormatService->deleteLater();
          });

  connect(getFormatService, &YtdlGetFormat::started, videoItem,
          [=]() { videoItem->setStatusLabelText("GetFormat..."); });

  connect(getFormatService, &YtdlGetFormat::startedGetUrl, videoItem,
          [=]() { videoItem->setStatusLabelText("GetUrl..."); });

  getFormatService->start_ytdl_play_audio();
}

void YtSearch::playVideo(const QString &videoLink) {

  YtVideo *videoItem = qobject_cast<YtVideo *>(sender());

  YtdlGetFormat *getFormatService = new YtdlGetFormat(
      this, m_engine_path, videoLink,
      static_cast<Format::preferred_format>(
          m_settings.getValue("preferedPlaybackQualityYtSearch", 1).toInt()));

  connect(getFormatService, &YtdlGetFormat::ready, videoItem, [=]() {
    if (getFormatService->combo_codes_are_incompitable() == false) {

      qDebug() << "USING COMBO URL";

      auto playerService = createPlayerService(videoItem);

      playerService->comboPlay(getFormatService->video_url(),
                               getFormatService->audio_url());

      getFormatService->deleteLater();
    } else {

      qDebug() << "USING OTHER URL";

      auto playerService = createPlayerService(videoItem);

      playerService->play(getFormatService->other_url());

      getFormatService->deleteLater();
    }
  });

  connect(getFormatService, &YtdlGetFormat::error, videoItem,
          [=](const QString &error) {
            qDebug() << error;
            videoItem->setStatusLabelText("Error!");
            getFormatService->deleteLater();
          });

  connect(getFormatService, &YtdlGetFormat::started, videoItem,
          [=]() { videoItem->setStatusLabelText("GetFormat..."); });

  connect(getFormatService, &YtdlGetFormat::startedGetUrl, videoItem,
          [=]() { videoItem->setStatusLabelText("GetUrl..."); });

  getFormatService->start_ytdl_play_video();
}

PlayerService *YtSearch::createPlayerService(YtVideo *videoItem) {

  PlayerService *playerService =
      new PlayerService(this, m_engine_path, m_settings);

  connect(playerService, &PlayerService::started, videoItem,
          [=]() { videoItem->setStatusLabelText("Opening..."); });

  connect(playerService, &PlayerService::ready, videoItem,
          [=]() { videoItem->setStatusLabelText("Playing..."); });

  connect(playerService, &PlayerService::finished, videoItem, [=]() {
    videoItem->setStatusLabelText("");
    playerService->deleteLater();
  });

  connect(playerService, &PlayerService::finished, this, [=]() {
    if (playerService != nullptr)
      playerService->deleteLater();
  });

  playerService->setPlayerTitle(videoItem->getTitle());

  return playerService;
}

void YtSearch::init_done() {
  m_ui.ytSearchLoadMore->setEnabled(m_ui.ytSearchResultsListWidget->count() >
                                    0);
  m_ui.ytSearchClear->setEnabled(m_ui.ytSearchResultsListWidget->count() > 0);
}

void YtSearch::enableAll() { m_ui.ytWidget->setEnabled(true); }

void YtSearch::disableAll() { m_ui.ytWidget->setEnabled(false); }

void YtSearch::resetMenu() {}

void YtSearch::retranslateUi() {}

void YtSearch::tabEntered() {

  resizeTrendingView();

  const QPalette appPalette = QApplication::palette();

  // set highlight color to listWidget
  QPalette customPalette = appPalette;
  QColor colorLighter = appPalette.highlight().color().lighter(110);
  colorLighter.setAlpha(50);
  customPalette.setColor(QPalette::Active, QPalette::Highlight, colorLighter);
  colorLighter.setAlpha(36);
  customPalette.setColor(QPalette::Inactive, QPalette::Highlight, colorLighter);
  m_ui.ytSearchResultsListWidget->setPalette(customPalette);

  // terndingWidget HVline fix
  if (trendingWidget != nullptr) {
    trendingWidget->fixVlineStyle();
  }

  if (firstLaunch) {
    showTrendingSelector();
  }

  m_ui.ytSearchLineEdit->setFocus();
}

void YtSearch::tabExited() {}
