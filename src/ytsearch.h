#ifndef YTSEARCH_H
#define YTSEARCH_H

#include "context.hpp"
#include "services/invidiousinstances.h"
#include "services/playerservice.h"
#include "services/ytdlgetformat.h"
#include "services/ytsearchservice.h"
#include "settings.h"
#include "trendingwidget.h"
#include "widgets/elidedlabel.h"
#include "ytplaylist.h"
#include "ytvideo.h"
#include <QObject>
#include <services/xp/xptriggerwords.h>

class YtSearch : public QObject {
  Q_OBJECT
public:
  YtSearch(const Context &);
  void init_done();
  void enableAll();
  void disableAll();
  void resetMenu();
  void retranslateUi();
  void tabEntered();
  void tabExited();

public slots:
  void resizeTrendingView();
private slots:
  void search(const QString &searchTerm);
  void processVideoResults(const QByteArray &data);
  void processPlaylistResults(const QByteArray &data);
  void processError(const QString &error);
  void setSearching(const bool &searching, const bool &resetRetires = true);
  void addNewVideoItemToListWidget(const QJsonObject &videoObject);
  void addNewPlaylistItemToListWidget(const QJsonObject &playlistObject);

  void initSearchService();
  void getTrending(const YtSearchService::SearchMode &searchMode =
                       YtSearchService::SearchMode::TRENDING_MUSIC,
                   const QString &region = "US");
  void addPsudoItemToListWidget(const QString &label);
  void addNoResultReturnedForQueryToListWidget();
  void addResultInfoHeaderItemToListWidget();
  void cancelAllsearchServiceRequests();

  void addTrendingResultInfoHeaderItemToListWidget(
      const YtSearchService::SearchMode &searchMode);

  void showTrendingSelector();

  void processTrendingResult(const QByteArray &data,
                             const YtSearchService::SearchMode &searchMode);

  void searchServiceRetry();
  PlayerService *createPlayerService(YtVideo *videoItem = nullptr);
  void playVideo(const QString &videoLink);
  void playAudio(const QString &audioLink);
  void hideTrendingSelector();

private:
  YtSearchService *m_searchService = nullptr;
  bool searching = false;
  const Context &m_ctx;
  settings &m_settings;
  Ui::MainWindow &m_ui;
  QWidget &m_mainWindow;
  tabManager &m_tabManager;
  QNetworkAccessManager *m_accessManager = nullptr;
  InvidiousInstances *invidiousInstances = nullptr;
  TrendingWidget *trendingWidget = nullptr;
  int searchServiceRetries = 0;
  QString m_engine_path;

  bool firstLaunch = true;

  XpTriggerWords m_xpTriggerWords;
};

#endif // YTSEARCH_H
