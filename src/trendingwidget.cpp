#include "trendingwidget.h"
#include "ui_trendingwidget.h"

#include <QFile>
#include <QStandardItem>
#include <QTextStream>

TrendingWidget::TrendingWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::TrendingWidget) {
  ui->setupUi(this);

  // init
  loadRegions();
  int regionIndex = settings.value("ytTrendingRegionIndex", 149).toInt();
  ui->regionComboBox->setCurrentIndex(regionIndex);
}

// regions
void TrendingWidget::loadRegions() {
  ui->regionComboBox->blockSignals(true);
  m_lpRegionModel = new QStandardItemModel(0, 1);
  ui->regionComboBox->setModel(m_lpRegionModel);

  QFile file("://others/regions");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine();
    QStringList itemdata = QString(line).split(":");
    QStandardItem *lpItem = new QStandardItem(itemdata.first());
    lpItem->setIcon(QIcon(":/icons/flags/" + itemdata.last() + ".png"));
    lpItem->setData(itemdata.last());
    m_lpRegionModel->appendRow(lpItem);
  }
  file.close();
  ui->regionComboBox->blockSignals(false);

  connect(this, &TrendingWidget::loadRequested, [=]() { this->close(); });
}

QString TrendingWidget::getCurrentRegion() {

  QStandardItem *lpItemRegion =
      m_lpRegionModel->item(ui->regionComboBox->currentIndex());
  return lpItemRegion->data().toString();
}

void TrendingWidget::on_regionComboBox_currentIndexChanged(int index) {
  settings.setValue("ytTrendingRegionIndex", index);
}

TrendingWidget::~TrendingWidget() { delete ui; }

void TrendingWidget::on_defaultPb_clicked() {
  emit loadRequested(YtSearchService::SearchMode::TRENDING_DEFAULT,
                     getCurrentRegion());
}

void TrendingWidget::on_moviesPb_clicked() {
  emit loadRequested(YtSearchService::SearchMode::TRENDING_MOVIES,
                     getCurrentRegion());
}

void TrendingWidget::on_gamingPb_clicked() {
  emit loadRequested(YtSearchService::SearchMode::TRENDING_GAMES,
                     getCurrentRegion());
}

void TrendingWidget::on_musicPb_clicked() {
  emit loadRequested(YtSearchService::SearchMode::TRENDING_MUSIC,
                     getCurrentRegion());
}

void TrendingWidget::on_resetRegion_clicked() {
  ui->regionComboBox->setCurrentIndex(149);
}

void TrendingWidget::showEvent(QShowEvent *event) {
  fixVlineStyle();
  QWidget::showEvent(event);
}

void TrendingWidget::fixVlineStyle() {
  auto appPalette = QApplication::palette();
  foreach (QFrame *f, this->findChildren<QFrame *>()) {
    if (f->frameShape() == QFrame::HLine || f->frameShape() == QFrame::VLine) {
      QPalette customPalette = appPalette;
      customPalette.setColor(QPalette::Normal, QPalette::WindowText,
                             appPalette.alternateBase().color());
      f->setPalette(customPalette);
    }
  }
}

void TrendingWidget::on_moviesPb_2_clicked() { this->close(); }
