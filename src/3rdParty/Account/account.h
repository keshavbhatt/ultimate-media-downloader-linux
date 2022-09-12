#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "widgets/waitingspinnerwidget.h"
#include <QObject>
#include <QSettings>
#include <QWidget>
#include <QtNetwork>

namespace Ui {
class account;
}

class account : public QWidget {
  Q_OBJECT

public:
  explicit account(QWidget *parent = nullptr);
  ~account();
  QString accountId;
  QSettings settings;
  QString setting_path;

  bool getEvaluation_used() const;

  QString getAccountId() const;

  bool isPro() const;

private:
  bool pro = false;
  bool evaluation_used = false;
  Ui::account *ui;
  qint64 ev_time;
  QString serverUID;
  QString checkUrl;
  QString purchaseUrl;
  WaitingSpinnerWidget *_loader = nullptr;

public slots:
  void check_evaluation_used();
  void showPurchaseMessage();
private slots:
  void createAndSaveAccount();
  void check_purchase_request_done();
  void check_purchased(const QString &username);
  void purchase_checked(const QString &response);
  void check_pro();
  void account_check_failed(const QString &error);
  void on_buy_external_clicked();
  void on_restore_purchase_clicked();
  void write_evaluation_val();

  QString cleanHost(QString hostStr);

  QString readIdFile();
  void save_time_in_settings(QString value);
  QByteArray read_time_value_from_settings();
  void append_time_to_id_file(QString time);
  void log(QVariantList messageStringList);
  void log(QString messageString);
  void on_copyId_clicked();

private:
  QString ipV6;
  QNetworkAccessManager *m_networkManager = nullptr;
  QString idFilePath;

signals:
  void showAccountWidget();
};

#endif // ACCOUNT_H
