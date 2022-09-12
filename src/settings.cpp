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

#include "settings.h"
#include "utility.h"

#include "logger.h"
#include "translator.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QStyleFactory>

static QString _configPath() {
  auto s = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
  if (s.isEmpty()) {
    return QDir::homePath() + "/.config/" + QApplication::applicationName() +
           "/";
  } else {
    return s.first();
  }
}

static QString _getOptionsHistoryTabName(settings::tabName e) {
  if (e == settings::tabName::basic) {

    return "BasicDownloaderOptionsHistory";

  } else if (e == settings::tabName::batch) {

    return "BatchDownloaderOptionsHistory";

  } else if (e == settings::tabName::playlist) {

    return "PlaylistDownloaderOptionsHistory";
  } else {
    return "";
  }
}

QStringList settings::getOptionsHistory(settings::tabName e) {
  auto m = _getOptionsHistoryTabName(e);

  if (!m_settings.contains(m)) {

    m_settings.setValue(m, QStringList());
  }

  return m_settings.value(m).toStringList();
}

void settings::clearOptionsHistory(settings::tabName e) {
  m_settings.setValue(_getOptionsHistoryTabName(e), QStringList());
}

static void _addToHistory(QSettings &settings, QStringList &&history,
                          const QString &key, const QString &input, int max) {
  if (!input.isEmpty() && !history.contains(input)) {

    if (history.size() == max) {

      history.removeLast();
    }

    history.insert(0, input);

    settings.setValue(key, history);
  }
}

void settings::addToplaylistRangeHistory(const QString &e) {
  if (this->saveHistory()) {

    _addToHistory(m_settings, this->playlistRangeHistory(),
                  "PlaylistRangeHistory", e, this->historySize());
  }
}

void settings::addOptionsHistory(const QString &e, settings::tabName s) {
  if (this->saveHistory()) {

    _addToHistory(m_settings, this->getOptionsHistory(s),
                  _getOptionsHistoryTabName(s), e, this->historySize());
  }
}

QVariant settings::getValue(const QString &key,
                            const QVariant &defaultValue) const {
  return m_settings.value(key, defaultValue);
}

void settings::setValue(const QString &key, const QVariant &value) {
  m_settings.setValue(key, value);
}

void settings::clearPlaylistRangeHistory() {
  m_settings.setValue("PlaylistRangeHistory",
                      QStringList{"--break-on-existing"});
}

void settings::clearPlaylistUrlHistory() {
  m_settings.setValue("PlaylistUrlHistory", QStringList());
}

QStringList settings::playlistRangeHistory() {
  if (!m_settings.contains("PlaylistRangeHistory")) {

    m_settings.setValue("PlaylistRangeHistory",
                        QStringList{"--break-on-existing"});
  }

  return m_settings.value("PlaylistRangeHistory").toStringList();
}

QStringList settings::playlistUrlHistory() {
  if (!m_settings.contains("PlaylistUrlHistory")) {

    m_settings.setValue("PlaylistUrlHistory", QStringList());
  }

  return m_settings.value("PlaylistUrlHistory").toStringList();
}

void settings::setDarkMode(const QString &e) {
  m_settings.setValue("DarkModeName", e);
}

void settings::setPlaylistRangeHistoryLastUsed(const QString &e) {
  m_settings.setValue("playlistRangeHistoryLastUsed", e);
}

QString settings::playlistRangeHistoryLastUsed() {
  if (!m_settings.contains("playlistRangeHistoryLastUsed")) {

    m_settings.setValue("playlistRangeHistoryLastUsed",
                        QString("--break-on-existing"));
  }

  return m_settings.value("playlistRangeHistoryLastUsed").toString();
}

static std::unique_ptr<QSettings> _init() {
  return std::make_unique<QSettings>();
}

settings::settings() : m_settingsP(_init()), m_settings(*m_settingsP) {
  this->initThemes();
  foreach (auto s, QStyleFactory::keys()) {
    if (s.contains("contrast", Qt::CaseInsensitive) == false) {
      styleKeys.append(s);
    }
  }

  // default thumbnail w&h
  double thumbnail_ratio = 320.0 / 180.0;
  thumbnail_height = 120.0;
  thumbnail_width = thumbnail_ratio * thumbnail_height;
}

QSettings &settings::bk() { return m_settings; }

void settings::setTabNumber(int s) { m_settings.setValue("TabNumber", s); }

