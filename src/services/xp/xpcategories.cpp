#include "xpcategories.h"

XpCategories::XpCategories() {}

QList<QPair<QString, QString>> XpCategories::categories() const {
  return m_categories;
}
