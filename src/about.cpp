#include "about.h"
#include "utility.h"

#include <QDesktopServices>

void about::enableAll() {}

void about::disableAll() {}

void about::resetMenu() {}

about::about(const Context &ctx) : m_ctx(ctx) {

  this->retranslateUi();

  QObject::connect(
      m_ctx.Ui().aboutLabel, &QLabel::linkActivated,
      [=](const QString &link) { QDesktopServices::openUrl(QUrl(link)); });

  QObject::connect(m_ctx.Ui().orion, &QPushButton::clicked, [=]() {
    QDesktopServices::openUrl(QUrl("https://snapcraft.io/orion-desktop"));
  });

  m_ctx.Ui().supportedSitesFilterEdit->setEnabled(false);

  QObject::connect(m_ctx.Ui().refresh, &QPushButton::clicked, [=]() {
    m_ctx.Ui().refresh->setEnabled(false);

    m_ctx.Ui().supportedSitesFilterEdit->setEnabled(false);

    m_ctx.Ui().supportedSitesListWidget->setEnabled(false);

    m_ctx.Ui().supportedSitesListWidget->clear();

    m_ctx.Ui().supportedSitesListWidget->addItem("Loading...");

    supportedSites->get(false);
  });

  QObject::connect(m_ctx.Ui().supportedSitesFilterEdit, &QLineEdit::textChanged,
                   [=](const QString &text) { filterSupportedSites(text); });

  QObject::connect(m_ctx.Ui().supportedSitesListWidget,
                   &QListWidget::itemDoubleClicked, [=](QListWidgetItem *item) {
                     QDesktopServices::openUrl(QUrl(
                         "https://www.google.com/search?q=" + item->text()));
                   });

  supportedSites = new SupportedSites(&m_ctx.mainWidget());

  QObject::connect(supportedSites, &SupportedSites::sitesReady, [=]() {
    m_ctx.Ui().supportedSitesListWidget->clear();

    m_ctx.Ui().supportedSitesListWidget->addItems(supportedSites->getSites());

    m_ctx.Ui().supportedSitesFilterEdit->setEnabled(true);

    m_ctx.Ui().refresh->setEnabled(true);

    m_ctx.Ui().supportedSitesListWidget->setEnabled(true);

    m_ctx.Ui().supportedSitesLabel->setText(
        QString("%1 supported sites")
            .arg(QString::number(supportedSites->getSites().count())));
  });

  supportedSites->get();
}

void about::filterSupportedSites(QString filter_string) {

  hideAllSupportedSites(m_ctx.Ui().supportedSitesListWidget);

  QList<QListWidgetItem *> matches(
      m_ctx.Ui().supportedSitesListWidget->findItems(
          filter_string, Qt::MatchFlag::MatchContains));

  for (QListWidgetItem *item : qAsConst(matches))
    item->setHidden(false);
}

void about::hideAllSupportedSites(QListWidget *listWidget) {

  for (int row(0); row < listWidget->count(); row++)
    listWidget->item(row)->setHidden(true);
}

void about::retranslateUi() {
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

  m_ctx.Ui().aboutLabel->setText(m);
}

void about::tabEntered() {}

void about::tabExited() {}

void about::init_done() {}
