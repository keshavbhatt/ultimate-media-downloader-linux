#ifndef YTVIDEO_H
#define YTVIDEO_H

#include "services/playerservice.h"
#include "services/ytdlgetformat.h"
#include "widgets/remotepixmaplabel2.h"
#include <QClipboard>
#include <QJsonObject>
#include <QWidget>
#include <QtNetwork>

namespace Ui {
class YtVideo;
}

class YtVideo : public QWidget {
  Q_OBJECT

public:
  explicit YtVideo(QWidget *parent = nullptr, QJsonObject obj = QJsonObject(),
                   QNetworkAccessManager *networkAccessManager = nullptr);
  ~YtVideo();

  // keep the order
  enum LinksType { PLAY, PLAY_AUDIO, DOWNLOAD, COPYLINK };

  Q_ENUM(LinksType)

public slots:
  void setStatusLabelText(const QString &txt);
  QString getTitle();
signals:
  void playVideo(const QString &videoLink);
  void playAudio(const QString &videoLink);

private slots:
  void on_linksLabel_linkActivated(const QString &link);

private:
  Ui::YtVideo *ui;
  QNetworkAccessManager *networkAccessManager = nullptr;

  static QString formatSeconds(int seconds) {
    return QDateTime::fromTime_t(seconds).toUTC().toString("hh:mm:ss");
  }

  static QString niceNumber(qint64 views) {

    if (views > 1000000000000)
      return QString::number(double(views) / 1000000000000.0, 'f', 0) + "T";
    else if (views > 1000000000)
      return QString::number(double(views) / 1000000000.0, 'f', 0) + "B";
    else if (views > 1000000)
      return QString::number(double(views) / 1000000.0, 'f', 0) + "M";
    else if (views > 1000)
      return QString::number(double(views) / 1000.0, 'f', 0) + "K";

    return QString::number(views);
  }
};

#endif // YTVIDEO_H
