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

#ifndef ADDTABBUTTON_H
#define ADDTABBUTTON_H

#include <QAbstractButton>
#include <QIcon>

class AddTabButton : public QAbstractButton {
  Q_OBJECT
public:
  explicit AddTabButton(int width, int height, QIcon icon, QWidget *parent = nullptr);

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override { return sizeHint(); }
  void paintEvent(QPaintEvent *event) override;

  void setHoverEnabled(bool enabled);

protected:
  int m_width;
  int m_height;
  QIcon m_newTabIcon;
  bool m_hoverEnabled = {false};
};

#endif // ADDTABBUTTON_H
