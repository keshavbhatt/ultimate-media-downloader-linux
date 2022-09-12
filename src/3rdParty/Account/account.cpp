#include "account.h"
#include "ui_account.h"
#include "utils.h"
#include <QClipboard>
#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStyle>
#include <QTextCodec>

account::account(QWidget *parent) : QWidget(parent), ui(new Ui::account) {
  ui->setupUi(this);

  idFilePath =
      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) +
      "/." + QApplication::applicationName() + ".id";

  m_networkManager = new QNetworkAccessManager(this);

  ipV6 = "cc67:6743:d6e5:6902:3f7a:0e4f:bef6:eec5";

  serverUID = "UMD2";

  checkUrl =
      "http://www.ktechpit.com/USS/" + serverUID + "/check/check.php?username=";

  purchaseUrl =
      "http://ktechpit.com/USS/" + serverUID + "/paypal/index.php?accountId=";

  ev_time = 15 * 86400;

  _loader = new WaitingSpinnerWidget(ui->status, true, false);
  _loader->setRoundness(70.0);
  _loader->setMinimumTrailOpacity(15.0);
  _loader->setTrailFadePercentage(70.0);
  _loader->setNumberOfLines(10);
  _loader->setLineLength(8);
  _loader->setLineWidth(2);
  _loader->setInnerRadius(2);
  _loader->setRevolutionsPerSecond(3);
  _loader->setColor(QColor(30, 144, 255));

  setting_path =
      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

  if (settings.value("accountId").isValid()) {
    accountId = settings.value("accountId").toString();
  }

  if (accountId.isEmpty()) {

    QString data = readIdFile();

    if (data.trimmed().isEmpty() == false) {

      accountId = QString(data).split("\n").first();

      if (accountId.trimmed().isEmpty() == false) {
        settings.setValue("accountId", accountId);
      }

      QString emit_val = QString(data).split("\n").last();

      if (emit_val.trimmed().isEmpty() == false) {
        settings.setValue(
            QApplication::applicationName() + "_emit",
            QByteArray::fromBase64(QByteArray(emit_val.toUtf8())).toBase64());
      }
    } else {
      createAndSaveAccount();
    }
  }

  ui->id->setText(accountId);

  check_pro();

  check_purchased(accountId);

  write_evaluation_val();

  check_evaluation_used();
}

QString account::readIdFile() {

  QString data;

  QFile idFile(idFilePath);

  if (!idFile.open(QIODevice::ReadOnly)) {

    log("unable to open id file");

  } else {
    QDataStream in(&idFile);

    in >> data;

    idFile.close();
  }
  return data;
}

void account::createAndSaveAccount() {

  accountId = utils::generateRandomId(20);

  settings.setValue("accountId", accountId);

  QFile file(idFilePath);

  if (!file.open(QIODevice::WriteOnly)) {

    log("unable to create account file");

  } else {

    QDataStream out(&file);

    out << accountId;

    file.close();
  }
}

void account::check_purchased(const QString &username) {

  ui->status->clear();

  ui->restore_purchase->setText("Checking..");

  _loader->start();

  ui->restore_purchase->setEnabled(false);

  QNetworkRequest accountCheckRequest(QUrl(checkUrl + username));

  QNetworkReply *reply = m_networkManager->get(accountCheckRequest);

  connect(reply, SIGNAL(finished()), this, SLOT(check_purchase_request_done()));

  log(QVariantList() << "checking account for" << username);
}

void account::check_purchase_request_done() {

  _loader->stop();

  log("check purchase request completed");

  QNetworkReply *reply = ((QNetworkReply *)sender());

  QNetworkRequest request = reply->request();

  QString username = request.url().toString().split("username=").last();

  QByteArray ans = reply->readAll();

  QString s_data = QTextCodec::codecForMib(106)->toUnicode(ans);

  if (reply->error() == QNetworkReply::NoError) {

    purchase_checked(username + " - " + s_data);

  } else {

    if (settings.value(QApplication::applicationName()).isValid()) {

      QByteArray b =
          settings.value(QApplication::applicationName()).toByteArray();

      if (QByteArray::fromBase64(b) == "safeyagsa") {
        this->check_pro();
        settings.setValue(QApplication::applicationName(),
                          QString("omelap").toUtf8().toBase64());
      } else {

        purchase_checked(username + " - " + cleanHost(reply->errorString()));
      }
    }
    account_check_failed(cleanHost(reply->errorString()));
  }

  reply->deleteLater();
}

