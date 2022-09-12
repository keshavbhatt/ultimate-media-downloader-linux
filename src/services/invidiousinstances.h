#ifndef INVIDIOUSINSTANCES_H
#define INVIDIOUSINSTANCES_H

#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QtNetwork>

class InvidiousInstances : public QObject {
  Q_OBJECT
public:
  explicit InvidiousInstances(QObject *parent = nullptr,
                              QNetworkAccessManager *networkManager = nullptr);

  const QStringList &getInstances() const;

  void setCurrentInstance(const int &newCurrentInstanceIndex);

  ~InvidiousInstances();

public slots:
  QString getRandomInstance();
  QString getFirstInstance();
  void reload();
  QString getCurrentInstance();
signals:
  void ready();

private slots:
  void setInstances(const QString &dataStr);
  void loadFallbackInstancesFromLocalDiskOrResources();
  void load();
  void stop();

private:
  QNetworkAccessManager *networkAccessManager = nullptr;
  QNetworkReply *reply = nullptr;
  QStringList instances;
  QString currentInstance;
  QString localInstancesFallbackPath;
  bool instanceBlackListed(QString uri);
  QStringList m_blackListed;
};

class Util {
public:
  static void saveJson(QJsonDocument document, QString fileName) {
    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(document.toJson());
    jsonFile.close();
  }

  static QJsonDocument loadJson(QString fileName) {
    QFile jsonFile(fileName);
    jsonFile.open(QFile::ReadOnly);
    QJsonDocument d = QJsonDocument().fromJson(jsonFile.readAll());
    jsonFile.close();
    return d;
  }
};

#endif // INVIDIOUSINSTANCES_H
