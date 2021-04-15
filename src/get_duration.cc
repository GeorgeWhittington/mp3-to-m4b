#include "get_duration.h"

// Code adapted from:
// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/transcoding.c
// https://github.com/h4tr3d/avcpp/blob/master/example/api2-samples/api2-decode-audio.cpp

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
    auto stream = input_format_ctx.stream(i);
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

    auto timestamp = input_format_ctx.duration();
    return timestamp.timestamp();
  }
}

// Write another function that finds *exact* duration,
// for chapter length purposes. 