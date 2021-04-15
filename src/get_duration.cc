#include <iostream>
#include <string>

#include <avcpp/formatcontext.h>
#include <avcpp/codeccontext.h>
#include <avcpp/av.h>

#include "get_duration.h"

// Code adapted from:
// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/transcoding.c
// https://github.com/h4tr3d/avcpp/blob/master/example/api2-samples/api2-decode-audio.cpp

// Find approximate duration of file in microseconds
// returning less than 0 means error
long long int get_audio_duration(std::string filename) {
  av::init();
  av::setFFmpegLoggingLevel(AV_LOG_ERROR);

  std::error_code err;
  av::FormatContext input_format_ctx;

  input_format_ctx.openInput(filename, err);
  if (err) {
    std::cerr << "Can't open input" << std::endl;
    return -1;
  }

  input_format_ctx.findStreamInfo(err);
  if (err) {
    std::cerr << "Can't find stream info" << std::endl;
    return -1;
  }

  av::Stream audio_stream;
  ssize_t audio_stream_ind = -1;

  for (size_t i = 0; i < input_format_ctx.streamsCount(); ++i) {
    av::Stream stream = input_format_ctx.stream(i);
    if (stream.isAudio()) {
      audio_stream_ind = i;
      audio_stream = stream;
      break;
    }
  }

  if (audio_stream.isNull()) {
    std::cerr << "Audio stream not found" << std::endl;
    return -1;
  }

  av::AudioDecoderContext decode_ctx;

  if (audio_stream.isValid()) {
    decode_ctx = av::AudioDecoderContext(audio_stream);
    av::Codec codec = av::findDecodingCodec(decode_ctx.raw()->codec_id);
    decode_ctx.open(av::Codec(), err);
    if (err) {
      std::cerr << "Can't open codec" << std::endl;
      return -1;
    }

    av::Timestamp timestamp = input_format_ctx.duration();
    return timestamp.timestamp();
  }
}

// Find exact duration of audio containing file in seconds
// returning less than 0 means error
double get_audio_duration_exact(std::string filename) {
  av::init();
  av::setFFmpegLoggingLevel(AV_LOG_ERROR);

  std::error_code err;
  av::FormatContext input_format_ctx;

  input_format_ctx.openInput(filename, err);
  if (err) {
    std::cerr << "Can't open input" << std::endl;
    return -1;
  }

  input_format_ctx.findStreamInfo(err);
  if (err) {
    std::cerr << "Can't find stream info" << std::endl;
    return -1;
  }

  av::Stream audio_stream;
  ssize_t audio_stream_ind = -1;

  for (size_t i = 0; i < input_format_ctx.streamsCount(); ++i) {
    av::Stream stream = input_format_ctx.stream(i);
    if (stream.isAudio()) {
      audio_stream_ind = i;
      audio_stream = stream;
      break;
    }
  }

  if (audio_stream.isNull()) {
    std::cerr << "Audio stream not found" << std::endl;
    return -1;
  }

  av::AudioDecoderContext decode_ctx;

  if (!audio_stream.isValid()) {
    std::cerr << "Audio stream is invalid" << std::endl;
    return -1;
  }

  decode_ctx = av::AudioDecoderContext(audio_stream);
  av::Codec codec = av::findDecodingCodec(decode_ctx.raw()->codec_id);
  decode_ctx.setCodec(codec);

  decode_ctx.open(av::Codec(), err);
  if (err) {
    std::cerr << "Can't open codec" << std::endl;
    return -1;
  }

  // Substract start time from the Packet PTS and DTS values: PTS starts from the zero
  input_format_ctx.substractStartTime(true);

  av::Timestamp pts;

  while (true) {
    av::Packet packet = input_format_ctx.readPacket(err);
    if (err) {
      std::cerr << "Error reading packet" << std::endl;
      return -1;
    }

    // EOF
    if (!packet) {
      break;
    }

    if (packet.streamIndex() != audio_stream_ind) {
      continue;
    }

    // track latest duration read from packet
    pts = packet.pts();
  }

  double duration = pts.seconds();
  return duration;
}