#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) : QWidget(parent), ui(new Ui::About) {
  ui->setupUi(this);

  this->setWindowTitle(tr("About") + " " + QApplication::applicationName());

  QObject::connect(
      ui->aboutLabel, &QLabel::linkActivated,
      [=](const QString &link) { QDesktopServices::openUrl(QUrl(link)); });

  QObject::connect(ui->orion, &QPushButton::clicked, [=]() {
    QDesktopServices::openUrl(QUrl("https://snapcraft.io/orion-desktop"));
  });

  ui->supportedSitesFilterEdit->setEnabled(false);

  QObject::connect(ui->refresh, &QPushButton::clicked, [=]() {
    ui->refresh->setEnabled(false);

    ui->supportedSitesFilterEdit->setEnabled(false);

    ui->supportedSitesListWidget->setEnabled(false);

    ui->supportedSitesListWidget->clear();

    ui->supportedSitesListWidget->addItem("Loading...");

    supportedSites->get(false);
  });

  QObject::connect(ui->supportedSitesFilterEdit, &QLineEdit::textChanged,
                   [=](const QString &text) { filterSupportedSites(text); });

  QObject::connect(ui->supportedSitesListWidget,
                   &QListWidget::itemDoubleClicked, [=](QListWidgetItem *item) {
                     emit browserOpenLinkRequested(QUrl(
                         "https://www.google.com/search?q=" + item->text()));
                   });

  supportedSites = new SupportedSites(this);

  QObject::connect(supportedSites, &SupportedSites::sitesReady, [=]() {
    ui->supportedSitesListWidget->clear();

    ui->supportedSitesListWidget->addItems(supportedSites->getSites());

    ui->supportedSitesFilterEdit->setEnabled(true);

    ui->refresh->setEnabled(true);

    ui->supportedSitesListWidget->setEnabled(true);

    ui->supportedSitesLabel->setText(
        QString("%1 supported sites")
            .arg(QString::number(supportedSites->getSites().count())));
  });

  ui->orion->installEventFilter(this);

  showAbout();

  supportedSites->get();
}

void About::filterSupportedSites(QString filter_string) {

  hideAllSupportedSites(ui->supportedSitesListWidget);

  QList<QListWidgetItem *> matches(ui->supportedSitesListWidget->findItems(
      filter_string, Qt::MatchFlag::MatchContains));

  for (QListWidgetItem *item : qAsConst(matches))
    item->setHidden(false);
}

void About::hideAllSupportedSites(QListWidget *listWidget) {

  for (int row(0); row < listWidget->count(); row++)
    listWidget->item(row)->setHidden(true);
}

void About::showAbout() {
  auto version = QObject::tr("Version");

  auto qtVersion = [&]() { return QString(qVersion()); }();

  auto about =
      QString("<p><img src=':/icons/app/icon-64.png'/><br><br> "
              "<b>%1</b></p>%2: %3 <br> <br> %4<br><br> Qt Runtime version: %5"
              "<br><br><a "
              "href='https://github.com/keshavbhatt/"
              "ultimate-media-download-linux/issues'>Report an "
              "Issue</a>"
              "<br><br><a href='https://snapcraft.io/search?q=keshavnrj'>More "
              "Applications</a>");

  auto about_dev = "Designed & developed by Keshav Bhatt";

  auto m = about.arg("Ultimate Media Downloader", version, VERSIONSTR,
                     about_dev, qtVersion);

  ui->aboutLabel->setText(m);
}

bool About::eventFilter(QObject *o, QEvent *e) {
  if (o == ui->orion &&
      (e->type() == QEvent::HoverEnter || e->type() == QEvent::HoverLeave)) {
    if (e->type() == QEvent::HoverLeave) {
      ui->orion->setIcon(QIcon(":/icons/app/orion-64-grayscale.png"));
    } else if (e->type() == QEvent::HoverEnter) {
      ui->orion->setIcon(QIcon(":/icons/app/orion-64.png"));
    }
  }

  return false;
}

About::~About() { delete ui; }
