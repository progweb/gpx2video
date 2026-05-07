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

#include <pulse/stream.h>

#include "../../src/stream.h"
#include "../../src/decoder.h"
#include "../../src/application.h"
#include "../../src/videowidget.h"
#include "../../src/telemetrymedia.h"
#include "../../src/telemetrysettings.h"
#include "audiodevice.h"
#include "videowidget.h"
#include "renderer.h"
#include "shader.h"
#include "stream.h"
#include "cursor.h"


class GPX2VideoArea : public Gtk::GLArea {
public:
	GPX2VideoArea(GPXApplication& app);
	GPX2VideoArea(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder, GPXApplication &app);
	virtual ~GPX2VideoArea();

	bool is_playing(void) const {
		return is_playing_;
	}

	void set_renderer(GPX2VideoRenderer *renderer);
	void set_audio_device(GPX2VideoAudioDevice *audio_device);
	void set_adjustment(Glib::RefPtr<Gtk::Adjustment> adjustment);

	void configure_adjustment(void);
	void update_adjustment(double value);
	void update_layout(void);

	void open_stream(MediaContainer *container);
	void close_stream(void);
	void stream_toggle_pause(void);

	void toggle_pause(void);
	void step_to_next_frame(void);
	void seek(double incr);
	void seeking(bool status);

	void refresh(void);

	void cursor_render(void);
	void video_render(void);
	void widgets_render(void);

	GPX2VideoWidget * get_widget_at(const double &x, const double &y);

//	void widget_append(VideoWidget *widget);

	// signal accessor
	using type_signal_widget_selected = sigc::signal<void(GPX2VideoWidget *)>;

	type_signal_widget_selected signal_widget_selected() {
		return signal_widget_selected_;
	}

	using type_signal_widget_position_changed = sigc::signal<void(GPX2VideoWidget *)>;

	type_signal_widget_position_changed signal_widget_position_changed() {
		return signal_widget_position_changed_;
	}

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::RefPtr<Gtk::Adjustment> adjustment_;

	sigc::slot<bool()> refresh_slot_;

	GPX2VideoShader *cursor_shader_;
	GPX2VideoShader *widget_shader_;

	void on_realize(void);
	void on_unrealize(void);
	bool on_render(const Glib::RefPtr<Gdk::GLContext> &context);
	void on_resize(gint width, gint height);
	void on_mouse_motion(double x, double y);
	void on_mouse_pressed(int n_press, double x, double y);
	void on_mouse_released(int n_press, double x, double y);
	void on_data_ready(void);
	void on_renderer_ready(void);
	bool on_timeout(void);

	void schedule_refresh(unsigned int delay);
	bool video_refresh(double &remaining_time);

	void video_display(void);

	void init_shaders(const std::string& vertex_path, const std::string& fragment_path);

	void load_cursor_shaders(void);
	void load_widgets_shaders(void);

	void init_video_buffers(void);
	void init_cursor_buffers(void);
	void init_widgets_buffers(void);

	void load_video_texture(FramePtr frame);
	void load_widgets_texture(FramePtr frame);
	void resize_viewport(gint width, gint height);

	void get_gl_position(const double &x, const double &y, double &glX, double &glY);

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

	GPX2VideoAudioDevice *audio_device_;

	sigc::connection timer_;

	GPX2VideoCursor *cursor_;

	GPX2VideoStream stream_;
	GPX2VideoRenderer *renderer_;

	GPX2VideoWidget *widget_selected_;

	FramePtr frame;

	double frame_timer_;

	double last_mouse_x_, last_mouse_y_;

	type_signal_widget_selected signal_widget_selected_;
	type_signal_widget_position_changed signal_widget_position_changed_;
};

#endif

