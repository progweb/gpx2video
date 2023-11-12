#ifndef __GPX2VIDEO__IMAGERENDERER_H__
#define __GPX2VIDEO__IMAGERENDERER_H__

#include "renderer.h"


class ImageRenderer : public Renderer {
public:
	virtual ~ImageRenderer();

	static ImageRenderer * create(GPX2Video &app); //, Map *map=NULL);

	bool start(void);
	bool run(void);
	bool stop(void);

protected:
	int64_t timecode_;

	char duration_[16];
	unsigned int duration_ms_;

	time_t started_at_;

	ImageRenderer(GPX2Video &app); //, Map *map);

	bool init(void);
};

#endif

