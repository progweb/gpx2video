#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "layoutlib/Parser.h"
#include "layoutlib/ReportCerr.h"

#include "oiioutils.h"
#include "decoder.h"
#include "audioparams.h"
#include "videoparams.h"
#include "encoder.h"
#include "widgets/gpx.h"
#include "widgets/date.h"
#include "widgets/distance.h"
#include "widgets/duration.h"
#include "widgets/grade.h"
#include "widgets/elevation.h"
#include "widgets/cadence.h"
#include "widgets/heartrate.h"
#include "widgets/position.h"
#include "widgets/image.h"
#include "widgets/speed.h"
#include "widgets/maxspeed.h"
#include "widgets/avgspeed.h"
#include "widgets/time.h"
#include "widgets/temperature.h"
#include "renderer.h"


Renderer::Renderer(GPX2Video &app)
	: Task(app) 
	, app_(app) {
	container_ = NULL;
	decoder_audio_ = NULL;
	decoder_video_ = NULL;
	encoder_ = NULL;

	overlay_ = NULL;

	frame_time_ = 0;
	duration_ms_ = 0;
}


Renderer::~Renderer() {
	if (encoder_)
		delete encoder_;
	if (decoder_audio_)
		delete decoder_audio_;
	if (decoder_video_)
		delete decoder_video_;
}


Renderer * Renderer::create(GPX2Video &app) {
	Renderer *renderer = new Renderer(app);

	renderer->init();
	renderer->load();
	renderer->computeWidgetsPosition();

	return renderer;
}


void Renderer::init(void) {
	time_t start_time;

	log_call();

	gpx_ = GPX::open(app_.settings().gpxfile(), app_.settings().telemetryFilter());

	// Media
	container_ = app_.media();

	// Set start time in GPX stream
	start_time = container_->startTime() + container_->timeOffset();
	if (gpx_) {
		// GPX limits
		gpx_->setFrom(app_.settings().gpxFrom());
		gpx_->setTo(app_.settings().gpxTo());

		// GPX time fixing
		gpx_->setStartTime(start_time);
		gpx_->setTimeOffset(app_.settings().offset());
		gpx_->retrieveFirst(data_);
	}

	// Retrieve audio & video streams
	VideoStreamPtr video_stream = container_->getVideoStream();
	AudioStreamPtr audio_stream = container_->getAudioStream();

	// Audio & Video encoder settings
	VideoParams video_params(video_stream->width(), video_stream->height(),
		// av_make_q(1,  50), 
		av_inv_q(video_stream->frameRate()),
		video_stream->format(),
		video_stream->nbChannels(),
		video_stream->pixelAspectRatio(),
		video_stream->interlacing());
	video_params.setPixelFormat(video_stream->pixelFormat());

	EncoderSettings settings;
	settings.setFilename(app_.settings().outputfile());
	settings.setVideoParams(video_params, AV_CODEC_ID_H264);
	settings.setVideoBitrate(4 * 1000 * 1000 * 8);
	settings.setVideoMaxBitrate(2 * 1000 * 1000 * 16);
	settings.setVideoBufferSize(4 * 1000 * 1000 / 2);

	if (audio_stream) {
		AudioParams audio_params(audio_stream->sampleRate(),
			audio_stream->channelLayout(),
			audio_stream->format());

		settings.setAudioParams(audio_params, AV_CODEC_ID_AAC);
		settings.setAudioBitrate(44 * 1000);
	}

	// Compute duration
	duration_ms_ = video_stream->duration() * av_q2d(video_stream->timeBase()) * 1000;
	duration_ms_ = MAX(duration_ms_, app_.settings().maxDuration());

	snprintf(duration_, sizeof(duration_), "%02d:%02d:%02d.%03d", 
		(unsigned int) (duration_ms_ / 3600000), (unsigned int) ((duration_ms_ / 60000) % 60), (unsigned int) ((duration_ms_ / 1000) % 60), (unsigned int) (duration_ms_ % 1000));
	duration_[sizeof(duration_) - 1] = '\0';

	// Open & decode input media
	decoder_video_ = Decoder::create();
	decoder_video_->open(video_stream);

	if (audio_stream) {
		decoder_audio_ = Decoder::create();
		decoder_audio_->open(audio_stream);
	}

	// Open & encode output video
	encoder_ = Encoder::create(settings);
	encoder_->open();
}


