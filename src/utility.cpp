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

#include "utility.h"
#include "mainwindow.h"
#include "utils.h"

#include "context.hpp"
#include "downloadmanager.h"
#include "settings.h"
#include "tableWidget.h"
#include "tabmanager.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QEventLoop>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QSysInfo>

const char *utility::selectedAction::CLEAROPTIONS = "Clear Options";
const char *utility::selectedAction::CLEARSCREEN = "Clear Screen";
const char *utility::selectedAction::OPENFOLDER = "Open Download Folder";

#ifdef Q_OS_LINUX

bool utility::platformIsLinux() { return true; }

bool utility::platformIsOSX() { return false; }

bool utility::platformIsWindows() { return false; }

QString utility::python3Path() {
  return QStandardPaths::findExecutable("python3");
}

util::result<int> utility::Terminator::terminate(int, char **) { return {}; }

#endif

#ifdef Q_OS_MACOS

QString utility::python3Path() {
  return QStandardPaths::findExecutable("python3");
}

bool utility::platformIsOSX() { return true; }

bool utility::platformIsLinux() { return false; }

bool utility::platformIsWindows() { return false; }

util::result<int> utility::Terminator::terminate(int, char **) { return {}; }

#endif

#ifdef Q_OS_WIN

#include <windows.h>

#include <cstdlib>
#include <cstring>

template <typename Function, typename Deleter, typename... Arguments>
auto unique_rsc(Function &&function, Deleter &&deleter, Arguments &&...args) {
  using A = std::remove_pointer_t<std::result_of_t<Function(Arguments && ...)>>;
  using B = std::decay_t<Deleter>;

  return std::unique_ptr<A, B>(function(std::forward<Arguments>(args)...),
                               std::forward<Deleter>(deleter));
}

template <typename Type, typename Deleter>
auto unique_ptr(Type type, Deleter &&deleter) {
  return unique_rsc([](auto arg) { return arg; },
                    std::forward<Deleter>(deleter), type);
}

static int _terminateWindowApp(unsigned long pid) {
  FreeConsole();

  if (AttachConsole(pid) == TRUE) {

    SetConsoleCtrlHandler(nullptr, true);

    if (GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0) == TRUE) {

      return 0;
    }
  }

  return 1;
}

util::result<int> utility::Terminator::terminate(int argc, char **argv) {
  if (argc > 2 && std::strcmp(argv[1], "-T") == 0) {

    return _terminateWindowApp(std::strtoul(argv[2], nullptr, 10));
  } else {
    return {};
  }
}

static HKEY _reg_open_key(const char *subKey, HKEY hkey) {
  HKEY m;
  REGSAM wow64 = KEY_QUERY_VALUE | KEY_WOW64_64KEY;
  REGSAM wow32 = KEY_QUERY_VALUE | KEY_WOW64_32KEY;
  unsigned long x = 0;

  if (RegOpenKeyExA(hkey, subKey, x, wow64, &m) == ERROR_SUCCESS) {

    return m;

  } else if (RegOpenKeyExA(hkey, subKey, x, wow32, &m) == ERROR_SUCCESS) {

    return m;
  } else {
    return nullptr;
  }
}

static void _reg_close_key(HKEY hkey) {
  if (hkey != nullptr) {

    RegCloseKey(hkey);
  }
}

static QByteArray _reg_get_value(HKEY hkey, const char *key) {
  if (hkey != nullptr) {

    DWORD dwType = REG_SZ;

    std::array<char, 4096> buffer;

    std::fill(buffer.begin(), buffer.end(), '\0');

    auto e = reinterpret_cast<BYTE *>(buffer.data());
    auto m = static_cast<DWORD>(buffer.size());

    if (RegQueryValueEx(hkey, key, nullptr, &dwType, e, &m) == ERROR_SUCCESS) {

      return {buffer.data(), static_cast<int>(m)};
    }
  }

  return {};
}

static QString _readRegistry(const char *subKey, const char *key, HKEY hkey) {
  auto s = unique_rsc(_reg_open_key, _reg_close_key, subKey, hkey);

  return _reg_get_value(s.get(), key);
}

