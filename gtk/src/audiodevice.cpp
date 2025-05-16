#include <pulse/error.h>

#include "log.h"
#include "audiodevice.h"


GPX2VideoAudioDevice::GPX2VideoAudioDevice()
	: dispatcher_()
	, api_(NULL)
	, loop_(NULL)
	, stream_(NULL)
	, context_(NULL) {
	log_call();

	is_connected = false;

	tlength_ = 0;
	data_length_ = 0;

	open();
}


GPX2VideoAudioDevice::~GPX2VideoAudioDevice() {
	log_call();
}


GPX2VideoAudioDevice * GPX2VideoAudioDevice::create(void) {
	log_call();

	auto audio_device = new GPX2VideoAudioDevice();

	return audio_device;
}


void GPX2VideoAudioDevice::onContextStateChanged(pa_context_state_t state) {
	log_call();

	switch (state) {
	case PA_CONTEXT_READY:
		log_info("PulseAudio connection ready");
		if (is_connected && (stream_ == NULL))
			connect_i();
		break;

	case PA_CONTEXT_FAILED:
		log_warn("PulseAudio connection terminated: %s", pa_strerror(pa_context_errno(context_)));
		break;

	case PA_CONTEXT_CONNECTING:
	case PA_CONTEXT_AUTHORIZING:
	case PA_CONTEXT_SETTING_NAME:
	default:
		log_info("PulseAudio connection state: %i", state);
		break;
	}
}


static void gpx2video_pa_context_state_changed(pa_context *context, void *userdata) {
	log_call();

	GPX2VideoAudioDevice *audiodevice = (GPX2VideoAudioDevice *) userdata;

	audiodevice->onContextStateChanged(pa_context_get_state(context));
}


void GPX2VideoAudioDevice::open(void) {
	log_call();

	int ok;

	// Create PulseAudio event loop
	loop_ = pa_glib_mainloop_new(NULL);

	api_ = pa_glib_mainloop_get_api(loop_);

	context_ = pa_context_new(api_, "gpx2video-gtk");

	pa_context_set_state_callback(context_, gpx2video_pa_context_state_changed, this);

	ok = pa_context_connect(context_, NULL, PA_CONTEXT_NOFLAGS, NULL);

	if (ok != 0) {
		log_error("pa_context_connect() failed: %s",
				pa_strerror(pa_context_errno(context_)));
	}
}


void GPX2VideoAudioDevice::close(void) {
	log_call();
}


void GPX2VideoAudioDevice::onPlaybackStateChanged(pa_stream_state_t state) {
	log_call();

	const pa_buffer_attr *attr;

	switch (state) {
	case PA_STREAM_CREATING:
		log_info("Audio playback stream connecting...");
		break;
		break;
	case PA_STREAM_READY:
		log_info("Audio playback stream is ready.");
		attr = pa_stream_get_buffer_attr(stream_);
		if (attr) {
			log_info(" - buffer max length: %u", attr->maxlength);
			log_info(" - target length: %u", attr->tlength);
			log_info(" - pre-buffering: %u", attr->prebuf);
			log_info(" - minimum request: %u", attr->minreq);

			tlength_ = attr->tlength;
		}
		break;
	case PA_STREAM_TERMINATED:
	case PA_STREAM_FAILED:
		log_info("Audio playback stream terminated: %s.",
				pa_strerror(pa_context_errno(context_)));
//		pa_stream_unref(stream_);
		stream_ = NULL;
		break;
	case PA_STREAM_UNCONNECTED:
	default:
		log_info("Audio playback stream state %i.", state);
		break;
	}
}


static void gpx2video_playback_state_callback(pa_stream *stream, void *userdata) {
	log_call();

	GPX2VideoAudioDevice *audiodevice = (GPX2VideoAudioDevice *) userdata;

	audiodevice->onPlaybackStateChanged(pa_stream_get_state(stream));
}


void GPX2VideoAudioDevice::onSuspendChanged(bool suspended) {
	log_call();

	log_info("Playback stream suspended: %s", 
			suspended ? "true" : "false");
}


static void gpx2video_suspended_callback(pa_stream *stream, void *userdata) {
	log_call();

	GPX2VideoAudioDevice *audiodevice = (GPX2VideoAudioDevice *) userdata;

	audiodevice->onSuspendChanged(pa_stream_is_suspended(stream));
}


void GPX2VideoAudioDevice::onWriteData(size_t length) {
	log_call();

	data_length_ = length;

	dispatcher_.emit();
}


