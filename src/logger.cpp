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

#include "logger.h"

#include "engines.h"

#include "utility.h"

Logger::Logger(QPlainTextEdit &e, QWidget *, settings &s)
    : m_logWindow(nullptr, s, *this), m_textEdit(e) {
  m_textEdit.setReadOnly(true);
}

void Logger::add(const QByteArray &s, int id) {
  if (s.startsWith("[UMD4]")) {

    m_lines.add(s, id);
  } else {
    m_lines.add("[UMD4] " + s, id);
  }

  this->update();
}

void Logger::clear() {
  m_lines.clear();
  m_textEdit.clear();
  m_logWindow.clear();
}

void Logger::showLogWindow() {
  m_logWindow.setText(m_lines.toString());
  m_logWindow.Show();
}

void Logger::updateView(bool e) {
  m_updateView = e;
  this->update();
}

void Logger::update() {
  auto m = m_lines.toString();

  m = m.replace(
      "Confirm you are on the latest version using  yt-dlp -U",
      "\n\nConfirm you are on the latest version, Go to Settings and click "
      "\"Update Engine\"");

  // authentication error
  if (m.contains("Sign in to confirm") || m.contains("User is not entitled") ||
      m.contains("Authentication required") || m.contains("LogIn to access") ||
      m.contains("access denied") ||
      m.contains("401 Unauthorized" || m.contains("Unauthorized"))) {
    m = "ERROR: Unable to download without authentication.\n\nSign-In to the "
        "website with an account from built-in browser or provide "
        "authentication cookie in \"Settings section\" and try again.";
  }

  if (m_updateView) {

    m_textEdit.setPlainText(m);
    m_textEdit.moveCursor(QTextCursor::End);
  }

  m_logWindow.update(m);
}

QList<QByteArray> Logger::Data::toStringList() const {
  return util::split(this->toString(), '\n');
}

bool Logger::Data::postProcessText(const QByteArray &data) {
  return utility::stringConstants::postProcessMarker(data);
}

void Logger::updateLogger::run(bool humanReadableJson, const QByteArray &data) {
  if (m_args.likeYoutubeDl && humanReadableJson) {

    if (data.startsWith('[') || data.startsWith('{')) {

      QJsonParseError err;

      auto json = QJsonDocument::fromJson(data, &err);

      if (err.error == QJsonParseError::NoError) {

        auto s = json.toJson(QJsonDocument::JsonFormat::Indented);

        m_outPut.add(s, m_id);

        return;
      }
    }
  }

  const auto &sp = m_args.splitLinesBy;

  if (sp.size() == 1 && sp[0].size() > 0) {

    this->add(data, sp[0][0]);

  } else if (sp.size() == 2 && sp[0].size() > 0 && sp[1].size() > 0) {

    for (const auto &m : util::split(data, sp[0][0])) {

      this->add(m, sp[1][0]);
    }
  } else {
    for (const auto &m : util::split(data, '\r')) {

      this->add(m, '\n');
    }
  }
}

bool Logger::updateLogger::meetCondition(const QByteArray &l,
                                         const QJsonObject &obj) const {
  const QString line = l;

  if (obj.contains("startsWith")) {

    return line.startsWith(obj.value("startsWith").toString());
  }

  if (obj.contains("endsWith")) {

    return line.endsWith(obj.value("endsWith").toString());
  }

  if (obj.contains("contains")) {

    return line.contains(obj.value("contains").toString());
  }

  if (obj.contains("containsAny")) {

    const auto arr = obj.value("containsAny").toArray();

    for (const auto &it : arr) {

      if (line.contains(it.toString())) {

        return true;
      }
    }

    return false;
  }

  if (obj.contains("containsAll")) {

    const auto arr = obj.value("containsAll").toArray();

    for (const auto &it : arr) {

      if (!line.contains(it.toString())) {

        return false;
      }
    }

    return true;
  }

  return false;
}

bool Logger::updateLogger::meetCondition(const QByteArray &line) const {
  const auto &obj = m_args.controlStructure;

  auto connector = obj.value("Connector").toString();

  if (connector.isEmpty()) {

    auto oo = obj.value("lhs");

    if (oo.isObject()) {

      return this->meetCondition(line, oo.toObject());
    } else {
      return false;
    }
  } else {
    auto obj1 = obj.value("lhs");
    auto obj2 = obj.value("rhs");

    if (obj1.isObject() && obj2.isObject()) {

      auto a = this->meetCondition(line, obj1.toObject());
      auto b = this->meetCondition(line, obj2.toObject());

      if (connector == "&&") {

        return a && b;

      } else if (connector == "||") {

        return a || b;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
}

bool Logger::updateLogger::skipLine(const QByteArray &line) const {
  if (line.isEmpty()) {

    return true;
  } else {
    for (const auto &it : m_args.skipLinesWithText) {

      if (line.contains(it.toUtf8())) {

        return true;
      }
    }

    return false;
  }
}

void Logger::updateLogger::add(const QByteArray &data, QChar token) const {
  for (const auto &e : util::split(data, token)) {

    if (this->skipLine(e)) {

      continue;

    } else if (this->meetCondition(e)) {

      if (m_id == -1) {

        if (m_outPut.isEmpty()) {

          m_outPut.add(e);
        } else {
          auto &s = m_outPut.lastText();

          if (this->meetCondition(s)) {

            m_outPut.replaceLast(e);
          } else {
            m_outPut.add(e);
          }
        }
      } else {
        m_outPut.replaceOrAdd(
            e, m_id,
            [this](const QByteArray &e) { return this->meetCondition(e); },
            [this](const QByteArray &e) {
              if (m_args.likeYoutubeDl) {

                return e.startsWith("[download] 100.0%");
              } else {
                return false;
              }
            });
      }
    } else {
      m_outPut.add(e, m_id);
    }
  }
}
