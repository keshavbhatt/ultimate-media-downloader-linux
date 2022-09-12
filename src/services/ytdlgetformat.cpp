#include "ytdlgetformat.h"

#include <QTextCodec>

YtdlGetFormat::YtdlGetFormat(QObject *parent, QString engine_path,
                             QString media_url,
                             Format::preferred_format preferred_format)
    : QObject(parent), m_engine_path(engine_path), m_media_url(media_url) {

  m_preferred_format = preferred_format;
}

void YtdlGetFormat::start_ytdl_play_video_default() {

  emit started();

  m_ytdl_proccess = new QProcess(this);

  connect(m_ytdl_proccess, SIGNAL(finished(int)), this,
          SLOT(ytdl_play_default_finished(int)));

  m_ytdl_proccess->start("python3", get_play_default_arguments());
}

void YtdlGetFormat::ytdl_play_default_finished(const int &exitCode) {

  // TODO impl
  Q_UNUSED(exitCode);
}

QStringList YtdlGetFormat::get_play_default_arguments() {
  QStringList arg;

  // TODO impl

  return arg;
}

QString YtdlGetFormat::get_ytdl_program_name() { return m_engine_path; }

QStringList YtdlGetFormat::get_format_arguments() {

  QStringList arg;

  arg << get_ytdl_program_name() << "--no-warnings"
      << "-F" << m_media_url;

  return arg;
}

QStringList YtdlGetFormat::get_extract_url_arguments_for_combo() {

  QStringList arg;
  arg << get_ytdl_program_name() << "--no-warnings"
      << "-f" << get_preferred_video_code() + "+" + get_preferred_audio_code()
      << "--get-url" << m_media_url;

  return arg;
}

QStringList YtdlGetFormat::get_extract_url_arguments_for_other_codes() {

  QStringList arg;
  arg << get_ytdl_program_name() << "--no-warnings"
      << "-f" << m_other_codes.last() << "--get-url" << m_media_url;

  return arg;
}

QStringList YtdlGetFormat::get_extract_url_arguments_for_audio() {
  QStringList arg;

  arg << get_ytdl_program_name() << "--no-warnings"
      << "-f"
      << "bestaudio"
      << "--extract-audio"
      << "--get-url" << m_media_url;

  return arg;
}

void YtdlGetFormat::start_ytdl_play_video() {

  emit started();

  m_ytdl_proccess = new QProcess(this);

  connect(m_ytdl_proccess, SIGNAL(finished(int)), this,
          SLOT(ytdl_finished(int)));

  m_ytdl_proccess->start("python3", get_format_arguments());
}

void YtdlGetFormat::ytdl_finished(const int &exitCode) {

  m_resolution_list.clear();

  m_code_list.clear();

  QByteArray output_byte_array;

  if (m_ytdl_proccess != nullptr) {
    output_byte_array = m_ytdl_proccess->readAll();
  } else {
    emit error("Process not initialized");
  }

  QString output_string =
      QTextCodec::codecForMib(106)->toUnicode(output_byte_array).trimmed();

  if (exitCode == 0) {
    parse_formats_from_ytdl_output(output_string);
  } else {
    emit error(output_string);
  }
}

void YtdlGetFormat::parse_formats_from_ytdl_output(const QString &ytdl_output) {

  // split after resolution column and remove duplicates entries
  QStringList formats;
  if (ytdl_output.isEmpty() == false) {
    formats =
        ytdl_output.split("resolution", Qt::KeepEmptyParts, Qt::CaseInsensitive)
            .last()
            .split("\n");
  }
  formats.removeDuplicates();

  // process formats and populate codes
  foreach (const QString &format, formats) {
    if (format.isEmpty())
      continue;

    if (format.contains("audio only", Qt::CaseInsensitive)) {
      m_audio_codes << format.split(" ").first();
    }

    if (format.contains("video only", Qt::CaseInsensitive)) {
      auto code = format.split(" ").first();
      auto codec = format.split(code + " ").last().trimmed().split(" ").first();
      auto resolution =
          format.split(codec + " ").at(1).trimmed().split(" ").first();

      if (m_code_list.contains(code) == false &&
          m_resolution_list.contains(resolution) == false) {
        m_video_codes << code;
        m_resolution_list.append(resolution);
        m_code_list.append(code);
      }
    }
  }

  // formats combo is empty, look for non audio only and video only formats
  if (combo_codes_are_incompitable()) {
    foreach (const QString &format, formats) {
      if (format.isEmpty())
        continue;
      auto code = format.split(" ").first();
      // avoid formats that are aready in audio and video code list
      if (m_audio_codes.contains(code) == false &&
          m_video_codes.contains(code) == false) {
        qDebug() << code;
        m_other_codes << code;
      }
    }
    getUrlsForOtherFormats();
  } else {
    getUrlsForPreferredFormats();
  }
}