QString utility::python3Path() {
  std::array<HKEY, 2> hkeys{HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE};

  std::string path = "Software\\Python\\PythonCore\\3.X\\InstallPath";

  char *str = &path[0];

  for (const auto &it : hkeys) {

    for (char s = '9'; s >= '0'; s--) {

      str[29] = s;

      auto c = _readRegistry(str, "ExecutablePath", it);

      if (!c.isEmpty()) {

        return c;
      }
    }
  }

  return {};
}

bool utility::platformIsWindows() { return true; }

bool utility::platformIsLinux() { return false; }

bool utility::platformIsOSX() { return false; }

#endif

utility::debug &utility::debug::operator<<(const QString &e) {
  return _print(e.toStdString().c_str());
}

utility::debug &utility::debug::operator<<(const QStringList &e) {
  if (e.isEmpty()) {

    return _print("()");
  } else {
    QString m = "(\"" + e.at(0) + "\"";

    for (int s = 1; s < e.size(); s++) {

      m += ", \"" + e.at(s) + "\"";
    }

    m += ")";

    return _print(m.toStdString().c_str());
  }
}

utility::debug &utility::debug::operator<<(const QByteArray &e) {
  return _print(e.data());
}

bool utility::Terminator::terminate(QProcess &exe) {
  if (utility::platformIsWindows()) {

    if (exe.state() == QProcess::ProcessState::Running) {

      QStringList args{"/F", "/T", "/PID", QString::number(exe.processId())};

      QProcess::startDetached("taskkill", args);
    }
  } else {
    exe.terminate();
  }

  return true;
}

bool utility::platformIsNOTWindows() { return !utility::platformIsWindows(); }

QMenu *utility::setUpMenu(const Context &ctx, const QStringList &,
                          bool addClear, bool addOpenFolder, bool combineText,
                          QWidget *parent) {
  auto menu = new QMenu(parent);

  auto &translator = ctx.Translator();
  auto &configure = ctx.TabManager().Configure();

  translator::entry ss(QObject::tr("Format Presets"), "Format Presets",
                       "Format Presets");
  auto ac = translator.addAction(menu, std::move(ss));

  ac->setEnabled(false);

  menu->addSeparator();

  configure.presetOptionsForEach(
      [&](const QString &uiName, const QString &options) {
        auto a = uiName;

        a.replace("Best-audiovideo", QObject::tr("Best-audiovideo"));
        a.replace("Best-audio", QObject::tr("Best-audio"));

        if (a == "Best-audio-with-thumbnail") {
          menu->addSeparator();
        }

        if (combineText) {

          menu->addAction(a)->setObjectName(options + "\n" + a);
        } else {
          menu->addAction(a)->setObjectName(options);
        }
      });

  if (addClear) {

    menu->addSeparator();

    translator::entry sx(QObject::tr("Clear"),
                         utility::selectedAction::CLEARSCREEN,
                         utility::selectedAction::CLEARSCREEN);

    translator.addAction(menu, std::move(sx));
  }

  if (addOpenFolder) {

    menu->addSeparator();

    translator::entry mm(QObject::tr("Open Download Folder"),
                         utility::selectedAction::OPENFOLDER,
                         utility::selectedAction::OPENFOLDER);

    translator.addAction(menu, std::move(mm));
  }

  return menu;
}

bool utility::hasDigitsOnly(const QString &e) {
  for (const auto &it : e) {

    if (!(it >= '0' && it <= '9')) {

      return false;
    }
  }

  return true;
}

QString utility::homePath() { return QDir::homePath(); }

void utility::waitForOneSecond() { utility::wait(1000); }

void utility::wait(int time) {
  QEventLoop e;

  util::Timer(time, [&]() { e.exit(); });

  e.exec();
}

void utility::openDownloadFolderPath(const QString &url) {
  utils::desktopOpenUrl(url);
}

