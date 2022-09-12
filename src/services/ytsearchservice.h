#ifndef YTSEARCHSERVICE_H
#define YTSEARCHSERVICE_H

#include "invidioustrendingparser.h"
#include <QObject>
#include <QtNetwork>

class YtSearchService : public QObject {
  Q_OBJECT
public:
  explicit YtSearchService(QObject *parent = nullptr,
                           QNetworkAccessManager *networkManager = nullptr);
  ~YtSearchService();

  enum SearchMode {
    VIDEO,
    PLAYLIST,
    TRENDING_DEFAULT,
    TRENDING_MUSIC,
    TRENDING_GAMES,
    TRENDING_MOVIES
  };

  int getPage() const;

  void setInstanceBaseUrl(const QString &instance);

signals:
  void finished(YtSearchService::SearchMode searchMode, const QByteArray &data);
  void error(QString errorString);

private slots:
  void stop();
  void replyFinished();

  QUrl getServiceUrl();
  QByteArray proccessUnformatedData(const QByteArray &data);

public slots:
  void init(const QString &instance, const QString &term, const YtSearchService::SearchMode &searchMode, const QString region = "US");
  void get();
  void cancel();

  void loadMore();

private:
  QNetworkAccessManager *networkAccessManager = nullptr;
  QNetworkReply *reply = nullptr;

  SearchMode searchMode = SearchMode::VIDEO;
  QUrl serviceUrl, serviceUrlTrending;
  QString instanceBaseUrl;
  int page = 0;
  QUrlQuery query;
};

#endif // YTSEARCHSERVICE_H
