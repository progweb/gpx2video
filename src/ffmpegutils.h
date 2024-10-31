#ifndef __GPX2VIDEO__FFMPEGUTILS_H__
#define __GPX2VIDEO__FFMPEGUTILS_H__

#include <string>
#include <vector>

extern "C" {
#include <libavutil/eval.h>
#include <libavutil/opt.h>
#include <libavutil/display.h>
#include <libavutil/channel_layout.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

}

#include "exportcodec.h"
#include "audioparams.h"
#include "videoparams.h"


#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(59, 24, 100)
#define HAVE_FFMPEG_CH_LAYOUT
#define HAVE_FFMPEG_FLAGS_FRAME
#define HAVE_FFMPEG_API_SIDE_DATA
#endif


class FFmpegUtils {
public:
	static const AVCodec * getEncoder(ExportCodec::Codec c);

	static AVSampleFormat getFFmpegSampleFormat(const AudioParams::Format &format);

	static AVPixelFormat getCompatiblePixelFormat(const AVPixelFormat &pix_fmt);
	static VideoParams::Format getCompatiblePixelFormat(const VideoParams::Format &format);
	static AVPixelFormat getFFmpegPixelFormat(const VideoParams::Format &format, int nb_channels);

	static AVPixelFormat overrideFFmpegDeprecatedPixelFormat(const AVPixelFormat &pix_fmt);

	static uint8_t *newSideData(AVStream* stream, enum AVPacketSideDataType type, size_t size);
	static uint8_t *getSideData(AVStream* stream, enum AVPacketSideDataType type);
};

#endif

