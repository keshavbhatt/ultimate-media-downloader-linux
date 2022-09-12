#include "cookiedialog.h"

CookieDialog::CookieDialog(const QVector<QNetworkCookie> &cookies,
                           QWidget *parent)
    : QDialog(parent) {
  QVBoxLayout *layout = new QVBoxLayout(this);

  QListWidget *cookieList = new QListWidget(this);
  for (const QNetworkCookie &cookie : cookies) {
    cookieList->addItem(cookie.toRawForm());
  }

  QPushButton *closeButton = new QPushButton("Close", this);

  layout->addWidget(cookieList);
  layout->addWidget(closeButton);

  connect(closeButton, &QPushButton::clicked, this, &CookieDialog::accept);
}
