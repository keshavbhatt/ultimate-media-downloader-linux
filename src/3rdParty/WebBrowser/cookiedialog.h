#ifndef COOKIEDIALOG_H
#define COOKIEDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QNetworkCookie>
#include <QPushButton>
#include <QVBoxLayout>

class CookieDialog : public QDialog {
  Q_OBJECT

public:
  explicit CookieDialog(const QVector<QNetworkCookie> &cookies,
                        QWidget *parent = nullptr);
};

#endif // COOKIEDIALOG_H
