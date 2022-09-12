#ifndef XPSEARCH_H
#define XPSEARCH_H

#include "context.hpp"
#include "engines.h"
#include "services/xp/xpsearchservice.h"
#include "xpvideo.h"
#include <QNetworkAccessManager>
#include <QObject>
#include <services/playerservice.h>

class XpSearch : public QObject {
  Q_OBJECT
public:
  explicit XpSearch(const Context &ctx);

  void setSearching(bool newSearching);

  void init_done();
  void tabEntered();
  void tabExited();
  void retranslateUi();
  void disableAll();
  void enableAll();

private:
  XpSearchService *m_searchService = nullptr;

  const Context &m_ctx;
  settings &m_settings;
  Ui::MainWindow &m_ui;
  QWidget &m_mainWindow;
  tabManager &m_tabManager;
  QNetworkAccessManager *m_accessManager = nullptr;
  QString m_engine_path;

  bool m_session_age_restricted_consent_taken = false;

  bool m_searching = false;

  void cancelAllsearchServiceRequests();

  void addPsudoItemToListWidget(const QString &labelStr);

  void addNewVideoItemToListWidget(const QJsonObject &videoObject);

  void playVideo(const QString &videoLink);

  PlayerService *createPlayerService(XpVideo *videoItem);

  void search(const QString &searchTerm);

  void initSearchService();

  void processVideoResults(const QByteArray &data);

  void addNoResultReturnedForQueryToListWidget();

  void addResultInfoHeaderItemToListWidget();

  void processError(const QString &error);

  void playFromUrl(const QString &mediaLink);
};

#endif // XPSEARCH_H
