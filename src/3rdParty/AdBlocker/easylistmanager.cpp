#include "easylistmanager.h"

#include <QDebug>
#include <QDir>
#include <QNetworkReply>
#include <QTextStream>

EasyListManager::EasyListManager(QObject *parent) : QObject(parent) {
  networkManager = new QNetworkAccessManager(this);
  easyListFilePath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
      QDir::separator() + "easylist.txt";

  connect(networkManager, &QNetworkAccessManager::finished, this,
          &EasyListManager::onDownloadFinished);
}

void EasyListManager::init() {
  if (!QFile::exists(easyListFilePath) || isUpdateNeeded()) {
    downloadEasyList();
  } else {
    qDebug() << "[AD_BLOCKER]"
             << "No Update needed, using existing EasyList file";
    emit easyListReady();
  }
}

QString EasyListManager::getEasyListFilePath() const {
  return easyListFilePath;
}

bool EasyListManager::isUpdateNeeded() {
  QFile file(easyListFilePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return true;
  }

  QTextStream in(&file);
  QString line;
  while (!in.atEnd()) {
    line = in.readLine();

    expirationDays = -1;
    if (line.startsWith("! Last modified:")) {
      QStringList dateTime = line.split(" ").mid(3, 4);
      lastModifiedTime =
          QDateTime::fromString(dateTime.join(" "), "d MMM yyyy hh:mm");
    }

    if (line.startsWith("! Expires:")) {
      expirationDays = line.split(" ").at(2).toInt();
    }

    if (line.startsWith("[") || line.startsWith("!")) {
      if (lastModifiedTime.isValid() && expirationDays != -1)
        break;
      else
        continue;
    } else {
      break;
    }
  }
  file.close();

  // Check if the expiration time has passed
  QDateTime currentDateTime = QDateTime::currentDateTime();
  return currentDateTime > lastModifiedTime.addDays(expirationDays);
}

void EasyListManager::downloadEasyList() {
  qDebug() << "[AD_BLOCKER] Downloading EasyList";
  QUrl easyListUrl("https://easylist.to/easylist/easylist.txt");
  networkManager->get(QNetworkRequest(easyListUrl));
}

void EasyListManager::onDownloadFinished(QNetworkReply *reply) {
  if (reply->error() != QNetworkReply::NoError) {
    qWarning() << "[AD_BLOCKER] Failed to download EasyList:"
               << reply->errorString();
    return;
  }

  QByteArray downloadedData = reply->readAll();
  QFile file(easyListFilePath);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    file.write(downloadedData);
    file.close();

    qDebug() << "[AD_BLOCKER] EasyList downloaded and saved.";
    emit easyListReady();
  } else {
    qWarning() << "[AD_BLOCKER] Failed to save EasyList file.";
  }

  reply->deleteLater();
}
