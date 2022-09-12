#include "requestinterceptor.h"

#include <QDir>
#include <QStandardPaths>

RequestInterceptor::RequestInterceptor(QObject *p)
    : QWebEngineUrlRequestInterceptor(p) {

  if (m_easyListManager == nullptr) {
    m_easyListManager = new EasyListManager(this);

    connect(m_easyListManager, &EasyListManager::easyListReady, this,
            &RequestInterceptor::init);

    m_easyListManager->init();
  }
}

void RequestInterceptor::init() {

  QThread *thread = QThread::create([this] {
    QFile file(m_easyListManager->getEasyListFilePath());
    QString easyListTxt;

    if (!file.exists()) {
      qDebug() << "[AD_BLOCKER]" << "No easylist.txt file found.";
    } else {
      if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        easyListTxt = file.readAll();
      }
      file.close();

      adBlockRulesLoaded =
          client.parse(easyListTxt.toStdString().c_str(), true);

      if (adBlockRulesLoaded) {
        qDebug() << "[AD_BLOCKER]" << "Rules successfully loaded.";
      } else {
        qWarning() << "[AD_BLOCKER]" << "Failed to load AdBlock rules.";
      }
    }
  });

  connect(thread, &QThread::finished, thread, &QObject::deleteLater);
  thread->start();
}

void RequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info) {
  if (!adBlockRulesLoaded) {
    qDebug() << "[AD_BLOCKER]" << "Client not ready yet";
    return; // Skip blocking if client is not ready
  }

  if (client.matches(info.requestUrl().toString().toStdString().c_str(),
                     FONoFilterOption,
                     info.requestUrl().host().toStdString().c_str())) {
    qDebug() << "[AD_BLOCKER]" << "Blocked: " << info.requestUrl();
    info.block(true);
  }
}
