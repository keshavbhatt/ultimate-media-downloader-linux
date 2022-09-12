#include "invidiousinstances.h"

InvidiousInstances::InvidiousInstances(QObject *parent,
                                       QNetworkAccessManager *networkManager)
    : QObject{parent} {
  this->networkAccessManager = networkManager;

  m_blackListed << "perennialte.ch";

  localInstancesFallbackPath =
      QStandardPaths::writableLocation(QStandardPaths::DataLocation) +
      QDir::separator() + "instances.json";

  load();
}

void InvidiousInstances::stop() {
  if (reply != nullptr) {
    if (!reply->isReadable()) {
      return;
    }
    reply->abort();
    reply->disconnect();
  }
}

void InvidiousInstances::setCurrentInstance(
    const int &newCurrentInstanceIndex) {
  auto newCurrentInstanceStr = instances.at(newCurrentInstanceIndex);
  currentInstance = newCurrentInstanceStr;
}

InvidiousInstances::~InvidiousInstances() {}

void InvidiousInstances::load() {
  this->stop();

  // fresh request
  QNetworkRequest req(
      QUrl("https://api.invidious.io/instances.json?pretty=1&sort_by=health"));

  req.setTransferTimeout(
      15000); // QNetworkRequest::DefaultTransferTimeoutConstant
  reply = networkAccessManager->get(req);

  connect(
      reply, &QNetworkReply::finished, this,
      [=]() {
        if (reply->error() == QNetworkReply::NoError) {
          QString dataStr = reply->readAll();

          // save network json to disk for fallback
          QJsonDocument jsonResponse =
              QJsonDocument::fromJson(dataStr.toUtf8());
          Util::saveJson(jsonResponse, localInstancesFallbackPath);

          this->setInstances(dataStr);
          qDebug() << "Instances Loaded from network";
        } else {
          loadFallbackInstancesFromLocalDiskOrResources();
        }
      },
      Qt::UniqueConnection);
}

void InvidiousInstances::reload() { this->load(); }

void InvidiousInstances::loadFallbackInstancesFromLocalDiskOrResources() {

  // use fallback dataset from disk
  QFileInfo fInfo(localInstancesFallbackPath);
  if (fInfo.exists()) {
    auto jsonDocumentFromDisk = Util::loadJson(localInstancesFallbackPath);
    if (jsonDocumentFromDisk.isEmpty() == false) {
      this->setInstances(jsonDocumentFromDisk.toJson());
      qDebug() << "Instances Loaded from disk";
    }
  }

  // check if instances loaded otherwise get from qrc
  if (instances.isEmpty()) {
    // use fallback dataset from qrc
    auto jsonDocument = Util::loadJson(":/others/instances.json");
    this->setInstances(jsonDocument.toJson());
    qDebug() << "Instances Loaded from qrc";
  }
}

const QStringList &InvidiousInstances::getInstances() const {
  return instances;
}

void InvidiousInstances::setInstances(const QString &dataStr) {
  // parse json data and set instances
  QJsonDocument jsonResponse = QJsonDocument::fromJson(dataStr.toUtf8());
  if (jsonResponse.isArray()) {
    QJsonArray jsonArray = jsonResponse.array();
    foreach (const QJsonValue &val, jsonArray) {
      auto arr = val.toArray();
      foreach (const QJsonValue &v, arr) {
        QString uri = v.toObject().value("uri").toString();
        bool api = v.toObject().value("api").toBool();
        if (uri.isEmpty() == false &&
            uri.contains("onion", Qt::CaseInsensitive) == false && api &&
            !instanceBlackListed(uri))
          instances << uri;
      }
    }
    instances.removeDuplicates();
  } else {
    loadFallbackInstancesFromLocalDiskOrResources();
    return;
  }
  if (instances.isEmpty() == false) {
    currentInstance = instances.first();
    emit ready();
  }
}

bool InvidiousInstances::instanceBlackListed(QString uri) {
  bool found = false;
  foreach (auto word, m_blackListed) {
    if (uri.contains(word)) {
      found = true;
      break;
    }
  }
  return found;
}

QString InvidiousInstances::getFirstInstance() { return instances.first(); }

QString InvidiousInstances::getCurrentInstance() { return currentInstance; }

QString InvidiousInstances::getRandomInstance() {
  int rand = QRandomGenerator::global()->bounded(instances.count());
  currentInstance = instances.at(rand);
  return currentInstance;
}
