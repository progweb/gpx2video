#include "videoparams.h"


const int VideoParams::InternalChannelCount = VideoParams::RGBAChannelCount;


VideoParams::VideoParams() : 
	width_(0), 
	height_(0) {
}


VideoParams::VideoParams(int width, int height,
	Format format, int nb_channels, 
	int orientation,
	const AVRational &pixel_aspect_ratio,
	Interlacing interlacing) 
	: width_(width), height_(height)
	, orientation_(orientation)
	, format_(format), nb_channels_(nb_channels)
	, pixel_aspect_ratio_(pixel_aspect_ratio)
	, interlacing_(interlacing) {
}


VideoParams::VideoParams(int width, int height, const AVRational &time_base,
	Format format, int nb_channels, 
	int orientation,
	const AVRational &pixel_aspect_ratio,
	Interlacing interlacing) 
	: width_(width), height_(height), time_base_(time_base)
	, orientation_(orientation)
	, format_(format), nb_channels_(nb_channels)
	, pixel_aspect_ratio_(pixel_aspect_ratio)
	, interlacing_(interlacing) {
	frame_rate_ = av_div_q(av_make_q(1, 1), time_base_);
}


VideoParams::~VideoParams() {
}


const int& VideoParams::width(void) const {
	return width_;
}


const int& VideoParams::height(void) const {
	return height_;
}


const AVRational& VideoParams::timeBase(void) const {
	return time_base_;
}


void VideoParams::setTimeBase(const AVRational &time_base) {
	time_base_ = time_base;
}


const AVRational& VideoParams::frameRate(void) const {
	return frame_rate_;
}


void VideoParams::setFrameRate(const AVRational &frame_rate) {
	frame_rate_ = frame_rate;
}

const VideoParams::Format& VideoParams::format(void) const {
	return format_;
}


const int& VideoParams::nbChannels(void) const {
	return nb_channels_;
}


const int& VideoParams::orientation(void) const {
	return orientation_;
}


const AVPixelFormat& VideoParams::pixelFormat(void) const {
	return pixel_format_;
}


void VideoParams::setPixelFormat(const AVPixelFormat &format) {
	pixel_format_ = format;
}


const AVRational& VideoParams::pixelAspectRatio(void) const {
	return pixel_aspect_ratio_;
}



const VideoParams::Interlacing& VideoParams::interlacing(void) const {
	return interlacing_;
}


int VideoParams::getBytesPerChannel(VideoParams::Format format) {
	switch (format) {
	case FormatInvalid:
	case FormatCount:
		break;
	case FormatUnsigned8:
		return 1;
	case FormatUnsigned16:
	case FormatFloat16:
		return 2;
	case FormatFloat32:
		return 4;
	}

	return  0;
}


int VideoParams::getBytesPerPixel(VideoParams::Format format, int nb_channels) {
	return getBytesPerChannel(format) * nb_channels;
}