static void gpx2video_write_data_callback(pa_stream *stream, size_t length, void *userdata) {
	log_call();

	GPX2VideoAudioDevice *audiodevice = (GPX2VideoAudioDevice *) userdata;

	(void) stream;

	audiodevice->onWriteData(length);
}


static void gpx2video_underflow_callback(pa_stream *stream, void *userdata) {
	log_call();

	(void) stream;
	(void) userdata;

	log_warn("AUDIO UNDERFLOW!");
}


static void gpx2video_overflow_callback(pa_stream *stream, void *userdata) {
	log_call();

	(void) stream;
	(void) userdata;

	log_warn("AUDIO OVERFLOW!\n");
}


bool GPX2VideoAudioDevice::connect_i(void) {
	log_call();

	int ok;

	pa_sample_spec spec = {
		.format = PA_SAMPLE_S16LE,
		.rate = 44100,
		.channels = 1 // MONO
//		.channels = 2 // STEREO
	};

	log_info("Connect playback stream [frame size: %ld]", pa_frame_size(&spec));

	// Create the playback stream
	if ((stream_ = pa_stream_new(context_, "Playback Stream", &spec, NULL)) == NULL) {
		log_error("pa_simple_new() failed: %s",
				pa_strerror(pa_context_errno(context_)));
		return false;
	}

	// Set stream state callback
	pa_stream_set_state_callback(stream_, gpx2video_playback_state_callback, this);

	// Set stream suspended callback
	pa_stream_set_suspended_callback(stream_, gpx2video_suspended_callback, this);

	// Set stream write callback
	pa_stream_set_write_callback(stream_, gpx2video_write_data_callback, this);

	// Set stream underflowcallback
	pa_stream_set_underflow_callback(stream_, gpx2video_underflow_callback, this);

	// Set stream overflowcallback
	pa_stream_set_overflow_callback(stream_, gpx2video_overflow_callback, this);

	ok = pa_stream_connect_playback(stream_,
			NULL,
			NULL,
			PA_STREAM_START_CORKED, // PA_STREAM_NOFLAGS,
			NULL,
			NULL);

	if (ok != 0) {
		log_error("pa_stream_connect_playback() failed: %s",
				pa_strerror(pa_context_errno(context_)));
		return false;
	}

	return true;
}


void GPX2VideoAudioDevice::connect(void) {
	log_call();

	is_connected = true;

	if (pa_context_get_state(context_) == PA_CONTEXT_READY)
		connect_i();
}


void GPX2VideoAudioDevice::disconnect(void) {
	log_call();

	is_connected = false;

	if (stream_ != NULL) {
		pa_stream_disconnect(stream_);

		pa_stream_drop(stream_);
	}

	stream_ = NULL;
}


//void op_cb(pa_operation *op, void *userdata) {
//	switch (pa_operation_get_state(op)) {
//	case PA_OPERATION_RUNNING:
//		printf("OP RUNNING\n");
//		break;
//	case PA_OPERATION_DONE:
//		printf("OP DONE\n");
//		break;
//	case PA_OPERATION_CANCELLED:
//		printf("OP CANCELLED\n");
//		break;
//	default:
//		break;
//	}
//}
//
//
//void cb(pa_stream *s, int ok, void *userdata) {
//	log_notice("OK = %d", ok);
//}


void GPX2VideoAudioDevice::suspend(bool enable) {
	log_call();

	pa_operation *op;

	int b = enable ? 1 : 0;

//	log_info("Audio suspended: %s", enable ? "true" : "false");

	op = pa_stream_cork(stream_, b, NULL, NULL);

	(void) op;

//	pa_operation_set_state_callback(op, op_cb, this);
}


void GPX2VideoAudioDevice::write(const void *data, size_t nbytes) {
	log_call();

	int ok;

	ok = pa_stream_write(stream_, data, nbytes, NULL, 0, PA_SEEK_RELATIVE);

	if (ok != 0) {
		log_error("pa_stream_write() failed: %s",
				pa_strerror(pa_context_errno(context_)));
	}
}


size_t GPX2VideoAudioDevice::dataLengthRequired(void) const {
	log_call();

	return pa_stream_is_corked(stream_) ? tlength_ : data_length_;
}


void GPX2VideoAudioDevice::flush(void) {
	log_call();

	pa_stream_trigger(stream_, NULL, 0);
}

