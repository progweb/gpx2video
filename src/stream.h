#ifndef __GPX2VIDEO__STREAM_H__
#define __GPX2VIDEO__STREAM_H__

#include <iostream>
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "videoparams.h"


class Stream;
class AudioStream;
class VideoStream;
class MediaContainer;

using StreamPtr = std::shared_ptr<Stream>;
using AudioStreamPtr = std::shared_ptr<AudioStream>;
using VideoStreamPtr = std::shared_ptr<VideoStream>;


class Stream {
public:
	Stream();
	virtual ~Stream();

	MediaContainer * container(void) const;
	void setContainer(MediaContainer *container);

	const int& index(void) const;
	void setIndex(const int &index);

	const AVMediaType& type(void) const;
	void setType(const AVMediaType &type);

	const AVRational& timeBase(void) const;
	void setTimeBase(const AVRational &time_base);

	const int64_t& duration(void) const;
	void setDuration(const int64_t &duration);

private:
	int index_;

	MediaContainer *container_;

	AVMediaType type_;
	AVRational time_base_;
	int64_t duration_;
};


class AudioStream : public Stream {
public:
	AudioStream();
	virtual ~AudioStream();
};


class VideoStream : public Stream {
public:
	VideoStream();
	virtual ~VideoStream();

	const int& width(void) const;
	void setWidth(const int &width);

	const int& height(void) const;
	void setHeight(const int &height);

	const VideoParams::Format& format(void) const;
	void setFormat(const VideoParams::Format &format);

	const VideoParams::Interlacing& interlacing(void) const;
	void setInterlacing(const VideoParams::Interlacing &interlacing);

	const AVRational& frameRate(void) const;
	void setFrameRate(const AVRational &rate);

	const AVPixelFormat& pixelFormat(void) const;
	void setPixelFormat(const AVPixelFormat &format);

	const AVRational& pixelAspectRatio(void) const;
	void setPixelAspectRatio(const AVRational &ratio);

	const int& nbChannels(void) const;
	void setNbChannels(const int &nb_channels);

	int64_t getTimeInTimeBaseUnits(const AVRational& time) const;

private:
	int width_;
	int height_;

	VideoParams::Format format_;
	VideoParams::Interlacing interlacing_;

	AVRational frame_rate_;
	AVPixelFormat pixel_format_;
	AVRational pixel_aspect_ratio_;
	int nb_channels_;
};

#endif