bool Renderer::load(void) {
	std::ifstream stream;

	layout::Layout *root;
//	layout::ReportCerr report;
	layout::Parser parser(NULL); //&report);

	std::list<layout::Map *> maps;
	std::list<layout::Track *> tracks;
	std::list<layout::Widget *> widgets;

	std::string filename = app_.settings().layoutfile();

	if (filename.empty()) {
		log_warn("None layout file");
		goto done;
	}

    stream = std::ifstream(filename);

	if (!stream.is_open()) {
		log_error("Open '%s' layout file failure, please check that file is readable", filename.c_str());
		goto failure;
	}

	root = parser.parse(stream);

	if (root == NULL) {
		log_error("Parsing of '%s' failed due to %s on line %d and column %d", 
			filename.c_str(), parser.errorText().c_str(),
			parser.errorLineNumber(), parser.errorColumnNumber());
		goto failure;
	}

	std::cout << "Parsing '" << filename << "' layout file" << std::endl;

	// Widgets
	widgets = root->widgets().list();

	for (std::list<layout::Widget *>::iterator node = widgets.begin(); node != widgets.end(); ++node) {
		layout::Widget *widget = (*node);
         
		if (widget == nullptr)
 			continue;
	
		// Display
		if ((bool) widget->display() == false) {
			log_info("Skip widget '%s'", (const char *) widget->type());
			continue;
		}

		loadWidget(widget);
	}

	// Tracks
	tracks = root->tracks().list();

	for (std::list<layout::Track *>::iterator node = tracks.begin(); node != tracks.end(); ++node) {
		layout::Track *track = (*node);
         
		if (track == nullptr)
 			continue;

		// Display
		if ((bool) track->display() == false) {
			log_info("Skip track widget");
			continue;
		}

		loadTrack(track);
	}

	// Maps
	maps = root->maps().list();

	for (std::list<layout::Map *>::iterator node = maps.begin(); node != maps.end(); ++node) {
		layout::Map *map = (*node);
         
		if (map == nullptr)
 			continue;

		// Display
		if ((bool) map->display() == false) {
			log_info("Skip map widget");
			continue;
		}

		loadMap(map);
	}

done:
	return true;

failure:
	return false;
}


bool Renderer::loadMap(layout::Map *m) {
	int x, y;
	int width, height;

	int marker_size;

	std::string s;

	unsigned int mapsource;

	VideoWidget::Align align = VideoWidget::AlignNone;

	log_call();

	// Map source
	mapsource = m->source();

	if ((MapSettings::Source) mapsource == MapSettings::SourceNull) {
		log_warn("Map source undefined, skip map widget");
		return false;
	}

	// Open GPX file
	GPX *gpx = GPX::open(app_.settings().gpxfile());

	if (gpx == NULL) {
		log_warn("Can't read GPS data, skip map widget");
		return false;
	}

	// GPX limits
	gpx->setFrom(app_.settings().gpxFrom());
	gpx->setTo(app_.settings().gpxTo());

	// Media
	MediaContainer *container = app_.media();

	VideoStreamPtr video_stream = container->getVideoStream();

	// Default size
	//   2704x1520 => 800x500
	//   1920x1080 =>   ?x?
	width = (m->width() > 0) ? m->width() : 800 * video_stream->width() / 2704;
	height = (m->height() > 0) ? m->height() : 500 * video_stream->height() / 1520;

	// Default position
	x = (m->x() > 0) ? m->x() : video_stream->width() - width - m->margin();
	y = (m->y() > 0) ? m->y() : video_stream->height() - height - m->margin();

	// Default marker size (132x200)
	// 2704x1520 => 40x60
	//  432x240  =>  ?x?
	marker_size = (m->marker() > 0) ? m->marker() : 60 * video_stream->height() / 1520.0;

	// Create map bounding box
	GPXData::point p1, p2;
	gpx->getBoundingBox(&p1, &p2);

	// Alignment
	s = (const char *) m->align();

	align = VideoWidget::string2align(s);

	if (align == VideoWidget::AlignUnknown)
		log_error("Map align value '%s' unknown", s.c_str());

	// Map settings
	MapSettings mapSettings;
	mapSettings.setSize(width, height);
	mapSettings.setSource((MapSettings::Source) mapsource);
	mapSettings.setZoom(m->zoom());
	mapSettings.setDivider(m->factor());
	mapSettings.setMarkerSize(marker_size);
	mapSettings.setBoundingBox(p1.lat, p1.lon, p2.lat, p2.lon);

	Map *map = Map::create(app_, mapSettings);

	log_info("Load map widget");

	// Widget settings
	map->setAlign(align);
	map->setPosition(x, y);
	map->setSize(mapSettings.width(), mapSettings.height());
	map->setMargin(VideoWidget::MarginAll, m->margin());
	map->setMargin(VideoWidget::MarginLeft, m->marginLeft());
	map->setMargin(VideoWidget::MarginRight, m->marginRight());
	map->setMargin(VideoWidget::MarginTop, m->marginTop());
	map->setMargin(VideoWidget::MarginBottom, m->marginBottom());
	map->setBorder(m->border());
	map->setBorderColor((const char *) m->borderColor());

	// Append
	app_.append(map);

	this->append(map);

	return true;
}


