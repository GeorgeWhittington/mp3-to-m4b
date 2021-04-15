#include <avcpp/formatcontext.h>
#include <avcpp/codeccontext.h>
#include <avcpp/av.h>

#include <iostream>

// Code to transcode to aac and remux to m4b (mpeg4)

bool has_ending(std::string const &full_string, std::string const &ending) {
  if (full_string.length() >= ending.length()) {
    return (0 == full_string.compare (full_string.length() - ending.length(), ending.length(), ending));
  } else {
    return false;
  }
}

int open_input_file(std::string filename) {
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

  // Loop through streams, find the audio one
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

  if (audio_stream.isNull() || !audio_stream.isValid()) {
    std::cerr << "Audio stream not found/invalid" << std::endl;
    return -1;
  }

  av::AudioDecoderContext decode_ctx;
  decode_ctx = av::AudioDecoderContext(audio_stream);
  av::Codec codec = av::findDecodingCodec(decode_ctx.raw()->codec_id);
  decode_ctx.open(av::Codec(), err);
  if (err) {
    std::cerr << "Can't open decoder" << std::endl;
    return -1;
  }

  // Create a frame? dunno.
}

int open_output_file(std::string filename) {

}

int main(int argc, char **argv) {
  av::init();
  av::setFFmpegLoggingLevel(AV_LOG_DEBUG);

  if (argc < 3) {
    std::cout <<  "Usage: ./transcoding <input file> <output file>" << std::endl;
    return 1;
  }

  std::string input_filename = argv[1];
  std::string output_filename = argv[2];

  if (!has_ending(output_filename, ".m4b")) {
    std::cout << "Invalid output filename, this code only exports m4b files" << std::endl;
    return 1;
  }

  // Probably instantiate objects here and pass them in?
  // (But ovs refactor that in later, for now just make them inside)
  int status = open_input_file(input_filename);
  if (status < 0) {
    return 1;
  }

  status = open_output_file(output_filename);
  if (status < 0) {
    return 1;
  }
}