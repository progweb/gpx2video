#ifndef __GPX2VIDEO__FRAME_H__
#define __GPX2VIDEO__FRAME_H__

#include <iostream>
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "videoparams.h"


class Frame;

using FramePtr = std::shared_ptr<Frame>;


class Frame {
public:
	Frame();
	virtual ~Frame();

	static FramePtr create(void);

	const VideoParams& videoParams(void) const;
	void setVideoParams(const VideoParams &video_params);

	static int generateLinesizeBytes(int width, VideoParams::Format format, int nb_channels);

	int linesizeBytes(void) {
		return linesize_;
	}

	const int64_t& timestamp(void) const;
	void setTimestamp(const int64_t &timestamp);

	uint8_t * data(void);
	const uint8_t * constData(void);

	void setData(uint8_t *data);

private:
	VideoParams video_params_;

	int linesize_;

	int64_t timestamp_;

	uint8_t *data_;
};

#endif