int settings::tabNumber() {
  return 0;
  //  if (!m_settings.contains("TabNumber")) {

  //    m_settings.setValue("TabNumber", 0);
  //  }

  //  return m_settings.value("TabNumber").toInt();
}

size_t settings::maxConcurrentDownloads() {
  if (!m_settings.contains("MaxConcurrentDownloads")) {

    m_settings.setValue("MaxConcurrentDownloads", 4);
  }

  return static_cast<size_t>(
      m_settings.value("MaxConcurrentDownloads").toInt());
}

void settings::setMaxConcurrentDownloads(int s) {
  m_settings.setValue("MaxConcurrentDownloads", s);
}

void settings::setDownloadFolder(const QString &m) {
  if (m.isEmpty()) {

    auto s = utility::stringConstants::mediaDownloaderDefaultDownloadPath();

    m_settings.setValue("CurrentDownloadFolder", s);
  } else {
    m_settings.setValue("CurrentDownloadFolder", m);
  }
}

static QString _downloadFolder(QSettings &settings) {

  if (!settings.contains("CurrentDownloadFolder")) {

    QString defaultPath =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) +
        QDir::separator() + QApplication::applicationName();
    settings.setValue("CurrentDownloadFolder", defaultPath);
  }

  auto m = settings.value("CurrentDownloadFolder").toString();

  // ensure dir exists
  QDir d;

  d.mkpath(m);

  return m;
}

QString settings::defaultEngineName() { return "core"; }

QString settings::downloadFolder() { return _downloadFolder(m_settings); }

bool settings::autoDownload() {
  if (!m_settings.contains("AutoDownload")) {

    m_settings.setValue("AutoDownload", false);
  }

  return m_settings.value("AutoDownload").toBool();
}

bool settings::showVersionInfoWhenStarting() {
  if (!m_settings.contains("ShowVersionInfoWhenStarting")) {

    m_settings.setValue("ShowVersionInfoWhenStarting", false);
  }

  return m_settings.value("ShowVersionInfoWhenStarting").toBool();
}

bool settings::concurrentDownloading() {
  if (!m_settings.contains("ConcurrentDownloading")) {

    m_settings.setValue("ConcurrentDownloading", true);
  }

  return m_settings.value("ConcurrentDownloading").toBool();
}

bool settings::useSystemProvidedVersionIfAvailable() {
  if (!m_settings.contains("UseSystemProvidedVersionIfAvailable")) {

    m_settings.setValue("UseSystemProvidedVersionIfAvailable", false);
  }

  return m_settings.value("UseSystemProvidedVersionIfAvailable").toBool();
}

QString settings::cookieFilePath(const QString &engineName) {
  auto m = "CookieFilePath" + engineName;

  if (!m_settings.contains(m)) {

    m_settings.setValue(m, QString());
  }

  return m_settings.value(m).toString();
}

void settings::setCookieFilePath(const QString &engineName,
                                 const QString &cookieFilePath) {
  m_settings.setValue("CookieFilePath" + engineName, cookieFilePath);
}

void settings::initThemes() {
  // Light
  lightPalette.setColor(QPalette::Window, QColor(247, 246, 245));
  lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
  lightPalette.setColor(QPalette::Light, QColor(180, 180, 180));
  lightPalette.setColor(QPalette::Midlight, QColor(200, 200, 200));
  lightPalette.setColor(QPalette::Dark, QColor(225, 225, 225));
  lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::BrightText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::Base, QColor(252, 252, 252));
  lightPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
  lightPalette.setColor(QPalette::Highlight, QColor(76, 163, 224));
  lightPalette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::Link, QColor(0, 162, 232));
  lightPalette.setColor(QPalette::AlternateBase, QColor(225, 225, 225));
  lightPalette.setColor(QPalette::ToolTipBase, QColor(240, 240, 240));
  lightPalette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::LinkVisited, QColor(222, 222, 222));
  lightPalette.setColor(QPalette::Disabled, QPalette::WindowText,
                        QColor(115, 115, 115));
  lightPalette.setColor(QPalette::Disabled, QPalette::Text,
                        QColor(115, 115, 115));
  lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                        QColor(115, 115, 115));
  lightPalette.setColor(QPalette::Disabled, QPalette::Highlight,
                        QColor(190, 190, 190));
  lightPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                        QColor(115, 115, 115));

  // Dark
  QColor baseColor = QColor(70, 70, 70);
  QColor alternateBaseColor = QColor(45, 45, 45);
  QColor disabledColor = QColor(127, 127, 127);
  darkPalette.setColor(QPalette::Window, alternateBaseColor);
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, baseColor);
  darkPalette.setColor(QPalette::AlternateBase, alternateBaseColor);
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
  darkPalette.setColor(QPalette::Button, alternateBaseColor);
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(255, 255, 255));
  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);
  darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                       disabledColor);
  darkPalette.setColor(QPalette::ToolTipBase, QColor(0, 0, 0));
  darkPalette.setColor(QPalette::ToolTipText, QColor(240, 240, 240));
}

