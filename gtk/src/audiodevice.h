#ifndef __GPX2VIDEO__GTK__AUDIODEVICE_H__
#define __GPX2VIDEO__GTK__AUDIODEVICE_H__

#include <glibmm/dispatcher.h>

#include <pulse/stream.h>
#include <pulse/context.h>
#include <pulse/glib-mainloop.h>


class GPX2VideoAudioDevice {
public:
	~GPX2VideoAudioDevice();

	static GPX2VideoAudioDevice * create(void);

	void connect(void);
	void disconnect(void);

	void suspend(bool enable);

	void write(const void *data, size_t nbytes);
	void flush(void);

	size_t dataLengthRequired(void) const;

	Glib::Dispatcher& data_requested(void) {
		return dispatcher_;
	}

	const uint32_t& bufferLength(void) const {
		return tlength_;
	}

	void onContextStateChanged(pa_context_state_t state);
	void onPlaybackStateChanged(pa_stream_state_t state);
	void onSuspendChanged(bool suspended);
	void onWriteData(size_t length);

private:
	GPX2VideoAudioDevice();

	void open(void);
	void close(void);

	bool connect_i(void);

//private:
public:
	bool is_connected = false;

	Glib::Dispatcher dispatcher_;

	pa_mainloop_api *api_;
	pa_glib_mainloop *loop_;

	pa_stream *stream_;

	pa_context *context_;

	uint32_t tlength_;

	size_t data_length_;
};

#endif

