#ifndef ITRACKINGSERVICE_H
#define ITRACKINGSERVICE_H

#include <QString>
#include <QJsonObject>

class ITrackingService
{
public:
    virtual ~ITrackingService() = default;
    virtual void trackEvent(const QString &eventName, const QJsonObject &properties) = 0;
};

#endif // ITRACKINGSERVICE_H