QString account::cleanHost(QString hostStr) {

  return hostStr.replace("www.ktechpit.com", ipV6)
      .remove("http://")
      .remove("USS/");
}

void account::account_check_failed(const QString &error) {

  ui->status->setText(error);

  ui->restore_purchase->setText("Restore Purchase");

  ui->restore_purchase->setEnabled(true);
}

void account::purchase_checked(const QString &response) {

  log(QVariantList() << "account found" << response);

  ui->restore_purchase->setText("Restore Purchase");

  ui->restore_purchase->setEnabled(true);

  if (response.contains("Account is active", Qt::CaseInsensitive)) {

    ui->acc_type->setText("Pro");

    ui->restore_purchase->setEnabled(false);

    settings.setValue(QApplication::applicationName(),
                      QString("sdfetegsa").toUtf8().toBase64());

    check_pro();

  } else {

    settings.setValue(QApplication::applicationName(),
                      QString("sfjhkfngkj").toUtf8().toBase64());

    ui->acc_type->setText("Evaluation");

    ui->restore_purchase->setText("Restore Account");

    ui->restore_purchase->setEnabled(true);

    check_pro();

    if (response.contains("plan expired on")) {
      QMessageBox msgBox;

      msgBox.setText("Account Type Evaluation");

      msgBox.setInformativeText(response);

      msgBox.setStandardButtons(QMessageBox::Cancel);

      msgBox.setWindowModality(Qt::ApplicationModal);

      QPushButton *account =
          msgBox.addButton(tr("Purchase Licence"), QMessageBox::YesRole);

      msgBox.setDefaultButton(account);

      msgBox.exec();

      if (msgBox.clickedButton() == account) {

        emit showAccountWidget();

        ui->buy_external->click();
      }
    }
  }

  ui->status->setText(response);
}

void account::check_pro() {

  if (settings.value(QApplication::applicationName()).isValid()) {

    QByteArray b =
        settings.value(QApplication::applicationName()).toByteArray();

    if (QByteArray::fromBase64(b) == "sfjhkfngkj") {

      evaluation_used = false;

      this->setWindowTitle(QApplication::applicationName() + " Pro");

      ui->acc_type->setText("Pro");

      ui->ev_label->hide();

      ui->time_left->hide();

      ui->buy_external->setEnabled(false);

      ui->restore_purchase->setEnabled(false);

    } else {

      if (settings.value(QApplication::applicationName()).isValid()) {

        QByteArray b =
            settings.value(QApplication::applicationName()).toByteArray();

        if (QByteArray::fromBase64(b) == "sfjhkfngkj") {

          check_evaluation_used();
        }
      }

      pro = false;

      this->setWindowTitle(QApplication::applicationName() + " Evaluation");

      ui->acc_type->setText("Evaluation");

      ui->buy_external->setEnabled(true);

      ui->restore_purchase->setEnabled(true);
    }
  }
}

void account::save_time_in_settings(QString value) {
  settings.setValue(QApplication::applicationName() + "_emit",
                    QByteArray(value.toUtf8()).toBase64());
}

QByteArray account::read_time_value_from_settings() {
  return QByteArray::fromBase64(
      settings.value(QApplication::applicationName() + "_emit").toByteArray());
}

