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

#include "tableWidget.h"
#include "downloadmanager.h"
#include "utility.h"

#include <QBuffer>
#include <QHeaderView>

QString tableWidget::thumbnailData(int row) const {
  const auto &s = m_items[static_cast<size_t>(row)];

  if (s.thumbnail.isSet) {

    QBuffer buffer;

    s.thumbnail.image.save(&buffer, "PNG");

    return buffer.buffer().toHex();
  } else {
    return {};
  }
}

QByteArray tableWidget::thumbnailData(const QPixmap &image) {
  QBuffer buffer;

  image.save(&buffer, "PNG");

  return buffer.buffer().toHex();
}

QString tableWidget::engineName() { return QObject::tr("Engine Name:") + " "; }

void tableWidget::setDownloadingOptions(tableWidget::type type, int row,
                                        const QString &mm,
                                        const QString &title) {
  auto optionName = [&]() {
    if (type == tableWidget::type::DownloadOptions) {

      return QObject::tr("Download Options") + ": ";
    } else {
      return tableWidget::engineName();
    }
  }();

  const auto txt = this->uiText(row);

  auto m = [&]() {
    if (title.isEmpty()) {

      return mm;
    } else {
      if (title.size() > 32) {

        return title.mid(0, 32) + " ...";
      } else {
        return title;
      }
    }
  }();

  if (txt.contains(optionName)) {

    auto mm = util::split(txt, '\n', true);

    for (auto &it : mm) {

      if (it.startsWith(optionName)) {

        it = optionName + m;

        break;
      }
    }
    this->setUiText(mm.join('\n'), row);
  } else {
    this->setUiText(optionName + m + "\n" + txt, row);
  }

  auto optText = this->downloadingOptionsUi(row);

  if (optText.contains(optionName)) {

    auto mm = util::split(optText, '\n', true);

    for (auto &it : mm) {

      if (it.startsWith(optionName)) {

        it = optionName + m;

        break;
      }
    }

    this->setDownloadingOptionsUi(mm.join('\n'), row);
  } else {
    if (optText.isEmpty()) {

      this->setDownloadingOptionsUi(optionName + m, row);
    } else {
      this->setDownloadingOptionsUi(optionName + m + "\n" + optText, row);
    }
  }

  if (type == tableWidget::type::DownloadOptions) {

    this->setDownloadingOptions(mm, row);

  } else if (type == tableWidget::type::EngineName) {

    this->setEngineName(mm, row);
  }
}

void tableWidget::setTableWidget(QTableWidget &table,
                                 const tableWidget::tableWidgetOptions &s) {
  table.verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  table.verticalHeader()->setMinimumSectionSize(30);

  table.horizontalHeader()->setStretchLastSection(true);

  table.setMouseTracking(s.mouseTracking);

  table.setContextMenuPolicy(s.customContextPolicy);

  table.setEditTriggers(s.editTrigger);
  table.setFocusPolicy(s.focusPolicy);
  table.setSelectionMode(s.selectionMode);
}

void tableWidget::replace(tableWidget::entry e, int r) {
  auto row = static_cast<size_t>(r);

  m_items[row] = std::move(e);

  auto thumbnailLabel = new QLabel();

  thumbnailLabel->setAlignment(Qt::AlignCenter);

  thumbnailLabel->setPixmap(m_items[row].thumbnail.image);

  m_table.setCellWidget(r, 0, thumbnailLabel);

  m_table.item(r, 1)->setText(m_items[row].uiText);
}

int tableWidget::addRow() {
  auto row = m_table.rowCount();

  m_table.insertRow(row);

  for (int i = 0; i < m_table.columnCount(); i++) {

    auto item = new QTableWidgetItem();
    item->setTextAlignment(Qt::AlignCenter);
    m_table.setItem(row, i, item);
  }

  return row;
}

