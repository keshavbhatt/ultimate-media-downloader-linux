#ifndef ABOUT_H
#define ABOUT_H

#include "supportedsites.h"
#include "utility.h"

#include <QDesktopServices>
#include <QWidget>

namespace Ui {
class About;
}

class About : public QWidget {
  Q_OBJECT

public:
  explicit About(QWidget *parent = nullptr);
  ~About();

protected:
  bool eventFilter(QObject *o, QEvent *e);

signals:
  void browserOpenLinkRequested(QUrl url);

private:
  Ui::About *ui;
  SupportedSites *supportedSites = nullptr;

  void showAbout();
  void hideAllSupportedSites(QListWidget *listWidget);

private slots:
  void filterSupportedSites(QString filter_string);
};

#endif // ABOUT_H
