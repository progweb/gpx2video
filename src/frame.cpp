#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "oiioutils.h"
#include "frame.h"


Frame::Frame() :
	data_(NULL) {
}


Frame::~Frame() {
	if (allocated_ && (data_ != NULL))
		free(data_);
}


FramePtr Frame::create(void) {
	FramePtr frame;

	frame = std::make_shared<Frame>();

	return frame;
}


const VideoParams& Frame::videoParams(void) const {
	return video_params_;
}


void Frame::setVideoParams(const VideoParams &video_params) {
	video_params_ = video_params;

	linesize_ = generateLinesizeBytes(video_params.width(), video_params_.format(), video_params.nbChannels());
}


int Frame::generateLinesizeBytes(int width, VideoParams::Format format, int nb_channels) {
	// Align to 32 bytes (not sure if this is necessary)
	return VideoParams::getBytesPerPixel(format, nb_channels) * ((width + 31) & ~31);
}


const int64_t& Frame::timestamp(void) const {
	return timestamp_;
}


void Frame::setTimestamp(const int64_t &timestamp) {
	timestamp_ = timestamp;
}


uint8_t * Frame::data(void) const {
	return data_;
}


const uint8_t * Frame::constData(void) const {
	return (const uint8_t *) data_;
}


void Frame::setData(uint8_t *data, bool allocated) {
	data_ = data;
	allocated_ = allocated;
}


OIIO::ImageBuf Frame::toImageBuf(void) const {
	// OIIO Convert frame to imagebuf
	OIIO::ImageBuf buffer(OIIO::ImageSpec(this->width(), this->height(), 
		this->nbChannels(), OIIOUtils::getOIIOBaseTypeFromFormat(this->format())));
	OIIOUtils::frameToBuffer(this, &buffer);

	return buffer;
}


void Frame::fromImageBuf(OIIO::ImageBuf &buffer) {
	OIIOUtils::bufferToFrame(&buffer, this);
}

