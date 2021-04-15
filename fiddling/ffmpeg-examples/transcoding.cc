#include <avcpp/formatcontext.h>
#include <avcpp/codeccontext.h>
#include <avcpp/av.h>

#include <iostream>

// Code to transcode audio files to aac and remux to m4b (mpeg4)

class TranscodeContext {
  public:
    int open_input_file(std::string filename);
    int open_output_file(std::string filename);

  protected:
    av::FormatContext input_format_ctx;
    av::FormatContext output_format_ctx;

    av::AudioDecoderContext decode_ctx;
    av::AudioEncoderContext encode_ctx;
};

bool has_ending(std::string const &full_string, std::string const &ending) {
  if (full_string.length() >= ending.length()) {
    return (0 == full_string.compare (full_string.length() - ending.length(), ending.length(), ending));
  } else {
    return false;
  }
}

// Open the input file, find decoder for the audio stream,
// open the decoder
int TranscodeContext::open_input_file(std::string filename) {
  std::error_code err;

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

  decode_ctx = av::AudioDecoderContext(audio_stream);
  av::Codec codec = av::findDecodingCodec(decode_ctx.raw()->codec_id);
  decode_ctx.open(av::Codec(), err);
  if (err) {
    std::cerr << "Can't open decoder" << std::endl;
    return -1;
  }

  // Create a frame? dunno.
}

// Prepare the encoder context and the stream that will be written to.
// Open the output file, add chapter metadata, write the header to the
// output file
int TranscodeContext::open_output_file(std::string filename) {
  // To add chapter data! See here: https://www.ffmpeg.org/doxygen/trunk/structAVChapter.html
  // Probably just need to create chapters, add them to output_format_ctx
  // and they'll write automatically!

  // General code to add chapters:

  // av::Dictionary metadata;
  // metadata.set("title", "Chapter One");
  // AVChapter *chapter;
  // chapter->id = 0;
  // chapter->time_base = av::Rational(1000, 1);
  // chapter->start = 0;
  // chapter->end = 10000; // 10s in? I think.
  // chapter->metadata = metadata.raw();
  // AVChapter **chapters;
  // chapters[0] = chapter;
  // output_format_ctx.raw()->chapters = chapters;
  // output_format_ctx.raw()->nb_chapters = 1;

  // Now that I know how to add chapters, need to decide when exact duration data
  // will be collected. One pass decoding full file to null buffer def needed
  // to get the exact length of each file, but will have to decide if that's done
  // on original load to table, or in a separate step just after convert button
  // pressed, before actual conversion.

  std::error_code err;

  av::guessOutputFormat("", "m4b", "");

  av::Codec output_codec = av::findEncodingCodec(AV_CODEC_ID_AAC);
  if (output_codec.canEncode()) {
    // Will have to see if this is enough of a check?
    std::cerr << "Encoder fetched cannot encode" << std::endl;
    return -1;
  }

  auto out_stream = output_format_ctx.addStream(output_codec, err);
  if (err) {
    std::cerr << "Couldn't add a stream to the output context" << std::endl;
    return -1;
  }

  encode_ctx = av::AudioEncoderContext(output_codec);

  encode_ctx.setChannels(2);
  encode_ctx.setChannelLayout(av_get_default_channel_layout(2));
  // cloning sample stuff, once there's multiple inputs will need to
  // convert inputs to a single output sample rate, maybe reference:
  // https://github.com/h4tr3d/avcpp/blob/master/example/api2-samples/api2-decode-rasample-audio.cpp
  // for that.
  encode_ctx.setSampleRate(decode_ctx.sampleRate());
  encode_ctx.setSampleFormat(output_codec.supportedSampleFormats()[0]);
  encode_ctx.setBitRate(96000);

  encode_ctx.setStrict(FF_COMPLIANCE_EXPERIMENTAL);

  encode_ctx.setTimeBase(av::Rational(decode_ctx.sampleRate(), 1));

  if (output_format_ctx.outputFormat().flags() & AVFMT_GLOBALHEADER) {
    encode_ctx.setFlags2(encode_ctx.flags() || AV_CODEC_FLAG_GLOBAL_HEADER);
  }

  encode_ctx.open(err);
  if (err) {
    std::cerr << "Can't open codec" << std::endl;
    return -1;
  }

  int status = avcodec_parameters_from_context(out_stream.raw()->codecpar, encode_ctx.raw());
  if (status < 0) {
    std::cerr << "Failed to copy encoder parameters to output stream" << std::endl;
    return status;
  }

  out_stream.setTimeBase(encode_ctx.timeBase());

  // log
  av_dump_format(output_format_ctx.raw(), 0, filename.c_str(), 1);

  // open output file itself
  output_format_ctx.openOutput(filename, err);
  if (err) {
    std::cerr << "Could not open output file: " << filename << std::endl;
    return -1;
  }

  // Need to add chapter data to context HERE

  // Init muxer by writing header to output
  output_format_ctx.writeHeader(err);
  if (err) {
    std::cerr << "Error occured writing header to output file" << std::endl;
    return -1;
  }
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

  TranscodeContext transcode_ctx;

  int status = transcode_ctx.open_input_file(input_filename);
  if (status < 0) {
    return 1;
  }

  status = transcode_ctx.open_output_file(output_filename);
  if (status < 0) {
    return 1;
  }
}