int tableWidget::addItem(tableWidget::entry e) {

  m_items.emplace_back(std::move(e));

  const auto &entry = m_items.back();

  auto row = m_table.rowCount();

  m_table.insertRow(row);

  auto thumbnailLabel = new QLabel();

  thumbnailLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

  thumbnailLabel->setPixmap(entry.thumbnail.image);

  m_table.setCellWidget(row, 0, thumbnailLabel);

  auto item = new QTableWidgetItem(entry.uiText);

  item->setTextAlignment(entry.alignment);

  m_table.setItem(row, 1, item);

  m_table.resizeRowsToContents();

  return row;
}

void tableWidget::selectRow(QTableWidgetItem *current,
                            QTableWidgetItem *previous, int firstColumnNumber) {
  if (current && previous && previous->row() == current->row()) {

    auto table = current->tableWidget();

    table->setCurrentCell(current->row(), table->columnCount() - 1);
  } else {
    auto _update_table_row = [&](QTableWidgetItem *item, bool setSelected) {
      if (item) {

        auto table = item->tableWidget();

        auto row = item->row();

        auto col = table->columnCount();

        for (int i = firstColumnNumber; i < col; i++) {

          table->item(row, i)->setSelected(setSelected);
        }

        if (setSelected) {

          table->setCurrentCell(row, col - 1);
        }

        table->setFocus();
      }
    };

    _update_table_row(current, true);
    _update_table_row(previous, false);
  }
}

void tableWidget::clear() {
  int m = m_table.rowCount();

  for (int i = 0; i < m; i++) {

    m_table.removeRow(0);
  }

  m_items.clear();
}

void tableWidget::setVisible(bool e) { m_table.setVisible(e); }

int tableWidget::rowCount() const { return m_table.rowCount(); }

void tableWidget::selectLast() {
  if (m_table.rowCount() > 0) {

    m_table.setCurrentCell(m_table.rowCount() - 1, m_table.columnCount() - 1);
    m_table.scrollToBottom();
  }
}

void tableWidget::setEnabled(bool e) { m_table.setEnabled(e); }

int tableWidget::currentRow() const { return m_table.currentRow(); }

void tableWidget::removeRow(int s) {
  m_table.removeRow(s);
  m_items.erase(m_items.begin() + s);
}

bool tableWidget::isSelected(int row) {
  return m_table.item(row, m_init)->isSelected();
}

bool tableWidget::noneAreRunning() {
  for (int i = 0; i < m_table.rowCount(); i++) {

    if (downloadManager::finishedStatus::running(this->runningState(i))) {

      return false;
    }
  }

  return true;
}

QString tableWidget::completeProgress(int) {
  int completed = 0;
  int errored = 0;
  int cancelled = 0;
  int notStarted = 0;

  for (int i = 0; i < m_table.rowCount(); i++) {

    const auto &s = this->runningState(i);

    if (downloadManager::finishedStatus::notStarted(s)) {

      notStarted++;
    }
    if (downloadManager::finishedStatus::finishedWithSuccess(s)) {

      completed++;
    }
    if (downloadManager::finishedStatus::finishedWithError(s)) {

      errored++;
    }
    if (downloadManager::finishedStatus::finishedCancelled(s)) {

      cancelled++;
    }
  }

  auto a = QString::number((completed + errored + cancelled) * 100 /
                           m_table.rowCount());
  auto b = QString::number(notStarted);
  auto c = QString::number(completed);
  auto d = QString::number(errored);
  auto e = QString::number(cancelled);

  if (a == "100.00" || a == "100,00") {
    a = "100";
  }

  return APPLICATION_NAME +
         QObject::tr(
             " | Completed: %1%, Not Started: %2, Succeeded: %3, Failed: "
             "%4, Cancelled: %5")
             .arg(a, b, c, d, e);
}

tableWidget::tableWidget(QTableWidget &t, const QFont &, int init)
    : m_table(t), m_init(init) {
  this->setTableWidget(m_table, tableWidget::tableWidgetOptions());
}

QTableWidgetItem &tableWidget::item(int row, int column) const {
  return *m_table.item(row, column);
}

QTableWidget &tableWidget::get() { return m_table; }
