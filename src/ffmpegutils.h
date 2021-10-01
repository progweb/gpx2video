#ifndef __GPX2VIDEO__FFMPEGUTILS_H__
#define __GPX2VIDEO__FFMPEGUTILS_H__

#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "videoparams.h"


class FFmpegUtils {
public:
	static AVPixelFormat getCompatiblePixelFormat(const AVPixelFormat &pix_fmt);
	static VideoParams::Format getCompatiblePixelFormat(const VideoParams::Format &format);
	static AVPixelFormat getFFmpegPixelFormat(const VideoParams::Format &format, int nb_channels);
};

#endif