void YtdlGetFormat::start_ytdl_play_audio() {

  m_ytdl_proccess_get_url = new QProcess(this);

  connect(m_ytdl_proccess_get_url, SIGNAL(finished(int)), this,
          SLOT(getUrlForAudioProcessFinished(int)));

  m_ytdl_proccess_get_url->start("python3",
                                 get_extract_url_arguments_for_audio());

  emit startedGetUrl();
}

void YtdlGetFormat::getUrlForAudioProcessFinished(int code) {
  QByteArray output_byte_array;

  if (m_ytdl_proccess_get_url != nullptr) {
    output_byte_array = m_ytdl_proccess_get_url->readAll();
  } else {
    emit error("Process not initialized");
  }

  QString output_string =
      QTextCodec::codecForMib(106)->toUnicode(output_byte_array).trimmed();

  if (code == 0) {
    m_audio_url = output_string.trimmed();
  } else {
    emit error(output_string);
  }
  emit ready();
}

void YtdlGetFormat::getUrlsForOtherFormats() {

  m_ytdl_proccess_get_url = new QProcess(this);

  connect(m_ytdl_proccess_get_url, SIGNAL(finished(int)), this,
          SLOT(getUrlsForOtherFormatsProcessFinished(int)));

  m_ytdl_proccess_get_url->start("python3",
                                 get_extract_url_arguments_for_other_codes());

  emit startedGetUrl();
}

void YtdlGetFormat::getUrlsForOtherFormatsProcessFinished(int code) {
  QByteArray output_byte_array;

  if (m_ytdl_proccess_get_url != nullptr) {
    output_byte_array = m_ytdl_proccess_get_url->readAll();
  } else {
    emit error("Process not initialized");
  }

  QString output_string =
      QTextCodec::codecForMib(106)->toUnicode(output_byte_array).trimmed();

  if (code == 0) {
    m_other_url = output_string.trimmed();
  } else {
    emit error(output_string);
  }
  emit ready();
}

const QString &YtdlGetFormat::other_url() const { return m_other_url; }

void YtdlGetFormat::getUrlsForPreferredFormats() {

  m_ytdl_proccess_get_url = new QProcess(this);

  connect(m_ytdl_proccess_get_url, SIGNAL(finished(int)), this,
          SLOT(getUrlProcessFinished(int)));

  m_ytdl_proccess_get_url->start("python3",
                                 get_extract_url_arguments_for_combo());

  emit startedGetUrl();
}

void YtdlGetFormat::getUrlProcessFinished(int code) {
  QByteArray output_byte_array;

  if (m_ytdl_proccess_get_url != nullptr) {
    output_byte_array = m_ytdl_proccess_get_url->readAll();
  } else {
    emit error("Process not initialized");
  }

  QString output_string =
      QTextCodec::codecForMib(106)->toUnicode(output_byte_array).trimmed();

  if (code == 0) {
    auto urlList = output_string.split("\n");
    m_video_url = urlList.first();
    m_audio_url = urlList.last();
  } else {
    emit error(output_string);
  }
  emit ready();
}

bool YtdlGetFormat::combo_codes_are_incompitable() const {
  return m_video_codes.isEmpty() || m_video_codes.isEmpty();
}

const QStringList &YtdlGetFormat::video_codes() const { return m_video_codes; }

const QStringList &YtdlGetFormat::audio_codes() const { return m_audio_codes; }

QString YtdlGetFormat::get_preferred_audio_code() {
  switch (m_preferred_format) {
  case Format::HIGH:
    return m_audio_codes.last();
    break;
  case Format::MEDIUM:
    return m_audio_codes.at(m_audio_codes.count() / 2);
    break;
  case Format::LOW:
    return m_audio_codes.first();
    break;
  default:
    return m_audio_codes.last();
    break;
  }
}

QString YtdlGetFormat::get_preferred_video_code() {
  switch (m_preferred_format) {
  case Format::HIGH:
    return m_video_codes.last();
    break;
  case Format::MEDIUM:
    return m_video_codes.at(m_video_codes.count() / 2);
    break;
  case Format::LOW:
    return m_video_codes.first();
    break;
  default:
    return m_video_codes.last();
    break;
  }
}

const QString &YtdlGetFormat::audio_url() const { return m_audio_url; }

const QString &YtdlGetFormat::video_url() const { return m_video_url; }
