#ifndef __GPX2VIDEO__VIDEORENDERER_H__
#define __GPX2VIDEO__VIDEORENDERER_H__

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
	Encoder *encoder_;

	char duration_[16];
	unsigned int duration_ms_;

	int64_t frame_time_ = 0;

	time_t started_at_;

	VideoRenderer(GPX2Video &app); //, Map *map);

	void init(void);
	void computeWidgetsPosition(void);
};

#endif

