#ifndef __GPX2VIDEO__AREA_H__
#define __GPX2VIDEO__AREA_H__

//#include <list>
#include <deque>

#include <glibmm/dispatcher.h>
#include <gtkmm/builder.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/glarea.h>
#include <gtkmm/adjustment.h>
#include <epoxy/gl.h>
#undef GLAPI

#include "../src/decoder.h"


class GPX2VideoStream {
public:
	GPX2VideoStream();
	~GPX2VideoStream();

	bool open(const Glib::ustring &media_file);
	void close(void);

	bool play(void);
	void stop(void);
	void seek(double pos); //, int64_t rel);

	int width(void) const;
	int height(void) const;

	double duration(void) const;

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
};


class GPX2VideoArea : public Gtk::GLArea {
protected: 
	class Shader {
	public:
		virtual ~Shader();

		static Shader * create(const std::string &vertex_path, const std::string &fragment_path);

		void use(void);

		GLuint id(void) { return id_; }

		void set(const std::string &name, bool value) const;
		void set(const std::string &name, int value) const;
		void set(const std::string &name, float value) const;

	protected:
		GLuint id_;

		Shader(const GLchar *vertex_path, const GLchar *fragment_path);

		GLuint build(int type, const char *src);
	};

public:
	GPX2VideoArea();
	GPX2VideoArea(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder);
	virtual ~GPX2VideoArea();

	bool is_playing(void) const {
		return is_playing_;
	}

	void set_adjustment(Glib::RefPtr<Gtk::Adjustment> adjustment);
	void configure_adjustment(void);
	void update_adjustment(double value);

	void open_stream(const Glib::ustring &video_file);
	void close_stream(void);
	void stream_toggle_pause(void);

	void toggle_pause(void);
	void step_to_next_frame(void);
	void seek(double incr);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::RefPtr<Gtk::Adjustment> adjustment_;

	sigc::slot<bool()> refresh_slot_;

	Shader *shader_;

	void on_realize(void);
	void on_unrealize(void);
	bool on_render(const Glib::RefPtr<Gdk::GLContext> &context);
	void on_resize(gint width, gint height);
	void on_data_ready(void);
	bool on_timeout(void);

	void schedule_refresh(unsigned int delay);
	bool video_refresh(double &remaining_time);

	void video_display(void);

	void init_buffers();
	void init_shaders(const std::string& vertex_path, const std::string& fragment_path);
	void load_texture(FramePtr frame);
	void resize_viewport(gint width, gint height);

private:
	bool is_init_;
	bool is_step_;
	bool is_playing_;
	bool force_refresh_;

	sigc::connection timer_;

	GPX2VideoStream stream_;

	FramePtr frame;

	double frame_timer_;
};

#endif