bool Renderer::loadTrack(layout::Track *t) {
	int x, y;
	int width, height;

	std::string s;

	VideoWidget::Align align = VideoWidget::AlignNone;

	log_call();

	// Open GPX file
	GPX *gpx = GPX::open(app_.settings().gpxfile());

	if (gpx == NULL) {
		log_warn("Can't read GPS data, skip map widget");
		return false;
	}

	// GPX limits
	gpx->setFrom(app_.settings().gpxFrom());
	gpx->setTo(app_.settings().gpxTo());

	// Media
	MediaContainer *container = app_.media();

	VideoStreamPtr video_stream = container->getVideoStream();

	// Default size
	//   2704x1520 => 800x500
	//   1920x1080 => 560x350
	width = (t->width() > 0) ? t->width() : 800 * video_stream->width() / 2704;
	height = (t->height() > 0) ? t->height() : 500 * video_stream->height() / 1520;

	// Default position
	x = (t->x() > 0) ? t->x() : video_stream->width() - width - t->margin();
	y = (t->y() > 0) ? t->y() : video_stream->height() - height - t->margin();

	// Create map bounding box
	GPXData::point p1, p2;
	gpx->getBoundingBox(&p1, &p2);

	// Alignment
	s = (const char *) t->align();

	align = VideoWidget::string2align(s);

	if (align == VideoWidget::AlignUnknown)
		log_error("Track align value '%s' unknown", s.c_str());

	// Track settings
	TrackSettings trackSettings;
	trackSettings.setSize(width, height);
	trackSettings.setBoundingBox(p1.lat, p1.lon, p2.lat, p2.lon);

	Track *track = Track::create(app_, trackSettings);

	// Widget settings
	track->setAlign(align);
	track->setPosition(x, y);
	track->setSize(trackSettings.width(), trackSettings.height());
	track->setMargin(VideoWidget::MarginAll, t->margin());
	track->setMargin(VideoWidget::MarginLeft, t->marginLeft());
	track->setMargin(VideoWidget::MarginRight, t->marginRight());
	track->setMargin(VideoWidget::MarginTop, t->marginTop());
	track->setMargin(VideoWidget::MarginBottom, t->marginBottom());
	track->setBorder(t->border());
	track->setBorderColor((const char *) t->borderColor());
	track->setBackgroundColor((const char *) t->backgroundColor());

	// Append
	app_.append(track);

	this->append(track);

	return true;
}


