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

#ifndef FANCYTABSTYLE_H
#define FANCYTABSTYLE_H

#include <QProxyStyle>

class FancyTabStyle : public QProxyStyle {
  Q_OBJECT
public:
  explicit FancyTabStyle(QObject *parent = nullptr);

  virtual QRect subElementRect(SubElement subElement,
                               const QStyleOption *option,
                               const QWidget *widget) const override;
};

#endif // FANCYTABSTYLE_H
