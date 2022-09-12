#include "playerservice.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>

PlayerService::PlayerService(QObject *parent, QString engine_path,
                             const settings &_settings)
    : QObject{parent}, m_engine_path(engine_path) {

  M_TMP_PATH = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
  M_SOCKET_DIR = M_TMP_PATH + QDir::separator() +
                 QApplication::applicationName() + "_sockets";

  QDir dir(M_SOCKET_DIR);
  if (!dir.exists())
    dir.mkpath(M_SOCKET_DIR);

  QString fifo_file_name =
      QString::number(QDateTime::currentMSecsSinceEpoch()) + ".socket";

  // delete fifoFiles which are older then 10hr
  QStringList fifo_file_info_list =
      dir.entryList(QDir::System); // filter devices files
  qint64 current_time = QDateTime::currentMSecsSinceEpoch();
  foreach (const QString &fifo_file_info, fifo_file_info_list) {
    if (current_time - QString(fifo_file_info)
                           .remove(".socket")
                           .remove("_videoPlayer")
                           .toLong() >
        36000000) {
      QFile::remove(M_SOCKET_DIR + QDir::separator() + fifo_file_info +
                    ".socket");
    }
  }

  // assign socket file NOTE: socket file will be created by mpv
  m_socket_file_path = M_SOCKET_DIR + QDir::separator() + fifo_file_name;

  // customize mpv conf before using it
  // we are using a custom dir in case of non snap|flatpak environment to not
  // mess with user's mpv conf
  QString snapEnvVarName = "SNAP";
  bool is_snap_install =
      qEnvironmentVariableIsSet(snapEnvVarName.toUtf8().constData());

  QString flatpakVarName =
      "PACKAGE_TYPE"; // we set this in our flatpak manifest
  QByteArray package_type = qgetenv(flatpakVarName.toUtf8());

  if (is_snap_install || package_type.contains("flatpak")) {
    m_mpv_conf_path =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
        QDir::separator() + "mpv" + QDir::separator();
  } else {
    m_mpv_conf_path =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
        QDir::separator() + QApplication::applicationName() + QDir::separator();
  }

  QDir mpv_conf_dir(m_mpv_conf_path);
  mpv_conf_dir.mkpath(".");

  QFile file(m_mpv_conf_path + "mpv.conf");
  file.open(QIODevice::WriteOnly | QIODevice::Text);

  // ensure mpv uses our copy of download engine
  QString p = "script-opts-append=ytdl_hook-ytdl_path=" + m_engine_path;

  // initial mpv volume
  p.append(
      "\nvolume=" +
      QString::number(_settings.getValue("playerStartupVolume", 50).toInt()));

  file.write(p.toUtf8());
  file.close();
}

void PlayerService::clearSocketDir() {
  QDir dir(M_SOCKET_DIR);
  dir.removeRecursively();
  dir.mkpath(".");
}

void PlayerService::play(const QString &audio_url_str) {

  QProcess *player = new QProcess(this);

  connect(player, SIGNAL(finished(int)), this, SLOT(playerFinished(int)));
  connect(player, SIGNAL(readyRead()), this, SLOT(playerReadyRead()));

  player->start("mpv", QStringList()
                           << "--config-dir=" + m_mpv_conf_path
                           << "--title=MPV for " +
                                  QApplication::applicationName() + " - " +
                                  m_player_title
                           << "--no-ytdl"
                           << "--force-window" << audio_url_str
                           << "--input-ipc-server=" + m_socket_file_path);
  emit started();
}

void PlayerService::comboPlay(QString video_url_str, QString audio_url_str) {

  QProcess *player = new QProcess(this);

  connect(player, SIGNAL(finished(int)), this, SLOT(playerFinished(int)));
  connect(player, SIGNAL(readyRead()), this, SLOT(playerReadyRead()));

  player->start("mpv", QStringList()
                           << "--config-dir=" + m_mpv_conf_path
                           << "--title=MPV for " +
                                  QApplication::applicationName() + " - " +
                                  m_player_title
                           << "--no-ytdl" << video_url_str
                           << "--audio-file=" + audio_url_str
                           << "--input-ipc-server=" + m_socket_file_path);
  emit started();
}

void PlayerService::setPlayerTitle(const QString &title) {
  m_player_title = title;
}

void PlayerService::playerFinished(int code) {
  Q_UNUSED(code);

  emit finished();
  sender()->deleteLater();
}

void PlayerService::playerReadyRead() { emit ready(); }