bool Renderer::loadWidget(layout::Widget *w) {
	std::string s;

	VideoWidget *widget = NULL;

	VideoWidget::Align align = VideoWidget::AlignNone;
	VideoWidget::Unit unit = VideoWidget::UnitNone;
	VideoWidget::Zoom zoom = VideoWidget::ZoomNone;

	// Type
	s = (const char *) w->type();

	// Create widget
	if (s == "gpx") 
		widget = GPXWidget::create(app_);
	else if (s == "date") 
		widget = DateWidget::create(app_);
	else if (s == "time") 
		widget = TimeWidget::create(app_);
	else if (s == "distance") 
		widget = DistanceWidget::create(app_);
	else if (s == "duration") 
		widget = DurationWidget::create(app_);
	else if (s == "position") 
		widget = PositionWidget::create(app_);
	else if (s == "speed") 
		widget = SpeedWidget::create(app_);
	else if (s == "maxspeed") 
		widget = MaxSpeedWidget::create(app_);
	else if (s == "avgspeed") 
		widget = AvgSpeedWidget::create(app_);
	else if (s == "grade") 
		widget = GradeWidget::create(app_);
	else if (s == "image")
		widget = ImageWidget::create(app_);
	else if (s == "elevation") 
		widget = ElevationWidget::create(app_);
	else if (s == "cadence") 
		widget = CadenceWidget::create(app_);
	else if (s == "heartrate") 
		widget = HeartRateWidget::create(app_);
	else if (s == "temperature")
		widget = TemperatureWidget::create(app_);
	else {
		log_error("Widget loading error, '%s' type unknown", s.c_str());
		goto error;
	}

	// Alignment
	s = (const char *) w->align();

	align = VideoWidget::string2align(s);

	if (align == VideoWidget::AlignUnknown) {
		log_error("Widget loading error, align value '%s' unknown", s.c_str());
		goto error;
	}

	// Unit
	s = (const char *) w->unit();

	unit = VideoWidget::string2unit(s);

	if (unit == VideoWidget::UnitUnknown) {
		log_error("Widget loading error, unit value '%s' unknown", s.c_str());
		goto error;
	}

	// Zoom
	s = (const char *) w->zoom();

	zoom = VideoWidget::string2zoom(s);

	log_info("Load widget '%s'", (const char *) w->type());

	// Widget settings
	widget->setAlign(align);
	widget->setPosition(w->x(), w->y());
	widget->setFormat((const char *) w->format());
	widget->setSize(w->width(), w->height());
	widget->setMargin(VideoWidget::MarginAll, w->margin());
	widget->setMargin(VideoWidget::MarginLeft, w->marginLeft());
	widget->setMargin(VideoWidget::MarginRight, w->marginRight());
	widget->setMargin(VideoWidget::MarginTop, w->marginTop());
	widget->setMargin(VideoWidget::MarginBottom, w->marginBottom());
	widget->setPadding(w->padding());
	widget->setLabel((const char *) w->name());
	widget->setTextColor((const char *) w->textColor());
	widget->setTextShadow(w->textShadow());
	widget->setBorder(w->border());
	widget->setBorderColor((const char *) w->borderColor());
	widget->setBackgroundColor((const char *) w->backgroundColor());
	if (unit != VideoWidget::UnitNone)
		widget->setUnit(unit);
	widget->setZoom(zoom);
	widget->setSource((const char *) w->source());

	// Append
	app_.append(widget);

	this->append(widget);

	return true;

error:
	if (widget)
		delete widget;

	return false;
}


void Renderer::append(VideoWidget *widget) {
	log_info("Initialize %s widget", widget->name().c_str());

	widgets_.push_back(widget);
}


