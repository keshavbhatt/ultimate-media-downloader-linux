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

#include "addtabbutton.h"

#include <QPainter>
#include <QStyleOption>

AddTabButton::AddTabButton(int width, int height, QIcon icon, QWidget *parent)
    : QAbstractButton(parent), m_width(width), m_height(height),
      m_newTabIcon(icon) {
  setFocusPolicy(Qt::NoFocus);
  setCursor(Qt::ArrowCursor);
  setToolTip(tr("New tab"));

  ensurePolished();
  resize(QSize(m_width, m_height));
}

QSize AddTabButton::sizeHint() const {
  ensurePolished();
  return QSize(m_width, m_height);
}

void AddTabButton::paintEvent(QPaintEvent *) {
  // adapted from qt5/qtbase/src/widgets/qtabbar.cpp: void
  // CloseButton::paintEvent(QPaintEvent *)
  QPainter p(this);
  QStyleOption opt;
  opt.init(this);

  if ((opt.state & QStyle::State_Enabled) &&
      (opt.state & QStyle::State_MouseOver) && m_hoverEnabled) {
    style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, &p, this);
  }

  // adapted from qt5/qtbase/src/widgets/styles/qfusionstyle.cpp:
  // drawPrimitive(QStyle::PE_IndicatorTabClose, ...

  if (m_newTabIcon.isNull()) {
    m_newTabIcon =
        QIcon::fromTheme(QLatin1String("list-add-symbolic.symbolic"));
  }

  int width =
      style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, nullptr, this);
  int height =
      style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, nullptr, this);

  QPixmap pixmap =
      m_newTabIcon.pixmap(QSize(width, height), QIcon::Normal, QIcon::On);
  style()->drawItemPixmap(&p, opt.rect, Qt::AlignCenter, pixmap);
}

void AddTabButton::setHoverEnabled(bool enabled) { m_hoverEnabled = enabled; }
