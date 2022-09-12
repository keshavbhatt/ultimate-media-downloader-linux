#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include "account.h"
#include "context.hpp"
#include <QWidget>

class tabManager;

class accountManager {
public:
  accountManager(const Context &);
  void init_done();
  void enableAll();
  void disableAll();
  void resetMenu();
  void retranslateUi();
  void tabEntered();
  void tabExited();

  QString getAccountId();
  bool isPro();
public slots:
  void showPurchaseMessage();
  bool evaluationUsed();

private:
  const Context &m_ctx;
  account *accountWidget = nullptr;
};

#endif // ACCOUNTMANAGER_H