QStringList utility::updateOptions(const updateOptionsStruct &s) {
  const engines::engine &engine = s.engine;
  const engines::enginePaths &ep = s.enginePaths;
  settings &settings = s.stts;
  const utility::args &args = s.args;
  const QString &indexAsString = s.indexAsString;
  const QStringList &urls = s.urls;
  bool forceDownload = s.forceDownload;
  const QString &downloadPath = settings.downloadFolder();

  auto opts = [&]() {
    return engine.defaultDownLoadCmdOptions();
  }();

  for (const auto &it : args.otherOptions()) {

    opts.append(it);
  }

  auto url = urls;

  engine.updateDownLoadCmdOptions(
      {args.quality(), args.otherOptions(), indexAsString, url, opts});

  opts.append(url);

  const auto &ca = engine.cookieArgument();
  const auto &cv = settings.cookieFilePath(engine.name());

  if (!ca.isEmpty() && !cv.isEmpty()) {
    opts.append(ca);
    opts.append(cv);
  }

  for (auto &it : opts) {

    it.replace(utility::stringConstants::mediaDownloaderDataPath(),
               ep.dataPath());
    it.replace(utility::stringConstants::mediaDownloaderDefaultDownloadPath(),
               downloadPath);
    it.replace(utility::stringConstants::mediaDownloaderCWD(),
               QDir::currentPath());
  }

  if (forceDownload) {

    utility::arguments(opts).removeOptionWithArgument("--download-archive");
  }

  return opts;
}

int utility::concurrentID() {
  static int id = -1;

  id++;

  return id;
}

QString utility::failedToFindExecutableString(const QString &cmd) {
  return QObject::tr("Failed to find executable \"%1\"").arg(cmd);
}

QString utility::clipboardText() {
  auto m = QApplication::clipboard();
  auto e = m->mimeData();

  if (e->hasText()) {

    return e->text();
  } else {
    return {};
  }
}

QString utility::downloadFolder(const Context &ctx) {
  return ctx.Settings().downloadFolder();
}

const QProcessEnvironment &utility::processEnvironment(const Context &ctx) {
  return ctx.Engines().processEnvironment();
}

void utility::saveDownloadList(const Context &ctx, QMenu &m,
                               tableWidget &tableWidget) {
  QObject::connect(m.addAction(QObject::tr("Save List To File")),
                   &QAction::triggered, [&ctx, &tableWidget]() {
                     auto e = QFileDialog::getSaveFileName(
                         &ctx.mainWidget(), QObject::tr("Save List To File"),
                         utility::homePath() + "/UMD4List.json");

                     if (!e.isEmpty()) {

                       QJsonArray arr;

                       tableWidget.forEach([&](const tableWidget::entry &e) {
                         using df = downloadManager::finishedStatus;
                         auto m = df::finishedWithSuccess(e.runningState);

                         if (e.url.isEmpty() || m) {
                           return;
                         }

                         arr.append([&]() {
                           QJsonObject obj;
                           obj.insert("url", e.url);
                           obj.insert("uiText", e.uiText);
                           return obj;
                         }());
                       });

                       auto stuff =
                           QJsonDocument(arr).toJson(QJsonDocument::Indented);

                       engines::file(e, ctx.logger()).write(stuff);
                     }
                   });
}

bool utility::isRelativePath(const QString &e) {
  return QDir::isRelativePath(e);
}

void utility::openGetListOptionHelp() {
  QDesktopServices::openUrl(
      QUrl("https://github.com/yt-dlp/yt-dlp#video-selection"));
}

void utility::openDownloadOptionHelp() {
  QDesktopServices::openUrl(
      QUrl("https://github.com/yt-dlp/yt-dlp#download-options"));
}

utility::MediaEntry::MediaEntry(const QString &url, const QByteArray &data)
    : m_json(data) {
  Q_UNUSED(url)

  if (m_json) {

    auto object = m_json.doc().object();

    m_title = object.value("title").toString();
    m_url = object.value("webpage_url").toString();
    m_uploadDate = object.value("upload_date").toString();
    m_id = object.value("id").toString();
    m_thumbnailUrl = object.value("thumbnail").toString();

    if (!m_uploadDate.isEmpty()) {

      m_uploadDate = QObject::tr("Upload Date:") + " " +
                     QDateTime::fromString(m_uploadDate, "yyyymmdd")
                         .toString("MM-dd-yyyy");
    }

    m_intDuration = object.value("duration").toInt();

    if (m_intDuration != 0) {

      auto s =
          engines::engine::functions::timer::duration(m_intDuration * 1000);
      m_duration = QObject::tr("Duration:") + " " + s;
    }
  }
}

