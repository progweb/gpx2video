#include "stream.h"


Stream::Stream() {
}


Stream::~Stream() {
}


MediaContainer * Stream::container(void) const {
	return container_;
}


void Stream::setContainer(MediaContainer *container) {
	container_ = container;
}


const int& Stream::index(void) const {
	return index_;
}


void Stream::setIndex(const int &index) {
	index_ = index;
}


const AVMediaType& Stream::type(void) const {
	return type_;
}


void Stream::setType(const AVMediaType &type) {
	type_ = type;
}


const AVRational& Stream::timeBase(void) const {
	return time_base_;
}


void Stream::setTimeBase(const AVRational &time_base) {
	time_base_ = time_base;
}


const int64_t& Stream::duration(void) const {
	return duration_;
}


void Stream::setDuration(const int64_t &duration) {
	duration_ = duration;
}


AudioStream::AudioStream() {
	setType(AVMEDIA_TYPE_AUDIO);
}


AudioStream::~AudioStream() {
}


const int& AudioStream::sampleRate(void) const {
	return sample_rate_;
}


void AudioStream::setSampleRate(const int &sample_rate) {
	sample_rate_ = sample_rate;
}


const AudioParams::Format& AudioStream::format(void) const {
	return format_;
}


void AudioStream::setFormat(const AudioParams::Format &format) {
	format_ = format;
}


const int& AudioStream::nbChannels(void) const {
	return nb_channels_;
}


void AudioStream::setNbChannels(const int &nb_channels) {
	nb_channels_ = nb_channels;
}


const uint64_t& AudioStream::channelLayout(void) const {
	return channel_layout_;
}


void AudioStream::setChannelLayout(const uint64_t &channel_layout) {
	channel_layout_ = channel_layout;
}


int64_t AudioStream::getTimeInTimeBaseUnits(const AVRational& time) const {
	return (int64_t) round(av_q2d(time) * av_q2d(av_inv_q(timeBase())));
}


VideoStream::VideoStream() {
	setType(AVMEDIA_TYPE_VIDEO);
}


VideoStream::~VideoStream() {
}


const int& VideoStream::width(void) const {
	return width_;
}


void VideoStream::setWidth(const int &width) {
	width_ = width;
}


const int& VideoStream::height(void) const {
	return height_;
}


void VideoStream::setHeight(const int &height) {
	height_ = height;
}


const VideoParams::Format& VideoStream::format(void) const {
	return format_;
}


void VideoStream::setFormat(const VideoParams::Format &format) {
	format_ = format;
}


const VideoParams::Interlacing& VideoStream::interlacing(void) const {
	return interlacing_;
}


void VideoStream::setInterlacing(const VideoParams::Interlacing &interlacing) {
	interlacing_ = interlacing;
}


const AVRational& VideoStream::frameRate(void) const {
	return frame_rate_;
}


void VideoStream::setFrameRate(const AVRational &rate) {
	frame_rate_ = rate;
}


const AVPixelFormat& VideoStream::pixelFormat(void) const {
	return pixel_format_;
}


void VideoStream::setPixelFormat(const AVPixelFormat &format) {
	pixel_format_ = format;
}


const AVRational& VideoStream::pixelAspectRatio(void) const {
	return pixel_aspect_ratio_;
}


void VideoStream::setPixelAspectRatio(const AVRational &ratio) {
	pixel_aspect_ratio_ = ratio;
}


const int& VideoStream::nbChannels(void) const {
	return nb_channels_;
}


void VideoStream::setNbChannels(const int &nb_channels) {
	nb_channels_ = nb_channels;
}


int64_t VideoStream::getTimeInTimeBaseUnits(const AVRational& time) const {
	return (int64_t) round(av_q2d(time) * av_q2d(av_inv_q(timeBase())));
}