void Renderer::computeWidgetsPosition(void) {
	int n;
	int width, height;

	int x, y;
	int space, offset;
	
	int margintop, marginbottom;
	int marginleft, marginright;

	VideoStreamPtr video_stream = container_->getVideoStream();

	// TopLeft, TopRight, BottomLeft, BottomRight
	//-----------------------------------------------------------

	// Get align position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		switch (widget->align()) {
		case VideoWidget::AlignTopLeft:
			x = widget->margin(VideoWidget::MarginLeft);
			y = widget->margin(VideoWidget::MarginTop);
			break;

		case VideoWidget::AlignTopRight:
			x = video_stream->width() - widget->margin(VideoWidget::MarginRight) - widget->width();
			y = widget->margin(VideoWidget::MarginTop);
			break;

		case VideoWidget::AlignBottomLeft:
			x = widget->margin(VideoWidget::MarginLeft);
			y = video_stream->height() - widget->margin(VideoWidget::MarginBottom) - widget->height();
			break;

		case VideoWidget::AlignBottomRight:
			x = video_stream->width() - widget->margin(VideoWidget::MarginRight) - widget->width();
			y = video_stream->height() - widget->margin(VideoWidget::MarginBottom) - widget->height();
			break;

		default:
			continue;
		}

		widget->setPosition(x, y);
	}

	// Left side
	//-----------------------------------------------------------
	n = 0;
	height = 0;
	margintop = 0;
	marginbottom = 0;

	// Get align position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (widget->align() == VideoWidget::AlignTopLeft) {
			margintop = MAX(widget->height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), margintop);
			continue;
		}

		if (widget->align() == VideoWidget::AlignBottomLeft) {
			marginbottom = MAX(widget->height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), marginbottom);
			continue;
		}

		if (widget->align() != VideoWidget::AlignLeft)
			continue;

		height += widget->height();
		height += widget->margin(VideoWidget::MarginTop);
		height += widget->margin(VideoWidget::MarginBottom);
		n++;
	}

	// Compute position for each widget
	space = video_stream->height() - (height + margintop + marginbottom);
	space = MAX(0, space);

	// Set position (for 'left' align)
	offset = space / 2;
	for (VideoWidget *widget : widgets_) {
		if (widget->align() != VideoWidget::AlignLeft)
			continue;

		x = widget->margin(VideoWidget::MarginLeft);
		y = margintop + offset + widget->margin(VideoWidget::MarginTop);

		widget->setPosition(x, y);

		offset += widget->height();
		offset += widget->margin(VideoWidget::MarginTop);
		offset += widget->margin(VideoWidget::MarginBottom);
	}

	// Right side
	//-----------------------------------------------------------
	n = 0;
	height = 0;
	margintop = 0;
	marginbottom = 0;

	// Get align position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (widget->align() == VideoWidget::AlignTopRight) {
			margintop = MAX(widget->height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), margintop);
			continue;
		}

		if (widget->align() == VideoWidget::AlignBottomRight) {
			marginbottom = MAX(widget->height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), marginbottom);
			continue;
		}

		if (widget->align() != VideoWidget::AlignRight)
			continue;

		height += widget->height();
		height += widget->margin(VideoWidget::MarginTop);
		height += widget->margin(VideoWidget::MarginBottom);
		n++;
	}

	// Compute position for each widget
	space = video_stream->height() - (height + margintop + marginbottom);
	space = MAX(0, space);

	// Set position (for 'right' align)
	offset = space / 2;
	for (VideoWidget *widget : widgets_) {
		if (widget->align() != VideoWidget::AlignRight)
			continue;

		x = video_stream->width() - widget->margin(VideoWidget::MarginRight) - widget->width();
		y = margintop + offset + widget->margin(VideoWidget::MarginTop);

		widget->setPosition(x, y);

		offset += widget->height();
		offset += widget->margin(VideoWidget::MarginTop);
		offset += widget->margin(VideoWidget::MarginBottom);
	}

	// Top side
	//-----------------------------------------------------------
	n = 0;
	width = 0;
	marginleft = 0;
	marginright = 0;

	// Get align position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (widget->align() == VideoWidget::AlignTopLeft) {
			marginleft = MAX(widget->width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginleft);
			continue;
		}

		if (widget->align() == VideoWidget::AlignTopRight) {
			marginright = MAX(widget->width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginright);
			continue;
		}

		if (widget->align() != VideoWidget::AlignTop)
			continue;

		width += widget->width();
		width += widget->margin(VideoWidget::MarginLeft);
		width += widget->margin(VideoWidget::MarginRight);
		n++;
	}

	// Compute position for each widget
	space = video_stream->width() - (width + marginleft + marginright);
	space = MAX(0, space);

	// Set position (for 'top' align)
	offset = space / 2;
	for (VideoWidget *widget : widgets_) {
		if (widget->align() != VideoWidget::AlignTop)
			continue;

		x = marginleft + offset + widget->margin(VideoWidget::MarginLeft);
		y = widget->margin(VideoWidget::MarginTop);

		widget->setPosition(x, y);

		offset += widget->width();
		offset += widget->margin(VideoWidget::MarginLeft);
		offset += widget->margin(VideoWidget::MarginRight);
	}

	// Bottom side
	//-----------------------------------------------------------
	n = 0;
	width = 0;
	marginleft = 0;
	marginright = 0;

	// Get align position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (widget->align() == VideoWidget::AlignBottomLeft) {
			marginleft = MAX(widget->width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginleft);
			continue;
		}

		if (widget->align() == VideoWidget::AlignBottomRight) {
			marginright = MAX(widget->width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginright);
			continue;
		}

		if (widget->align() != VideoWidget::AlignBottom)
			continue;

		width += widget->width();
		width += widget->margin(VideoWidget::MarginLeft);
		width += widget->margin(VideoWidget::MarginRight);
		n++;
	}

	// Compute position for each widget
	space = video_stream->width() - (width + marginleft + marginright);
	space = MAX(0, space);

	// Set position (for 'bottom' align)
	offset = space / 2;
	for (VideoWidget *widget : widgets_) {
		if (widget->align() != VideoWidget::AlignBottom)
			continue;

		x = marginleft + offset + widget->margin(VideoWidget::MarginLeft);
		y = video_stream->height() - widget->margin(VideoWidget::MarginBottom) - widget->height();

		widget->setPosition(x, y);

		offset += widget->width();
		offset += widget->margin(VideoWidget::MarginLeft);
		offset += widget->margin(VideoWidget::MarginRight);
	}
}


