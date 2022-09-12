#include "invidioustrendingparser.h"
#include "Document.h"
#include "Node.h"
#include <QDebug>

InvidiousTrendingParser::InvidiousTrendingParser() {}

QList<VideoItem> InvidiousTrendingParser::parseFromHtml(const QString &html) {

  QList<VideoItem> videos;
  CDocument doc;

  doc.parse(QString(html).toStdString());

  CSelection c = doc.find(".pure-u-1 .pure-u-md-1-4 > .h-box");

  if (c.nodeNum() > 0) {
    for (unsigned int i = 0; i < c.nodeNum(); ++i) {
      CNode root = c.nodeAt(i);

      QString title, length, videoId, channelName, channelId, published, views;

      // length
      CSelection lengthFind = root.find("p.length");
      if (lengthFind.nodeNum() > 0) {
        length = QString::fromStdString(lengthFind.nodeAt(0).text()).trimmed();
      }

      // title
      CSelection titleFind = root.find("a > p");
      if (titleFind.nodeNum() > 0) {
        title = QString::fromStdString(titleFind.nodeAt(0).text()).trimmed();
      }

      // id
      CSelection idFind = root.find("a");
      if (idFind.nodeNum() > 0) {
        videoId = QString::fromStdString(idFind.nodeAt(0).attribute("href"))
                      .split("watch?v=")
                      .last()
                      .trimmed();
      }

      // channelName
      CSelection channelFind = root.find(".channel-name");
      if (channelFind.nodeNum() > 0) {
        channelName =
            QString::fromStdString(channelFind.nodeAt(0).text()).trimmed();
      }

      // channelId
      if (channelFind.nodeNum() > 0) {
        channelId = QString::fromStdString(
                        channelFind.nodeAt(0).parent().attribute("href"))
                        .split("channel/")
                        .last()
                        .trimmed();
      }

      // shared/published
      CSelection videoDataFind = root.find(".video-data");
      if (videoDataFind.nodeNum() > 1) {
        published = QString::fromStdString(videoDataFind.nodeAt(0).text())
                        .split("Shared")
                        .last()
                        .trimmed();
        views =
            QString::fromStdString(videoDataFind.nodeAt(1).text()).trimmed();
      }

      VideoItem v(title, length, videoId, channelName, channelId, published,
                  views);

      videos.append(v);
    }
  }
  return videos;
}

const QString VideoItem::toString() const {
  return title + "\n" + length + "\n" + videoId + "\n" + channelName + "\n" +
         channelId + "\n" + published + "\n" + views + "\n";
}

const QString &VideoItem::getTitle() const { return title; }

const QString &VideoItem::getLength() const { return length; }

const QString &VideoItem::getVideoId() const { return videoId; }

const QString &VideoItem::getChannelName() const { return channelName; }

const QString &VideoItem::getChannelId() const { return channelId; }

const QString &VideoItem::getPublished() const { return published; }

const QString &VideoItem::getViews() const { return views; }
