#include "customformatselector.h"
#include "ui_customformatselector.h"
#include <QClipboard>
#include <QDebug>
#include <QScrollBar>
#include <QTimer>

CustomFormatSelector::CustomFormatSelector(QWidget *parent,
                                           QTableWidget &tableWidget)
    : QWidget(parent), ui(new Ui::CustomFormatSelector),
      m_formatTableWidget(tableWidget) {
  ui->setupUi(this);

  this->setWindowTitle(QApplication::applicationName() + "|" +
                       "Format selection");

  for (int i = 0; i < m_formatTableWidget.rowCount(); ++i) {
    auto formatCodeItem = m_formatTableWidget.item(i, 0);
    auto exetensionItem = m_formatTableWidget.item(i, 1);
    auto formatNameItem = m_formatTableWidget.item(i, 2);
    auto noteItem = m_formatTableWidget.item(i, 3);

    if (formatNameItem != nullptr && formatCodeItem != nullptr) {
      m_formatCodes << formatCodeItem->text().trimmed();
      m_formatNames << formatNameItem->text().trimmed();
      m_exetensionNames << QString(exetensionItem->text().isEmpty()
                                       ? "-"
                                       : exetensionItem->text().trimmed());
      m_notesNames << QString(
          noteItem->text().isEmpty() ? "-" : noteItem->text().trimmed());
    }
  }
  if (!m_formatCodes.isEmpty() && !m_formatNames.isEmpty()) {
    populateView();
  } else {
    // TODO: show error
  }

  QTimer::singleShot(500, this, [=]() {
    updatePreferredSelect(listHasDuplicates(m_formatNames));
  });
}

bool CustomFormatSelector::listHasDuplicates(const QStringList &listOfStrings) {
  QMap<QString, int> countOfStrings;
  for (int i = 0; i < listOfStrings.count(); i++) {
    countOfStrings[listOfStrings[i]]++;
  }
  return listOfStrings.count() > countOfStrings.count();
}

void CustomFormatSelector::updatePreferredSelect(bool hasDuplicates) {

  int preferedQuality = 0;
  if (settings.value("preferedQuality").isValid()) {
    preferedQuality = settings.value("preferedQuality", 0).toInt();
    if ((preferedQuality < 0) || (preferedQuality > 2)) {
      preferedQuality = 0;
    }
  }
  foreach (QRadioButton *radio,
           ui->preferedFormatWidget->findChildren<QRadioButton *>()) {
    connect(radio, &QRadioButton::toggled, [=](bool checked) {
      if (checked) {
        if (radio == ui->preferedFormatWidget->findChild<QRadioButton *>(
                         "preferHighest")) {
          // select high quality
          settings.setValue("preferedQuality", 0);
          if (ui->quality_box_widget_audio->findChildren<QRadioButton *>()
                  .isEmpty() == false)
            ui->quality_box_widget_audio->findChildren<QRadioButton *>()
                .last()
                ->setChecked(true);

          if (ui->quality_box_widget_video->findChildren<QRadioButton *>()
                  .isEmpty() == false)
            ui->quality_box_widget_video->findChildren<QRadioButton *>()
                .last()
                ->setChecked(true);

          ui->videoFormatScrollArea->verticalScrollBar()->setValue(
              ui->videoFormatScrollArea->verticalScrollBar()->maximum());
          ui->audioFormatScrollArea->verticalScrollBar()->setValue(
              ui->audioFormatScrollArea->verticalScrollBar()->maximum());
        } else if (radio == ui->preferedFormatWidget->findChild<QRadioButton *>(
                                "preferLowest")) {
          // select lowest quality
          settings.setValue("preferedQuality", 2);
          if (ui->quality_box_widget_audio->findChildren<QRadioButton *>()
                  .isEmpty() == false)
            ui->quality_box_widget_audio->findChildren<QRadioButton *>()
                .first()
                ->setChecked(true);

          if (ui->quality_box_widget_video->findChildren<QRadioButton *>()
                  .isEmpty() == false)
            ui->quality_box_widget_video->findChildren<QRadioButton *>()
                .first()
                ->setChecked(true);
          ui->videoFormatScrollArea->verticalScrollBar()->setValue(0);
          ui->audioFormatScrollArea->verticalScrollBar()->setValue(0);
        } else {
          // select medium quality
          settings.setValue("preferedQuality", 1);
          if (ui->quality_box_widget_audio->findChildren<QRadioButton *>()
                  .isEmpty() == false) {
            int c = ui->quality_box_widget_audio->findChildren<QRadioButton *>()
                        .count() /
                    2;
            ui->quality_box_widget_audio->findChildren<QRadioButton *>()
                .at(c)
                ->setChecked(true);
          }
          if (ui->quality_box_widget_video->findChildren<QRadioButton *>()
                  .isEmpty() == false) {
            int c = ui->quality_box_widget_video->findChildren<QRadioButton *>()
                        .count() /
                    2;
            ui->quality_box_widget_video->findChildren<QRadioButton *>()
                .at(c)
                ->setChecked(true);
          }
          ui->videoFormatScrollArea->verticalScrollBar()->setValue(
              ui->videoFormatScrollArea->verticalScrollBar()->maximum() / 2);
          ui->audioFormatScrollArea->verticalScrollBar()->setValue(
              ui->audioFormatScrollArea->verticalScrollBar()->maximum() / 2);
        }
      }
    });
  }

  if (ui->preferedFormatWidget->findChildren<QRadioButton *>().isEmpty() ==
      false) {
    switch (preferedQuality) {
    case 0:
      ui->preferedFormatWidget->findChild<QRadioButton *>("preferHighest")
          ->setChecked(true);
      break;
    case 1:
      ui->preferedFormatWidget->findChild<QRadioButton *>("preferMedium")
          ->setChecked(true);
      break;
    case 2:
      ui->preferedFormatWidget->findChild<QRadioButton *>("preferLowest")
          ->setChecked(true);
      break;
    default:
      break;
    }
  }

  ui->preferMedium->setVisible(!hasDuplicates);
}

