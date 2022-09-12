#ifndef XPTRIGGERWORDS_H
#define XPTRIGGERWORDS_H

#include <QSet>
#include <QStringList>

class XpTriggerWords {
public:
  XpTriggerWords();

  bool contains(const QString &word);

private:
  QStringList m_baseKeyWords;
  QStringList m_Keywords;
  QStringList m_additionalKeywords;
  QSet<QString> m_combinations;
};

#endif // XPTRIGGERWORDS_H
