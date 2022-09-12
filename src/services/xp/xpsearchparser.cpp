#include "xpsearchparser.h"

#include "Document.h"
#include "Node.h"

#include <QDebug>

XpSearchParser::XpSearchParser() {}

QList<XVideoItem> XpSearchParser::parseFromHtml(const QString &html) {
  QList<XVideoItem> videos;
  CDocument doc;

  doc.parse(QString(html).toStdString());

  CSelection c =
      doc.find(".list-videos div div.item");

  if (c.nodeNum() > 0) {
    for (unsigned int i = 0; i < c.nodeNum(); ++i) {
      CNode root = c.nodeAt(i);

      QString title, length, views, thumbnail_url, preview_url, video_url;

      // length
      CSelection lengthFind = root.find("a > div.wrap > div.duration");
      if (lengthFind.nodeNum() > 0) {
        length = QString::fromStdString(lengthFind.nodeAt(0).text()).trimmed();
      }

      // title
      CSelection titleFind = root.find("a > strong.title");
      if (titleFind.nodeNum() > 0) {
        title = QString::fromStdString(titleFind.nodeAt(0).text()).trimmed();
      }

      // thumbnail url
      CSelection thumbnailFind = root.find("a > div.img > img");
      if (thumbnailFind.nodeNum() > 0) {
        thumbnail_url = QString::fromStdString(
                            thumbnailFind.nodeAt(0).attribute("data-original"))
                            .trimmed();
      }

      // preview url
      CSelection previewFind = root.find("a > div.img > img");
      if (previewFind.nodeNum() > 0) {
        preview_url = QString::fromStdString(
                          previewFind.nodeAt(0).attribute("data-preview"))
                          .trimmed();
      }

      // views
      CSelection viewsFind = root.find("a > div.wrap > div.views");
      if (viewsFind.nodeNum() > 0) {
        views = QString::fromStdString(viewsFind.nodeAt(0).text()).trimmed();
      }

      // video url
      CSelection urlFind = root.find("a");
      if (urlFind.nodeNum() > 0) {
        video_url =
            QString::fromStdString(urlFind.nodeAt(0).attribute("href")).trimmed();
      }

      XVideoItem v(title, length, views, thumbnail_url, preview_url, video_url);

      videos.append(v);
    }
  }
  return videos;
}

const QString &XVideoItem::getTitle() const { return title; }

const QString &XVideoItem::getLength() const { return length; }

const QString &XVideoItem::getViews() const { return views; }

QString XVideoItem::getThumbnail_url() const { return thumbnail_url; }

QString XVideoItem::getPreview_url() const { return preview_url; }

QString XVideoItem::getVideo_url() const { return video_url; }
