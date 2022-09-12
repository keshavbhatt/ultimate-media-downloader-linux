#include "posthogtrackingservice.h"

PostHogTrackingService::PostHogTrackingService(
    QNetworkAccessManager *networkManager, const QString &host,
    const QString &apiKey, const QString &distinctId)
    : m_networkManager(networkManager), m_host(host), m_apiKey(apiKey),
      m_distinctId(distinctId) {

  if (m_networkManager == nullptr) {
    m_networkManager = new QNetworkAccessManager();
    m_networkManager->setObjectName("ph_internal_nm");
  }
}

void PostHogTrackingService::trackEvent(const QString &eventName,
                                        const QJsonObject &properties) {
  Q_UNUSED(eventName);
  Q_UNUSED(properties);
}
