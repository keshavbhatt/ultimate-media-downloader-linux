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

#include "fancytabstyle.h"

FancyTabStyle::FancyTabStyle(QObject *parent) : QProxyStyle() {
  setParent(parent);
}

QRect FancyTabStyle::subElementRect(SubElement subElement,
                                    const QStyleOption *option,
                                    const QWidget *widget) const {
  if (subElement == QStyle::SE_TabWidgetRightCorner) {
    // adapted from void QTabWidget::setUpLayout(bool onlyCheck)
    QRect tabRect =
        QProxyStyle::subElementRect(QStyle::SE_TabWidgetTabBar, option, widget);
    QRect rightCornerRect = QProxyStyle::subElementRect(
        QStyle::SE_TabWidgetRightCorner, option, widget);

    rightCornerRect.setRect(tabRect.left() + tabRect.width() + 4, 3,
                            rightCornerRect.width(), rightCornerRect.height());

    return rightCornerRect;
  }

  return QProxyStyle::subElementRect(subElement, option, widget);
}
