#include "xpvideo.h"
#include "ui_xpvideo.h"

XpVideo::XpVideo(QWidget *parent, QJsonObject obj,
                 QNetworkAccessManager *networkAccessManager)
    : QWidget(parent), ui(new Ui::XpVideo) {
  ui->setupUi(this);
  this->networkAccessManager = networkAccessManager;

  // normal video item
  QString videoTitle = obj.value("title").toString();
  QString thumbnailUrl = obj.value("thumbnailUrl").toString();
  QString viewCountStr = obj.value("viewCountStr").toString();
  QString lengthStr = obj.value("lengthStr").toString();
  QString videoLink = obj.value("videoLink").toString();
  QString previewUrl = obj.value("previewUrl").toString();

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

  videoMeta += "Duration: " + lengthStr + "<br>";
  videoMeta += "Views: " + viewCountStr + "<br>";

  ui->metaLabel->setText(videoMeta);

  QString linkMeta = R"(
        <span style='font-weight: bold;'>
            <a href="3<:>%1">Copy Video Link</a> &nbsp;
            <a href='4<:>%2'>Preview Video</a>&nbsp;
            <a href='0<:>%3'>Play Video</a>
        </span>
    )";

  ui->linksLabel->setText(linkMeta.arg(videoLink, previewUrl, videoLink));
}

void XpVideo::on_linksLabel_linkActivated(const QString &link) {

  auto action = link.split("<:>").first().toInt();
  auto data = link.split("<:>").last();

  switch (action) {
  case PLAY:
    emit playVideo(data);
    break;
  case DOWNLOAD:
    qDebug() << "download" << data;
    break;
  case COPYLINK: {
    auto m = QApplication::clipboard();
    m->setText(data);
  } break;
  case PLAY_PREVIEW: {
    emit playFromUrl(data);
  } break;
  default:
    qDebug() << "unhandled link" << data;
    break;
  }
}

XpVideo::~XpVideo() { delete ui; }

void XpVideo::setStatusLabelText(const QString &txt) {
  ui->statusLabel->setText(txt);
}

QString XpVideo::getTitle() { return ui->titileLabel->text(); }
