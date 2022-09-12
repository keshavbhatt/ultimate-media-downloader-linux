#ifndef PLAYERSERVICE_H
#define PLAYERSERVICE_H

#include <QDebug>
#include <QObject>
#include <settings.h>

class PlayerService : public QObject {
  Q_OBJECT
public:
  explicit PlayerService(QObject *parent = nullptr, QString engine_path = "",
                         const settings &_settings = settings());

  void clearSocketDir();
signals:
  void ready();    // implies that the player process is ready
  void finished(); // implies the player process finished
  void started();  // implies that the player proc has started

public slots:
  void comboPlay(QString video_url_str, QString audio_url_str);

  void play(const QString &audio_url_str);
  void setPlayerTitle(const QString &title);

private slots:
  void playerFinished(int code);
  void playerReadyRead();

private:
  QString m_socket_file_path;
  QString m_player_title;
  QString m_engine_path;
  QString m_mpv_conf_path;

  QString M_TMP_PATH;
  QString M_SOCKET_DIR;
};

#endif // PLAYERSERVICE_H
