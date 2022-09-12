#ifndef TRENDINGWIDGET_H
#define TRENDINGWIDGET_H

#include "services/ytsearchservice.h"
#include <QSettings>
#include <QStandardItemModel>
#include <QWidget>

namespace Ui {
class TrendingWidget;
}

class TrendingWidget : public QWidget {
  Q_OBJECT

public:
  explicit TrendingWidget(QWidget *parent = nullptr);
  ~TrendingWidget();

public slots:
    void fixVlineStyle();
private slots:
  void loadRegions();
  void on_regionComboBox_currentIndexChanged(int index);
  void on_defaultPb_clicked();

  void on_moviesPb_clicked();

  void on_gamingPb_clicked();

  void on_musicPb_clicked();

  QString getCurrentRegion();

  void on_resetRegion_clicked();

  void on_moviesPb_2_clicked();

private:
  Ui::TrendingWidget *ui;
  QSettings settings;
  QStandardItemModel *m_lpRegionModel = nullptr;

signals:
  void loadRequested(const YtSearchService::SearchMode &searchMode =
                         YtSearchService::SearchMode::TRENDING_MUSIC,
                     const QString &region = "US");
protected slots:
  void showEvent(QShowEvent *event);
};

#endif // TRENDINGWIDGET_H