bool Renderer::start(void) {
	time_t now = time(NULL);

	time_t start_time;

	VideoStreamPtr video_stream = container_->getVideoStream();

	log_call();

	log_notice("Rendering...");

	// Compute start time
	start_time = container_->startTime() + container_->timeOffset();

	// Update start time in GPX stream (start_time can change after sync step)
	if (gpx_) {
		gpx_->setStartTime(start_time);
//		data_.init();
	}

	started_at_ = now;

	// Create overlay buffer
	overlay_ = new OIIO::ImageBuf(OIIO::ImageSpec(video_stream->width(), video_stream->height(), 
		video_stream->nbChannels(), OIIOUtils::getOIIOBaseTypeFromFormat(video_stream->format())));

	// Prepare each widget, map...
	for (VideoWidget *widget : widgets_)
		widget->prepare(overlay_);

	return true;
}


bool Renderer::run(void) {
	FramePtr frame;

	time_t start_time;

	int64_t timecode;
	int64_t timecode_ms;

	AVRational real_time;

	VideoStreamPtr video_stream = container_->getVideoStream();
//	AudioStreamPtr audio_stream = container_->getAudioStream();

	start_time = container_->startTime() + container_->timeOffset();

	real_time = av_mul_q(av_make_q(frame_time_, 1), encoder_->settings().videoParams().timeBase());

	// Read audio data
	if (decoder_audio_) {
		frame = decoder_audio_->retrieveAudio(encoder_->settings().audioParams(), real_time);

		if (frame != NULL)
			encoder_->writeAudio(frame, real_time);
	}

	// Read video data
	frame = decoder_video_->retrieveVideo(real_time);

	if (frame == NULL)
		goto done;

	timecode = frame->timestamp();
	timecode_ms = timecode * av_q2d(video_stream->timeBase()) * 1000;

	// Compute video time
	app_.setTime(start_time + (timecode_ms / 1000));

	if (gpx_) {
		// Read GPX data
		gpx_->retrieveNext(data_, timecode_ms);

		// Draw
		this->draw(frame, data_);
	}

	// Max rendering duration
	if (app_.settings().maxDuration() > 0) {
		if (timecode_ms > app_.settings().maxDuration())
			goto done;
	}

	// Dump frame info
	{
		char s[128];
		struct tm time;

		time_t now = ::time(NULL);

		localtime_r(&app_.time(), &time);

		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &time);

		if (app_.progressInfo()) {
			printf("FRAME: %ld - PTS: %ld - TIMESTAMP: %ld ms - TIME: %s\n", 
				frame_time_, timecode, timecode_ms, s);
		}
		else {
			int percent = 100 * timecode_ms / duration_ms_;
			int remaining = (timecode_ms > 0) ? (now - started_at_) * (duration_ms_ - timecode_ms) / timecode_ms : -1;

			printf("\r[FRAME %5ld] %02d:%02d:%02d.%03d / %s | %3d%% - Remaining time: %02d:%02d:%02d", 
				frame_time_, 
				(int) (timecode_ms / 3600000), (int) ((timecode_ms / 60000) % 60), (int) ((timecode_ms / 1000) % 60), (int) (timecode_ms % 1000),
				duration_,
				percent,
				(remaining / 3600), (remaining / 60) % 60, (remaining) % 60
				); //label, buf, percent,
			fflush(stdout);
		}
	}

	// Dump GPX data
	if (gpx_ && app_.progressInfo())
		data_.dump();

	real_time = av_mul_q(av_make_q(timecode, 1), video_stream->timeBase());

	encoder_->writeFrame(frame, real_time);

	frame_time_++;

	schedule();

	return true;

