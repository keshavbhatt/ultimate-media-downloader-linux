#ifndef YTPLAYLIST_H
#define YTPLAYLIST_H

#include "widgets/remotepixmaplabel2.h"
#include <QClipboard>
#include <QJsonObject>
#include <QWidget>
#include <QtNetwork>

namespace Ui {
class YtPlaylist;
}

class YtPlaylist : public QWidget {
  Q_OBJECT

public:
  explicit YtPlaylist(QWidget *parent = nullptr,
                      QJsonObject obj = QJsonObject(),
                      QNetworkAccessManager *networkAccessManager = nullptr);
  ~YtPlaylist();

  enum LinksType { PLAY, DOWNLOAD, COPYLINK };

private slots:
  void on_linksLabel_linkActivated(const QString &link);

private:
  Ui::YtPlaylist *ui;
  QNetworkAccessManager *networkAccessManager = nullptr;

  static QString formatSeconds(int seconds) {
    return QDateTime::fromTime_t(seconds).toUTC().toString("hh:mm:ss");
  }
};

#endif // YTPLAYLIST_H
