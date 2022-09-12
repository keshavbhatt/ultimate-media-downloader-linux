#ifndef XPCATEGORIES_H
#define XPCATEGORIES_H

#include <QList>
#include <QPair>
#include <QString>

class XpCategories {
public:
  XpCategories();

    QList<QPair<QString, QString>> m_categories;
    QList<QPair<QString, QString> > categories() const;
};

#endif // XPCATEGORIES_H
