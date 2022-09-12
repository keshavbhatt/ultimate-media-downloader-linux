#include "xpsearch.h"

#include "tabmanager.h"

#include <services/xp/xpcategories.h>

#include <QMessageBox>

XpSearch::XpSearch(const Context &ctx)
    : m_ctx(ctx), m_settings(m_ctx.Settings()), m_ui(m_ctx.Ui()),
      m_mainWindow(m_ctx.mainWidget()), m_tabManager(m_ctx.TabManager()),
      m_engine_path(m_ctx.Engines().defaultEngine("core").exePath().realExe()) {

  const QIcon xpIcon(":/icons/search.png");
  m_ui.xpSearchLineEdit->addAction(xpIcon, QLineEdit::LeadingPosition);

  m_ui.xpCategoryComboBox->addItem("Categories");
  m_ui.xpCategoryComboBox->insertSeparator(1);
  m_ui.xpCategoryComboBox->setMaxVisibleItems(10);
  XpCategories xpCategories;
  QList<QPair<QString, QString>> categories = xpCategories.categories();
  std::sort(
      categories.begin(), categories.end(),
      [](const QPair<QString, QString> &a, const QPair<QString, QString> &b) {
        return a.second < b.second;
      });
  for (const auto &pair : qAsConst(categories)) {
    QString text = pair.second;
    QVariant data = QVariant::fromValue(pair.first);
    m_ui.xpCategoryComboBox->addItem(text, data);
  }

  m_ui.xpSearchButton->setEnabled(m_ui.xpSearchLineEdit->text().isEmpty() ==
                                  false);

  m_accessManager = new QNetworkAccessManager(this);
  QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
  diskCache->setCacheDirectory(
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
  m_accessManager->setCache(diskCache);

  initSearchService();

  connect(m_ui.xpCategoryComboBox,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [=](int index) {
            m_ui.xpSearchLineEdit->clear();
            m_ui.xpSearchResultsListWidget->clear();
            QVariant data = m_ui.xpCategoryComboBox->itemData(index);
            if (data.isValid()) {
              QJsonObject trackingData{{"categoryName", data.toString()}};
              m_tabManager.trackingService()->trackEvent(
                  "xpSearchBrowseCategory", trackingData);
              m_searchService->loadByCategory(data.toString());
              setSearching(true);
            } else {
              m_ui.xpClearButton->click();
            }
          });

  connect(
      m_ui.xpSearchLineEdit, &QLineEdit::textChanged, this,
      [=](const QString &str) {
        m_ui.xpSearchButton->setEnabled(str.isEmpty() == false);
      },
      Qt::UniqueConnection);

  connect(
      m_ui.xpSearchLineEdit, &QLineEdit::returnPressed, this,
      [=]() {
        if (m_ui.xpSearchButton->isEnabled()) {
          m_ui.xpSearchButton->click();
        };
      },
      Qt::UniqueConnection);

  connect(
      m_ui.xpSearchLoadMore, &QPushButton::clicked, this,
      [=]() {
        setSearching(true);
        m_searchService->loadMore();
      },
      Qt::UniqueConnection);

  connect(
      m_ui.xpClearButton, &QPushButton::clicked, this,
      [=]() {
        setSearching(false);
        m_searchService->stop();
        m_ui.xpSearchLineEdit->clear();
        m_ui.xpSearchResultsListWidget->clear();
        m_ui.xpSearchLoadMore->setEnabled(false);
        m_ui.xpCategoryComboBox->setCurrentIndex(0);
        m_ui.xpClearButton->setEnabled(m_ui.xpSearchResultsListWidget->count() >
                                       0);
      },
      Qt::UniqueConnection);

  connect(
      m_ui.xpLatestButton, &QPushButton::clicked, this,
      [=]() {
        m_ui.xpSearchLineEdit->clear();
        m_searchService->loadLatest();
        setSearching(true);
      },
      Qt::UniqueConnection);

  connect(
      m_ui.xpTopRatedButton, &QPushButton::clicked, this,
      [=]() {
        m_ui.xpSearchLineEdit->clear();
        m_searchService->loadTopRated();
        setSearching(true);
      },
      Qt::UniqueConnection);

  connect(
      m_ui.xpMostViewedButton, &QPushButton::clicked, this,
      [=]() {
        m_ui.xpSearchLineEdit->clear();
        m_searchService->loadMostViewed();
        setSearching(true);
      },
      Qt::UniqueConnection);

  connect(
      m_ui.xpSearchButton, &QPushButton::clicked, this,
      [=]() {
        if (this->m_searching) {
          // cancel search
          this->cancelAllsearchServiceRequests();
          this->setSearching(false);
        } else {
          // start search
          this->search(m_ui.xpSearchLineEdit->text());
        }
      },
      Qt::UniqueConnection);
}

void XpSearch::initSearchService() {

  m_searchService = new XpSearchService(this, m_accessManager);

  connect(
      m_searchService, &XpSearchService::finished, this,
      [=](const QByteArray &data) {
        this->processVideoResults(data);
        this->setSearching(false);
      },
      Qt::UniqueConnection);

  connect(
      m_searchService, &XpSearchService::error, this,
      [=](const QString &error) {
        this->processError(error);
        this->setSearching(false);
      },
      Qt::UniqueConnection);

  m_ui.xpSearchLoadMore->setEnabled(m_searchService->page() > 0);
}

void XpSearch::processError(const QString &error) {
  m_ui.xpSearchResultsListWidget->clear();
  this->addPsudoItemToListWidget(error);
}

void XpSearch::setSearching(bool newSearching) {
  m_searching = newSearching;

  if (m_searching) {
    m_ui.xpSearchButton->setText(tr("Cancel"));
    m_ui.xpSearchButton->setEnabled(true);
    m_ui.xpSearchLineEdit->setEnabled(false);
    m_ui.xpSearchLoadMore->setEnabled(false);
    m_ui.xpSearchResultsListWidget->clear();
    this->addPsudoItemToListWidget("Loading...");
  } else {
    m_ui.xpSearchButton->setText(tr("Search"));
    m_ui.xpSearchButton->setEnabled(m_ui.xpSearchLineEdit->text().isEmpty() ==
                                    false);
    m_ui.xpSearchLineEdit->setEnabled(true);
    m_ui.xpSearchLoadMore->setEnabled(m_searchService->page() > 0);
  }
}

void XpSearch::search(const QString &searchTerm) {
  // cancel all
  this->cancelAllsearchServiceRequests();

  this->setSearching(true);

  m_searchService->initSearch(searchTerm);

  QJsonObject trackingData{{"searchTerm", searchTerm}};

  m_tabManager.trackingService()->trackEvent("xpSearchQuery", trackingData);

  m_searchService->search();
}

void XpSearch::processVideoResults(const QByteArray &data) {

  if (m_session_age_restricted_consent_taken == false) {
    QMessageBox msgBox(
        QMessageBox::Warning,
        qApp->applicationName() + " | " + QObject::tr("Warning"),
        QObject::tr("Age restricted content warning!\nThis plugin may contain "
                    "age restricted content!\n\nProceed?"),
        QMessageBox::Yes | QMessageBox::No);

    auto ret = msgBox.exec();
    if (ret == QMessageBox::No) {
      m_ui.xpClearButton->setEnabled(true);
      m_ui.xpClearButton->click();
      return;
    } else if (ret == QMessageBox::Yes) {
      m_session_age_restricted_consent_taken = true;
    }
  }

  m_ui.xpSearchResultsListWidget->clear();

  QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
  if (jsonResponse.isArray()) {

    this->addResultInfoHeaderItemToListWidget();

    QJsonArray jsonArray = jsonResponse.array();
    if (jsonArray.isEmpty()) {
      this->addNoResultReturnedForQueryToListWidget();
    } else {
      foreach (const QJsonValue &val, jsonArray) {
        this->addNewVideoItemToListWidget(val.toObject());
      }
    }
    m_ui.xpClearButton->setEnabled(m_ui.xpSearchResultsListWidget->count() > 0);
  }
}

void XpSearch::addResultInfoHeaderItemToListWidget() {
  auto searchTerm = m_ui.xpSearchLineEdit->text();
  if (searchTerm.trimmed().isEmpty() == false) {
    QString pageNumberLabel =
        QString("Results for \"%1\" on page %2")
            .arg(searchTerm, QString::number(m_searchService->page()));
    this->addPsudoItemToListWidget(pageNumberLabel);
  } else {
    QString pageNumberLabel =
        QString("Results from page %1")
            .arg(QString::number(m_searchService->page()));
    this->addPsudoItemToListWidget(pageNumberLabel);
  }
}

void XpSearch::addNoResultReturnedForQueryToListWidget() {
  m_ui.xpSearchResultsListWidget->clear();
  auto searchTerm = m_ui.xpSearchLineEdit->text();
  QString label = QString("No results returned for \"%1\"").arg(searchTerm);
  this->addPsudoItemToListWidget(label);
}

void XpSearch::cancelAllsearchServiceRequests() {
  foreach (XpSearchService *srv, this->findChildren<XpSearchService *>()) {
    srv->stop();
  }
  m_ui.xpSearchResultsListWidget->clear();
  this->addPsudoItemToListWidget("Request cancelled");
}

void XpSearch::addPsudoItemToListWidget(const QString &labelStr) {
  QLabel *label = new ElidedLabel(m_ui.xpSearchResultsListWidget);
  label->setText(labelStr);
  label->setToolTip(labelStr);
  label->setAlignment(Qt::AlignHCenter);
  label->setEnabled(false);
  QListWidgetItem *item = new QListWidgetItem(m_ui.xpSearchResultsListWidget);
  m_ui.xpSearchResultsListWidget->setItemWidget(item, label);
  item->setFlags(Qt::NoItemFlags);
  m_ui.xpSearchResultsListWidget->addItem(item);
}

void XpSearch::addNewVideoItemToListWidget(const QJsonObject &videoObject) {

  XpVideo *video =
      new XpVideo(m_ui.xpSearchResultsListWidget, videoObject, m_accessManager);

  QListWidgetItem *item = new QListWidgetItem(m_ui.xpSearchResultsListWidget);

  m_ui.xpSearchResultsListWidget->setItemWidget(item, video);

  item->setSizeHint(video->sizeHint());

  connect(video, &XpVideo::playVideo, this, &XpSearch::playVideo);

  connect(video, &XpVideo::playFromUrl, this, &XpSearch::playFromUrl);

  m_ui.xpSearchResultsListWidget->addItem(item);
}

void XpSearch::playFromUrl(const QString &mediaLink) {
  XpVideo *videoItem = qobject_cast<XpVideo *>(sender());

  auto playerService = createPlayerService(videoItem);

  playerService->play(mediaLink);
}

void XpSearch::playVideo(const QString &videoLink) {

  XpVideo *videoItem = qobject_cast<XpVideo *>(sender());

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

PlayerService *XpSearch::createPlayerService(XpVideo *videoItem) {

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

void XpSearch::init_done() {
  m_ui.xpSearchLoadMore->setEnabled(m_ui.xpSearchResultsListWidget->count() >
                                    0);
  m_ui.xpClearButton->setEnabled(m_ui.xpSearchResultsListWidget->count() > 0);
}

void XpSearch::tabEntered() {

  const QPalette appPalette = QApplication::palette();

  // set highlight color to listWidget
  QPalette customPalette = appPalette;
  QColor colorLighter = appPalette.highlight().color().lighter(110);
  colorLighter.setAlpha(50);
  customPalette.setColor(QPalette::Active, QPalette::Highlight, colorLighter);
  colorLighter.setAlpha(36);
  customPalette.setColor(QPalette::Inactive, QPalette::Highlight, colorLighter);
  m_ui.xpSearchResultsListWidget->setPalette(customPalette);

  m_ui.xpSearchLineEdit->setFocus();
}

void XpSearch::tabExited() {}

void XpSearch::retranslateUi() {}

void XpSearch::disableAll() { m_ui.xpSearchTab->setEnabled(false); }

void XpSearch::enableAll() { m_ui.xpSearchTab->setEnabled(true); }
