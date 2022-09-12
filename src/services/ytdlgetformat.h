#ifndef YTDLGETFORMAT_H
#define YTDLGETFORMAT_H

#include <QDebug>
#include <QObject>
#include <QProcess>

class Format {
public:
  enum preferred_format { HIGH, MEDIUM, LOW };
};

class YtdlGetFormat : public QObject {
  Q_OBJECT
public:
  explicit YtdlGetFormat(QObject *parent = nullptr, QString engine_path = "",
                         QString media_url = "",
                         Format::preferred_format preferred_format =
                             Format::preferred_format::MEDIUM);

  const QStringList &audio_codes() const;

  const QStringList &video_codes() const;

  bool combo_codes_are_incompitable() const;

  const QString &video_url() const;

  const QString &audio_url() const;

  const QString &other_url() const;

public slots:
  void start_ytdl_play_video();
  void start_ytdl_play_audio();
  void start_ytdl_play_video_default();
signals:
  void started(); // implies that the process to parse formats has started
  void ready();   // implies that the formats are ready
  void error(const QString &error); // implies that the process has got error
                                    // while execution
  void startedGetUrl();             // implies that the geturl proc has started

private slots:
  QStringList get_format_arguments();

  void ytdl_finished(const int &exitCode);
  QString get_ytdl_program_name();
  void parse_formats_from_ytdl_output(const QString &ytdl_output);

  void getUrlsForPreferredFormats();
  QStringList get_extract_url_arguments_for_combo();
  void getUrlProcessFinished(int code);

  QString get_preferred_audio_code();
  QString get_preferred_video_code();
  QStringList get_extract_url_arguments_for_audio();
  void ytdl_play_default_finished(const int &exitCode);
  QStringList get_play_default_arguments();
  void getUrlForAudioProcessFinished(int code);

  void getUrlsForOtherFormats();
  QStringList get_extract_url_arguments_for_other_codes();

  void getUrlsForOtherFormatsProcessFinished(int code);
private:
  QString m_engine_path;
  QString m_media_url;
  QProcess *m_ytdl_proccess = nullptr;
  QProcess *m_ytdl_proccess_get_url = nullptr;
  QStringList m_resolution_list, m_code_list;
  QStringList m_audio_codes, m_video_codes, m_other_codes;
  Format::preferred_format m_preferred_format;
  QString m_video_url, m_audio_url, m_other_url;
};

#endif // YTDLGETFORMAT_H
