#ifndef POSTHOGTRACKINGSERVICE_H
#define POSTHOGTRACKINGSERVICE_H

#include "ITrackingService.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>

class PostHogTrackingService : public ITrackingService {

public:
  PostHogTrackingService(QNetworkAccessManager *networkManager,
                         const QString &host, const QString &apiKey,
                         const QString &distinctId);

  void trackEvent(const QString &eventName, const QJsonObject &properties);

private:
  QNetworkAccessManager *m_networkManager = nullptr;
  QString m_host;
  QString m_apiKey;
  QString m_distinctId;
};

#endif // POSTHOGTRACKINGSERVICE_H
