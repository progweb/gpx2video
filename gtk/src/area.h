#ifndef __GPX2VIDEO__AREA_H__
#define __GPX2VIDEO__AREA_H__

#include <list>

#include <giommconfig.h>
#include <glibmmconfig.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/builder.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/glarea.h>
#include <epoxy/gl.h>
#undef GLAPI

#include "../src/decoder.h"


#ifndef GIOMM_CHECK_VERSION
#define GIOMM_CHECK_VERSION(major, minor, micro) \
  (GIOMM_MAJOR_VERSION > (major) || \
  (GIOMM_MAJOR_VERSION == (major) && GIOMM_MINOR_VERSION > (minor)) || \
  (GIOMM_MAJOR_VERSION == (major) && GIOMM_MINOR_VERSION == (minor) && \
   GIOMM_MICRO_VERSION >= (micro)))
#endif


#ifndef GLIBMM_CHECK_VERSION
#define GLIBMM_CHECK_VERSION(major, minor, micro) \
  (GLIBMM_MAJOR_VERSION > (major) || \
  (GLIBMM_MAJOR_VERSION == (major) && GLIBMM_MINOR_VERSION > (minor)) || \
  (GLIBMM_MAJOR_VERSION == (major) && GLIBMM_MINOR_VERSION == (minor) && \
   GLIBMM_MICRO_VERSION >= (micro)))
#endif


class GPX2VideoStream {
public:
	GPX2VideoStream();
	~GPX2VideoStream();

	bool open(const Glib::ustring &media_file);

	bool play(void);
//	bool pause(void);
//	bool stop(void);

	int width(void) const;
	int height(void) const;

	FramePtr getFrame(void);

protected:
//	void on_data_ready(void);

	bool read(void);

	void run(void);

private:
	std::thread* thread_;
//	Glib::Dispatcher dispatcher_;

	mutable std::mutex mutex_;

	MediaContainer *container_;

	Decoder *decoder_video_;

	std::list<FramePtr> queue_;
	int frame_time_;
	FramePtr frame_;
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

	void open_stream(const Glib::ustring &video_file);
	void play(void);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	sigc::slot<bool()> refresh_slot_;

	Shader *shader_;

	void on_realize(void);
	void on_unrealize(void);
	bool on_render(const Glib::RefPtr<Gdk::GLContext> &context);
	void on_resize(gint width, gint height);
	bool on_timeout(void);

	void schedule_refresh(int delay);

	void init_buffers();
	void init_shaders(const std::string& vertex_path, const std::string& fragment_path);
	void load_texture(FramePtr frame);

private:
	sigc::connection timer_;

	GPX2VideoStream stream_;
};

#endif

