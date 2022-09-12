#ifndef SUPPORTEDSITES_H
#define SUPPORTEDSITES_H

#include <QObject>
#include <QtNetwork>

class SupportedSites : public QObject
{
    Q_OBJECT
public:
    explicit SupportedSites(QObject *parent = nullptr);

public slots:
    void get(bool preferCache = true);
    QStringList &getSites();
signals:
    void sitesReady();
private slots:
    void supportedSiteReplyFinished(QNetworkReply *rep);
private:
    QNetworkAccessManager *networkManager = nullptr;
    QString supportedSitesUrl;
    QStringList supportedSitesList;

};

#endif // SUPPORTEDSITES_H
