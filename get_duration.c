#ifdef __cplusplus
extern "C" {
#endif

#include "get_duration.h"

static AVFormatContext *input_format_ctx;

typedef struct StreamContext {
    AVCodecContext *decode_ctx;
    AVFrame *decode_frame;
} StreamContext;
static StreamContext *stream_ctx;

int open_input_file(const char *filename, long long int *duration) {
  int ret;
  unsigned int i;

  input_format_ctx = NULL;
  if ((ret = avformat_open_input(&input_format_ctx, filename, NULL, NULL)) < 0) {
    av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
    return ret;
  }

  if ((ret = avformat_find_stream_info(input_format_ctx, NULL)) < 0) {
    av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
    return ret;
  }

  stream_ctx = (StreamContext*)av_mallocz_array(input_format_ctx->nb_streams, sizeof(*stream_ctx));
  if (!stream_ctx) {
    return AVERROR(ENOMEM);
  }

  for (i = 0; i < input_format_ctx->nb_streams; i++) {
    AVStream *stream = input_format_ctx->streams[i];
    AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
    AVCodecContext *codec_ctx;

    if (!dec) {
      av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
      return AVERROR_DECODER_NOT_FOUND;
    }

    codec_ctx = avcodec_alloc_context3(dec);
    if (!codec_ctx) {
      av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
      return AVERROR(ENOMEM);
    }

    ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
    if (ret < 0) {
      av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context for stream #%u\n", i);
      return ret;
    }

    if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
      ret = avcodec_open2(codec_ctx, dec, NULL);
      if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
        return ret;
      }
    }

    stream_ctx[i].decode_ctx = codec_ctx;

    stream_ctx[i].decode_frame = av_frame_alloc();
    if (!stream_ctx[i].decode_frame) {
      return AVERROR(ENOMEM);
    }
  }

  // av_dump_format(input_format_ctx, 0, filename, 0);

  // Sourced from av_dump_format:
  // av_log(NULL, AV_LOG_INFO, "Duration in microseconds: ");
  if (input_format_ctx->duration != AV_NOPTS_VALUE) {
    int64_t duration_microseconds = input_format_ctx->duration + (input_format_ctx->duration <= INT64_MAX - 5000 ? 5000 : 0);
    // av_log(NULL, AV_LOG_INFO, "%d\n", duration_microseconds);
    *duration = (long long int)duration_microseconds;
  } else {
    // av_log(NULL, AV_LOG_INFO, "N/A\n");
    *duration = -1;
  }

  return 0;
}

long long int get_audio_duration(const char *filename) {
  long long int duration;
  int ret, i;

  ret = open_input_file(filename, &duration);

  // free resources before close
  for (i = 0; i < input_format_ctx->nb_streams; i++) {
    avcodec_free_context(&stream_ctx[i].decode_ctx);
    av_frame_free(&stream_ctx[i].decode_frame);
  }
  av_free(stream_ctx);
  avformat_close_input(&input_format_ctx);

  if (ret < 0) {
    char a[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_log(NULL, AV_LOG_ERROR, "Error occurred: %s\n", av_make_error_string(a, AV_ERROR_MAX_STRING_SIZE, ret) );
  }

  if (ret) {
    return -1;
  } else {
    return duration;
  }
}

#ifdef __cplusplus
}
#endif