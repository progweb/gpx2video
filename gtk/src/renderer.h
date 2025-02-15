#ifndef __GPX2VIDEO__GTK__RENDERER_H__
#define __GPX2VIDEO__GTK__RENDERER_H__

#include <glibmm/ustring.h>

#include "../../src/renderer.h"
#include "../../src/telemetrysettings.h"
#include "videowidget.h"


class GPX2VideoRenderer : public Renderer {
public:
	virtual ~GPX2VideoRenderer();

	static GPX2VideoRenderer * create(GPXApplication &app, 
			RendererSettings &rendererSettings, TelemetrySettings &telemetrySettings, 
			MediaContainer *container=NULL);

	void setMediaContainer(MediaContainer *container);
	void setLayoutFile(const Glib::ustring &layout_file);

	bool run(void) {
		return true;
	};

protected:
	GPX2VideoRenderer(GPXApplication &app,
			RendererSettings &rendererSettings, TelemetrySettings &telemetrySettings);

	bool init(MediaContainer *container);

//private:
public:
	std::list<GPX2VideoWidget *> widgets_;

};

#endif

