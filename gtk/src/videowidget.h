#ifndef __GPX2VIDEO__GTK__VIDEOWIDGET_H__
#define __GPX2VIDEO__GTK__VIDEOWIDGET_H__

#include <gtkmm/box.h>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "../../src/decoder.h"
#include "../../src/videowidget.h"
#include "shader.h"


class GPX2VideoWidget {
private:
	class Buffer;

	using BufferPtr = std::shared_ptr<GPX2VideoWidget::Buffer>;

	class Buffer {
	public:
		Buffer();
		virtual ~Buffer();

		static BufferPtr create(void);

		const uint64_t& timestamp(void) const;
		void setTimestamp(const uint64_t &timestamp);

		int index(void);
		uint8_t * data(void);
		void setData(int index, uint8_t *data);

	private:
		uint64_t timestamp_;

		int index_;
		uint8_t *data_;
	};

public:
	static GPX2VideoWidget * create(VideoWidget *widget);
	virtual ~GPX2VideoWidget();

	ShapeBase * shape(void);
	VideoWidget * widget(void);

	std::mutex& mutex(void) {
		return mutex_;
	}

	const std::string& name(void) const {
		return widget_->name();
	}

	VideoWidget::Theme& theme(void) {
		return widget_->theme();
	}

	TelemetryData& data(void) {
		return data_;
	}

	void dispatchEvent(bool schedule=false) {
		signal_changed_.emit(schedule);
	}

	bool is_over(const double &x, const double &y);

	// signal accessor:
	using type_signal_changed = sigc::signal<void(bool)>;
	type_signal_changed signal_changed();

	void move(double dx, double dy);

	double glX(void) const;
	double glY(void) const;
	double glWidth(void) const;
	double glHeight(void) const;

	void setSize(int width, int height);
	void setLayoutSize(int width, int height);

	void set_timestamp(uint64_t timestamp);

	void clear(void);

	bool full(void);
	bool ready(void);

	void stats(void);

	void init_buffers(void);
	void resize_buffers(void);
	void write_buffers(const TelemetryData &data, bool &loop);
	void clear_buffers(void);
	uint64_t load_texture(void);
	void unload_texture(void);
	void render(GPX2VideoShader *shader);

	void check_gl_error(void);

protected:
	type_signal_changed signal_changed_;

	GPX2VideoWidget(VideoWidget *widget);

	bool draw(const TelemetryData &data);

	BufferPtr get_last_buffer(void);

private:
	bool is_update_;
	bool is_buffer_init_;

	uint64_t timestamp_;

	int layout_width_;
	int layout_height_;

	VideoWidget *widget_;

	OIIO::ImageBuf *overlay_;

	mutable std::mutex mutex_;
	mutable std::mutex queue_mutex_;

	std::deque<BufferPtr> queue_;

	// Stats
	int stats_texture_reused_;
	int stats_texture_dropped_;
	int stats_texture_updated_;

	bool clear_req_;

	int index_;
	size_t queue_size_;
	uint8_t **buffer_;

	TelemetryData data_;

	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLuint ebo_ = 0;
//	GLuint pbo_ = 0;
	GLuint pbo_[5];
	GLuint texture_;
};

#endif

