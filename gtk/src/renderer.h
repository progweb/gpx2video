#ifndef __GPX2VIDEO__GTK__RENDERER_H__
#define __GPX2VIDEO__GTK__RENDERER_H__

#include <glibmm/ustring.h>
#include <glibmm/dispatcher.h>

#include "../../src/renderer.h"
#include "../../src/telemetrysettings.h"
#include "videowidget.h"
#include "shader.h"


class GPX2VideoRenderer : public Renderer {
public:
	virtual ~GPX2VideoRenderer();

	static GPX2VideoRenderer * create(GPXApplication &app, 
			RendererSettings &rendererSettings, TelemetrySettings &telemetrySettings, 
			MediaContainer *container=NULL);

	void setMediaContainer(MediaContainer *container);
	void setLayoutFile(const Glib::ustring &layout_file);

	void set_telemetry(TelemetrySource *source);
	void set_layout_size(int width, int heigth);

	const uint64_t& time(void) const;
	void set_timestamp(uint64_t timestamp);

	const std::list<GPX2VideoWidget *>& widgets(void);

	void append(GPX2VideoWidget *widget);
	void remove(GPX2VideoWidget *widget);

	void seek(double pos);
	void load(void);
	void reset(void);

	void draw(void);
	void clear(GPX2VideoWidget *widget=NULL);
	void compute(void);
	void refresh(GPX2VideoWidget *widget=NULL);

	Glib::Dispatcher& ready(void) {
		return dispatcher_;
	}

	void init_buffers(void);
	void load_texture(void);
	void render(GPX2VideoShader *shader);

	void update(uint64_t timestamp);
	bool run(void);

protected:
	GPX2VideoRenderer(GPXApplication &app,
			RendererSettings &rendererSettings, TelemetrySettings &telemetrySettings);

	bool init(MediaContainer *container);
	void restart(void);

private:
//public:
	bool is_ready_;

	uint64_t timestamp_;

	TelemetryData data_;
	TelemetrySource *source_;

	Glib::Dispatcher dispatcher_;

	std::list<GPX2VideoWidget *> widgets_;

	bool seek_req_;
	double seek_pos_;

};

#endif

