#ifndef XPSEARCHSERVICE_H
#define XPSEARCHSERVICE_H

#include <QObject>
#include <QtNetwork>

class XpSearchService : public QObject {

  Q_OBJECT
public:
  explicit XpSearchService(QObject *parent = nullptr,
                           QNetworkAccessManager *networkManager = nullptr);

  ~XpSearchService();

  enum OperationType { SEARCH, SORT_BROWSE };

  void initSearch(const QString &term, int page = 1);
  void search();
  void stop();

  void loadMore();
  int page() const;

  void loadLatest();
  void loadMostViewed();
  void loadTopRated();
  void loadByCategory(QString categoryName = "anal");
signals:
  void finished(const QByteArray &data);
  void error(const QString &errorString);

private slots:
  void replyFinished();

private:
  QNetworkAccessManager *p_networkManager;

  void initSort(QString page_name = "new", QString sort_by = "post_date",
                int page = 1);

  QNetworkReply *p_reply = nullptr;

  int m_page = 1;

  QUrl m_serviceUrl;

  QUrlQuery m_query;

  OperationType m_oprationType;

  QByteArray proccessUnformatedData(const QByteArray &data);
};

#endif // XPSEARCHSERVICE_H
