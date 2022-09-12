/*
 *  Copyright (c) 2021 Keshav Bhatt
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "accountmanager.h"
#include "moreapps.h"
#include "utility.h"

void accountManager::enableAll() {}

void accountManager::disableAll() {}

void accountManager::resetMenu() {}

accountManager::accountManager(const Context &ctx) : m_ctx(ctx) {
  if (accountWidget == nullptr) {
    accountWidget = new account();
    QObject::connect(accountWidget, &account::showAccountWidget,
                     m_ctx.Ui().tabWidget, [=]() {
                       m_ctx.Ui().tabWidget->setCurrentWidget(
                           m_ctx.Ui().tabAccount);
                     });
  }

  m_ctx.Ui().accountFrame->layout()->addWidget(accountWidget);

  // add moreapps widget
  MoreApps *moreApps =
      new MoreApps(m_ctx.Ui().tabAccount, nullptr, "keshavnrj",
                   QUrl("https://raw.githubusercontent.com/keshavbhatt/appdata/"
                        "main/moreapps.txt"),
                   false, true, 3);
  moreApps->setFixedHeight(110);

  m_ctx.Ui().accountFrame->layout()->addWidget(moreApps);

  this->retranslateUi();
}

QString accountManager::getAccountId() {
  if (accountWidget != nullptr) {
    return accountWidget->getAccountId();
  } else {
    return "null";
  }
}

bool accountManager::isPro() {
  if (accountWidget != nullptr) {
    return accountWidget->isPro();
  } else {
    return false;
  }
}

bool accountManager::evaluationUsed() {
  if (accountWidget != nullptr) {
    return accountWidget
        ->getEvaluation_used(); // we just check evaluation_used since we set it
                                // to false if the account is found pro in
                                // network test
  } else {
    return false;
  }
}

void accountManager::showPurchaseMessage() {
  if (accountWidget != nullptr && accountWidget->getEvaluation_used()) {
    accountWidget->showPurchaseMessage();
  }
}

void accountManager::retranslateUi() {}

void accountManager::tabEntered() {

  if (accountWidget != nullptr) {
    // update the evaluation date time
    if (accountWidget->getEvaluation_used() == false)
      accountWidget->check_evaluation_used();

    // update palette
    QPalette palette = QApplication::palette();
    foreach (QFrame *f, accountWidget->findChildren<QFrame *>()) {
      if (f->frameShape() == QFrame::HLine ||
          f->frameShape() == QFrame::VLine) {
        QPalette customPalette = palette;

        customPalette.setColor(QPalette::Normal, QPalette::WindowText,
                               palette.alternateBase().color());

        f->setPalette(customPalette);
      }
    }
  }
}

void accountManager::tabExited() {}

void accountManager::init_done() {}