void CustomFormatSelector::populateView() {

  for (int i = 0; i < m_formatNames.count(); ++i) {

    QString formatName = m_formatNames.at(i);

    QString formatCode = m_formatCodes.at(i);

    if (formatName.contains("video only", Qt::CaseInsensitive) &&
        formatName.isEmpty() == false) {
      // add video format
      QRadioButton *button = new QRadioButton(
          formatName + " (" + m_exetensionNames.at(i) + ")", this);

      button->setProperty("type", "video");
      button->setProperty("code", formatCode);

      button->setToolTip(QString(m_notesNames.at(i)).replace(",", "\n"));

      connect(button, SIGNAL(toggled(bool)), this,
              SLOT(updateAudioVideoCode(bool)));

      ui->quality_box_widget_video->layout()->addWidget(button);

    } else if (formatName.contains("audio only", Qt::CaseInsensitive) &&
               formatName.isEmpty() == false) {
      // add audio format
      QRadioButton *button = new QRadioButton(
          formatName + " (" + m_exetensionNames.at(i) + ")", this);

      button->setProperty("type", "audio");
      button->setProperty("code", formatCode);

      button->setToolTip(QString(m_notesNames.at(i)).replace(", ", "\n"));

      connect(button, SIGNAL(toggled(bool)), this,
              SLOT(updateAudioVideoCode(bool)));

      ui->quality_box_widget_audio->layout()->addWidget(button);
    }
  }
}

void CustomFormatSelector::updateAudioVideoCode(bool checked) {

  QRadioButton *button = qobject_cast<QRadioButton *>(sender());
  if (button->property("type").toString() == "audio" && checked) {
    audioCode = button->property("code").toString();
  }

  if (button->property("type").toString() == "video" && checked) {
    videoCode = button->property("code").toString();
  }

  if (!audioCode.isEmpty() && !videoCode.isEmpty()) {
    ui->setPb->setEnabled(true);
  } else {
    ui->setPb->setEnabled(false);
  }

  ui->selectedFormatLabel->setText(videoCode + "+" + audioCode);

  int a = m_formatCodes.lastIndexOf(audioCode);
  int v = m_formatCodes.lastIndexOf(videoCode);

  QString formatNotes;

  if (a != -1 && v != -1) {
    formatNotes += "-------Audio Format-------\n";
    formatNotes += QString(m_notesNames.at(a)).replace(", ", "\n") + "\n";
    formatNotes += "--------------------------\n";
    formatNotes += "\n";
    formatNotes += "-------Video Format-------\n";
    formatNotes += QString(m_notesNames.at(v)).replace(", ", "\n") + "\n";
    formatNotes += "--------------------------\n";
  }

  ui->formatNotes->setText(formatNotes);
  ui->formatNotes->setVisible(formatNotes.trimmed().isEmpty() == false);
}

CustomFormatSelector::~CustomFormatSelector() { delete ui; }

void CustomFormatSelector::on_cancelPb_clicked() { this->close(); }

void CustomFormatSelector::on_setPb_clicked() {
  emit formatSelected(ui->selectedFormatLabel->text());
  this->close();
}

void CustomFormatSelector::on_copyFormatAndClosePb_clicked() {
  qApp->clipboard()->setText(ui->selectedFormatLabel->text());
  this->close();
}
