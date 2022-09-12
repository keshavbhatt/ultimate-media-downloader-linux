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

#include "logwindow.h"
#include "ui_logwindow.h"

#include "logger.h"
#include "settings.h"
#include "utility.h"

logWindow::logWindow(QWidget *parent, settings &s, Logger &logger)
    : QWidget(parent), m_ui(new Ui::logWindow), m_settings(s) {
  m_ui->setupUi(this);

  m_ui->plainTextEdit->setReadOnly(true);

  connect(m_ui->pbClose, &QPushButton::clicked, [this]() { this->Hide(); });

  connect(m_ui->pbClear, &QPushButton::clicked,
          [&logger]() { logger.clear(); });
}

logWindow::~logWindow() { delete m_ui; }

void logWindow::setText(const QByteArray &e) {
  m_ui->plainTextEdit->setPlainText(e);
  m_ui->plainTextEdit->moveCursor(QTextCursor::End);
}

void logWindow::update(const QByteArray &e) {
  if (this->isVisible()) {

    this->setText(e);
  }
}

void logWindow::Hide() {
  const auto &r = this->window()->geometry();

  auto x = QString::number(r.x());
  auto y = QString::number(r.y());
  auto w = QString::number(r.width());
  auto h = QString::number(r.height());

  m_settings.setWindowDimensions("LogWindow", x + "-" + y + "-" + w + "-" + h);

  this->hide();
  this->clear();
}

void logWindow::Show() {
  auto w = m_settings.windowsDimensions("LogWindow");

  if (!w.isEmpty()) {

    auto m = util::split(w, '-', true);

    if (m.size() == 4) {

      QRect r;

      r.setX(m.at(0).toInt());
      r.setY(m.at(1).toInt());
      r.setWidth(m.at(2).toInt());
      r.setHeight(m.at(3).toInt());

      this->window()->setGeometry(r);
    }
  }

  this->show();
}

void logWindow::clear() { m_ui->plainTextEdit->clear(); }

void logWindow::closeEvent(QCloseEvent *e) {
  e->ignore();
  this->Hide();
}
