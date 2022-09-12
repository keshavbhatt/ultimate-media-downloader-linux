/*
 * Copyright 2022 Sascha Muenzberg
 * Copyright 2024 Keshav Bhatt <keshavnrj@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FANCYTABWIDGET_H
#define FANCYTABWIDGET_H

#include <QTabWidget>

class FancyTabWidget : public QTabWidget {
  Q_OBJECT
public:
  explicit FancyTabWidget(QWidget *parent = nullptr,
                          const QIcon &addButtonIcon = QIcon());

signals:
  void tabAddRequested();

protected slots:
  void addTabButtonClicked(bool checked);
  void paintEvent(QPaintEvent *event) override;

protected:
  int m_editingTabIndex = {-1};
};

#endif // FANCYTABWIDGET_H
