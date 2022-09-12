#ifndef ENGINEUPDATECHECK_H
#define ENGINEUPDATECHECK_H

#include <QNetworkAccessManager>
#include <QObject>

class EngineUpdateCheck : public QObject {
  Q_OBJECT
public:
  explicit EngineUpdateCheck(QObject *parent = nullptr, QString enginePath = "",
                             QNetworkAccessManager *networkManager = nullptr);

  void checkForUpdate(bool afterUpdate = false);
signals:
  void updateFound();

private:
  void refreshInstalledEngineVerion(bool afterUpdate = false);
  QString m_enginePath;
  QByteArray m_installed_engine_version;
  bool updateAvailable(QString remote_engine_date);
  QNetworkAccessManager *m_networkManager = nullptr;
  void performComparison();
};

#endif // ENGINEUPDATECHECK_H