void settings::setTheme(QApplication &app) {

  QString styleName = "Fusion";
  if (m_settings.value("widgetStyleIndex").isValid()) {
    int styleIndex = m_settings.value("widgetStyleIndex").toInt();
    if (styleIndex > -1 && styleIndex < styleKeys.count()) {
      styleName = styleKeys.at(styleIndex);
    }
  }

  qApp->setStyle(QStyleFactory::create(styleName));

  // only use light and dark palette for fusion and windows style
  if (styleName == "Fusion" || styleName == "Windows") {
    if (m_settings.value("windowThemeIndex", 0).toInt() == 1) {
      qApp->setPalette(darkPalette);
    } else {
      qApp->setPalette(lightPalette);
    }
  } else {
    QPalette p = qApp->style()->standardPalette();
    qApp->setPalette(p);
  }

  QList<QWidget *> widgets = app.findChildren<QWidget *>();
  foreach (QWidget *w, widgets) {
    w->setPalette(qApp->palette());
  }
}

void settings::setShowThumbnails(bool e) {
  m_settings.setValue("ShowThumbnails", e);
}

bool settings::showThumbnails() {
  if (!m_settings.contains("ShowThumbnails")) {

    m_settings.setValue("ShowThumbnails", true);
  }

  return m_settings.value("ShowThumbnails").toBool();
}

bool settings::saveHistory() {
  if (!m_settings.contains("SaveHistory")) {

    m_settings.setValue("SaveHistory", true);
  }

  return m_settings.value("SaveHistory").toBool();
}

bool settings::playlistDownloaderSaveHistory() {
  if (!m_settings.contains("PlaylistDownloaderSaveHistory")) {

    m_settings.setValue("PlaylistDownloaderSaveHistory", true);
  }

  return m_settings.value("PlaylistDownloaderSaveHistory").toBool();
}

void settings::setPlaylistDownloaderSaveHistory(bool e) {
  m_settings.setValue("PlaylistDownloaderSaveHistory", e);
}

int settings::stringTruncationSize() {
  if (!m_settings.contains("StringTruncationSize")) {

    m_settings.setValue("StringTruncationSize", 100);
  }

  return m_settings.value("StringTruncationSize").toInt();
}

int settings::historySize() {
  if (!m_settings.contains("HistorySize")) {

    m_settings.setValue("HistorySize", 10);
  }

  return m_settings.value("HistorySize").toInt();
}

static QString _thumbnailTabName(const QString &s, settings::tabName e) {
  if (e == settings::tabName::batch) {

    return s + "Batch";

  } else if (e == settings::tabName::playlist) {

    return s + "Playlist";
  } else {
    return "";
  }
}

double settings::thumbnailWidth(settings::tabName s) {

  auto m = _thumbnailTabName("ThumbnailWidth", s);

  if (!m_settings.contains(m)) {

    m_settings.setValue(m, thumbnail_width);
  }

  return m_settings.value(m).toDouble();
}

double settings::thumbnailHeight(settings::tabName s) {
  auto m = _thumbnailTabName("ThumbnailHeight", s);

  if (!m_settings.contains(m)) {

    m_settings.setValue(m, thumbnail_height);
  }

  return m_settings.value(m).toDouble();
}

void settings::setShowVersionInfoWhenStarting(bool e) {
  m_settings.setValue("ShowVersionInfoWhenStarting", e);
}

void settings::setHighDpiScalingFactor(const QString &m) {
  m_settings.setValue("EnabledHighDpiScalingFactor", m.toUtf8());
}

void settings::setlibraryDownloadFolder(const QString &e) {
  m_settings.setValue("LibraryDownloadFolder", QDir::fromNativeSeparators(e));
}

QString settings::libraryDownloadFolder() {
  if (!m_settings.contains("LibraryDownloadFolder")) {

    auto m = QDir::fromNativeSeparators(this->downloadFolder());

    m_settings.setValue("LibraryDownloadFolder", m);
  }

  return m_settings.value("LibraryDownloadFolder").toString();
}

