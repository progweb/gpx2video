#ifndef __GPX2VIDEO__GTK__AREA_H__
#define __GPX2VIDEO__GTK__AREA_H__

//#include <list>
#include <deque>

#include <glibmm/dispatcher.h>
#include <gtkmm/builder.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/glarea.h>
#include <gtkmm/adjustment.h>
#include <epoxy/gl.h>
#undef GLAPI

#include <glm/glm.hpp>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "../../src/stream.h"
#include "../../src/decoder.h"
#include "../../src/application.h"
#include "../../src/videowidget.h"
#include "../../src/telemetrymedia.h"
#include "../../src/telemetrysettings.h"
#include "videowidget.h"
#include "renderer.h"
#include "shader.h"



class GPX2VideoStream {
public:
	GPX2VideoStream();
	~GPX2VideoStream();

	MediaContainer * media();

//	bool open(const Glib::ustring &media_file);
	bool open(MediaContainer *container);
	void close(void);

	void init(void);
	void load(FramePtr frame);
	void render(GPX2VideoShader *shader);

	bool play(void);
	void stop(void);
	void seek(double pos); //, int64_t rel);

	int width(void) const;
	int height(void) const;

	double duration(void) const;
	double timeBase(void) const;

	int nbChannels(void) const;

	VideoParams::Format format(void) const;

	Glib::Dispatcher& data_ready(void) {
		return dispatcher_;
	}

	void nextFrame(void);

	FramePtr getFrame(void);
	FramePtr getNextFrame(void);

	void flushFrame(void);

	double getFrameDuration(void);

protected:
	bool read(void);

	void run(void);
	void wait(void);

private:
	bool loop_;
	bool is_init_;

	std::thread* thread_;
	Glib::Dispatcher dispatcher_;

	mutable std::mutex mutex_;
	mutable std::condition_variable cond_;

	MediaContainer *container_;

	Decoder *decoder_video_;

	std::deque<FramePtr> queue_;
	int frame_time_;
	FramePtr frame_;

	bool seek_req_;
	double seek_pos_;

	int index_;
	size_t queue_size_;
	uint8_t **buffer_;

	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLuint ebo_ = 0;
	GLuint pbo_[5];
	GLuint texture_;
};


class GPX2VideoArea : public Gtk::GLArea {
public:
	GPX2VideoArea(GPXApplication& app);
	GPX2VideoArea(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder, GPXApplication &app);
	virtual ~GPX2VideoArea();

	bool is_playing(void) const {
		return is_playing_;
	}

	void set_renderer(GPX2VideoRenderer *renderer);
	void set_adjustment(Glib::RefPtr<Gtk::Adjustment> adjustment);
	void configure_adjustment(void);
	void update_adjustment(double value);
	void update_layout(void);

	void open_telemetry(const Glib::ustring &telemetry_file);

//	void open_stream(const Glib::ustring &video_file);
	void open_stream(MediaContainer *container);
	void close_stream(void);
	void stream_toggle_pause(void);

	void toggle_pause(void);
	void step_to_next_frame(void);
	void seek(double incr);
	void seeking(bool status);

	void video_render(void);

//	void widget_append(VideoWidget *widget);
	void widgets_draw(void);
	void widgets_render(void);
	void widgets_resize(int width, int height);
	void widgets_clear(void);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::RefPtr<Gtk::Adjustment> adjustment_;

	sigc::slot<bool()> refresh_slot_;

	GPX2VideoShader *shader_;

	void on_realize(void);
	void on_unrealize(void);
	bool on_render(const Glib::RefPtr<Gdk::GLContext> &context);
	void on_resize(gint width, gint height);
	void on_data_ready(void);
	bool on_timeout(void);

	void refresh(void);
	void schedule_refresh(unsigned int delay);
	bool video_refresh(double &remaining_time);

	void video_display(void);

	void init_shaders(const std::string& vertex_path, const std::string& fragment_path);

	void init_video_buffers();

	void init_widgets_buffers();

	void load_video_texture(FramePtr frame);
	void load_widgets_texture(FramePtr frame);
	void resize_viewport(gint width, gint height);

	void check_gl_error(void);

private:
	bool is_init_;
	bool is_step_;
	bool is_playing_;
	bool is_seeking_;
	bool force_refresh_;
	bool widgets_refresh_;

	bool is_gl_context_ready_;

	GPXApplication &app_;

	sigc::connection timer_;

	GPX2VideoStream stream_;

	FramePtr frame;

	double frame_timer_;




	unsigned int real_duration_ms_;

	uint64_t last_timecode_ms_ = 0;

	TelemetrySource *source_;
	TelemetryData data_;

//	GPX2VideoWidget *widget_;
	GPX2VideoRenderer *renderer_;
};

#endif

