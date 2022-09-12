/*
 *  Copyright (c) 2021 Keshav Bhatt
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "configure.h"
#include "mainwindow.h"
#include "networkAccess.h"
#include "tabmanager.h"
#include "utility.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyleFactory>

void configure::updatePreferredYtPlaybackQuality(int preferedQuality) {
  switch (preferedQuality) {
  case 0:
    m_ui.configPreferredYtQuality
        ->findChild<QRadioButton *>("rbConfigYtHighQual")
        ->setChecked(true);
    break;
  case 1:
    m_ui.configPreferredYtQuality
        ->findChild<QRadioButton *>("rbConfigYtMediumQual")
        ->setChecked(true);
    break;
  case 2:
    m_ui.configPreferredYtQuality
        ->findChild<QRadioButton *>("rbConfigYtLowQual")
        ->setChecked(true);
    break;
  default:
    break;
  }

  // check if account evaluation used and high is selected
  // select medium quality
  auto accountManager = m_tabManager.AccountManager();
  if (accountManager.evaluationUsed() && preferedQuality == 0) {
    m_ui.configPreferredYtQuality
        ->findChild<QRadioButton *>("rbConfigYtMediumQual")
        ->setChecked(true);
  }
}

configure::configure(const Context &ctx)
    : m_ctx(ctx), m_settings(m_ctx.Settings()), m_ui(m_ctx.Ui()),
      m_mainWindow(m_ctx.mainWidget()), m_tabManager(m_ctx.TabManager()),
      m_engines(m_ctx.Engines()), m_presetOptions(m_ctx, m_settings),
      m_downloadDefaultOptions(m_ctx) {

  // event filter to prevent wheel event on certain widgets
  foreach (QSlider *slider,
           m_ui.scrollAreaWidgetContents->findChildren<QSlider *>()) {
    slider->installEventFilter(this);
  }
  foreach (QComboBox *box,
           m_ui.scrollAreaWidgetContents->findChildren<QComboBox *>()) {
    box->installEventFilter(this);
  }
  foreach (QSpinBox *spinBox,
           m_ui.scrollAreaWidgetContents->findChildren<QSpinBox *>()) {
    spinBox->installEventFilter(this);
  }

  m_ui.comboBoxWidgetStyle->blockSignals(true);
  foreach (auto s, QStyleFactory::keys()) {
    if (s.contains("contrast", Qt::CaseInsensitive) == false) {
      m_ui.comboBoxWidgetStyle->addItem(s);
    }
  }
  m_ui.comboBoxWidgetStyle->blockSignals(false);

  connect(m_ui.onlyCompletedCheckBox, &QCheckBox::toggled, this,
          [=](bool checked) {
            m_settings.setValue("onlyCompletedCheckBox", checked);
            emit m_ui.pbLibraryRefresh->clicked();
          });

  int preferedQuality =
      m_settings.getValue("preferedPlaybackQualityYtSearch", 1).toInt();
  if ((preferedQuality < 0) || (preferedQuality > 2)) {
    preferedQuality = 1;
  }

  updatePreferredYtPlaybackQuality(preferedQuality);

  foreach (QRadioButton *rad,
           m_ui.configPreferredYtQuality->findChildren<QRadioButton *>()) {
    connect(rad, &QRadioButton::toggled, this, [=](bool checked) {
      if (checked) {
        if (rad == m_ui.configPreferredYtQuality->findChild<QRadioButton *>(
                       "rbConfigYtLowQual")) {
          m_settings.setValue("preferedPlaybackQualityYtSearch", 2);
        }
        if (rad == m_ui.configPreferredYtQuality->findChild<QRadioButton *>(
                       "rbConfigYtMediumQual")) {
          m_settings.setValue("preferedPlaybackQualityYtSearch", 1);
        }
        if (rad == m_ui.configPreferredYtQuality->findChild<QRadioButton *>(
                       "rbConfigYtHighQual")) {
          auto lastPreferredPlaybackQuality =
              m_settings.getValue("preferedPlaybackQualityYtSearch", 1).toInt();
          auto accountManager = m_tabManager.AccountManager();
          if (accountManager.evaluationUsed()) {
            accountManager.showPurchaseMessage();
            updatePreferredYtPlaybackQuality(lastPreferredPlaybackQuality);
          } else {
            m_settings.setValue("preferedPlaybackQualityYtSearch", 0);
          }
        }
      }
    });
  }

  connect(
      m_ui.comboBoxWidgetStyle,
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      this, [=](int index) {
        m_settings.setValue("widgetStyleIndex", index);
        QApplication *application =
            static_cast<QApplication *>(QApplication::instance());
        auto itemText = m_ui.comboBoxWidgetStyle->itemText(index);
        if (itemText.contains("dark", Qt::CaseInsensitive)) {
          m_settings.setValue("windowThemeIndex", 1);
          m_ui.comboBoxTheme->setCurrentIndex(
              m_settings.getValue("windowThemeIndex", 0).toInt());
        } else if (itemText.compare("Adwaita", Qt::CaseInsensitive) == 0) {
          m_settings.setValue("windowThemeIndex", 0);
          m_ui.comboBoxTheme->setCurrentIndex(
              m_settings.getValue("windowThemeIndex", 0).toInt());
        }
        m_settings.setTheme(*application);
        m_ui.tabWidget->tabBar()->setDrawBase(false);
      });

  connect(
      m_ui.comboBoxTheme,
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      this, [=](int index) {
        m_settings.setValue("windowThemeIndex", index);
        QApplication *application =
            static_cast<QApplication *>(QApplication::instance());
        m_settings.setTheme(*application);
        m_ui.tabWidget->tabBar()->setDrawBase(false);
      });

  int fusionIndex = m_ui.comboBoxWidgetStyle->findText("Fusion");

  m_ui.comboBoxWidgetStyle->setCurrentIndex(
      m_settings.getValue("widgetStyleIndex", fusionIndex).toInt());

  m_ui.comboBoxTheme->setCurrentIndex(
      m_settings.getValue("windowThemeIndex", 0).toInt());

  m_ui.onlyCompletedCheckBox->setChecked(
      m_settings.getValue("onlyCompletedCheckBox", true).toBool());

  QApplication *application =
      static_cast<QApplication *>(QApplication::instance());
  m_settings.setTheme(*application);

  connect(m_ui.pbConfigureCookiePath, &QPushButton::clicked, this, [=]() {
    auto m = QFileDialog::getOpenFileName(
        &m_ctx.mainWidget(), tr("Select a Cookie file"), utility::homePath());

    if (!m.isEmpty()) {

      m_ui.lineEditConfigureCookiePath->setText(m);
    }
  });

  connect(m_ui.pbConfigureDownload, &QPushButton::clicked, this, [&]() {
    auto m = m_ctx.Engines().getEngines().begin()->name();
    m_ctx.logger().clear();
    this->downloadFromGitHub(m_ctx.Engines().defaultEngine(m));
  });

  this->resetMenu();

  auto s = static_cast<void (QComboBox::*)(int)>(&QComboBox::activated);

  connect(m_ui.cbConfigureLanguage, s, this, [&](int s) {
    const auto &languages = m_settings.localizationLanguages();

    if (s != -1 && s < languages.size()) {

      const auto &m = languages.at(s);

      m_settings.setLocalizationLanguage(m);

      m_ctx.Translator().setLanguage(m);

      m_ctx.mainWindow().retranslateUi();

      m_tabManager.reTranslateUi();
    }
  });

  connect(m_ui.pbConfigureDownloadPath, &QPushButton::clicked, this, [=]() {
    QString d =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) +
        QDir::separator() + QApplication::applicationName();

    const QVariant qv(d);

    auto e = QFileDialog::getExistingDirectory(
        &m_mainWindow, tr("Select download directory"),
        m_settings.getValue(QString("CurrentDownloadFolder"), qv).toString(),
        QFileDialog::ShowDirsOnly);

    if (!e.isEmpty()) {

      m_ui.lineEditConfigureDownloadPath->setText(e);
    }
  });

  connect(
      m_ui.comboBoxConfigureTabPosition,
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      this, [=](int index) {
        switch (index) {
        case 0:
          m_ui.tabWidget->setTabPosition(QTabWidget::North);
          break;
        case 1:
          m_ui.tabWidget->setTabPosition(QTabWidget::South);
          break;
        case 2:
          m_ui.tabWidget->setTabPosition(QTabWidget::West);
          break;
        case 3:
          m_ui.tabWidget->setTabPosition(QTabWidget::East);
          break;
        default:
          m_ui.tabWidget->setTabPosition(QTabWidget::North);
          break;
        }

        m_settings.setValue("CurrentTabWidgetTabPosition", index);
      });

  connect(m_ui.pbConfigureClearEngineCache, &QPushButton::clicked, this, [=]() {
    m_ui.tabWidget->setCurrentWidget(m_ui.tabBasickDownloader);

    auto &logger = m_ctx.logger();

    logger.add(tr("Cleaning Engine Cache ..."));

    const auto &engines = m_ctx.Engines().getEngines();

    const auto &engine = &engines.at(0); // engine() ;

    engines::engine::exeArgs::cmd cmd(engine->exePath(),
                                      {engine->versionArgument()});

    auto exe = cmd.exe();

    util::run(
        exe, QStringList() << "--rm-cache-dir",
        [this, &logger](const util::run_result &r) {
          if (r.success()) {

            logger.add(QObject::tr("Engine cache cleaned."));

            m_ctx.TabManager().enableAll();

          } else {

            m_ctx.logger().add(tr("Failed to clear engine cache."));

            m_ctx.TabManager().enableAll();
          }

          m_ctx.TabManager().enableAll();
        },
        QProcess::ProcessChannelMode::MergedChannels);
  });

  m_ui.comboBoxConfigureTabPosition->setCurrentIndex(
      m_settings.getValue("CurrentTabWidgetTabPosition", 0).toInt());

  // start lineEditConfigureDownloadPath
  m_ui.lineEditConfigureDownloadPath->setText(m_settings.downloadFolder());

  connect(m_ui.lineEditConfigureDownloadPath, &QLineEdit::textChanged, this,
          [=](const QString &arg1) { m_settings.setDownloadFolder(arg1); });
  // end lineEditConfigureDownloadPath

  m_ui.cbConfigureShowVersionInfo->setChecked(
      m_settings.showVersionInfoWhenStarting());

  // start cbConfigureShowThumbnails
  m_ui.cbConfigureShowThumbnails->setChecked(m_settings.showThumbnails());
  connect(m_ui.cbConfigureShowThumbnails, &QCheckBox::toggled, this,
          [=](bool checked) {
            m_settings.setShowThumbnails(checked);
            m_ctx.TabManager().batchDownloader().setThumbnailColumnSize(
                checked);
          });

  // end cbConfigureShowThumbnails

  // start pbConfigureCookieHowTo
  connect(m_ui.pbConfigureCookieHowTo, &QPushButton::clicked, [=]() {
    QDesktopServices::openUrl(QUrl("https://gist.github.com/keshavbhatt/"
                                   "66a344f39192176c968708ef7f55c915"));
  });
  // end pbConfigureCookieHowTo

  // start spinBoxConfigureMaximuConcurrentDownloads
  m_ui.spinBoxConfigureMaximuConcurrentDownloads->setRange(1, 20);

  m_ui.spinBoxConfigureMaximuConcurrentDownloads->setValue(
      m_settings.maxConcurrentDownloads());

  connect(m_ui.spinBoxConfigureMaximuConcurrentDownloads,
          QOverload<int>::of(&QSpinBox::valueChanged), this,
          [=](int i) { m_settings.setMaxConcurrentDownloads(i); });
  // end spinBoxConfigureMaximuConcurrentDownloads

  // start xpSearcgEnableCheckBox
  if (m_settings.getValue("xpSearch", false).toBool()) {
    this->enableXpSearchSilently();
    m_ui.xpSearchGroupBox->setVisible(true);
    m_ui.xpSearcgEnableCheckBox->setChecked(true);
  } else {
    this->toggleXpSearch(false);
    m_ui.xpSearcgEnableCheckBox->setChecked(false);
    m_ui.xpSearchGroupBox->setVisible(false);
  }

  connect(m_ui.xpSearcgEnableCheckBox, &QCheckBox::toggled, this,
          [=](bool checked) {
            m_settings.setValue("xpSearch", checked);
            if (checked) {
              this->enableXpSearchSilently();
            } else {
              this->toggleXpSearch(checked);
            }
          });
  // end xpSearcgEnableCheckBox

  // start browserSessionSharingCheckBox
  auto cookieSharing =
      m_settings.getValue("browserSessionSharing", true).toBool();
  m_ui.browserSessionSharingCheckBox->setChecked(cookieSharing);
  m_ui.cookieWidget->setEnabled(!cookieSharing);
  connect(m_ui.browserSessionSharingCheckBox, &QCheckBox::toggled, this,
          [=](bool checked) {
            m_settings.setValue("browserSessionSharing", checked);
            m_ui.cookieWidget->setEnabled(!checked);

            if (checked) {
              qDebug() << "CookieSharing";
              MainWindow *mainwindow =
                  qobject_cast<MainWindow *>(&m_mainWindow);
              QString browser_cookie_dump_file_path =
                  mainwindow->dumpBrowserCookie();
              qDebug() << browser_cookie_dump_file_path;
              m_ui.lineEditConfigureCookiePath->setText(
                  browser_cookie_dump_file_path);
            } else {
              m_ui.lineEditConfigureCookiePath->setText("");
              m_settings.setCookieFilePath(m_settings.defaultEngineName(), "");
            }
          });
  // end browserSessionSharingCheckBox

  // start playerStartupVolume
  m_ui.playerVolumeSlider->setRange(0, 100);
  m_ui.playerVolumeSlider->setValue(
      m_settings.getValue("playerStartupVolume", 50).toInt());
  m_ui.playerVolumeConfigureLabel->setText(
      QString::number(m_ui.playerVolumeSlider->value()));

  connect(m_ui.playerVolumeSlider, &QSlider::valueChanged, this,
          [=](int value) {
            m_ui.playerVolumeConfigureLabel->setText(QString::number(value));
            m_settings.setValue("playerStartupVolume", value);
          });
  // end playerStartupVolume

  m_ui.engineNameWidget->hide();

  m_ui.defaultDownloadOptions->hide();

  m_ui.cbConfigureShowVersionInfo->hide();

  m_ui.languageSelectionWidget->hide();
}

void configure::updateCookiePath() {
  auto cookieSharing =
      m_settings.getValue("browserSessionSharing", true).toBool();
  if (cookieSharing) {
    // toggle so that we get latest cookies
    m_ui.browserSessionSharingCheckBox->setChecked(false);
    m_ui.browserSessionSharingCheckBox->setChecked(cookieSharing);
  }
}

void configure::enableXpSearchSilently() {
  m_ui.tabWidget->setTabVisible(m_ui.tabWidget->indexOf(m_ui.xpSearchTab),
                                true);
  m_ui.xpSearchTab->setEnabled(true);
}

bool configure::eventFilter(QObject *obj, QEvent *event) {

  if (isChildOf(m_ui.scrollAreaWidgetContents, obj)) {
    if (event->type() == QEvent::Wheel) {
      return true;
    }
  }
  return QObject::eventFilter(obj, event);
}

bool configure::isChildOf(QObject *Of, QObject *self) {
  bool ischild = false;
  if (Of->findChild<QWidget *>(self->objectName())) {
    ischild = true;
  }
  return ischild;
}

// show/hide and enable/disable xp Search tab
void configure::toggleXpSearch(bool enable) {
  auto xpSearchTabIndex = m_ui.tabWidget->indexOf(m_ui.xpSearchTab);

  if (enable) {
    QMessageBox msgBox(
        QMessageBox::Warning,
        qApp->applicationName() + " | " + QObject::tr("Warning"),
        QObject::tr("Age restricted content warning!\nThis plugin may contain "
                    "age restricted content!\n\nProceed?"),
        QMessageBox::Cancel);

    QPushButton *enableXpSearchButtonSession =
        msgBox.addButton(QObject::tr("Enable XP search for this session"),
                         QMessageBox::ActionRole);

    QPushButton *enableXpSearchButtonPermanent = msgBox.addButton(
        QObject::tr("Enable XP search permanently"), QMessageBox::ActionRole);

    msgBox.exec();

    QJsonObject json{{"accountIsPro", m_tabManager.AccountManager().isPro()}};

    if (msgBox.clickedButton() == msgBox.button(QMessageBox::Cancel)) {
      m_tabManager.trackingService()->trackEvent("enableXpSearchCancel", json);
      return;
    } else if (msgBox.clickedButton() == enableXpSearchButtonSession) {
      m_ui.tabWidget->setTabVisible(xpSearchTabIndex, true);
      m_ui.tabWidget->setCurrentIndex(xpSearchTabIndex);
      m_ui.xpSearchTab->setEnabled(true);

      m_tabManager.trackingService()->trackEvent("enableXpSearchForSession",
                                                 json);
    } else if (msgBox.clickedButton() == enableXpSearchButtonPermanent) {
      m_settings.setValue("xpSearch", true);
      m_ui.tabWidget->setTabVisible(xpSearchTabIndex, true);
      m_ui.tabWidget->setCurrentIndex(xpSearchTabIndex);
      m_ui.xpSearchTab->setEnabled(true);
      m_ui.xpSearchGroupBox->setVisible(true);
      m_ui.xpSearcgEnableCheckBox->setChecked(true);

      m_tabManager.trackingService()->trackEvent("enableXpSearchForPermanent",
                                                 json);
    }
  } else {
    m_settings.setValue("xpSearch", false);
    m_ui.tabWidget->setTabVisible(xpSearchTabIndex, false);
    m_ui.xpSearchTab->setEnabled(false);
  }
}

void configure::init_done() {}

void configure::retranslateUi() { this->resetMenu(); }

void configure::downloadFromGitHub(const engines::Iterator &iter) {
  m_ctx.versionInfo().network().download(iter);
  m_ui.tabWidget->setCurrentWidget(m_ui.tabBasickDownloader);
}

void configure::tabExited() { this->saveOptions(); }

void configure::tabEntered() {

  m_ui.lineEditConfigureDownloadPath->clearFocus();

  m_ui.lineEditConfigureCookiePath->setText(
      m_settings.cookieFilePath(m_settings.defaultEngineName()));
}

void configure::updateEnginesList(const QStringList &e) {
  auto &cb = *m_ui.cbConfigureEngines;

  cb.clear();

  for (const auto &it : e) {

    cb.addItem(it);

    this->setEngineOptions(it);
  }

  cb.setCurrentIndex(0);
  this->setEngineOptions(cb.currentText());
}

void configure::setDownloadOptions(int row, tableWidget &table) {
  m_downloadDefaultOptions.setDownloadOptions(row, table);
}

void configure::saveOptions() {

  m_downloadDefaultOptions.save();

  m_settings.setShowVersionInfoWhenStarting(
      m_ui.cbConfigureShowVersionInfo->isChecked());

  auto mm = m_ui.cbConfigureEngines->currentText();

  const auto &ss = m_engines.getEngineByName(mm);

  if (ss) {

    if (!ss->cookieArgument().isEmpty()) {

      m_settings.setCookieFilePath(ss->name(),
                                   m_ui.lineEditConfigureCookiePath->text());
    }
  }

  m_ctx.TabManager().resetMenu();
}

void configure::setEngineOptions(const QString &e) {
  const auto &s = m_engines.getEngineByName(e);

  if (s) {

    auto enable = !s->cookieArgument().isEmpty();

    m_ui.lineEditConfigureCookiePath->setText(
        m_settings.cookieFilePath(s->name()));
    m_ui.lineEditConfigureCookiePath->setEnabled(enable);
    m_ui.pbConfigureCookiePath->setEnabled(enable);
    m_ui.labelPathToCookieFile->setEnabled(enable);
  }
}

void configure::resetMenu() {
  const auto &languages = m_settings.localizationLanguages();

  const auto &language = m_settings.localizationLanguage();

  auto &translator = m_ctx.Translator();

  m_ui.cbConfigureLanguage->clear();

  int index = 0;

  for (int i = 0; i < languages.size(); i++) {

    auto &e = languages[i];

    if (e == language) {

      index = i;
    }

    m_ui.cbConfigureLanguage->addItem(translator.UIName(e));
  }

  m_ui.cbConfigureLanguage->setCurrentIndex(index);
}

void configure::enableAll() {
  //  const auto &s =
  //      m_engines.getEngineByName(m_ui.cbConfigureEngines->currentText());

  //  if (s) {

  //    auto enable = !s->cookieArgument().isEmpty();

  //    m_ui.lineEditConfigureCookiePath->setEnabled(enable);
  //    m_ui.pbConfigureCookiePath->setEnabled(enable);
  //    m_ui.labelPathToCookieFile->setEnabled(enable);
  //  }

  m_ui.lineEditConfigureCookiePath->setEnabled(true);
  m_ui.pbConfigureCookiePath->setEnabled(true);
  m_ui.labelPathToCookieFile->setEnabled(true);
  m_ui.cbConfigureEngines->setEnabled(true);
  m_ui.labelConfigureEngines->setEnabled(true);
  m_ui.spinBoxConfigureMaximuConcurrentDownloads->setEnabled(true);
  m_ui.comboBoxTheme->setEnabled(true);
  m_ui.comboBoxWidgetStyle->setEnabled(true);
  m_ui.pbConfigureDownload->setEnabled(true);
  m_ui.labelConfigureTheme->setEnabled(true);
  m_ui.labelConfigureWidgetStyle->setEnabled(true);
  m_ui.cbConfigureShowVersionInfo->setEnabled(true);
  m_ui.cbConfigureLanguage->setEnabled(true);
  m_ui.labelConfigureLanguage->setEnabled(true);
  m_ui.pbConfigureDownloadPath->setEnabled(true);
  m_ui.labelConfigureDownloadPath->setEnabled(true);
  m_ui.cbConfigureShowThumbnails->setEnabled(true);
  m_ui.labelMaximumConcurrentDownloads->setEnabled(true);
  m_ui.pbConfigureCookieHowTo->setEnabled(true);
  m_ui.pbConfigureClearEngineCache->setEnabled(true);
}

void configure::disableAll() {
  m_ui.labelPathToCookieFile->setEnabled(false);
  m_ui.lineEditConfigureCookiePath->setEnabled(false);
  m_ui.pbConfigureCookiePath->setEnabled(false);
  m_ui.cbConfigureEngines->setEnabled(false);
  m_ui.labelConfigureEngines->setEnabled(false);
  m_ui.comboBoxTheme->setEnabled(false);
  m_ui.comboBoxWidgetStyle->setEnabled(false);
  m_ui.pbConfigureDownload->setEnabled(false);
  m_ui.labelConfigureTheme->setEnabled(false);
  m_ui.labelConfigureWidgetStyle->setEnabled(false);
  m_ui.spinBoxConfigureMaximuConcurrentDownloads->setEnabled(false);
  m_ui.labelMaximumConcurrentDownloads->setEnabled(false);
  m_ui.cbConfigureShowVersionInfo->setEnabled(false);
  m_ui.pbConfigureDownload->setEnabled(false);
  m_ui.cbConfigureLanguage->setEnabled(false);
  m_ui.labelConfigureLanguage->setEnabled(false);
  m_ui.lineEditConfigureDownloadPath->setEnabled(false);
  m_ui.pbConfigureDownloadPath->setEnabled(false);
  m_ui.labelConfigureDownloadPath->setEnabled(false);
  m_ui.cbConfigureShowThumbnails->setEnabled(false);
  m_ui.pbConfigureCookieHowTo->setEnabled(false);
  m_ui.pbConfigureClearEngineCache->setEnabled(false);
}

configure::presetOptions::presetOptions(const Context &ctx, settings &s)
    : m_path(ctx.Engines().engineDirPaths().dataPath("presetOptions.json")) {
  QJsonParseError err;

  QSettings &m = s.bk();

  QByteArray data;

  // FORCE LOAD PRESETS FROM MEMORY INSTEAD OF PRESET FILE
  //  if (QFile::exists(m_path)) {

  //    QFile f(m_path);

  //    f.open(QIODevice::ReadOnly);

  //    data = f.readAll();

  //  } else
  if (m.contains("PresetJsonOptions")) {

    auto a = m.value("PresetJsonOptions").toByteArray();

    m.remove("PresetJsonOptions");

    data = QByteArray::fromHex(a);
  } else {
    data = this->defaultData();
  }

  auto json = QJsonDocument::fromJson(data, &err);

  if (err.error == QJsonParseError::NoError) {

    m_array = json.array();
  }
}

configure::presetOptions::~presetOptions() {
  QFile f(m_path);
  f.open(QIODevice::WriteOnly | QIODevice::Truncate);
  f.write(QJsonDocument(m_array).toJson(QJsonDocument::Indented));
}

void configure::presetOptions::clear() {
  auto s = m_array.size();

  for (int i = 0; i < s; i++) {

    m_array.removeAt(0);
  }
}

void configure::presetOptions::setDefaults() {
  this->clear();

  QJsonParseError err;

  auto json = QJsonDocument::fromJson(this->defaultData(), &err);

  if (err.error == QJsonParseError::NoError) {

    m_array = json.array();
  }
}

void configure::presetOptions::add(const QString &uiName,
                                   const QString &options) {
  QJsonObject o;

  o.insert("uiName", uiName);
  o.insert("options", options);

  m_array.append(o);
}

QByteArray configure::presetOptions::defaultData() {
  return R"R([
    {
        "options": "bestvideo[height=144][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height=144]+bestaudio",
        "uiName": "144p"
    },
    {
        "options": "bestvideo[height=240][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height=240]+bestaudio",
        "uiName": "240p"
    },
    {
        "options": "bestvideo[height=360][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height=360]+bestaudio",
        "uiName": "360p"
    },
    {
        "options": "bestvideo[height=480][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height=480]+bestaudio",
        "uiName": "480p"
    },
    {
        "options": "bestvideo[height=720][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height=720]+bestaudio",
        "uiName": "720p"
    },
    {
        "options": "bestvideo[height=1080][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height=1080]+bestaudio",
        "uiName": "1080p"
    },
    {
        "options": "bestvideo[height=1440][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height=1440]+bestaudio",
        "uiName": "1440p"
    },
    {
        "options": "bestvideo[height=2160][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height=2160]+bestaudio",
        "uiName": "2160p"
    },
    {
        "options": "Default",
        "uiName": "Default"
    },
    {
        "options": "bestvideo+bestaudio",
        "uiName": "Best-audio-video"
    },
    {
        "options": "bestaudio -x --embed-thumbnail --audio-format mp3",
        "uiName": "Best-audio-with-thumbnail"
    },
    {
        "options": "bestaudio -x --audio-format mp3",
        "uiName": "Best-audio"
    }
])R";
}

configure::downloadDefaultOptions::downloadDefaultOptions(const Context &ctx)
    : m_path(ctx.Engines().engineDirPaths().dataPath(
          "downloadDefaultOptions.json")) {
  QJsonParseError err;

  if (QFile::exists(m_path)) {

    QFile f(m_path);

    f.open(QIODevice::ReadOnly);

    auto json = QJsonDocument::fromJson(f.readAll(), &err);

    if (err.error == QJsonParseError::NoError) {

      m_array = json.array();
    }
  }
}

void configure::downloadDefaultOptions::save() {
  QFile f(m_path);
  f.open(QIODevice::WriteOnly | QIODevice::Truncate);
  f.write(QJsonDocument(m_array).toJson(QJsonDocument::Indented));
}

QJsonObject configure::downloadDefaultOptions::add(const QString &url,
                                                   const QString &opts,
                                                   const QString &engineName) {
  QJsonObject obj;

  obj.insert("comparator", "contains");
  obj.insert("downloadOption", opts);
  obj.insert("engine", engineName);
  obj.insert("url", url);

  m_array.append(obj);

  return obj;
}

QJsonObject configure::downloadDefaultOptions::add(
    const configure::downloadDefaultOptions::opts &e) {
  QJsonObject obj;

  obj.insert("comparator", e.comparator);
  obj.insert("downloadOption", e.downloadOptions);
  obj.insert("engine", e.engine);
  obj.insert("url", e.url);

  m_array.append(obj);

  return obj;
}

void configure::downloadDefaultOptions::remove(const QJsonObject &e) {
  for (int i = 0; i < m_array.size(); i++) {

    if (m_array[i].toObject() == e) {

      m_array.removeAt(i);
      break;
    }
  }
}
