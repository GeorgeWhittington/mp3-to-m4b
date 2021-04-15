#include <avcpp/formatcontext.h>
#include <avcpp/codeccontext.h>
#include <avcpp/av.h>
#include <avcpp/audioresampler.h>
// Needs -I/usr/local/include/avcpp to compile the stuff from 
// /filters/ correctly, the pkg-config must be broken lol
#include <avcpp/filters/filtercontext.h>
#include <avcpp/filters/filtergraph.h>

extern "C"
{
  #include <libavutil/opt.h>
}

#include <iostream>
#include <inttypes.h>

// Code to transcode audio files to aac and remux to m4b (mpeg4)

// Rough Process:
// 1. Open input file
// 2. Open container (mp3, m4b, wav etc) to expose streams
// 3. Select a decoder for the codec of the stream wanted
// 4. Open output file
// 5. Select an encoder for the output file
// 6. Specify the output container
// 7. Write the header of the output file
// 8. Create a filter to pass the decoded audio from the decoder to the encoder
// 9. Decode, filter and encode the data, before writing it to the output file
// 10. Write the trailer of the output file

class TranscodeContext {
  public:
    int transcode(std::string input_filename, std::string output_filename);

  protected:
    int open_input_file(std::string filename);
    int open_output_file(std::string filename);
    int init_filters();

    av::FormatContext input_format_ctx;
    av::FormatContext output_format_ctx;

    av::AudioDecoderContext decode_ctx;
    av::AudioEncoderContext encode_ctx;

    av::FilterContext filter_src_ctx;
    av::FilterContext filter_sink_ctx;
    av::FilterGraph filter_graph;

    ssize_t audio_stream_ind;
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
  audio_stream_ind = -1;

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

  av::OutputFormat output_format("", "m4b", "");
  output_format_ctx.setFormat(output_format);

  av::Codec output_codec = av::findEncodingCodec(AV_CODEC_ID_AAC);

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

  encode_ctx.setTimeBase(av::Rational(1, decode_ctx.sampleRate()));

  if (output_format_ctx.outputFormat().flags() & AVFMT_GLOBALHEADER) {
    encode_ctx.setFlags2(encode_ctx.flags() || AV_CODEC_FLAG_GLOBAL_HEADER);
  }

