#include "ytplaylist.h"
#include "ui_ytplaylist.h"

YtPlaylist::YtPlaylist(QWidget *parent, QJsonObject obj,
                       QNetworkAccessManager *networkAccessManager)
    : QWidget(parent), ui(new Ui::YtPlaylist) {
  ui->setupUi(this);
  this->networkAccessManager = networkAccessManager;

  QString playlistTitle = obj.value("title").toString();
  QString playlistAuthor = obj.value("author").toString();
  int videoCount = obj.value("videoCount").toInt();
  QString playlistId = obj.value("playlistId").toString();
  QString playlistAuthorId = obj.value("authorId").toString();
  QString playlistThumbnail = obj.value("playlistThumbnail").toString();
  QString thumbnailUrl = playlistThumbnail.replace("hqdefault", "mqdefault");

  QJsonArray videoArray = obj.value("videos").toArray();
  QString videoMetaStr;
  foreach (const QJsonValue &videoVal, videoArray) {
    QJsonObject videoObject = videoVal.toObject();
    QString videoTitle = videoObject.value("title").toString();
    int lengthSeconds = videoObject.value("lengthSeconds").toInt();
    // QString videoId = videoObject.value("videoId").toString();

    videoMetaStr.append(videoTitle + " (" + formatSeconds(lengthSeconds) +
                        ")<br>");
  }

  double ratio = 320.0 / 180.0;
  double height = this->height();
  double width = ratio * height;
  ui->thumbnail->setFixedSize(width, height);
  ui->thumbnail->setPixmap(QPixmap(":/icons/video.png")
                               .scaled(ui->thumbnail->size(),
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));

  ui->thumbnail->init(this->networkAccessManager, thumbnailUrl,
                      "qrc:///icons/video.png");

  // elided
  ui->titleLabel->setText(playlistTitle);
  ui->titleLabel->setToolTip(playlistTitle);

  QString playlistMeta;
  playlistMeta += "By: " + playlistAuthor + "<br>";
  playlistMeta += "Videos: " + QString::number(videoCount) + "<br>";

  auto li = videoMetaStr.lastIndexOf("<br>");

  playlistMeta += videoMetaStr.leftRef(li);

  ui->metaLabel->setText(playlistMeta);

  QString linkMeta = R"(
        <span style='font-weight: bold;'>
            <a href="2<:>%1">Copy Playlist Link</a> &nbsp;
            <a href="2<:>%2">Copy Author link</a>
        </span>
    )";

  auto playlistLink = "https://www.youtube.com/playlist?list=" + playlistId;
  auto playlistAuthorLink =
      "https://www.youtube.com/channel/" + playlistAuthorId + "/playlists";

  ui->linksLabel->setText(linkMeta.arg(playlistLink, playlistAuthorLink));
}

YtPlaylist::~YtPlaylist() { delete ui; }

void YtPlaylist::on_linksLabel_linkActivated(const QString &link) {
  auto first = link.split("<:>").first().toInt();
  auto second = link.split("<:>").last();

  switch (first) {
  case PLAY:
    qDebug() << "play" << second;
    break;
  case DOWNLOAD:
    qDebug() << "download" << second;
    break;
  case COPYLINK: {
    auto m = QApplication::clipboard();
    m->setText(second);
  } break;
  default:
    qDebug() << "unhandled link" << second;
    break;
  }
}
