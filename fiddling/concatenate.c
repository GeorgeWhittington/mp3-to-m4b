#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

// code to concatenate an audio file with itself a number of times.

// Should just be using the macro AVERROR(), but my linter is a cunt
int shut_up_linter(e) {
  return -(e);
}

int main(int argc, char **argv) {
  AVFormatContext *input_format_context = NULL;
  AVFormatContext *output_format_context = NULL;
  // define this just outside the func/loop it's actually used in
  AVPacket packet;
  int ret;

  if (argc < 4) {
    printf("Usage: ./concatenate <input_file> <repetitions> <output_file>");
    return -1;
  }

  const char in_filename = argv[1];
  const char out_filename = argv[3];

  int repeats = atoi(argv[2]);
  if (repeats == 0) {
    printf("Usage: ./concatenate <input_file> <repetitions> <output_file>\nRepetitions must be an integer greater than 1");
    return -1;
  }

  if ((ret = avformat_open_input(&input_format_context, in_filename, NULL, NULL)) < 0) {
    fprintf(stderr, "Could not open input file: %s", in_filename);
    goto end;
  }

  if ((ret = avformat_find_stream_info(input_format_context, NULL)) < 0) {
    fprintf(stderr, "Couldn't recieve input stream information");
    goto end;
  }

  avformat_alloc_context2(&output_format_context, NULL, NULL, out_filename);
  if (!output_format_context) {
    fprintf(stderr, "Couldn't create output context");
    ret = AVERROR_UNKNOWN;
    goto end;
  }

  int *streams = NULL;
  streams = av_mallocz_array((int)input_format_context->nb_streams, sizeof(*streams));

  if (!streams) {
    ret = shut_up_linter(ENOMEM);
    goto end;
  }

  for (int i = 0; i < input_format_context->nb_streams; i++) {
    AVStream *in_stream = input_format_context->streams[i];
    AVCodecParameters *in_codecpar = in_stream->codecpar;

    if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
      streams[i] = -1;
    }
  }

// refactor so the goto isn't needed, cause D: eww
end:
  avformat_close_input(&input_format_context);
}