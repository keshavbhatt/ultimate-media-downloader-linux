#include "ytvideo.h"
#include "ui_ytvideo.h"

YtVideo::YtVideo(QWidget *parent, QJsonObject obj,
                 QNetworkAccessManager *networkAccessManager)
    : QWidget(parent), ui(new Ui::YtVideo) {
  ui->setupUi(this);
  this->networkAccessManager = networkAccessManager;

  // normal video item
  QString videoTitle = obj.value("title").toString();
  QString videoAuthor = obj.value("author").toString();
  qint32 lengthSeconds = obj.value("lengthSeconds").toInt();
  qint64 videoViews = obj.value("viewCount").toInt();
  QString publishedText = obj.value("publishedText").toString();
  QString videoId = obj.value("videoId").toString();
  QString videoAuthorId = obj.value("authorId").toString();

  // trending video item
  QString viewCountStr = obj.value("viewCountStr").toString();
  QString lengthStr = obj.value("lengthStr").toString();

  QString thumbnailUrl = "https://i.ytimg.com/vi/" + videoId + "/mqdefault.jpg";

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
  ui->titileLabel->setText(videoTitle);
  ui->titileLabel->setToolTip(videoTitle);

  QString videoMeta;
  // trending item data
  if (obj.value("lengthStr").isUndefined() == false) {
    videoMeta += "Duration: " + lengthStr + "<br>";
    videoMeta += "By: " + videoAuthor + "<br>";
    videoMeta += "Views: " + viewCountStr + "<br>";
    videoMeta += "Publised: " + publishedText;
  } else {
    videoMeta += "Duration: " + formatSeconds(lengthSeconds) + "<br>";
    videoMeta += "By: " + videoAuthor + "<br>";
    videoMeta += "Views: " + niceNumber(videoViews) + "<br>";
    videoMeta += "Publised: " + publishedText;
  }

  ui->metaLabel->setText(videoMeta);

  QString linkMeta = R"(
        <span style='font-weight: bold;'>
            <a href="3<:>%1">Copy Video Link</a> &nbsp;
            <a href="3<:>%2">Copy Channel link</a>&nbsp;
            <a href='1<:>%3'>Play Audio</a>&nbsp;
            <a href='0<:>%3'>Play Video</a>
        </span>
    )";

  auto videoLink = "https://youtube.com/watch?v=" + videoId;
  auto channelLink = "https://youtube.com/channel/" + videoAuthorId;

  ui->linksLabel->setText(linkMeta.arg(videoLink, channelLink, videoLink));
}

YtVideo::~YtVideo() { delete ui; }

void YtVideo::on_linksLabel_linkActivated(const QString &link) {

  auto action = link.split("<:>").first().toInt();
  auto data = link.split("<:>").last();

  switch (action) {
  case PLAY:
    emit playVideo(data);
    break;
  case PLAY_AUDIO:
    emit playAudio(data);
    break;
  case DOWNLOAD:
    qDebug() << "download" << data;
    break;
  case COPYLINK: {
    auto m = QApplication::clipboard();
    m->setText(data);
  } break;
  default:
    qDebug() << "unhandled link" << data;
    break;
  }
}

void YtVideo::setStatusLabelText(const QString &txt) {
  ui->statusLabel->setText(txt);
}

QString YtVideo::getTitle() { return ui->titileLabel->text(); }