static QString _getDefaultEngineName(settings::tabName e) {
  if (e == settings::tabName::basic) {

    return "BasicDownloaderDefaultEngine";

  } else if (e == settings::tabName::batch) {

    return "BatchDownloaderDefaultEngine";

  } else if (e == settings::tabName::playlist) {

    return "PlaylistDownloaderDefaultEngine";
  } else {
    return "";
  }
}

void settings::setDefaultEngine(const QString &e, settings::tabName n) {
  m_settings.setValue(_getDefaultEngineName(n), e);
}

QString settings::defaultEngine(settings::tabName n,
                                const QString &engineName) {
  auto m = _getDefaultEngineName(n);

  if (!m_settings.contains(m)) {

    m_settings.setValue(m, engineName);
  }

  return m_settings.value(m).toString();
}

QPixmap settings::defaultVideoThumbnailIcon(settings::tabName m) {
  auto width = this->thumbnailWidth(m);
  auto height = this->thumbnailHeight(m);

  return QIcon(":/icons/video.png").pixmap(width, height);
}

static QStringList _directoryList(const QString &e) {
  QDir d(e);

  auto s = d.entryList();

  s.removeOne(".");
  s.removeOne("..");

  return s;
}

QStringList settings::localizationLanguages() {
  QStringList m;

  const auto e = _directoryList(this->localizationLanguagePath());

  for (const auto &it : e) {

    if (!it.startsWith("qt_") && it.endsWith(".qm")) {

      auto name = it;
      name.remove(".qm");

      m.append(name);
    }
  }

  return m;
}

QString settings::configPaths() { return _configPath(); }

QString settings::commandOnSuccessfulDownload() {
  if (!m_settings.contains("CommandOnSuccessfulDownload")) {

    m_settings.setValue("CommandOnSuccessfulDownload", QString());
  }

  return m_settings.value("CommandOnSuccessfulDownload").toString();
}

QString settings::commandWhenAllFinished() {
  if (!m_settings.contains("CommandWhenAllFinished")) {

    m_settings.setValue("CommandWhenAllFinished", QString());
  }

  return m_settings.value("CommandWhenAllFinished").toString();
}

static QString _getTabOption(const QString &s, settings::tabName e) {
  if (e == settings::tabName::basic) {

    return "LastUsedOptionBasicTab_" + s;

  } else if (e == settings::tabName::batch) {

    return "LastUsedOptionBatchTab_" + s;

  } else if (e == settings::tabName::playlist) {

    return "LastUsedOptionPlayListTab_" + s;
  } else {
    return "";
  }
}

QString settings::lastUsedOption(const QString &m, settings::tabName e) {
  auto s = _getTabOption(m, e);

  if (!m_settings.contains(s)) {

    m_settings.setValue(s, QString());
  }

  return m_settings.value(s).toString();
}

void settings::setLastUsedOption(const QString &m, const QString &e,
                                 settings::tabName s) {
  m_settings.setValue(_getTabOption(m, s), e);
}

QString settings::localizationLanguagePath() {
  if (!m_settings.contains("TranslationsPath")) {

    if (utility::platformIsWindows()) {

      m_settings.setValue("TranslationsPath",
                          QDir().currentPath() + "/translations");

    } else if (utility::platformIsOSX()) {

      m_settings.setValue("TranslationsPath",
                          QApplication::applicationDirPath());
    } else {
#ifdef QT_DEBUG
      m_settings.setValue("TranslationsPath",
                          QApplication::applicationDirPath() +
                              QDir::separator() + "translations");
#else
      m_settings.setValue("TranslationsPath",
                          QApplication::applicationDirPath());
#endif
    }
  }

  return m_settings.value("TranslationsPath").toString();
}

void settings::setLocalizationLanguage(const QString &language) {
  m_settings.setValue("Language", language);
}

void settings::setWindowDimensions(const QString &window,
                                   const QString &dimenstion) {
  m_settings.setValue("WindowDimensions_" + window, dimenstion);
}

QString settings::windowsDimensions(const QString &window) {
  auto m = "WindowDimensions_" + window;

  if (!m_settings.contains(m)) {

    m_settings.setValue(m, QString());
  }

  return m_settings.value(m).toString();
}

QString settings::localizationLanguage() {
  if (!m_settings.contains("Language")) {

    m_settings.setValue("Language", "en_US");
  }

  return m_settings.value("Language").toString();
}
