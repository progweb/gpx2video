#ifndef __GPX2VIDEO__RENDERER_H__
#define __GPX2VIDEO__RENDERER_H__

#include <string>
#include <vector>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "layoutlib/Map.h"
#include "layoutlib/Track.h"
#include "layoutlib/Widget.h"

#include "map.h"
#include "track.h"
#include "frame.h"
#include "media.h"
#include "decoder.h"
#include "encoder.h"
#include "exportcodec.h"
#include "videowidget.h"
#include "telemetrymedia.h"
#include "telemetrysettings.h"
#include "application.h"


class RendererSettings {
public:
	RendererSettings(
			std::string media_file="", 
			std::string layout_file="",
			bool time_factor_auto=false,
			double time_factor_value = 1.0,
			ExportCodec::Codec video_codec=ExportCodec::CodecH264,
			std::string video_hw_device="",
			std::string video_preset="medium",
			int32_t video_crf=-1,
			int64_t video_bit_rate=0,
			int64_t video_min_bit_rate=0,
			int64_t video_max_bit_rate=0)
		: media_file_(media_file)
		, layout_file_(layout_file)
		, time_factor_auto_(time_factor_auto)
		, time_factor_value_(time_factor_value)
		, video_codec_(video_codec)
		, video_hw_device_(video_hw_device)
		, video_preset_(video_preset)
		, video_crf_(video_crf)
		, video_bit_rate_(video_bit_rate)
		, video_min_bit_rate_(video_min_bit_rate)
		, video_max_bit_rate_(video_max_bit_rate) {
	}
	virtual ~RendererSettings() {
	}

	const std::string& mediafile(void) const {
		return media_file_;
	}

	const std::string& layoutfile(void) const {
		return layout_file_;
	}

	void setLayoutfile(const std::string &layout_file) {
		layout_file_ = layout_file;
	}

	const bool& isTimeFactorAuto(void) const {
		return time_factor_auto_;
	}

	const double& timeFactor(void) const {
		return time_factor_value_;
	}

	const ExportCodec::Codec& videoCodec(void) const {
		return video_codec_;
	}

	const std::string& videoHardwareDevice(void) const {
		return video_hw_device_;
	}

	const std::string& videoPreset(void) const {
		return video_preset_;
	}

	const int32_t& videoCRF(void) const {
		return video_crf_;
	}

	const int64_t& videoBitrate(void) const {
		return video_bit_rate_;
	}

	const int64_t& videoMinBitrate(void) const {
		return video_min_bit_rate_;
	}

	const int64_t& videoMaxBitrate(void) const {
		return video_max_bit_rate_;
	}

private:
	std::string media_file_;
	std::string layout_file_;

	bool time_factor_auto_;
	double time_factor_value_;

	ExportCodec::Codec video_codec_;
	std::string video_hw_device_;
	std::string video_preset_;
	int32_t video_crf_;
	int64_t video_bit_rate_;
	int64_t video_min_bit_rate_;
	int64_t video_max_bit_rate_;
};


class Renderer : public GPXApplication::Task {
public:
	virtual ~Renderer();

	const RendererSettings& rendererSettings(void) {
		return renderer_settings_;
	}

	const TelemetrySettings& telemetrySettings(void) {
		return telemetry_settings_;
	}

	void append(VideoWidget *widget);

protected:
	GPXApplication &app_;

	RendererSettings &renderer_settings_;
	TelemetrySettings &telemetry_settings_;

	TelemetrySource *source_;
	TelemetryData data_;

	MediaContainer *container_;

	std::list<VideoWidget *> widgets_;

	int layout_width_;
	int layout_height_;

	OIIO::ImageBuf *overlay_;

	Renderer(GPXApplication &app,
			RendererSettings &rendererSettings, TelemetrySettings &telemetrySettings);

	bool init(MediaContainer *container);
	bool start(void);
	bool load(void);
	bool loadMap(layout::Map *m);
	bool loadTrack(layout::Track *t);
	bool loadWidget(layout::Widget *w);
	void computeWidgetsPosition(void);

	void rotate(OIIO::ImageBuf *buf, int orientation);
	void resize(OIIO::ImageBuf *buf, int width, int height);
	void add(OIIO::ImageBuf *frame, int x, int y, const char *picto, const char *label, const char *value, double divider=1.9);
};


#endif

