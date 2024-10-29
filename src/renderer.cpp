#include <iostream>
#include <memory>

#include <OpenImageIO/fmath.h>
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
#include "widgets/power.h"
#include "widgets/lap.h"
#include "widgets/position.h"
#include "widgets/image.h"
#include "widgets/speed.h"
#include "widgets/maxspeed.h"
#include "widgets/avgspeed.h"
#include "widgets/avgridespeed.h"
#include "widgets/text.h"
#include "widgets/time.h"
#include "widgets/temperature.h"
#include "renderer.h"


Renderer::Renderer(GPXApplication &app, 
		RendererSettings &renderer_settings, TelemetrySettings &telemetry_settings)
	: Task(app) 
	, app_(app)
	, renderer_settings_(renderer_settings) 
	, telemetry_settings_(telemetry_settings) {
	container_ = NULL;

	overlay_ = NULL;
}


Renderer::~Renderer() {
}


bool Renderer::init(MediaContainer *container) {
//	time_t start_time;

	TelemetrySettings::Method telemetry_method = telemetrySettings().telemetryMethod();

	log_call();

	if (container == NULL)
		return false;

	if (telemetry_method == TelemetrySettings::MethodNone)
		telemetry_method = TelemetrySettings::MethodSample;

	source_ = TelemetryMedia::open(app_.settings().inputfile(), telemetry_method);

	// Media
	container_ = container;

//	// Set start time in GPX stream
//	start_time = container_->startTime() + container_->timeOffset();

	// Telemetry data initialization
	if (source_) {
		// Telemetry filter
		source_->skipBadPoint(telemetrySettings().telemetryCheck());
		source_->setFilter(telemetrySettings().telemetryFilter());

		// Telemetry data limits
		source_->setFrom(app_.settings().from());
		source_->setTo(app_.settings().to());

		// Telemetry data time fixing
//		source_->setStartTime(start_time);
		source_->setTimeOffset(app_.settings().offset());
		source_->retrieveFirst(data_);
	}

	return true;
}


