#include "xpsearchservice.h"
#include "xpsearchparser.h"

XpSearchService::XpSearchService(QObject *parent,
                                 QNetworkAccessManager *networkManager)
    : QObject(parent), p_networkManager(networkManager) {}

XpSearchService::~XpSearchService() {}

void XpSearchService::initSearch(const QString &term, int page) {
  m_oprationType = OperationType::SEARCH;

  m_page = page;

  m_query.clear();

  QString searchTerm = QString(term).replace(" ", "-");
  m_query.addQueryItem("q", searchTerm);
  m_query.addQueryItem("from_videos", QString::number(m_page));
  m_query.addQueryItem("from_album", QString::number(m_page));

  m_serviceUrl = QUrl("https://localhost/search/" + searchTerm);
  m_serviceUrl.setQuery(m_query);
}

void XpSearchService::search() {
  this->stop();

  QNetworkRequest req(m_serviceUrl);
  req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
  req.setRawHeader("User-Agent",
                   "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, "
                   "like Gecko) Chrome/100.0.4896.127 Safari/537.36");
  req.setRawHeader("accept-language", "en-US,en;q=0.9");
  req.setTransferTimeout(15000);
  p_reply = p_networkManager->get(req);
  connect(p_reply, SIGNAL(finished()), this, SLOT(replyFinished()),
          Qt::UniqueConnection);
  qDebug() << "Used service url" << req.url().toString();
}

void XpSearchService::initSort(QString page_name, QString sort_by, int page) {
  m_oprationType = OperationType::SORT_BROWSE;
  m_page = page;

  m_query.clear();
  m_query.addQueryItem("sort_by", sort_by);
  m_query.addQueryItem("from", QString::number(m_page));

  m_serviceUrl = QUrl("https://localhost/" + page_name);
  m_serviceUrl.setQuery(m_query);
}

void XpSearchService::loadByCategory(QString categoryName) {
  this->stop();

  this->initSort(categoryName, "post_date");
  this->search();
}

void XpSearchService::loadLatest() {
  this->stop();

  this->initSort("new", "post_date");
  this->search();
}

void XpSearchService::loadTopRated() {
  this->stop();

  this->initSort("best", "rating_today");
  this->search();
}

void XpSearchService::loadMostViewed() {
  this->stop();

  this->initSort("popular-videos", "video_viewed_today");
  this->search();
}

void XpSearchService::stop() {
  if (p_reply != nullptr) {
    if (!p_reply->isReadable()) {
      return;
    }
    p_reply->abort();
  }
}

void XpSearchService::loadMore() {

  int nextPage = m_page + 1;

  if (m_oprationType == OperationType::SEARCH) {
    QUrlQuery newQuery;
    foreach (auto k, m_query.queryItems()) {
      if (k.first == "from_album") {
        newQuery.addQueryItem(k.first, QString::number(nextPage));
      } else {
        newQuery.addQueryItem(k.first, k.second);
      }
      if (k.first == "from_videos") {
        newQuery.addQueryItem(k.first, QString::number(nextPage));
      } else {
        newQuery.addQueryItem(k.first, k.second);
      }
    }
    m_serviceUrl.setQuery(newQuery);
    this->search();
  } else {
    QUrlQuery newQuery;
    foreach (auto k, m_query.queryItems()) {
      if (k.first == "from") {
        newQuery.addQueryItem(k.first, QString::number(nextPage));
      } else {
        newQuery.addQueryItem(k.first, k.second);
      }
    }
    m_serviceUrl.setQuery(newQuery);
    this->search();
  }
}

void XpSearchService::replyFinished() {
  if (p_reply->error() == QNetworkReply::NoError) {
    auto data = p_reply->readAll();

    QUrlQuery tempQuery(m_serviceUrl);
    if (tempQuery.hasQueryItem("from_videos"))
      m_page = tempQuery.queryItemValue("from_videos").toInt();
    if (tempQuery.hasQueryItem("from_album"))
      m_page = tempQuery.queryItemValue("from_album").toInt();
    if (tempQuery.hasQueryItem("from"))
      m_page = tempQuery.queryItemValue("from").toInt();

    emit finished(proccessUnformatedData(
        data.replace("class=\"item \"", "class=\"item\"")));

  } else {
    // handle error
    QString errorString = p_reply->errorString();
    if (errorString.contains(m_serviceUrl.host())) {
      emit error(
          QDateTime::currentDateTime().toLocalTime().toString("hh:mm:ss ap") +
          ": " + errorString.replace(m_serviceUrl.host(), ""));
    }
  }
}

QByteArray XpSearchService::proccessUnformatedData(const QByteArray &data) {
  QJsonArray array;

  foreach (auto a, XpSearchParser::parseFromHtml(data)) {
    QJsonObject o;

    o.insert("title", a.getTitle());
    o.insert("thumbnailUrl", a.getThumbnail_url());
    o.insert("previewUrl", a.getPreview_url());
    o.insert("lengthStr", a.getLength());
    o.insert("viewCountStr", a.getViews());
    o.insert("videoLink", a.getVideo_url());

    array.append(QJsonValue(o));
  };
  QJsonDocument doc;
  doc.setArray(array);
  return doc.toJson();
}

int XpSearchService::page() const { return m_page; }
