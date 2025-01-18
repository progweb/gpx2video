#ifndef __GPX2VIDEO__GTK__VIDEOWIDGET_H__
#define __GPX2VIDEO__GTK__VIDEOWIDGET_H__

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "../../src/decoder.h"
#include "../../src/videowidget.h"
#include "shader.h"


class GPX2VideoWidget {
public:
	static GPX2VideoWidget * create(VideoWidget *widget);
	virtual ~GPX2VideoWidget();

//	int x(void) const;
//	int y(void) const;

	double glX(void) const;
	double glY(void) const;
	double glWidth(void) const;
	double glHeight(void) const;

	void setLayoutSize(int width, int height);

	bool draw(void);
	void init_buffers(void);
	void load_texture(void);
	void render(GPX2VideoShader *shader);

protected:
	GPX2VideoWidget(VideoWidget *widget);

private:
	bool is_update_;

	int layout_width_;
	int layout_height_;

	VideoWidget *widget_;

	OIIO::ImageBuf *overlay_;
};

#endif