bool Renderer::load(void) {
	std::ifstream stream;

	layout::Layout *root;
//	layout::ReportCerr report;
	layout::Parser parser(NULL); //&report);

	std::list<layout::Node *> nodes;

	std::list<layout::Map *> maps;
	std::list<layout::Track *> tracks;
	std::list<layout::Widget *> widgets;

	std::string filename = rendererSettings().layoutfile();

	log_call();

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

	// For each layout elements
	nodes = root->getElements();

	for (std::list<layout::Node *>::iterator item = nodes.begin(); item != nodes.end(); ++item) {
		layout::Node *node = (*item);

		if (node == nullptr)
 			continue;

		const std::string name = node->getName();

		if (name == "widget")
			loadWidget((layout::Widget *) node);
		else if (name == "map")
			loadMap((layout::Map *) node);
		else if (name == "track")
			loadTrack((layout::Track *) node);
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
	VideoWidget::Position position = VideoWidget::PositionNone;

	log_call();

	// Display
	if ((bool) m->display() == false) {
		log_info("Skip map widget");
		return true;
	}

	// Map source
	mapsource = m->source();

	if ((MapSettings::Source) mapsource == MapSettings::SourceNull) {
		log_warn("Map source undefined, skip map widget");
		return false;
	}

	// Open telemetry data file
	TelemetrySource *source = TelemetryMedia::open(app_.settings().inputfile());

	if (source == NULL) {
		log_warn("Can't read telemetry data, skip map widget");
		return false;
	}

	// Telemetry data limits
	source->setFrom(app_.settings().from());
	source->setTo(app_.settings().to());

	// Media
//	MediaContainer *container = app_.media();
//	TODO FIX check no break....

	VideoStreamPtr video_stream = container_->getVideoStream();

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
	TelemetryData p1, p2;
	source->getBoundingBox(&p1, &p2);

	// Alignment
	s = (const char *) m->align();
	align = VideoWidget::string2align(s);

	if (align == VideoWidget::AlignUnknown)
		log_error("Map align value '%s' unknown", s.c_str());

	// Position
	s = (const char *) m->position();
	position = VideoWidget::string2position(s);

	if (position == VideoWidget::PositionUnknown)
		log_error("Map position value '%s' unknown", s.c_str());

	// Map settings
	MapSettings mapSettings;
	mapSettings.setSize(width, height);
	mapSettings.setSource((MapSettings::Source) mapsource);
	mapSettings.setZoom(m->zoom());
	mapSettings.setDivider(m->factor());
	mapSettings.setMarkerSize(marker_size);
	mapSettings.setBoundingBox(p1.latitude(), p1.longitude(), p2.latitude(), p2.longitude());
	mapSettings.setPathThick((double) m->pathThick());
	mapSettings.setPathBorder((double) m->pathBorder());

	Map *map = Map::create(app_, mapSettings);

	log_info("Load map widget");

	// Widget settings
	map->setPosition(position);
	map->setAlign(align);
	map->setPosition(x, y);
	map->setAtTime(m->at(), m->at() + m->duration());
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
	VideoWidget::Position position = VideoWidget::PositionNone;

	log_call();

	// Display
	if ((bool) t->display() == false) {
		log_info("Skip track widget");
		return true;
	}

	// Open telemetry data file
	TelemetrySource *source = TelemetryMedia::open(app_.settings().inputfile());

	if (source == NULL) {
		log_warn("Can't read telemetry data, skip map widget");
		return false;
	}

	// Telemetry data limits
	source->setFrom(app_.settings().from());
	source->setTo(app_.settings().to());

	// Media
//	TODO FIX check no break....
//	MediaContainer *container = app_.media();

	VideoStreamPtr video_stream = container_->getVideoStream();

	// Default size
	//   2704x1520 => 800x500
	//   1920x1080 => 560x350
	width = (t->width() > 0) ? t->width() : 800 * video_stream->width() / 2704;
	height = (t->height() > 0) ? t->height() : 500 * video_stream->height() / 1520;

	// Default position
	x = (t->x() > 0) ? t->x() : video_stream->width() - width - t->margin();
	y = (t->y() > 0) ? t->y() : video_stream->height() - height - t->margin();

	// Create map bounding box
	TelemetryData p1, p2;
	source->getBoundingBox(&p1, &p2);

	// Alignment
	s = (const char *) t->align();
	align = VideoWidget::string2align(s);

	if (align == VideoWidget::AlignUnknown)
		log_error("Track align value '%s' unknown", s.c_str());

	// Position
	s = (const char *) t->position();
	position = VideoWidget::string2position(s);

	if (position == VideoWidget::PositionUnknown)
		log_error("Track position value '%s' unknown", s.c_str());

	// Track settings
	TrackSettings trackSettings;
	trackSettings.setSize(width, height);
	trackSettings.setBoundingBox(p1.latitude(), p1.longitude(), p2.latitude(), p2.longitude());
	trackSettings.setPathThick((double) t->pathThick());
	trackSettings.setPathBorder((double) t->pathBorder());

	Track *track = Track::create(app_, trackSettings);

	// Widget settings
	track->setPosition(position);
	track->setAlign(align);
	track->setPosition(x, y);
	track->setAtTime(t->at(), t->at() + t->duration());
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

	int flags = VideoWidget::FlagNone;

	VideoWidget::Unit unit = VideoWidget::UnitNone;
	VideoWidget::Zoom zoom = VideoWidget::ZoomNone;

	VideoWidget::Align align = VideoWidget::AlignNone;
	VideoWidget::Position position = VideoWidget::PositionNone;

	VideoWidget::TextAlign label_align = VideoWidget::TextAlignLeft;
	VideoWidget::TextAlign value_align = VideoWidget::TextAlignLeft;

	// Display
	if ((bool) w->display() == false) {
		log_info("Skip widget '%s'", (const char *) w->type());
		goto skip;
	}

	// Type
	s = (const char *) w->type();

	// Create widget
	if (s == "gpx") 
		widget = GPXWidget::create(app_);
	else if (s == "date") 
		widget = DateWidget::create(app_);
	else if (s == "time") 
		widget = TimeWidget::create(app_);
	else if (s == "text") 
		widget = TextWidget::create(app_);
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
	else if (s == "avgridespeed") 
		widget = AvgRideSpeedWidget::create(app_);
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
	else if (s == "power") 
		widget = PowerWidget::create(app_);
	else if (s == "lap") {
		LapWidget *lap = LapWidget::create(app_);
		lap->setTargetLap(w->nbrLap());
		widget = lap;
	}
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

	// Position
	s = (const char *) w->position();
	position = VideoWidget::string2position(s);

	if (position == VideoWidget::PositionUnknown) {
		log_error("Widget position value '%s' unknown", s.c_str());
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

	// Text alignment for label
	s = (const char *) w->labelAlign();
	label_align = VideoWidget::string2textAlign(s);

	if (label_align == VideoWidget::TextAlignUnknown) {
		log_error("Widget loading error, label align value '%s' unknown", s.c_str());
		goto error;
	}

	// Text alignment for value
	s = (const char *) w->valueAlign();
	value_align = VideoWidget::string2textAlign(s);

	if (value_align == VideoWidget::TextAlignUnknown) {
		log_error("Widget loading error, value align value '%s' unknown", s.c_str());
		goto error;
	}

	// Flags
	if (w->withLabel())
		flags |= VideoWidget::FlagLabel;
	if (w->withValue())
		flags |= VideoWidget::FlagValue;
	if (w->withPicto())
		flags |= VideoWidget::FlagPicto;
	if (w->withUnit())
		flags |= VideoWidget::FlagUnit;

	// Widget settings
	widget->setPosition(position);
	widget->setAlign(align);
	widget->setPosition(w->x(), w->y());
	widget->setAtTime(w->at(), w->at() + w->duration());
	widget->setFormat((const char *) w->format());
	widget->setSize(w->width(), w->height());
	widget->setMargin(VideoWidget::MarginAll, w->margin());
	widget->setMargin(VideoWidget::MarginLeft, w->marginLeft());
	widget->setMargin(VideoWidget::MarginRight, w->marginRight());
	widget->setMargin(VideoWidget::MarginTop, w->marginTop());
	widget->setMargin(VideoWidget::MarginBottom, w->marginBottom());
	widget->setPadding(VideoWidget::PaddingAll, w->padding());
	widget->setPadding(VideoWidget::PaddingLeft, w->paddingLeft());
	widget->setPadding(VideoWidget::PaddingRight, w->paddingRight());
	widget->setPadding(VideoWidget::PaddingTop, w->paddingTop());
	widget->setPadding(VideoWidget::PaddingBottom, w->paddingBottom());
	widget->setFont((const char *) w->font());
	widget->setLabel((const char *) w->name());
	widget->setText((const char *) w->text());
	widget->setTextColor((const char *) w->textColor());
	widget->setTextRatio((double) w->textRatio());
	widget->setTextShadow(w->textShadow());
	widget->setTextLineSpace(w->textLineSpace());
	widget->setLabelAlign(label_align);
	widget->setValueAlign(value_align);
	widget->setBorder(w->border());
	widget->setBorderColor((const char *) w->borderColor());
	widget->setBackgroundColor((const char *) w->backgroundColor());
	if (unit != VideoWidget::UnitNone)
		widget->setUnit(unit);
	widget->setZoom(zoom);
	widget->setSource((const char *) w->source());
	widget->setFlags(flags);

	// Append
	app_.append(widget);

	this->append(widget);

skip:
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
	bool is_first;

	int n;
	int width, height;

	int x, y;
	int space, offset;
	int offset_x, offset_y;
	
	int margintop, marginbottom;
	int marginleft, marginright;

	VideoStreamPtr video_stream = container_->getVideoStream();

	log_call();

	// Initialize widget
	//-----------------------------------------------------------
	for (VideoWidget *widget : widgets_) {
		widget->initialize();
	}

	// TopLeft
	//-----------------------------------------------------------
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	// Get position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (widget->position() != VideoWidget::PositionTopLeft)
			continue;

		x = widget->margin(VideoWidget::MarginLeft);
		y = widget->margin(VideoWidget::MarginTop);

		if (widget->align() == VideoWidget::AlignHorizontal)
			x += offset_x;

		if (widget->align() == VideoWidget::AlignVertical)
			y += offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->align() == VideoWidget::AlignHorizontal)) {
			offset_x += widget->width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->align() == VideoWidget::AlignVertical)) {
			offset_y += widget->height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// TopRight
	//-----------------------------------------------------------
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	// Get position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (widget->position() != VideoWidget::PositionTopRight)
			continue;

		x = layout_width_ - widget->margin(VideoWidget::MarginRight) - widget->width();
		y = widget->margin(VideoWidget::MarginTop);

		if (widget->align() == VideoWidget::AlignHorizontal)
			x -= offset_x;

		if (widget->align() == VideoWidget::AlignVertical)
			y += offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->align() == VideoWidget::AlignHorizontal)) {
			offset_x += widget->width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->align() == VideoWidget::AlignVertical)) {
			offset_y += widget->height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}
	
	// BottomLeft
	//-----------------------------------------------------------
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	// Get position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (widget->position() != VideoWidget::PositionBottomLeft)
			continue;

		x = widget->margin(VideoWidget::MarginLeft);
		y = layout_height_ - widget->margin(VideoWidget::MarginBottom) - widget->height();

		if (widget->align() == VideoWidget::AlignHorizontal)
			x += offset_x;

		if (widget->align() == VideoWidget::AlignVertical)
			y -= offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->align() == VideoWidget::AlignHorizontal)) {
			offset_x += widget->width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->align() == VideoWidget::AlignVertical)) {
			offset_y += widget->height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// BottomRight
	//-----------------------------------------------------------
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	// Get position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (widget->position() != VideoWidget::PositionBottomRight)
			continue;

		x = layout_width_ - widget->margin(VideoWidget::MarginRight) - widget->width();
		y = layout_height_ - widget->margin(VideoWidget::MarginBottom) - widget->height();

		if (widget->align() == VideoWidget::AlignHorizontal)
			x -= offset_x;

		if (widget->align() == VideoWidget::AlignVertical)
			y -= offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->align() == VideoWidget::AlignHorizontal)) {
			offset_x += widget->width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->align() == VideoWidget::AlignVertical)) {
			offset_y += widget->height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// Left side
	//-----------------------------------------------------------
	n = 0;
	height = 0;
	margintop = 0;
	marginbottom = 0;

	// Compute available height on the left side
	for (VideoWidget *widget : widgets_) {
		if (widget->position() == VideoWidget::PositionTopLeft) {
			if ((margintop == 0) || (widget->align() == VideoWidget::AlignVertical)) {
				margintop = MAX(widget->height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), margintop);
				continue;
			}
		}

		if (widget->position() == VideoWidget::PositionBottomLeft) {
			if ((marginbottom == 0) || (widget->align() == VideoWidget::AlignVertical)) {
				marginbottom = MAX(widget->height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), marginbottom);
				continue;
			}
		}

		if (widget->position() != VideoWidget::PositionLeft)
			continue;

		if ((n == 0) || (widget->align() == VideoWidget::AlignVertical)) {
			height += widget->height();
			height += widget->margin(VideoWidget::MarginTop);
			height += widget->margin(VideoWidget::MarginBottom);
			n++;
		}
	}

	// Compute position for each widget
	space = layout_height_ - (height + margintop + marginbottom);
	space = MAX(0, space);

	// Set position (for 'left' align)
	offset = space / 2;
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	for (VideoWidget *widget : widgets_) {
		if (widget->position() != VideoWidget::PositionLeft)
			continue;

		x = widget->margin(VideoWidget::MarginLeft);
		y = margintop + offset + widget->margin(VideoWidget::MarginTop);

		if (widget->align() == VideoWidget::AlignHorizontal)
			x += offset_x;

		if (widget->align() == VideoWidget::AlignVertical)
			y += offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->align() == VideoWidget::AlignHorizontal)) {
			offset_x += widget->width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->align() == VideoWidget::AlignVertical)) {
			offset_y += widget->height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// Right side
	//-----------------------------------------------------------
	n = 0;
	height = 0;
	margintop = 0;
	marginbottom = 0;

	// Compute available height on the right side
	for (VideoWidget *widget : widgets_) {
		if (widget->position() == VideoWidget::PositionTopRight) {
			if ((margintop == 0) || (widget->align() == VideoWidget::AlignVertical)) {
				margintop = MAX(widget->height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), margintop);
				continue;
			}
		}

		if (widget->position() == VideoWidget::PositionBottomRight) {
			if ((marginbottom == 0) || (widget->align() == VideoWidget::AlignVertical)) {
				marginbottom = MAX(widget->height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), marginbottom);
				continue;
			}
		}

		if (widget->position() != VideoWidget::PositionRight)
			continue;

		if ((n == 0) || (widget->align() == VideoWidget::AlignVertical)) {
			height += widget->height();
			height += widget->margin(VideoWidget::MarginTop);
			height += widget->margin(VideoWidget::MarginBottom);
			n++;
		}
	}

	// Compute position for each widget
	space = layout_height_ - (height + margintop + marginbottom);
	space = MAX(0, space);

	// Set position (for 'right' align)
	offset = space / 2;
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	for (VideoWidget *widget : widgets_) {
		if (widget->position() != VideoWidget::PositionRight)
			continue;

		x = layout_width_ - widget->margin(VideoWidget::MarginRight) - widget->width();
		y = margintop + offset + widget->margin(VideoWidget::MarginTop);

		if (widget->align() == VideoWidget::AlignHorizontal)
			x -= offset_x;

		if (widget->align() == VideoWidget::AlignVertical)
			y += offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->align() == VideoWidget::AlignHorizontal)) {
			offset_x += widget->width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->align() == VideoWidget::AlignVertical)) {
			offset_y += widget->height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// Top side
	//-----------------------------------------------------------
	n = 0;
	width = 0;
	marginleft = 0;
	marginright = 0;

	// Compute available width on the top side
	for (VideoWidget *widget : widgets_) {
		if (widget->position() == VideoWidget::PositionTopLeft) {
			if ((marginleft == 0) || (widget->align() == VideoWidget::AlignHorizontal)) {
				marginleft = MAX(widget->width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginleft);
				continue;
			}
		}

		if (widget->position() == VideoWidget::PositionTopRight) {
			if ((marginright == 0) || (widget->align() == VideoWidget::AlignHorizontal)) {
				marginright = MAX(widget->width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginright);
				continue;
			}
		}

		if (widget->position() != VideoWidget::PositionTop)
			continue;

		if ((n == 0) || (widget->align() == VideoWidget::AlignHorizontal)) {
			width += widget->width();
			width += widget->margin(VideoWidget::MarginLeft);
			width += widget->margin(VideoWidget::MarginRight);
			n++;
		}
	}

	// Compute position for each widget
	space = layout_width_ - (width + marginleft + marginright);
	space = MAX(0, space);

	// Set position (for 'top' align)
	offset = space / 2;
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	for (VideoWidget *widget : widgets_) {
		if (widget->position() != VideoWidget::PositionTop)
			continue;

		x = marginleft + offset + widget->margin(VideoWidget::MarginLeft);
		y = widget->margin(VideoWidget::MarginTop);

		if (widget->align() == VideoWidget::AlignHorizontal)
			x += offset_x;

		if (widget->align() == VideoWidget::AlignVertical)
			y += offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->align() == VideoWidget::AlignHorizontal)) {
			offset_x += widget->width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->align() == VideoWidget::AlignVertical)) {
			offset_y += widget->height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// Bottom side
	//-----------------------------------------------------------
	n = 0;
	width = 0;
	marginleft = 0;
	marginright = 0;

	// Compute available width on the bottom side
	for (VideoWidget *widget : widgets_) {
		if (widget->position() == VideoWidget::PositionBottomLeft) {
			if ((marginleft == 0) || (widget->align() == VideoWidget::AlignHorizontal)) {
				marginleft = MAX(widget->width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginleft);
				continue;
			}
		}

		if (widget->position() == VideoWidget::PositionBottomRight) {
			if ((marginright == 0) || (widget->align() == VideoWidget::AlignHorizontal)) {
				marginright = MAX(widget->width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginright);
				continue;
			}
		}

		if (widget->position() != VideoWidget::PositionBottom)
			continue;

		if ((n == 0) || (widget->align() == VideoWidget::AlignHorizontal)) {
			width += widget->width();
			width += widget->margin(VideoWidget::MarginLeft);
			width += widget->margin(VideoWidget::MarginRight);
			n++;
		}
	}

	// Compute position for each widget
	space = layout_width_ - (width + marginleft + marginright);
	space = MAX(0, space);

	// Set position (for 'bottom' align)
	offset = space / 2;
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	for (VideoWidget *widget : widgets_) {
		if (widget->position() != VideoWidget::PositionBottom)
			continue;

		x = marginleft + offset + widget->margin(VideoWidget::MarginLeft);
		y = layout_height_ - widget->margin(VideoWidget::MarginBottom) - widget->height();

		if (widget->align() == VideoWidget::AlignHorizontal)
			x += offset_x;

		if (widget->align() == VideoWidget::AlignVertical)
			y -= offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->align() == VideoWidget::AlignHorizontal)) {
			offset_x += widget->width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->align() == VideoWidget::AlignVertical)) {
			offset_y += widget->height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}
}


void Renderer::rotate(OIIO::ImageBuf *buf, int orientation) {
	switch (orientation) {
	case 180:
	case -180:
		OIIO::ImageBufAlgo::rotate180(*buf, *buf);
		break;

	case 90:
	case -270:
		OIIO::ImageBufAlgo::rotate270(*buf, *buf);
		break;

	case -90:
	case 270:
		OIIO::ImageBufAlgo::rotate90(*buf, *buf);
		break;

	default:
		break;
	}
}


void Renderer::resize(OIIO::ImageBuf *buf, int width, int height) {
	const OIIO::ImageSpec& spec = buf->spec();
	OIIO::TypeDesc::BASETYPE type = (OIIO::TypeDesc::BASETYPE) spec.format.basetype;

	// If no change, skip
	if ((width == spec.width) && (height == spec.height))
		return;

	OIIO::ImageBuf out(OIIO::ImageSpec(width, height, spec.nchannels, type));
	OIIO::ImageBufAlgo::resize(out, *buf);

	*buf = out;
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

