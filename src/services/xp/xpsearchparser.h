#ifndef XPSEARCHPARSER_H
#define XPSEARCHPARSER_H

#include <QObject>

class XVideoItem {
public:
  explicit XVideoItem(QString title, QString length, QString views,
                      QString thumbnail_url, QString preview_url,
                      QString video_url) {
    this->title = title;
    this->length = length;
    this->views = views;
    this->thumbnail_url = thumbnail_url;
    this->preview_url = preview_url;
    this->video_url = video_url;
  }

  const QString &getTitle() const;

  const QString &getLength() const;

  const QString &getViews() const;

  QString getThumbnail_url() const;

  QString getPreview_url() const;

  QString getVideo_url() const;

private:
  QString title, length, published, views, thumbnail_url, preview_url,
      video_url;
};

class XpSearchParser {
public:
  XpSearchParser();

  static QList<XVideoItem> parseFromHtml(const QString &html);
};

#endif // XPSEARCHPARSER_H
