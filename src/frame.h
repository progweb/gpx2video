#ifndef __GPX2VIDEO__FRAME_H__
#define __GPX2VIDEO__FRAME_H__

#include <iostream>
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "videoparams.h"


class Frame;

using FramePtr = std::shared_ptr<Frame>;


class Frame {
public:
	Frame();
	virtual ~Frame();

	static FramePtr create(void);

	OIIO::ImageBuf toImageBuf(void) const;
	void fromImageBuf(OIIO::ImageBuf &buffer);

	const VideoParams& videoParams(void) const;
	void setVideoParams(const VideoParams &video_params);

	int width(void) const {
		return videoParams().width();
	}

	int height(void) const {
		return video_params_.height();
	}

	VideoParams::Format format(void) const {
		return video_params_.format();
	}

	int nbChannels(void) const {
		return video_params_.nbChannels();
	}

	static int generateLinesizeBytes(int width, VideoParams::Format format, int nb_channels);

	int linesizeBytes(void) const {
		return linesize_;
	}

	uint64_t time(void) const;

	const int64_t& timestamp(void) const;
	void setTimestamp(const int64_t &timestamp);

	uint8_t * data(void) const;
	const uint8_t * constData(void) const;

	void setData(uint8_t *data, bool allocated=true);

	int index_;

private:
	VideoParams video_params_;

	int linesize_;

	int64_t timestamp_;

	bool allocated_;
	uint8_t *data_;
};

#endif

