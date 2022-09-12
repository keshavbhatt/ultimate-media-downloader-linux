#ifndef INVIDIOUSTRENDINGPARSER_H
#define INVIDIOUSTRENDINGPARSER_H

#include <QObject>

class VideoItem {
public:
  explicit VideoItem(QString title, QString length, QString videoId,
                     QString channelName, QString channelId, QString published,
                     QString views) {
    this->title = title;
    this->length = length;
    this->videoId = videoId;
    this->channelName = channelName;
    this->channelId = channelId;
    this->published = published;
    this->views = views;
  }

  const QString toString() const;

  const QString &getTitle() const;

  const QString &getLength() const;

  const QString &getVideoId() const;

  const QString &getChannelName() const;

  const QString &getChannelId() const;

  const QString &getPublished() const;

  const QString &getViews() const;

private:
  QString title, length, videoId, channelName, channelId, published, views;
};

class InvidiousTrendingParser {

public:
  explicit InvidiousTrendingParser();

  static QList<VideoItem> parseFromHtml(const QString &html);
signals:
};

#endif // INVIDIOUSTRENDINGPARSER_H
