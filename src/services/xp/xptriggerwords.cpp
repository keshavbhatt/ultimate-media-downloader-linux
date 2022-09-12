#include "xptriggerwords.h"

XpTriggerWords::XpTriggerWords() {
  // Pre-calculate all possible combinations of base keywords with
  // additional keywords inserted between spaces
  for (const QString &baseKeyword : qAsConst(m_baseKeyWords)) {
    for (const QString &additionalKeyword : qAsConst(m_additionalKeywords)) {
      m_combinations.insert(baseKeyword + " " + additionalKeyword);
      m_combinations.insert(additionalKeyword + " " + baseKeyword);
    }
  }
}

bool XpTriggerWords::contains(const QString &word) {

  QString lowerWord = word.toLower();

  // check by splitting word
  if (lowerWord.contains(" ")) {
    auto parts = lowerWord.split(" ");
    auto found = false;
    foreach (auto part, parts) {
      if (m_Keywords.contains(part)) {
        found = true;
        break;
      }
    }
    if (found) {
      return true;
    }
  }

  // check whole word in both dictionaries
  return m_Keywords.contains(lowerWord) ||
         m_combinations.contains(lowerWord);
}
