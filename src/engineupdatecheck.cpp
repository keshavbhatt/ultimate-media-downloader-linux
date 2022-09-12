#include "engineupdatecheck.h"

#include <QDate>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QProcess>
#include <QStandardPaths>

QString ENGINE_VERSION_URL =
    "https://api.github.com/repos/yt-dlp/yt-dlp/releases/latest";

EngineUpdateCheck::EngineUpdateCheck(QObject *parent, QString enginePath,
                                     QNetworkAccessManager *networkManager)
    : QObject{parent}, m_networkManager(networkManager) {
  m_enginePath = "main";
}

void EngineUpdateCheck::checkForUpdate(bool afterUpdate) {
  refreshInstalledEngineVerion(afterUpdate);
}

void EngineUpdateCheck::refreshInstalledEngineVerion(bool afterUpdate) {

  QProcess *vprocess = new QProcess(this);

  connect(vprocess, &QProcess::readyRead, this, [=]() {
    this->m_installed_engine_version =
        vprocess->readAll().trimmed().simplified();
    if (afterUpdate) {
      qWarning() << "EGNINE: Engine updated to version: " +
                        this->m_installed_engine_version;
    } else {
      qWarning() << "EGNINE: Installed engine version: " +
                        this->m_installed_engine_version;
      performComparison();
    }
  });

  connect(vprocess,
          static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
              &QProcess::finished),
          this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
            Q_UNUSED(exitCode)
            if (exitStatus != QProcess::NormalExit) {
              qWarning()
                  << "EGNINE: Failed while getting engine version information";
            }
            if (m_installed_engine_version.trimmed().isEmpty()) {
              qWarning() << "ENGINE: Engine using fallback engine version...";
              m_installed_engine_version = "2019.01.01";
              performComparison();
            }
            vprocess->deleteLater();
          });

  vprocess->start("python3", QStringList() << m_enginePath << "--version");
}

void EngineUpdateCheck::performComparison() {
  if (m_networkManager == nullptr) {
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this,
            [=](QNetworkReply *reply) {
              qDebug() << "EGNINE: Remote engine version check finished.";
              QString remote_engine_version;
              if (reply->error() == QNetworkReply::NoError) {
                QString replyStr = reply->readAll();
                // qDebug() << "EGNINE: Remote engine version check replied" <<
                // replyStr;
                if (reply->request().url().toString().contains("api.")) {
                  QJsonDocument doc =
                      QJsonDocument::fromJson(replyStr.toUtf8());
                  if (!doc.isNull()) {
                    QJsonObject obj = doc.object();
                    remote_engine_version =
                        obj.value("tag_name").toString().trimmed();
                  } else {
                    qWarning()
                        << "EGNINE: Engine update check failed, API returned "
                           "invalid JSON reply.";
                  }
                } else {
                  remote_engine_version = replyStr.trimmed();
                }
              } else {
                qWarning() << "EGNINE: Engine update check failed:"
                           << reply->errorString();
              }
              qWarning() << "EGNINE: Remote Engine version: " +
                                remote_engine_version;
              if (this->updateAvailable(remote_engine_version))
                emit updateFound();
              reply->deleteLater();
            });
  }
  qWarning() << "EGNINE: Checking remote engine version...";
  m_networkManager->get(QNetworkRequest(QUrl(ENGINE_VERSION_URL)));
}

bool EngineUpdateCheck::updateAvailable(QString remote_engine_date) {

  int year, month, day, n_year, n_month, n_day;

  year = QDate::fromString(m_installed_engine_version, Qt::ISODate).year();
  month = QDate::fromString(m_installed_engine_version, Qt::ISODate).month();
  day = QDate::fromString(m_installed_engine_version, Qt::ISODate).day();

  n_year = QDate::fromString(remote_engine_date, Qt::ISODate).year();
  n_month = QDate::fromString(remote_engine_date, Qt::ISODate).month();
  n_day = QDate::fromString(remote_engine_date, Qt::ISODate).day();

  if (n_year > year || n_month > month || n_day > day) {
    return true;
  } else {
    return false;
  }
}
