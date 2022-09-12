#include "supportedsites.h"

static QString toCamelCase(const QString &s) {
  QStringList parts = s.split(' ', Qt::SkipEmptyParts);
  for (int i = 0; i < parts.size(); ++i)
    parts[i].replace(0, 1, parts[i][0].toUpper());

  return parts.join(" ");
}

SupportedSites::SupportedSites(QObject *parent) : QObject{parent} {
  networkManager = new QNetworkAccessManager(this);
  QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
  auto cachePath =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
      QDir::separator() + "nmCache";
  QDir d(cachePath);
  if (d.exists() == false)
    d.mkpath(cachePath);
  diskCache->setCacheDirectory(cachePath);
  networkManager->setCache(diskCache);
  supportedSitesUrl = "https://raw.githubusercontent.com/yt-dlp/yt-dlp/master/"
                      "supportedsites.md";
}

void SupportedSites::get(bool preferCache) {

  supportedSitesList.clear();

  connect(networkManager, &QNetworkAccessManager::finished, this,
          &SupportedSites::supportedSiteReplyFinished);

  QNetworkRequest request((QUrl(supportedSitesUrl)));

  if (preferCache) {
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                         QNetworkRequest::PreferCache);
  } else {
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                         QNetworkRequest::AlwaysNetwork);
  }

  networkManager->get(request);
}

void SupportedSites::supportedSiteReplyFinished(QNetworkReply *rep) {
  if (rep->error() == QNetworkReply::NoError) {
    QString data = rep->readAll();
    if (data.isEmpty() == false) {
      QStringList sites = data.split("- **");
      sites.removeFirst();
      foreach (QString site, sites) {
        auto s = site.split(":").first();
        auto final =
            toCamelCase(s.remove("*").trimmed().simplified().toLower());
        if (s.isEmpty() == false &&
            supportedSitesList.contains(final) == false) {
          supportedSitesList.append(final);
        }
      }
    }
  } else {
    supportedSitesList.clear();
    supportedSitesList.append(rep->errorString());
  }
  emit sitesReady();
}

QStringList &SupportedSites::getSites() { return supportedSitesList; }