done:
	complete();

	return true;
}


bool Renderer::stop(void) {
	int working;

	time_t now = ::time(NULL);

	if (!app_.progressInfo())
		printf("\n");

	// Retrieve audio & video streams
	VideoStreamPtr video_stream = container_->getVideoStream();

	// Sum-up
	working = now - started_at_;

	printf("%ld frames %dx%d to %dx%d proceed in %02d:%02d:%02d\n",
		frame_time_,
		video_stream->width(), video_stream->height(),
		encoder_->settings().videoParams().width(), encoder_->settings().videoParams().height(),
		(working / 3600), (working / 60) % 60, (working) % 60);

	encoder_->close();
	if (decoder_audio_)
		decoder_audio_->close();
	decoder_video_->close();

	if (overlay_)
		delete overlay_;

	decoder_audio_ = NULL;
	decoder_video_ = NULL;
	overlay_ = NULL;

	return true;
}


void Renderer::draw(FramePtr frame, const GPXData &data) {
	OIIO::ImageBuf frame_buffer = frame->toImageBuf();

	// Draw overlay
	OIIO::ImageBufAlgo::over(frame_buffer, *overlay_, frame_buffer, OIIO::ROI());

	// Draw each widget, map...
	for (VideoWidget *widget : widgets_)
		widget->render(&frame_buffer, data);

	frame->fromImageBuf(frame_buffer);
}


void Renderer::add(OIIO::ImageBuf *frame, int x, int y, const char *picto, const char *label, const char *value, double divider) {
	int w, h;

	// Open picto
	auto img = OIIO::ImageInput::open(picto);
	const OIIO::ImageSpec& spec = img->spec();
	VideoParams::Format img_fmt = OIIOUtils::getFormatFromOIIOBaseType((OIIO::TypeDesc::BASETYPE) spec.format.basetype);
	OIIO::TypeDesc::BASETYPE type = OIIOUtils::getOIIOBaseTypeFromFormat(img_fmt);

	OIIO::ImageBuf *buf = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	img->read_image(type, buf->localpixels());

	// Resize picto
	OIIO::ImageBuf dst(OIIO::ImageSpec(spec.width * divider, spec.height * divider, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	OIIO::ImageBufAlgo::resize(dst, *buf);

//	x = 50;
//	y = 900;
	w = spec.width * divider;
	h = spec.height * divider;

	// Image over
	dst.specmod().x = x;
	dst.specmod().y = y;
	OIIO::ImageBufAlgo::over(*frame, dst, *frame, OIIO::ROI());

	delete buf;


	// Add label
	int pt;
	int space = h / 10;
	pt = (h - 3 * space) / 3;
	float white[] = { 1.0, 1.0, 1.0, 1.0 };
	if (OIIO::ImageBufAlgo::render_text(*frame, x + w + (w/10), y + space + pt, label, pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render text error\n");
	pt *= 2;
	if (OIIO::ImageBufAlgo::render_text(*frame, x + w + (w/10), y + h - (pt/2), value, pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render text error\n");
}

