#ifndef __GPX2VIDEO__VIDEORENDERER_H__
#define __GPX2VIDEO__VIDEORENDERER_H__

#include "gpmf.h"
#include "renderer.h"


class VideoRenderer : public Renderer {
public:
	virtual ~VideoRenderer();

	static VideoRenderer * create(GPX2Video &app); //, Map *map=NULL);

	bool start(void);
	bool run(void);
	bool stop(void);

protected:
	Decoder *decoder_audio_;
	Decoder *decoder_video_;
	GPMFDecoder *decoder_gpmf_;
	Encoder *encoder_;

	char duration_[16];
	unsigned int duration_ms_;
	unsigned int real_duration_ms_;

	int64_t frame_time_ = 0;

	time_t started_at_;

	GPMFData gpmf_data_;

	VideoRenderer(GPX2Video &app); //, Map *map);

	bool init(void);
	void computeWidgetsPosition(void);
};

#endif

