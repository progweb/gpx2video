#ifndef __GPX2VIDEO__VIDEOPARAMS_H__
#define __GPX2VIDEO__VIDEOPARAMS_H__

#include <iostream>
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


class VideoParams {
public:
	enum Format {
		FormatInvalid = -1,
		FormatUnsigned8,
		FormatUnsigned16,
		FormatFloat16,
		FormatFloat32,
		//FormatFloat64,
		FormatCount
	};

	enum Interlacing {
		InterlaceNone,
		InterlacedTopFirst,
		InterlacedBottomFirst
	};

	VideoParams();
	VideoParams(int width, int height,
			Format format, int nb_channels,
			int orientation,
			const AVRational &pixel_aspect_ratio,
			Interlacing interlacing);
	VideoParams(int width, int height, const AVRational &time_base,
			Format format, int nb_channels,
			int orientation,
			const AVRational &pixel_aspect_ratio,
			Interlacing interlacing);
	virtual ~VideoParams();

	const int& width(void) const;
	const int& height(void) const;

	const AVRational& timeBase(void) const;
	void setTimeBase(const AVRational &time_base);

	const AVRational& frameRate(void) const;
	void setFrameRate(const AVRational& frame_rate);

	const Format& format(void) const;

	const int& nbChannels(void) const;

	const int& orientation(void) const;

	const AVPixelFormat& pixelFormat(void) const;
	void setPixelFormat(const AVPixelFormat &format);

	const AVRational& pixelAspectRatio(void) const;

	const Interlacing& interlacing(void) const;

	static int getBytesPerChannel(Format format);
	int getBytesPerChannel(void) const {
		return getBytesPerChannel(format_);
	}

	static int getBytesPerPixel(Format format, int nb_channels);
	int getBytesPerPixel(void) const {
		return getBytesPerPixel(format_, nb_channels_);
	}

	static int getBufferSize(int width, int height, Format format, int nb_channels) {
		return width * height * getBytesPerPixel(format, nb_channels);
	}
	int getBufferSize(void) const {
		return getBufferSize(width_, height_, format_, nb_channels_);
	}

	static const int InternalChannelCount;

	static const int HSVChannelCount = 3;
	static const int RGBChannelCount = 3;
	static const int RGBAChannelCount = 4;

private:
	int width_;
	int height_;

	AVRational time_base_;
	AVRational frame_rate_;
	Format format_;
	int nb_channels_;
	int orientation_;
	AVPixelFormat pixel_format_;
	AVRational pixel_aspect_ratio_;
	Interlacing interlacing_;
};

#endif

