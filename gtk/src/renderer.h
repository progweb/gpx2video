#ifndef __GPX2VIDEO__GTK__RENDERER_H__
#define __GPX2VIDEO__GTK__RENDERER_H__

#include <chrono>

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

	void compute_telemetry_rate(void);
	void update_telemetry_settings(void);

	const uint64_t& time(void) const;
	void set_timestamp(uint64_t timestamp);
	void reset_timestamp(void);

	const std::list<GPX2VideoWidget *>& widgets(void);

	void append(VideoWidget::Widget type);
	void remove(GPX2VideoWidget *widget);

	GPX2VideoWidget * get_at(const double &x, const double &y);

	void seek(double pos);
	void load(void);
	void reset(void);

	void draw(void);
	void clear(GPX2VideoWidget *widget=NULL);
	void compute(void);
	void refresh(GPX2VideoWidget *widget=NULL, bool schedule=false);

	Glib::Dispatcher& ready(void) {
		return dispatcher_;
	}

	// signal accessor
	using type_signal_widget_appened = sigc::signal<void(GPX2VideoWidget *)>;

	type_signal_widget_appened signal_widget_appened() {
		return signal_widget_appened_;
	}

	// signal accessor
	using type_signal_widget_position_changed = sigc::signal<void(GPX2VideoWidget *)>;

	type_signal_widget_position_changed signal_widget_position_changed() {
		return signal_widget_position_changed_;
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

	uint64_t get_system_clock() {
		using namespace std::chrono;
		return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
	}


private:
//public:
	bool is_ready_;

	uint64_t rate_;
	uint64_t timestamp_;
	uint64_t player_timestamp_;

	TelemetryData data_;
	TelemetrySource *source_;

	Glib::Dispatcher dispatcher_;
	type_signal_widget_appened signal_widget_appened_;
	type_signal_widget_position_changed signal_widget_position_changed_;

	std::list<GPX2VideoWidget *> widgets_;

	bool seek_req_;
	double seek_pos_;
};

#endif