  // open output file itself
  output_format_ctx.openOutput(filename, err);
  if (err) {
    std::cerr << "Could not open output file: " << filename << std::endl;
    return -1;
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

  // Need to add chapter data to context HERE

  // log with av_dump_format
  output_format_ctx.dump();

  // Init muxer by writing header to output
  output_format_ctx.writeHeader(err);
  if (err) {
    std::cerr << "Error occured writing header to output file" << std::endl;
    return -1;
  }
  output_format_ctx.flush();
}

// Create the filter graph, each of it's filter contexts and their attached filters
int TranscodeContext::init_filters() {
  // Go back and look at how transcoding.c decides to create a filter or not,
  // it seems not to when it doesn't need to (same encoding on input+output)
  // and just remuxes when we get to that step

  std::error_code err;

  // check if this gets set for me by avcpp, it could be
  if (!decode_ctx.channelLayout()) {
    decode_ctx.setChannelLayout(
      av_get_default_channel_layout(decode_ctx.channels()) );
  }

  av::Filter buffer_src("abuffer");
  av::Filter buffer_sink("abuffersink");
  if (buffer_src.isNull() && buffer_sink.isNull()) {
    std::cerr << "Filtering source or sink element not found" << std::endl;
    return -1;
  }

  char args[512];
  snprintf(args, sizeof(args),
          "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
          decode_ctx.timeBase().getNumerator(),
          decode_ctx.timeBase().getDenominator(),
          decode_ctx.sampleRate(),
          decode_ctx.sampleFormat().name(),
          decode_ctx.channelLayout());
    
  filter_src_ctx = filter_graph.createFilter(buffer_src, "in", args, err);
  if (err) {
    std::cerr << "Cannot create audio buffer source" << std::endl;
    return -1;
  }

  filter_sink_ctx = filter_graph.createFilter(buffer_sink, "out", "", err);
  if (err) {
    std::cerr << "Cannot create audio buffer sink" << std::endl;
    return -1;
  }

  // Might need to do filter_graph.allocFilter() here? Will see.

  // av_opt_set_bin causes a seg fault. Will need to learn more lldb
  // to figure out why, hahahaha

  // Set options on base C filter sink context obj
  // I think the orig code does this so it can support many
  // kinds of output? But since we're only going to ever export
  // aac wrapped in m4b, mayyyybe this could be squashed? Try later.
  // Hopefully this can just get shifted to the args input when
  // creating the filter sink context.

#if 0
  int status;
  status = av_opt_set_bin(filter_sink_ctx.raw(), "sample_fmts",
    (uint8_t*)encode_ctx.raw()->sample_fmt, sizeof(encode_ctx.raw()->sample_fmt),
    AV_OPT_SEARCH_CHILDREN);
  if (status < 0) {
    std::cerr << "Cannot set output sample format" << std::endl;
    return -1;
  }

  status = av_opt_set_bin(filter_sink_ctx.raw(), "channel_layouts",
    (uint8_t*)encode_ctx.channelLayout(), sizeof(encode_ctx.channelLayout()),
    AV_OPT_SEARCH_CHILDREN);
  if (status < 0) {
    std::cerr << "Cannot set output channel layout" << std::endl;
    return -1;
  }

  status = av_opt_set_bin(filter_sink_ctx.raw(), "sample_rates",
    (uint8_t*)encode_ctx.sampleRate(), sizeof(encode_ctx.sampleRate()),
    AV_OPT_SEARCH_CHILDREN);
  if (status < 0) {
    std::cerr << "Cannot set output channel sample rate" << std::endl;
    return -1;
  }
#endif

  // "anull" is the passthrough filter for audio
  filter_graph.parse("anull", filter_src_ctx, filter_src_ctx, err);
  if (err) {
    std::cerr << "Error parsing filter graph " << err << ", " << err.message() << std::endl;
    return -1;
  }

  filter_graph.config(err);
  if (err) {
    std::cerr << "Error configuring filter graph" << std::endl;
    return -1;
  }

  // May want to allocate frame for the filtering here
}

int TranscodeContext::transcode(std::string input_filename, std::string output_filename) {
  int status;
  std::error_code err;
  
  status = open_input_file(input_filename);
  if (status < 0) {
    return -1;
  }

  status = open_output_file(output_filename);
  if (status < 0) {
    return -1;
  }

  // status = init_filters();
  // if (status < 0) {
  //   return -1;
  // }

  av::AudioResampler resampler(
    encode_ctx.channelLayout(), encode_ctx.sampleRate(), encode_ctx.sampleFormat(),
    decode_ctx.channelLayout(), decode_ctx.sampleRate(), decode_ctx.sampleFormat());

  while (true) {
    av::Packet packet = input_format_ctx.readPacket(err);
    if (err) {
      std::cerr << "Error reading packet: " << err << ", " << err.message() << std::endl;
      break;
    } else if (!packet) {
      // EOF
      std::clog << "End of file reached" << std::endl;
      break;
    }
      
    if (packet.streamIndex() != audio_stream_ind)
      std::clog << "Packet read from non-audio stream, skipped" << std::endl;
      continue;
    
#if 1
    // remux and reencode
    
    // av_packet_rescale_ts(
    //   &packet.raw(),
    //   input_format_ctx.stream(audio_stream_ind).timeBase().raw(),
    //   decode_ctx.timeBase().raw());
    auto samples = decode_ctx.decode(packet, err);
    if (err) {
      std::cerr << "Error decoding packet" << std::endl;
      return -1;
    }

    std::clog << "  Samples [in]: " << samples.samplesCount()
         << ", ch: " << samples.channelsCount()
         << ", freq: " << samples.sampleRate()
         << ", name: " << samples.channelsLayoutString()
         << ", pts: " << samples.pts().seconds()
         << ", ref=" << samples.isReferenced() << ":" << samples.refCount()
         << std::endl;

    if (samples) {
      resampler.push(samples, err);
      if (err) {
        std::cerr << "Resampler push error" << std::endl;
        continue;
      }
    }

    bool get_all = !samples;
    while (true) {
      av::AudioSamples output_samples(
        encode_ctx.sampleFormat(), encode_ctx.frameSize(),
        encode_ctx.channelLayout(), encode_ctx.sampleFormat());
      
      bool has_frame = resampler.pop(output_samples, get_all, err);
      if (err) {
        std::clog << "Resampling status: " << err << ", text: " << err.message() << std::endl;
        break;
      } else if (!has_frame) {
        break;
      } else {
        std::clog << "  Samples [ou]: " << output_samples.samplesCount()
             << ", ch: " << output_samples.channelsCount()
             << ", freq: " << output_samples.sampleRate()
             << ", name: " << output_samples.channelsLayoutString()
             << ", pts: " << output_samples.pts().seconds()
             << ", ref=" << output_samples.isReferenced() << ":" << output_samples.refCount()
             << std::endl;
      }

      output_samples.setStreamIndex(0);
      output_samples.setTimeBase(encode_ctx.timeBase());

      av::Packet output_packet = encode_ctx.encode(output_samples, err);
      if (err) {
        std::cerr << "Encoding error" << std::endl;
        return -1;
      } else if (!output_packet) {
        std::clog << "Empty packet" << std::endl;
        continue;
      }

      output_packet.setStreamIndex(0);

      std::clog << "Write packet: pts=" 
                << output_packet.pts()
                << ", dts=" << output_packet.dts() 
                << " / " 
                << output_packet.pts().seconds() 
                << " / " 
                << output_packet.timeBase() 
                << " / st: " 
                << output_packet.streamIndex() 
                << std::endl;

      output_format_ctx.writePacket(output_packet, err);
      if (err) {
        std::cerr << "Error writing packet" << std::endl;
        return -1;
      }
    }

#else
    // remux frame without reencoding

    // writePacket also rescales packet, will have to see
    // if this function does anything more complex
    av_packet_rescale_ts(
      &packet.raw(),
      input_format_ctx.stream(audio_stream_ind).timeBase().raw(),
      output_format_ctx.stream(audio_stream_ind).timeBase().raw());
    packet.writePacket(packet, err);
    if (err) {
      std::cerr << "Error writing packet: " << err << ", " << err.message() << endl;
      return -1;
    }
#endif
  if (!packet && !samples)
    std::clog << "No packet, no samples" << std::endl;
    break;
  }

  // Flush encoder
  std::clog << "Flush encoder" << std::endl;
  while (true) {
    av::AudioSamples null(nullptr);
    av::Packet output_packet = encode_ctx.encode(null, err);
    if (err || !output_packet)
      break;
    
    output_packet.setStreamIndex(0);
    output_format_ctx.writePacket(output_packet, err);
    if (err) {
      std::cerr << "Error writing packet during encoder flush" << std::endl;
      return -1;
    }
  }

  output_format_ctx.flush();
  output_format_ctx.writeTrailer(err);
  if (err) {
    std::cerr << "error writing trailer" << std::endl;
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
  int status = transcode_ctx.transcode(input_filename, output_filename);
  if (status < 0) {
    return 1;
  }
}