// write once
void account::write_evaluation_val() {

  QFile dbnFile(setting_path + "/.dbn");

  if (!dbnFile.open(QIODevice::ReadWrite))
    return;

  QString time;

  QDataStream in(&dbnFile);

  in >> time;

  if (settings.value(QApplication::applicationName() + "_emit").isValid() ==
          false &&
      time.trimmed().isEmpty()) {

    save_time_in_settings(
        QString::number(QDateTime::currentMSecsSinceEpoch() / 1000));

    QDataStream out(&dbnFile);

    QString time_value_from_settings = read_time_value_from_settings();

    log(QVariantList() << "TIME VALUE FROM SETTINGS"
                       << time_value_from_settings);

    out << time_value_from_settings;

    dbnFile.close();

    append_time_to_id_file(time_value_from_settings);

  } else {

    save_time_in_settings(time);
  }
}

void account::append_time_to_id_file(QString time) {

  QFile idFile(idFilePath);

  if (!idFile.open(QIODevice::Append)) {

    return;

  } else {

    QDataStream out(&idFile);

    out << time;

    idFile.close();
  }
}

void account::log(QVariantList messageStringList) {

  QString out;

  for (QVariantList::iterator j = messageStringList.begin();
       j != messageStringList.end(); j++) {
    out += (out.isEmpty() ? "" : " ") + (*j).toString();
  }

  qDebug() << "ACC:" << out;
}

void account::log(QString messageString) {
  qDebug() << "ACC:" << messageString;
}

void account::check_evaluation_used() {

  log("checking evaluation used");

  QString time_from_settings = read_time_value_from_settings();

  if (pro == false) {

    if (QDateTime::currentSecsSinceEpoch() >
        time_from_settings.toLongLong() + ev_time) {

      evaluation_used = true;

    } else {

      evaluation_used = false;
    }
  }
  log(QVariantList() << "evaluatio used =" << evaluation_used);

  if (!pro) {

    if (settings.value(QApplication::applicationName() + "_emit").isValid()) {

      if (evaluation_used == true) {

        ui->time_left->setText("0 days 0 hours 0 minutes");

      } else {

        ui->time_left->setText(
            utils::convertSectoDay(QDateTime::currentSecsSinceEpoch() -
                                   (ev_time + time_from_settings.toLongLong()))
                .remove("-")
                .split("minutes")
                .first() +
            "minutes");
      }
    } else {

      ui->time_left->setText(
          utils::convertSectoDay(ev_time).remove("-").split("minutes").first() +
          "minutes");
    }
  }

  // unused
  if (!pro && evaluation_used) {
    // do something emit signal to tell user must purchase etc
  }

  if (pro) {
    ui->ev_label->hide();
    ui->time_left->hide();
  } else {
    ui->ev_label->show();
    ui->time_left->show();
  }
}

account::~account() { delete ui; }

QString account::getAccountId() const { return accountId; }

bool account::isPro() const { return pro; }

bool account::getEvaluation_used() const { return evaluation_used; }

void account::on_buy_external_clicked() {

  QString linkStr = purchaseUrl + accountId;

  QApplication::clipboard()->setText(linkStr);

  QDesktopServices::openUrl(QUrl(linkStr));
}

void account::on_restore_purchase_clicked() { check_purchased(accountId); }

void account::showPurchaseMessage() {

  QMessageBox msgBox;

  msgBox.setWindowTitle(QApplication::applicationName() + " | Account");

  msgBox.setText("Evaluation period ended, Upgrade to Pro");

  msgBox.setInformativeText(
      "This feature is not availabe after free trail (evaluation version) of "
      "application has been used.\n\nPlease purchase a licence to continue "
      "your support in "
      "development of application.");

  msgBox.setStandardButtons(QMessageBox::Cancel);

  msgBox.setWindowModality(Qt::ApplicationModal);

  QPushButton *account =
      msgBox.addButton(tr("Purchase Licence"), QMessageBox::YesRole);

  QIcon icon = QIcon::fromTheme("gtk-yes", QIcon(":/icons/accept.png"));
  account->setIcon(icon);

  msgBox.setDefaultButton(account);

  msgBox.exec();

  if (msgBox.clickedButton() == account) {

    emit showAccountWidget();

    ui->buy_external->click();

  } else {

    msgBox.close();
  }
}

void account::on_copyId_clicked() {
  qApp->clipboard()->setText(ui->id->text());
}
