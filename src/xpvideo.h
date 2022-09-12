#ifndef XPVIDEO_H
#define XPVIDEO_H

#include "services/playerservice.h"
#include "services/ytdlgetformat.h"
#include "widgets/remotepixmaplabel2.h"

#include <QClipboard>
#include <QJsonObject>
#include <QWidget>
#include <QtNetwork>

namespace Ui {
class XpVideo;
}

class XpVideo : public QWidget {
  Q_OBJECT

public:
  explicit XpVideo(QWidget *parent = nullptr, QJsonObject obj = QJsonObject(),
                   QNetworkAccessManager *networkAccessManager = nullptr);
  ~XpVideo();

  // keep the order
  enum LinksType { PLAY, PLAY_AUDIO, DOWNLOAD, COPYLINK, PLAY_PREVIEW };

  void setStatusLabelText(const QString &txt);
  QString getTitle();
signals:
  void playVideo(const QString &videoLink);
  void playFromUrl(const QString &mediaUrl);

private slots:
  void on_linksLabel_linkActivated(const QString &link);

private:
  Ui::XpVideo *ui;
  QNetworkAccessManager *networkAccessManager = nullptr;
};

#endif // XPVIDEO_H
