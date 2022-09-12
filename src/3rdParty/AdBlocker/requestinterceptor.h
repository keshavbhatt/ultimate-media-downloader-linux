#ifndef REQUESTINTERCEPTOR_H
#define REQUESTINTERCEPTOR_H

#include <QDebug>
#include <QFile>
#include <QThread>
#include <QWebEngineUrlRequestInterceptor>

#include "3rdParty/AdBlocker/easylistmanager.h"
#include "ad_block_client.h"

class RequestInterceptor : public QWebEngineUrlRequestInterceptor {
  Q_OBJECT

public:
  RequestInterceptor(QObject *p = nullptr);

private slots:
  void init();

private:
  AdBlockClient client;
  EasyListManager *m_easyListManager = nullptr;
  bool adBlockRulesLoaded = false;

public:
  void interceptRequest(QWebEngineUrlRequestInfo &info);
};

#endif // REQUESTINTERCEPTOR_H
