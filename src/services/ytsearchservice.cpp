#include "ytsearchservice.h"

YtSearchService::YtSearchService(QObject *parent,
                                 QNetworkAccessManager *networkManager)
    : QObject{parent} {

  this->networkAccessManager = networkManager;
}

void YtSearchService::setInstanceBaseUrl(const QString &instance) {
  this->instanceBaseUrl = instance;
  QUrl u(instance);
  if(u.isValid()){
      this->serviceUrl.setHost(u.host());
      this->serviceUrlTrending.setHost(u.host());
  }
}

void YtSearchService::init(const QString &instance, const QString &term,
                           const SearchMode &searchMode, const QString region) {

  setInstanceBaseUrl(instance);

  this->searchMode = searchMode;

  this->page = 1;

  serviceUrl.setUrl(instanceBaseUrl + "/api/v1/search/");
  serviceUrlTrending.setUrl(instanceBaseUrl + "/feed/trending/");

  // TODO check of we can also utilize region in video and playlist search
  if (searchMode == YtSearchService::SearchMode::VIDEO) {
    query.clear();
    query.addQueryItem("type", "video");
    query.addQueryItem("sort_by", "relevance");
    query.addQueryItem(
        "fields",
        "title,videoId,author,viewCount,publishedText,authorId,lengthSeconds");
    query.addQueryItem("page", QString::number(page));
    query.addQueryItem("q", term);
    serviceUrl.setQuery(query);
  } else if (searchMode == YtSearchService::SearchMode::PLAYLIST) {
    query.clear();
    query.addQueryItem("type", "playlist");
    query.addQueryItem("sort_by", "relevance");
    query.addQueryItem(
        "fields",
        "title,playlistId,author,authorId,videoCount,videos,playlistThumbnail");
    query.addQueryItem("page", QString::number(page));
    query.addQueryItem("q", term);
    serviceUrl.setQuery(query);
  } else if (searchMode == YtSearchService::SearchMode::TRENDING_DEFAULT) {
    query.clear();
    query.addQueryItem("type", "Default");
    query.addQueryItem("region", region);
    serviceUrlTrending.setQuery(query);
  } else if (searchMode == YtSearchService::SearchMode::TRENDING_MUSIC) {
    query.clear();
    query.addQueryItem("type", "Music");
    query.addQueryItem("region", region);
    serviceUrlTrending.setQuery(query);
  } else if (searchMode == YtSearchService::SearchMode::TRENDING_GAMES) {
    query.clear();
    query.addQueryItem("type", "Gaming");
    query.addQueryItem("region", region);
    serviceUrlTrending.setQuery(query);
  } else if (searchMode == YtSearchService::SearchMode::TRENDING_MOVIES) {
    query.clear();
    query.addQueryItem("type", "Movies");
    query.addQueryItem("region", region);
    serviceUrlTrending.setQuery(query);
  }
}

YtSearchService::~YtSearchService() {}

// search
void YtSearchService::get() {

  QNetworkRequest req(getServiceUrl());
  req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
  req.setRawHeader("User-Agent",
                   "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, "
                   "like Gecko) Chrome/100.0.4896.127 Safari/537.36");
  req.setRawHeader("accept-language", "en-US,en;q=0.9");
  req.setTransferTimeout(15000);
  reply = networkAccessManager->get(req);
  connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()),
          Qt::UniqueConnection);
  qDebug() << "Used service url" << req.url().toString();
}

QUrl YtSearchService::getServiceUrl() {
  switch (this->searchMode) {
  case YtSearchService::SearchMode::VIDEO:
    return serviceUrl;
    break;
  case YtSearchService::SearchMode::PLAYLIST:
    return serviceUrl;
    break;
  default: // other cases are trending for now
    return serviceUrlTrending;
    break;
  }
}

// search load more
void YtSearchService::loadMore() {

  int nextPage = page + 1;
  QUrlQuery newQuery;
  foreach (auto k, query.queryItems()) {
    if (k.first == "page") {
      newQuery.addQueryItem(k.first, QString::number(nextPage));
    } else {
      newQuery.addQueryItem(k.first, k.second);
    }
  }
  serviceUrl.setQuery(newQuery);

  this->get();
}

int YtSearchService::getPage() const { return page; }

void YtSearchService::cancel() { this->stop(); }

// stop request
void YtSearchService::stop() {
  if (reply != nullptr) {
    if (!reply->isReadable()) {
      return;
    }
    reply->abort();
  }
}

void YtSearchService::replyFinished() {
  if (reply->error() == QNetworkReply::NoError) {
    auto data = reply->readAll();
    if (searchMode == YtSearchService::SearchMode::VIDEO ||
        searchMode == YtSearchService::SearchMode::PLAYLIST) {
      QUrlQuery tempQuery(serviceUrl);
      page = tempQuery.queryItemValue("page").toInt();
      emit finished(this->searchMode, data);
    } else {
      page = 0;
      auto refined_data = proccessUnformatedData(data);
      emit finished(this->searchMode, refined_data);
    }
  } else {
    // handle error
    QString errorString = reply->errorString();
    if (errorString.contains(serviceUrl.host()) ||
        errorString.contains(serviceUrlTrending.host())) {
      // remove host info from error string and emit the error signal
      emit error(
          QDateTime::currentDateTime().toLocalTime().toString("hh:mm:ss ap") +
          ": " +
          errorString.replace(serviceUrl.host(), "")
              .replace(serviceUrlTrending.host(), "") +
          ", check internet connection or reconnect to search server");
    }
  }
}

QByteArray YtSearchService::proccessUnformatedData(const QByteArray &data) {
  QJsonArray array;
  //{"title":"King Misuzulu promises unity in the Zulu
  // nation","videoId":"WCz8QPEHv4M","author":"SABC
  // News","authorId":"UC8yH-uI81UUtEMDsowQyx1g","viewCount":1230,"publishedText":"1
  // hour ago","lengthSeconds":107}

  foreach (auto a, InvidiousTrendingParser::parseFromHtml(data)) {
    QJsonObject o;

    o.insert("title", a.getTitle());
    o.insert("author", a.getChannelName());
    o.insert("videoId", a.getVideoId());
    o.insert("authorId", a.getChannelId());
    o.insert("publishedText", a.getPublished());

    // these entries are modified one
    o.insert("lengthStr", a.getLength());
    o.insert("viewCountStr", a.getViews());

    array.append(QJsonValue(o));
  };
  QJsonDocument doc;
  doc.setArray(array);
  return doc.toJson();
}
