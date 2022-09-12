#include "mainwindow.h"
#include "settings.h"
#include "translator.h"
#include "util.hpp"
#include "utility"

#include <QMessageBox>

class myApp {
public:
  struct args {
    QApplication &app;
    settings &Settings;
    const QStringList &args;
  };
  myApp(const myApp::args &args)
      : m_traslator(args.Settings, args.app),
        m_app(args.app, args.Settings, m_traslator, args.args) {}
  void start(const QByteArray &e) {
    m_app.Show();
    m_app.processEvent(e);
  }
  void exit() { m_app.quitApp(); }
  void event(const QByteArray &e) { m_app.processEvent(e); }

private:
  translator m_traslator;
  MainWindow m_app;
};

int main(int argc, char *argv[]) {

  const auto m = utility::Terminator::terminate(argc, argv);

  if (m) {

    return m.value();
  } else {

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
            "--disable-logging "
            "--no-sandbox "
            "--ignore-gpu-blocklist "
            "--disable-extensions"); // --single-process

    QApplication mqApp(argc, argv);

    mqApp.setApplicationName(APPLICATION_NAME);
    mqApp.setOrganizationName("org.keshavnrj.ubuntu");
    mqApp.setApplicationVersion(VERSIONSTR);

    settings settings;

    settings.setTheme(mqApp);

    auto args = mqApp.arguments();

    auto spath = engines::enginePaths(settings).socketPath();

    utility::arguments opts(args);

    QJsonObject jsonArgs;

    jsonArgs.insert("-u", opts.hasValue("-u"));
    jsonArgs.insert("-a", opts.hasOption("-a"));
    jsonArgs.insert("-s", opts.hasOption("-s"));

    auto json = QJsonDocument(jsonArgs).toJson(QJsonDocument::Indented);

    myApp::args mArgs{mqApp, settings, args};

    auto instanceArgs = util::make_oneinstance_args(
        [&]() {
          std::cout << QString(
                           "Another instance running of %1 is already running")
                           .arg(APPLICATION_NAME)
                           .toStdString()
                    << std::endl;
          QMessageBox::information(
              0, APPLICATION_NAME,
              QString("Another instance running of %1 is already running")
                  .arg(APPLICATION_NAME));
          mqApp.exit();
        },
        []() {
          std::cout
              << QString("Previous instance of %1 seem to have crashed,trying "
                         "to clean up before starting new one!")
                     .arg(APPLICATION_NAME)
                     .toStdString()
              << std::endl;
        });

    using type = decltype(instanceArgs);

    util::oneinstance<myApp, myApp::args, type> instance(
        spath, json, std::move(mArgs), std::move(instanceArgs));
    return mqApp.exec();
  }
}
