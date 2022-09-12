#ifndef EASYLISTMANAGER_H
#define EASYLISTMANAGER_H

#include <QDateTime>
#include <QFile>
#include <QNetworkAccessManager>
#include <QObject>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>

class EasyListManager : public QObject {
  Q_OBJECT
public:
  explicit EasyListManager(QObject *parent = nullptr);
  QString getEasyListFilePath() const;
  bool isUpdateNeeded();
  void downloadEasyList();

  void init();

signals:
  void easyListReady();

private slots:
  void onDownloadFinished(QNetworkReply *reply);

private:
  QString easyListFilePath;
  QNetworkAccessManager *networkManager;
  QDateTime lastModifiedTime;
  int expirationDays;
};

#endif // EASYLISTMANAGER_H