QString utility::MediaEntry::uiText() const {
  // resolve title
  auto title = [&]() {
    if (m_title.isEmpty() || m_title == "\n") {

      return m_url;
    } else {
      return m_title;
    }
  }();

  // create final string
  if (m_duration.isEmpty()) {
    if (m_uploadDate.isEmpty()) {
      return title;
    } else {
      return title + "\n" + m_uploadDate;
    }
  } else {
    if (m_uploadDate.isEmpty()) {
      return title + "\n" + m_duration;
    } else {
      return title + "\n" + m_duration + "\n" + m_uploadDate;
    }
  }
}

const engines::engine &utility::resolveEngine(const tableWidget &table,
                                              const engines::engine &engine,
                                              const engines &engines, int row) {
  const auto &engineName = table.engineName(row);

  if (engineName.isEmpty()) {

    return engine;
  } else {
    const auto &ee = engines.getEngineByName(engineName);

    if (ee.has_value()) {

      return ee.value();
    } else {
      return engine;
    }
  }
}

QString utility::locale::formattedDataSize(qint64 s) const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  return m_locale.formattedDataSize(s);
#else
  std::array<const char *, 7> sizes = {"EiB", "PiB", "TiB", "GiB",
                                       "MiB", "KiB", "B"};

  qint64 multiplier = 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL;

  QString result;

  for (size_t i = 0; i < sizes.size(); i++, multiplier /= 1024) {

    if (s < multiplier) {

      continue;
    }

    if (s % multiplier == 0) {

      auto a = QString::number(s / multiplier);
      auto b = sizes[i];

      result = QString("%1 %2").arg(a, b);
    } else {
      auto a = static_cast<double>(s) / static_cast<double>(multiplier);
      auto b = sizes[i];
      auto c = QString::number(a, 'f', 2);

      result = QString("%1 %2").arg(c, b);
    }

    return result;
  }

  return {};
#endif
}

void utility::versionInfo::check(const engines::Iterator &iter) {
  const auto &engine = iter.engine();

  if (!engine.downloadUrl().isEmpty() && networkAccess::hasNetworkSupport()) {

    if (engine.backendExists()) {

      this->printEngineVersionInfo(iter);

    } else if (!engine.exePath().realExe().isEmpty()) {

      m_networkAccess->download(iter);
    }
  } else {
    if (engine.exePath().isEmpty()) {

      m_ctx->logger().add(
          QObject::tr("Failed to find version information, make sure \"%1\" is "
                      "installed and works properly")
              .arg(engine.name()));
    } else {
      this->printEngineVersionInfo(iter);
    }
  }
}

utility::versionInfo::~versionInfo() {}

void utility::versionInfo::printEngineVersionInfo(
    const engines::Iterator &iter) {
  const auto &engine = iter.engine();

  m_ctx->TabManager().disableAll();

  engines::engine::exeArgs::cmd cmd(engine.exePath(),
                                    {engine.versionArgument()});

  // auto exe = cmd.exe();

  // hide logger info
  m_ctx->logger().add(QObject::tr("Checking installed version of") + " " +
                      engine.name());

  if (!m_ctx->debug().isEmpty()) {

    auto exe = "cmd: \"" + cmd.exe() + "\"";

    for (const auto &it : cmd.args()) {

      exe += " \"" + it + "\"";
    }

    m_ctx->logger().add(exe);
  }

  util::run(
      cmd.exe(), cmd.args(),
      [iter, this](const util::run_result &r) {
        const auto &engine = iter.engine();

        if (r.success()) {

          auto &logger = m_ctx->logger();

          // hide logger info
          // logger.add( QObject::tr( "Found version" ) + ": " +
          // engine.versionString( r.stdOut ) ) ;
          logger.clear();
          logger.add(QObject::tr("Ready"));

          m_ctx->TabManager().enableAll();
        } else {
          m_ctx->logger().add(
              QObject::tr("Failed to find version information, make sure "
                          "\"%1\" is installed and works properly")
                  .arg(engine.name()));

          m_ctx->TabManager().enableAll();

          engine.setBroken();
        }

        if (iter.hasNext()) {

          this->check(iter.next());
        } else {
          emit vinfoDone();
        }
      },
      QProcess::ProcessChannelMode::MergedChannels);
}

bool utility::platformIs32Bit() {
  return QSysInfo::currentCpuArchitecture() != "x86_64";
}
