#ifndef __GPX2VIDEO__STREAM_H__
#define __GPX2VIDEO__STREAM_H__

#include <iostream>
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "audioparams.h"
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

	const std::string& name(void) const;
	void setName(const std::string &name);

	const AVMediaType& type(void) const;
	void setType(const AVMediaType &type);

	const AVRational& timeBase(void) const;
	void setTimeBase(const AVRational &time_base);

	const int64_t& duration(void) const;
	void setDuration(const int64_t &duration);

	int64_t getTimeInTimeBaseUnits(const AVRational& time) const;

private:
	int index_;
	std::string name_;

	MediaContainer *container_;

	AVMediaType type_;
	AVRational time_base_;
	int64_t duration_;
};


class AudioStream : public Stream {
public:
	AudioStream();
	virtual ~AudioStream();

	const int& sampleRate(void) const;
	void setSampleRate(const int &sample_rate);

	const AudioParams::Format& format(void) const;
	void setFormat(const AudioParams::Format &format);

	const int& nbChannels(void) const;
	void setNbChannels(const int &nb_channels);

	const uint64_t& channelLayout(void) const;
	void setChannelLayout(const uint64_t &channel_layout);

//	int64_t getTimeInTimeBaseUnits(const AVRational& time) const;

private:
	int sample_rate_;
	int nb_channels_;
	uint64_t channel_layout_;

	AudioParams::Format format_;
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
	void fixPixelAspectRatio(void);
	void setPixelAspectRatio(const AVRational &ratio);

	const int& nbChannels(void) const;
	void setNbChannels(const int &nb_channels);

	const int& orientation(void) const;
	void setOrientation(const double &theta);

//	int64_t getTimeInTimeBaseUnits(const AVRational& time) const;

private:
	int width_;
	int height_;

	VideoParams::Format format_;
	VideoParams::Interlacing interlacing_;

	AVRational frame_rate_;
	AVPixelFormat pixel_format_;
	AVRational pixel_aspect_ratio_;
	int nb_channels_;
	int orientation_;
};

#endif

