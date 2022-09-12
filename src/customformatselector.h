#ifndef CUSTOMFORMATSELECTOR_H
#define CUSTOMFORMATSELECTOR_H

#include <QSettings>
#include <QTableWidget>
#include <QWidget>

namespace Ui {
class CustomFormatSelector;
}

class CustomFormatSelector : public QWidget {
  Q_OBJECT

public:
  explicit CustomFormatSelector(QWidget *parent, QTableWidget &);
  ~CustomFormatSelector();

signals:
  void formatSelected(const QString &format);

protected slots:
private slots:
  void populateView();
  void updateAudioVideoCode(bool checked);

  void on_cancelPb_clicked();

  void on_setPb_clicked();

  void updatePreferredSelect(bool hasDuplicates = true);
  bool listHasDuplicates(const QStringList &listOfStrings);
  void on_copyFormatAndClosePb_clicked();

private:
  Ui::CustomFormatSelector *ui;
  QTableWidget &m_formatTableWidget;
  QStringList m_formatCodes, m_formatNames, m_exetensionNames, m_notesNames;
  QString audioCode, videoCode;
  QSettings settings;
};

#endif // CUSTOMFORMATSELECTOR_H
